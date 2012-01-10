/* Go Game Record (droceRoG)
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#include "inkview.h"
#include "gogame.h"
#include "fileselector.h"

/******************************************************************************/

/* prototypes */
int main_handler(int type, int par1, int par2);
void msg(char *s);
void cb_update_sgf(char *filename);

/******************************************************************************/

ifont *times12;
char *init_filename = NULL;

static imenu menu1[] = {

  { ITEM_HEADER,   0, "Menu", NULL },
  { ITEM_ACTIVE, 101, "Open SGF file...", NULL },
  { ITEM_ACTIVE, 102, "Go to move...", NULL },
  { 0, 0, NULL, NULL }

};

/******************************************************************************/

void cb_page_selected(int page) 
{
    if (gogame_move_to_page(page))
        gogame_draw_update();
}

void menu1_handler(int index)
{
    switch (index) {
        case 101:
            fileselector_chooseFile(&cb_update_sgf);
            break;
        case 102:
            OpenPageSelector(cb_page_selected);
            break;
    }
}

void msg(char *s) 
{
  FillArea(350, 770, 250, 20, WHITE);
  SetFont(times12, BLACK);
  DrawString(350, 770, s);
  PartialUpdate(350, 770, 250, 20);
}

void cb_update_sgf(char *filename)
{
    // fprintf(stderr, "drocerog.c: callback called: %s\n", filename);

    gogame_new_from_file(filename);
    gogame_draw_fullrepaint();
}

int main_handler(int type, int par1, int par2) 
{
    fprintf(stderr, "[%i %i %i]\n", type, par1, par2);

    if (type == EVT_INIT) {
        // occurs once at startup, only in main handler

        times12 = OpenFont("DejaVuSerif", 12, 1);
        // fonts = EnumFonts();
        // fprintf(stderr, "%d\n", i);
        // i = 0;
        // while (fonts[i] != NULL) {
            // fprintf(stderr, "font: %s\n", fonts[i]);
            // i += 1;
        // }

        gogame_new_from_file(init_filename);

        gogame_printGameInfo();
    }

    if (type == EVT_SHOW) {
        // occurs when this event handler becomes active
        gogame_draw_fullrepaint();
    }

    // if (type == EVT_KEYPRESS) {
    if (type == EVT_KEYUP) {
        switch (par1) {
            case KEY_OK:
                if (gogame_isGameOpened()) {                    /* in game, switch full screen comment */
                    if (gogame_switch_fullComment())
                        gogame_draw_fullrepaint();
                } else {                                        /* while no game opened, open one directly */
                    fileselector_chooseFile(&cb_update_sgf);
                }
                break;

            case KEY_BACK:
                CloseApp();
                break;

            case KEY_LEFT:
                gogame_move_to_prevEvt();
                gogame_draw_update();
                break;

            case KEY_RIGHT:
                gogame_move_to_nextEvt();
                gogame_draw_update();
                break;

            case KEY_UP:
                gogame_moveVar_up();
                gogame_draw_update();
                break;

            case KEY_DOWN:
                gogame_moveVar_down();
                gogame_draw_update();
                break;

            case KEY_NEXT:
                gogame_move_forward();
                gogame_draw_update();
                break;

            case KEY_PREV:
                gogame_move_back();
                gogame_draw_update();
                break;

            case KEY_MENU:
                OpenMenu(menu1, 1, 20, 20, menu1_handler);
                break;
        }
    }

    if (type == EVT_EXIT) {
        gogame_cleanup();
    }

    return 0;
}

int main(int argc, char *argv[])
{
    /* initialise file name */
    if (argc == 2) {
        init_filename = argv[1];
    } else {
        init_filename = "";
    }

    InkViewMain(main_handler);
    return 0;
}


/* Go Game Record (droceRoG)
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#include "inkview.h"
// #include "goboard.h"
#include "gogame.h"

ifont *times12;

/* prototypes */
int main_handler(int type, int par1, int par2);
void msg(char *s);

void msg(char *s) 
{
  FillArea(350, 770, 250, 20, WHITE);
  SetFont(times12, BLACK);
  DrawString(350, 770, s);
  PartialUpdate(350, 770, 250, 20);
}

int main_handler(int type, int par1, int par2) 
{
    // int i=0;
    // char **fonts;
    int retVal;
    char message[256];

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

        // board_new(9, 50);
        // board_print();
        // board_test_placeStones();

        // retVal = gogame_new_from_file("8457-Dieter-bakhtiari-joeseki.sgf");
        // retVal = gogame_new_from_file("/mnt/ext1/applications/8457-Dieter-bakhtiari-joeseki.sgf");

        retVal = gogame_new_from_file("testSGF.sgf");
        // retVal = gogame_new_from_file("/mnt/ext1/applications/testSGF.sgf");

        if (retVal > 0) {
            snprintf(message, sizeof(message), "Could not open sgf file. return value = %d", retVal);
            msg(message);
        }
        // gogame_new_from_file("testSGF_problem.sgf");
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
                msg("KEY_OK");
                break;

            case KEY_BACK:
                CloseApp();
                break;

            case KEY_LEFT:
                msg("KEY_LEFT");
                break;

            case KEY_RIGHT:
                msg("KEY_RIGHT");
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
                // OpenMenu(menu1, cindex, 20, 20, menu1_handler);
                msg("KEY_MENU");
                break;

            case KEY_DELETE:
                msg("KEY_DELETE");
                break;
        }
    }

    if (type == EVT_EXIT) {
        gogame_cleanup();
    }

    return 0;
}

int main()
{
    InkViewMain(main_handler);
    return 0;
}


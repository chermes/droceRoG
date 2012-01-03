/* Go Game Record (droceRoG)
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#include "inkview.h"

ifont *times12;

/* prototypes */
int main_handler(int type, int par1, int par2);
void msg(char *s);
void mainscreen_repaint();

void msg(char *s) 
{
  FillArea(350, 770, 250, 20, WHITE);
  SetFont(times20, BLACK);
  DrawString(350, 770, s);
  PartialUpdate(350, 770, 250, 20);
}

int main_handler(int type, int par1, int par2) 
{
    int i;

    fprintf(stderr, "[%i %i %i]\n", type, par1, par2);

    if (type == EVT_INIT) {
        // occurs once at startup, only in main handler

        times20 = OpenFont("times", 20, 1);
    }

    if (type == EVT_SHOW) {
        // occurs when this event handler becomes active
        mainscreen_repaint();
    }

    if (type == EVT_KEYPRESS) {
        switch (par1) {
            case KEY_OK:
                // OpenMenu(menu1, cindex, 20, 20, menu1_handler);
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
                msg("KEY_UP");
                break;

            case KEY_DOWN:
                msg("KEY_DOWN");
                break;

            case KEY_MUSIC:
                msg("KEY_MUSIC");
                break;

            case KEY_MENU:
                msg("KEY_MENU");
                break;

            case KEY_DELETE:
                msg("KEY_DELETE");
                break;
        }
    }

    if (type == EVT_EXIT) {
        // occurs only in main handler when exiting or when SIGINT received.
        // save configuration here, if needed
    }

    return 0;
}

void mainscreen_repaint() 
{

  // char buf[64];
  // ibitmap *b;
  // int i;

  ClearScreen();

  SetFont(arialb12, BLACK);
  DrawString(5, 2, "droceRoG - Go Game Record Viewer");

  // DrawBitmap(0, 20, &background);
  // DrawBitmap(120, 30, &books);

  // DrawLine(5, 500, 595, 500, BLACK);
  // DrawLine(5, 502, 595, 502, DGRAY);
  // DrawLine(5, 504, 595, 504, LGRAY);
  // DrawLine(19, 516, 20, 517, BLACK);
  // DrawLine(22, 516, 23, 516, BLACK);

  // for (i=5; i<595; i+=3) 
      // DrawPixel(i, 507, BLACK);

  // DrawRect(5, 510, 590, 10, BLACK);

  // for (i=0; i<256; i++) FillArea(35+i*2, 524, 2, 12, i | (i << 8) | (i << 16));

  // b = BitmapFromScreen(0, 520, 600, 20);
  // DrawBitmap(0, 550, b);
  // free(b);
  // InvertArea(0, 550, 600, 20);
  // DimArea(0, 575, 600, 10, BLACK);

  // if (! orient) {
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 600,  6,  6, 0);
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 600, 10,  6, 0);
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 600, 30,  6, 0);
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 610,  6, 10, 0);
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 610, 10, 10, 0);
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 610, 30, 10, 0);
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 625,  6, 30, 0);
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 625, 10, 30, 0);
    // Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 625, 30, 30, 0);
  // }

  // SetFont(arial8n, BLACK);
  // DrawString(350, 600, "Arial 8 with no antialiasing");
  // SetFont(arial12, BLACK);
  // DrawString(350, 615, "Arial 12 regular");
  // SetFont(arialb12, BLACK);
  // DrawString(350, 630, "Arial 12 bold");
  // SetFont(cour16, BLACK);
  // DrawString(350, 645, "Courier 16");
  // SetFont(cour24, BLACK);
  // DrawString(350, 660, "Courier 24");
  // DrawSymbol(500, 660, ARROW_LEFT);
  // DrawSymbol(520, 660, ARROW_RIGHT);
  // DrawSymbol(540, 660, ARROW_UP);
  // DrawSymbol(560, 660, ARROW_DOWN);
  // SetFont(times20, BLACK);
  // DrawString(350, 680, "Times 20");
  // DrawSymbol(450, 680, ARROW_LEFT);
  // DrawSymbol(470, 680, ARROW_RIGHT);
  // DrawSymbol(490, 680, ARROW_UP);
  // DrawSymbol(510, 680, ARROW_DOWN);

  //DrawTextRect(25, 400, 510, 350, sometext, ALIGN_LEFT);

  FullUpdate();

}

int main(int argc, char **argv)
{
    InkViewMain(main_handler);
    return 0;
}


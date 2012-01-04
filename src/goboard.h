/* Definition of a Go board.
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#ifndef GOBOARD_H
#define GOBOARD_H

#ifdef __cplusplus
extern "C"
{
#endif

/* initialise new board of size x size */
void board_new(int size, int offset_y);

/* delete allocated memory and reset board */
void board_cleanup();

/* print board to console */
void board_print();

/* draw board on screen 
 * bPartialUpdate: update screen at parts which have been changed. If set to 0,
 *      the FullUpdate() call is left to user.
 * */
void board_draw_update(int bPartialUpdate);

#ifdef __cplusplus
}
#endif

#endif /* GOBOARD_H */


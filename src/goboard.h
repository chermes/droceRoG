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
void board_new(short size);
/* delete allocated memory and reset board */
void board_cleanup();
/* print board to console */
void board_print();

#ifdef __cplusplus
}
#endif

#endif /* GOBOARD_H */


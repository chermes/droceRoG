/* Go game
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#ifndef GOGAME_H
#define GOGAME_H

#ifdef __cplusplus
extern "C"
{
#endif

int gogame_new_from_file(const char *filename);

void gogame_cleanup();

void gogame_printGameInfo();

void gogame_draw_fullrepaint();
void gogame_draw_update();

void gogame_move_forward();
void gogame_move_back();
void gogame_moveVar_down();
void gogame_moveVar_up();

/* move to next and previous event, e.g. a comment or a variation */
void gogame_move_to_nextEvt();
void gogame_move_to_prevEvt();

#ifdef __cplusplus
}
#endif

#endif /* GOGAME_H */


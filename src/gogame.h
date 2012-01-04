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

#ifdef __cplusplus
}
#endif

#endif /* GOGAME_H */


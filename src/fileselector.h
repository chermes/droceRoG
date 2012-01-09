/* droceRoG - SGF file selector
 *
 * Author: Christoph Hermes (hermes<AT>hausmilbe<DOT>net)
 */

#ifndef FILESELECTOR_H
#define FILESELECTOR_H

/* Choose an SGF file by opening a selection tool and save the result by
 * calling "cb_update(filename)".
 */
void fileselector_chooseFile(void (*cb_update)(char *filename));

#endif /* FILESELECTOR_H */


/* Implementation of Go board methods.
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#include "goboard.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <inkview.h>

/******************************************************************************/

typedef enum { /* WARNING: when changing the element number, consider the bit
                * field length in GoBoardElement */
    GRID_TL, GRID_T, GRID_TR,        /* top left, top, top right          */
    GRID_L, GRID_R, GRID_C, GRID_CP, /* left, right, center, center point */
    GRID_BL, GRID_B, GRID_BR         /* bottom left, bottom, bottom right */
} GridType;
const char gridTypeString[10][2] = { "B", "C", "D",
                                     "E", "F", "A", "J",
                                     "G", "I", "H" };

typedef enum { /* WARNING: when changing the element number, consider the bit
                * field length in GoBoardElement */
    FIELD_EMPTY,              /* empty field */
    FIELD_BLACK, FIELD_WHITE  /* black and white stone */
} FieldType;
const char fieldTypeString[3][2] = { " ", /* empty field not used */
                                     "K", "L" };

typedef enum { /* WARNING: when changing the element number, consider the bit
                * field length in GoBoardElement */
    MARKER_EMPTY,                                   /* no marker */
    MARKER_KO,                                      /* ko marker */
    MARKER_SQUARE, MARKER_TRIANGLE, MARKER_CIRC     /* shape marker */
} MarkerType;
const char markerTypeString[5][2] = { " ", /* empty marker, not used */
                                      "M",
                                      "M", "N", "O" };

typedef struct
{
    unsigned grid_type:4;   /* GridType */
    unsigned field_type:2;  /* FieldType */
    unsigned marker_type:3; /* MarkerType */
    unsigned draw_update:1; /* Update this field? */
} GoBoardElement;

typedef struct
{
    int size;               /* size x size */
    GoBoardElement *board;  /* board[col * size + row] */
    int draw_elemSize;      /* size in points for each field element */
    ifont *draw_font;       /* ttf handler for the drocerog ttf */
    int draw_offset_x;      /* move the board the specified points right */
    int draw_offset_y;      /* move the board the specified points down */
} GoBoard;

enum BOOL { FALSE, TRUE };

/******************************************************************************/

static GoBoard *curBoard = NULL;

/******************************************************************************/

void clearDeadGroups(int cur_r, int cur_c);

/******************************************************************************/

void board_new(int size, int offset_y)
{/*{{{*/
    int r, c, i, hoshi;

    if (curBoard != NULL)
        board_cleanup();

    /* allocate memory */
    curBoard = (GoBoard*) malloc( sizeof(GoBoard) );
    curBoard->size = size;
    curBoard->board = (GoBoardElement *) malloc( sizeof(GoBoardElement) * size * size );

    /* init board grid and fields */
    for (r=0; r<size; r++) {
        for (c=0; c<size; c++) {
            i = c * size + r;

            /* empty field */
            curBoard->board[i].field_type = FIELD_EMPTY;
            /* empty marker */
            curBoard->board[i].marker_type = MARKER_EMPTY;
            /* update field when drawing */
            curBoard->board[i].draw_update = TRUE;

            /* common grid point */
            curBoard->board[i].grid_type = GRID_C;

            /* curBoard borders */
            if (r == 0)      curBoard->board[i].grid_type = GRID_T;
            if (r == size-1) curBoard->board[i].grid_type = GRID_B;
            if (c == 0)      curBoard->board[i].grid_type = GRID_L;
            if (c == size-1) curBoard->board[i].grid_type = GRID_R;

            /* corners */
            if (r == 0 && c == 0)           curBoard->board[i].grid_type = GRID_TL;
            if (r == 0 && c == size-1)      curBoard->board[i].grid_type = GRID_TR;
            if (r == size-1 && c == 0)      curBoard->board[i].grid_type = GRID_BL;
            if (r == size-1 && c == size-1) curBoard->board[i].grid_type = GRID_BR;

        }
    }
    /* star points (hoshi) */
    if (size % 2 == 1) {
        curBoard->board[(int)(size/2) * size + (int)(size/2)].grid_type = GRID_CP;
    }
    if (size < 9)  hoshi = -1;
    if (size == 9) hoshi = 2;
    if (size > 9)  hoshi = 3;
    if (hoshi > 0) {
        curBoard->board[hoshi * size + hoshi].grid_type = GRID_CP;
        if (size % 2 == 1)
            curBoard->board[hoshi * size + (int)(size/2)].grid_type = GRID_CP;
        curBoard->board[hoshi * size + (size - hoshi - 1)].grid_type = GRID_CP;
        if (size % 2 == 1) {
            curBoard->board[(int)(size/2) * size + hoshi].grid_type = GRID_CP;
            curBoard->board[(int)(size/2) * size + (size - hoshi - 1)].grid_type = GRID_CP;
        }
        curBoard->board[(size - hoshi - 1) * size + hoshi].grid_type = GRID_CP;
        if (size % 2 == 1)
            curBoard->board[(size - hoshi - 1) * size + (int)(size/2)].grid_type = GRID_CP;
        curBoard->board[(size - hoshi - 1) * size + (size - hoshi - 1)].grid_type = GRID_CP;
    }

    /* set font size and load ttf */
    curBoard->draw_elemSize = (int) (ScreenWidth() / size);
    curBoard->draw_font = OpenFont("drocerog", curBoard->draw_elemSize, 1);
    curBoard->draw_offset_x = (int) ((ScreenWidth() - curBoard->draw_elemSize * size) / 2);
    curBoard->draw_offset_y = offset_y;

}/*}}}*/

void board_test_placeStones()
{/*{{{*/
    assert( curBoard != NULL );
    assert( curBoard->size > 6 );

    curBoard->board[2 * curBoard->size + 2].field_type = FIELD_BLACK;
    curBoard->board[3 * curBoard->size + 2].field_type = FIELD_WHITE;

    curBoard->board[2 * curBoard->size + 3].marker_type = MARKER_KO;
    curBoard->board[3 * curBoard->size + 3].marker_type = MARKER_SQUARE;
    curBoard->board[4 * curBoard->size + 3].marker_type = MARKER_TRIANGLE;
    curBoard->board[5 * curBoard->size + 3].marker_type = MARKER_CIRC;

    curBoard->board[3 * curBoard->size + 4].marker_type = MARKER_SQUARE;
    curBoard->board[3 * curBoard->size + 4].field_type = FIELD_BLACK;
    curBoard->board[4 * curBoard->size + 4].marker_type = MARKER_TRIANGLE;
    curBoard->board[4 * curBoard->size + 4].field_type = FIELD_BLACK;
    curBoard->board[5 * curBoard->size + 4].marker_type = MARKER_CIRC;
    curBoard->board[5 * curBoard->size + 4].field_type = FIELD_BLACK;

    curBoard->board[3 * curBoard->size + 5].marker_type = MARKER_SQUARE;
    curBoard->board[3 * curBoard->size + 5].field_type = FIELD_WHITE;
    curBoard->board[4 * curBoard->size + 5].marker_type = MARKER_TRIANGLE;
    curBoard->board[4 * curBoard->size + 5].field_type = FIELD_WHITE;
    curBoard->board[5 * curBoard->size + 5].marker_type = MARKER_CIRC;
    curBoard->board[5 * curBoard->size + 5].field_type = FIELD_WHITE;
}/*}}}*/

void board_placeStone(int r, int c, BoardPlayer player, int bIsMove)
{/*{{{*/
    assert( curBoard != NULL );
    assert( r >= 0 );
    assert( c >= 0 );
    assert( r < curBoard->size );
    assert( c < curBoard->size );

    switch (player) {
        case BOARD_BLACK:
            curBoard->board[c * curBoard->size + r].field_type = FIELD_BLACK;
            curBoard->board[c * curBoard->size + r].draw_update = 1;
            break;

        case BOARD_WHITE:
            curBoard->board[c * curBoard->size + r].field_type = FIELD_WHITE;
            curBoard->board[c * curBoard->size + r].draw_update = 1;
            break;
    }

    /* remove stones if necessary */
    if (bIsMove) {
        clearDeadGroups(r, c);
    }
}/*}}}*/

void clearDeadGroups(int cur_r, int cur_c)
{/*{{{*/
    short *groups = NULL;
    short *idxs = NULL;
    short lastIdx = 1;
    short oldIdx = 0;
    short neighbors[5];
    int neighborsLibs[5];
    int r, c, sz, i, j;
    int bFoundElem;

    assert( curBoard != NULL );

    sz = curBoard->size;

    /* TODO: only two rows are necessary! */
    groups = (short *) malloc(sizeof(short) * sz * sz);
    idxs = (short *) malloc(sizeof(short) * sz * sz);

    /* init index list */
    for (r=0; r<(sz*sz); r++) {
        idxs[r] = r-1;
        groups[r] = 99;
    }

    for (r=0; r<sz; r++) {
        for (c=0; c<sz; c++) {
            if (curBoard->board[c * sz + r].field_type == FIELD_EMPTY) {
                groups[c * sz + r] = 0;
                continue;
            }

            if (r == 0 && c == 0) {
                groups[c * sz + r] = lastIdx;
                lastIdx += 1;
            } else if (r == 0 && c > 0) {
                if (curBoard->board[c * sz + r].field_type == curBoard->board[(c-1) * sz + r].field_type) {
                    groups[c * sz + r] = groups[(c-1) * sz + r];
                } else {
                    groups[c * sz + r] = lastIdx;
                    lastIdx += 1;
                }
            } else if (r > 0 && c == 0) {
                if (curBoard->board[c * sz + r].field_type == curBoard->board[c * sz + (r-1)].field_type) {
                    groups[c * sz + r] = groups[c * sz + (r-1)];
                } else {
                    groups[c * sz + r] = lastIdx;
                    lastIdx += 1;
                }
            } else {
                if (curBoard->board[c * sz + r].field_type != curBoard->board[c * sz + (r-1)].field_type 
                    && curBoard->board[c * sz + r].field_type != curBoard->board[(c-1) * sz + r].field_type) {
                    groups[c * sz + r] = lastIdx;
                    lastIdx += 1;
                } else if (curBoard->board[c * sz + r].field_type == curBoard->board[c * sz + (r-1)].field_type 
                           && curBoard->board[c * sz + r].field_type != curBoard->board[(c-1) * sz + r].field_type) {
                    groups[c * sz + r] = groups[c * sz + (r-1)];
                } else if (curBoard->board[c * sz + r].field_type != curBoard->board[c * sz + (r-1)].field_type 
                           && curBoard->board[c * sz + r].field_type == curBoard->board[(c-1) * sz + r].field_type) {
                    groups[c * sz + r] = groups[(c-1) * sz + r];
                } else if (curBoard->board[c * sz + r].field_type == curBoard->board[c * sz + (r-1)].field_type 
                           && curBoard->board[c * sz + r].field_type == curBoard->board[(c-1) * sz + r].field_type) {
                    groups[c * sz + r] = groups[c * sz + (r-1)];
                    oldIdx = idxs[groups[(c-1) * sz + r]];
                    for (i=0; i<lastIdx; i++) {
                        if (idxs[i] == oldIdx)
                            idxs[i] = idxs[groups[c * sz + r]];
                    }
                }
            }
        }
    }

    // /* print groups and idxs[groups] */
    // fprintf(stderr, "Idxs: ");
    // for (i=0; i<lastIdx; i++) {
        // fprintf(stderr, "I[%d]=%d, ", i, idxs[i]);
    // }
    // fprintf(stderr, "\n");
    // for (r=0; r<sz; r++) {
        // for (c=0; c<sz; c++) {
            // fprintf(stderr, "%2d ", groups[c * sz + r]);
        // }
        // fprintf(stderr, "    ");
        // for (c=0; c<sz; c++) {
            // fprintf(stderr, "%2d ", idxs[groups[c * sz + r]]);
        // }
        // fprintf(stderr, "\n");
    // }

    /* create consistent numbers in groups */
    for (r=0; r<sz; r++) {
        for (c=0; c<sz; c++) {
            groups[c * sz + r] = idxs[groups[c * sz + r]];
        }
    }

    /* get neighbor groups for current stone */
    i = 0;
    if ((cur_r - 1) >= 0 && groups[cur_c * sz + (cur_r - 1)] >= 0) {
        neighbors[i] = groups[cur_c * sz + (cur_r - 1)];
        neighborsLibs[i] = 0;
        i += 1;
    }
    if ((cur_r + 1) < sz && groups[cur_c * sz + (cur_r + 1)] >= 0) {
        bFoundElem = 0;
        for (j=0; j<i; j++) {
            if (neighbors[j] == groups[cur_c * sz + (cur_r + 1)])
                bFoundElem = 1;
        }
        if (!bFoundElem) {
            neighbors[i] = groups[cur_c * sz + (cur_r + 1)];
            neighborsLibs[i] = 0;
            i += 1;
        }
    }
    if ((cur_c - 1) >= 0 && groups[(cur_c - 1) * sz + cur_r] >= 0) {
        bFoundElem = 0;
        for (j=0; j<i; j++) {
            if (neighbors[j] == groups[(cur_c - 1) * sz + cur_r])
                bFoundElem = 1;
        }
        if (!bFoundElem) {
            neighbors[i] = groups[(cur_c - 1) * sz + cur_r];
            neighborsLibs[i] = 0;
            i += 1;
        }
    }
    if ((cur_c + 1) < sz && groups[(cur_c + 1) * sz + cur_r] >= 0) {
        bFoundElem = 0;
        for (j=0; j<i; j++) {
            if (neighbors[j] == groups[(cur_c + 1) * sz + cur_r])
                bFoundElem = 1;
        }
        if (!bFoundElem) {
            neighbors[i] = groups[(cur_c + 1) * sz + cur_r];
            neighborsLibs[i] = 0;
            i += 1;
        }
    }
    neighbors[i] = -1; /* terminal indicator */

    /* determine number of liberties for each neighbor group */
    for (r=0; r<sz; r++) {
        for (c=0; c<sz; c++) {
            if (groups[c * sz + r] < 0)
                continue;

            for (i=0; neighbors[i] >= 0; i++) {
                if (groups[c * sz + r] != neighbors[i])
                    continue;

                if ((r-1) >= 0 && groups[c * sz + (r-1)] == -1)
                    neighborsLibs[i] += 1;
                if ((r+1) < sz && groups[c * sz + (r+1)] == -1)
                    neighborsLibs[i] += 1;
                if ((c-1) >= 0 && groups[(c-1) * sz + r] == -1)
                    neighborsLibs[i] += 1;
                if ((c+1) < sz && groups[(c+1) * sz + r] == -1)
                    neighborsLibs[i] += 1;
            }
        }
    }

    /* remove groups with no liberties */
    for (i=0; neighbors[i] >= 0; i++) {
        if (neighborsLibs[i] > 0)
            continue;

        for (r=0; r<sz; r++) {
            for (c=0; c<sz; c++) {
                if (groups[c * sz + r] == neighbors[i]) {
                    curBoard->board[c * sz + r].field_type = FIELD_EMPTY;
                    curBoard->board[c * sz + r].draw_update = 1;
                }
            }
        }
    }

    // /* print groups */
    // for (r=0; r<sz; r++) {
        // for (c=0; c<sz; c++) {
            // fprintf(stderr, "%2d ", groups[c * sz + r]);
        // }
        // fprintf(stderr, "\n");
    // }
    // /* print neighbor groups and number of libs */
    // fprintf(stderr, "Neighbor groups (+ num libs) of current stone [%d,%d]: ", cur_r, cur_c);
    // for (i=0; neighbors[i] >= 0; i++)
        // fprintf(stderr, "%d (%d), ", neighbors[i], neighborsLibs[i]);
    // fprintf(stderr, "\n");


    free(idxs);
    free(groups);

}/*}}}*/

void board_cleanup()
{/*{{{*/
    if (curBoard != NULL) {
        free(curBoard->board);
        curBoard->board = NULL;

        CloseFont(curBoard->draw_font);

        free(curBoard);
        curBoard = NULL;
    }
}/*}}}*/

void board_print()
{/*{{{*/
    int r, c;

    assert( curBoard != NULL );

    for (r=0; r<curBoard->size; r++) {
        for (c=0; c<curBoard->size; c++) {
            fprintf(stderr, "%d ", curBoard->board[c * curBoard->size + r].grid_type);
        }
        fprintf(stderr, "\n");
    }
}/*}}}*/

void board_draw_update(int bPartialUpdate)
{/*{{{*/
    int r, c, i, x, y;

    assert( curBoard != NULL );

    SetFont(curBoard->draw_font, BLACK);

    for (r=0; r<curBoard->size; r++) {
        for (c=0; c<curBoard->size; c++) {
            i = c * curBoard->size + r;
            x = curBoard->draw_offset_x + c * curBoard->draw_elemSize;
            y = curBoard->draw_offset_y + curBoard->draw_elemSize * r;

            /* check if update necessary */
            if (curBoard->board[i].draw_update) {
                if (bPartialUpdate)
                    FillArea(x, y, curBoard->draw_elemSize, curBoard->draw_elemSize, WHITE);

                switch (curBoard->board[i].field_type) {
                    case FIELD_EMPTY:
                        DrawString(x, y, gridTypeString[curBoard->board[i].grid_type]);
                        break;

                    case FIELD_BLACK:
                    case FIELD_WHITE:
                        DrawString(x, y, fieldTypeString[curBoard->board[i].field_type]);
                        break;
                }

                switch (curBoard->board[i].marker_type) {
                    case MARKER_KO:
                        DrawString(x, y, markerTypeString[curBoard->board[i].marker_type]);
                        break;

                    case MARKER_SQUARE:
                    case MARKER_TRIANGLE:
                    case MARKER_CIRC:
                        if (curBoard->board[i].field_type == FIELD_BLACK)
                            SetFont(curBoard->draw_font, WHITE);
                        DrawString(x, y, markerTypeString[curBoard->board[i].marker_type]);
                        SetFont(curBoard->draw_font, BLACK);
                        break;
                }

                if (bPartialUpdate)
                    PartialUpdate(x, y, curBoard->draw_elemSize, curBoard->draw_elemSize);

                curBoard->board[i].draw_update = 0;
            }

        }
    }
    /*
    if (bPartialUpdate)
        PartialUpdate(0, curBoard->draw_offset_y, curBoard->draw_elemSize * curBoard->size, curBoard->draw_elemSize * curBoard->size);
    */
}/*}}}*/


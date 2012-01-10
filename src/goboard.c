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
    MARKER_SQUARE, MARKER_CIRC, MARKER_TRIANGLE     /* shape marker */
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
    int num_caps_b;         /* captured stones, black and white */
    int num_caps_w;
    int draw_elemSize;      /* size in points for each field element */
    ifont *draw_font;       /* ttf handler for the drocerog ttf */
    int draw_offset_x;      /* move the board the specified points right */
    int draw_offset_y;      /* move the board the specified points down */
} GoBoard;

enum BOOL { FALSE, TRUE };

typedef struct ListElem_s {
    struct ListElem_s *next;
    int data;               /* FieldType and MarkerType are integers (enum) */
    int r;                  /* element placed on pos (r,c) */
    int c;
} ListElem;

typedef struct HistoryElem_s {
    struct HistoryElem_s *prev;
    ListElem *stones_placed;
    ListElem *stones_removed;
    ListElem *marker_set;
} HistoryElem;

/******************************************************************************/

static GoBoard *curBoard = NULL;

static HistoryElem *history_curNode = NULL;

/******************************************************************************/

void clearDeadGroups(int cur_r, int cur_c);
void hist_free(HistoryElem *curNode);
HistoryElem *hist_newElem(HistoryElem *prevElem);   /* prevElem is allowed to be NULL */
void list_free(ListElem *curNode);
ListElem *list_newElem(ListElem *prevElem, int r, int c, int data); /* prevElem is allowed to be NULL */

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

    /* init captured stones */
    curBoard->num_caps_b = 0;
    curBoard->num_caps_w = 0;

    /* init history */
    history_curNode = hist_newElem(NULL);

}/*}}}*/

HistoryElem *hist_newElem(HistoryElem *prevElem)
{/*{{{*/
    HistoryElem *newElem;

    newElem = (HistoryElem *) malloc(sizeof(HistoryElem));
    newElem->prev = prevElem;
    newElem->stones_placed = NULL;
    newElem->stones_removed = NULL;
    newElem->marker_set = NULL;

    return newElem;
}/*}}}*/

ListElem *list_newElem(ListElem *prevElem, int r, int c, int data)
{/*{{{*/
    ListElem *newElem;

    newElem = (ListElem *) malloc(sizeof(ListElem));
    newElem->next = NULL;
    newElem->r = r;
    newElem->c = c;
    newElem->data = data;
    if (prevElem != NULL) {
        while (prevElem->next)
            prevElem = prevElem->next;
        prevElem->next = newElem;
    }

    return newElem;
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
            break;

        case BOARD_WHITE:
            curBoard->board[c * curBoard->size + r].field_type = FIELD_WHITE;
            break;
    }
    curBoard->board[c * curBoard->size + r].draw_update = 1;

    /* update history */
    if (bIsMove) /* new move creates a new history element */
        history_curNode = hist_newElem(history_curNode);
    if (history_curNode->stones_placed) {
        list_newElem(history_curNode->stones_placed, r, c, curBoard->board[c * curBoard->size + r].field_type);
    } else {
        history_curNode->stones_placed = list_newElem(history_curNode->stones_placed, r, c, curBoard->board[c * curBoard->size + r].field_type);
    }

    /* remove stones if necessary */
    if (bIsMove)
        clearDeadGroups(r, c);

    // /* print history after this move */
    // {
        // HistoryElem *curHist;
        // ListElem *curLstElem;
        // for (curHist=history_curNode; curHist; curHist = curHist->prev) {
            // fprintf(stderr, "H[");
            // for (curLstElem=curHist->stones_placed; curLstElem; curLstElem=curLstElem->next)
                // fprintf(stderr, " P[%d,%d]", curLstElem->r, curLstElem->c);
            // for (curLstElem=curHist->stones_removed; curLstElem; curLstElem=curLstElem->next)
                // fprintf(stderr, " R[%d,%d]", curLstElem->r, curLstElem->c);
            // fprintf(stderr, " ]\n <- ");
        // }
        // fprintf(stderr, "END \n");
    // }

}/*}}}*/

int board_undo()
{/*{{{*/
    HistoryElem *oldHist = NULL;
    ListElem *curLstElem = NULL;

    assert( curBoard != NULL );
    assert( history_curNode != NULL );

    /* check if undo is possible */
    if (!history_curNode->prev)
        return 0;

    oldHist = history_curNode;
    history_curNode = history_curNode->prev;

    /* undo stone removal */
    for (curLstElem=oldHist->stones_removed; curLstElem; curLstElem=curLstElem->next) {
        curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].field_type = curLstElem->data;
        curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].draw_update = 1;
        /* notice undo removal in number of captured stones */
        switch (curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].field_type) {
            case FIELD_BLACK:
                    curBoard->num_caps_b -= 1;
                break;
            case FIELD_WHITE:
                    curBoard->num_caps_w -= 1;
                break;
        }
    }
    /* undo stone placement */
    for (curLstElem=oldHist->stones_placed; curLstElem; curLstElem=curLstElem->next) {
        curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].field_type = FIELD_EMPTY;
        curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].draw_update = 1;
    }
    /* undo marker */
    for (curLstElem=oldHist->marker_set; curLstElem; curLstElem=curLstElem->next) {
        curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].marker_type = MARKER_EMPTY;
        curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].draw_update = 1;
    }
    for (curLstElem=history_curNode->marker_set; curLstElem; curLstElem=curLstElem->next) {
        curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].marker_type = curLstElem->data;
        curBoard->board[curLstElem->c * curBoard->size + curLstElem->r].draw_update = 1;
    }

    /* unchain old history element and delete it */
    oldHist->prev = NULL;
    hist_free(oldHist);

    return 1;
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
                    if (history_curNode->stones_removed) {
                        list_newElem(history_curNode->stones_removed, r, c, curBoard->board[c * sz + r].field_type);
                    } else {
                        history_curNode->stones_removed = list_newElem(history_curNode->stones_removed, r, c, curBoard->board[c * sz + r].field_type);
                    }

                    /* notice removal in numbers of captured stones */
                    switch (curBoard->board[c * sz + r].field_type) {
                        case FIELD_BLACK:
                            curBoard->num_caps_b += 1;
                            break;
                        case FIELD_WHITE:
                            curBoard->num_caps_w += 1;
                            break;
                    }

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

        /* free history */
        if (history_curNode != NULL) {
            hist_free(history_curNode);
            history_curNode = NULL;
        }
    }
}/*}}}*/

void hist_free(HistoryElem *curNode)
{/*{{{*/
    HistoryElem *prevNode;

    while (curNode) {
        prevNode = curNode->prev;

        list_free(curNode->stones_placed);
        list_free(curNode->stones_removed);
        list_free(curNode->marker_set);
        free(curNode);

        curNode = prevNode;
    }
}/*}}}*/

void list_free(ListElem *curNode)
{/*{{{*/
    ListElem *nextNode;

    while (curNode) {
        nextNode = curNode->next;
        free(curNode);
        curNode = nextNode;
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
    int r_min, r_max, c_min, c_max;

    assert( curBoard != NULL );

    /* init min and max values with unreachable numbers */
    r_min = curBoard->size;
    r_max = -1;
    c_min = curBoard->size;
    c_max = -1;

    SetFont(curBoard->draw_font, BLACK);

    for (r=0; r<curBoard->size; r++) {
        for (c=0; c<curBoard->size; c++) {
            i = c * curBoard->size + r;
            x = curBoard->draw_offset_x + c * curBoard->draw_elemSize;
            y = curBoard->draw_offset_y + r * curBoard->draw_elemSize;

            /* check if update necessary */
            if (!bPartialUpdate || curBoard->board[i].draw_update) {

                if (r_min > r) r_min = r;
                if (r_max < r) r_max = r;
                if (c_min > c) c_min = c;
                if (c_max < c) c_max = c;

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

                // if (bPartialUpdate)
                    // PartialUpdate(x, y, curBoard->draw_elemSize, curBoard->draw_elemSize);

                curBoard->board[i].draw_update = 0;
            }
        }

    }
    /*
    if (bPartialUpdate)
        PartialUpdate(0, curBoard->draw_offset_y, curBoard->draw_elemSize * curBoard->size, curBoard->draw_elemSize * curBoard->size);
    */

    if (bPartialUpdate && r_min < curBoard->size) { /* ... && any element updated? */
        // fprintf(stderr, "r_min = %d, r_max = %d, c_min = %d, c_max = %d\n", r_min, r_max, c_min, c_max);
        x = curBoard->draw_offset_x + c_min * curBoard->draw_elemSize;
        y = curBoard->draw_offset_y + r_min * curBoard->draw_elemSize;
        PartialUpdateBW(x, y, curBoard->draw_elemSize * (c_max - c_min + 1), curBoard->draw_elemSize * (r_max - r_min + 1));
    }

}/*}}}*/

void board_get_captured(int *black, int *white)
{/*{{{*/
    assert(black);
    assert(white);

    *black = curBoard->num_caps_w;
    *white = curBoard->num_caps_b;
}/*}}}*/


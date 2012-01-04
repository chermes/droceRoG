/* Implementation of Go board methods.
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <goboard.h>

/******************************************************************************/

typedef enum { /* WARNING: when changing the element number, consider the bit
                * field length in GoBoardElement */
    GRID_TL, GRID_T, GRID_TR,        /* top left, top, top right          */
    GRID_L, GRID_R, GRID_C, GRID_CP, /* left, right, center, center point */
    GRID_BL, GRID_B, GRID_BR         /* bottom left, bottom, bottom right */
} GridType;

typedef enum { /* WARNING: when changing the element number, consider the bit
                * field length in GoBoardElement */
    FIELD_EMPTY,              /* empty field */
    FIELD_BLACK, FIELD_WHITE, /* black and white stone */
    FIELD_KO                  /* ko marker  */
} FieldType;

typedef struct
{
    unsigned grid_type:4;   /* GridType */
    unsigned field_type:2;  /* FieldType */
} GoBoardElement;

typedef struct
{
    short size;             /* size x size             */
    GoBoardElement *board;  /* board[col * size + row] */
} GoBoard;

/******************************************************************************/

static GoBoard *curBoard = NULL;

/******************************************************************************/

void board_new(short size)
{
    int r, c, i, hoshi;

    if (curBoard != NULL)
        board_cleanup();

    curBoard = (GoBoard*) malloc( sizeof(GoBoard) );
    curBoard->size = size;
    curBoard->board = (GoBoardElement *) malloc( sizeof(GoBoardElement) * size * size );

    for (r=0; r<size; r++) {
        for (c=0; c<size; c++) {
            i = c * size + r;
            curBoard->board[i].field_type = FIELD_EMPTY;

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
        curBoard->board[(short)(size/2) * size + (short)(size/2)].grid_type = GRID_CP;
    }
    if (size < 9)  hoshi = -1;
    if (size == 9) hoshi = 2;
    if (size > 9)  hoshi = 3;
    if (hoshi > 0) {
        curBoard->board[hoshi * size + hoshi].grid_type = GRID_CP;
        if (size % 2 == 1)
            curBoard->board[hoshi * size + (short)(size/2)].grid_type = GRID_CP;
        curBoard->board[hoshi * size + (size - hoshi - 1)].grid_type = GRID_CP;
        if (size % 2 == 1) {
            curBoard->board[(short)(size/2) * size + hoshi].grid_type = GRID_CP;
            curBoard->board[(short)(size/2) * size + (size - hoshi - 1)].grid_type = GRID_CP;
        }
        curBoard->board[(size - hoshi - 1) * size + hoshi].grid_type = GRID_CP;
        if (size % 2 == 1)
            curBoard->board[(size - hoshi - 1) * size + (short)(size/2)].grid_type = GRID_CP;
        curBoard->board[(size - hoshi - 1) * size + (size - hoshi - 1)].grid_type = GRID_CP;
    }
}

void board_cleanup()
{
    if (curBoard != NULL) {
        free(curBoard->board);
        curBoard->board = NULL;
        free(curBoard);
        curBoard = NULL;
    }
}

void board_print()
{
    int r, c;

    assert( curBoard != NULL );

    for (r=0; r<curBoard->size; r++) {
        for (c=0; c<curBoard->size; c++) {
            fprintf(stderr, "%d ", curBoard->board[c * curBoard->size + r].grid_type);
        }
        fprintf(stderr, "\n");
    }
}


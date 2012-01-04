/* Implementation of Go board methods.
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#include <goboard.h>

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
    int draw_offset_y;      /* move the board the specified points down */
} GoBoard;

enum BOOL { FALSE, TRUE };

/******************************************************************************/

static GoBoard *curBoard = NULL;

/******************************************************************************/

void board_new(int size, int offset_y)
{
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
    curBoard->draw_offset_y = offset_y;

}

void board_test_placeStones()
{
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
}

void board_cleanup()
{
    if (curBoard != NULL) {
        free(curBoard->board);
        curBoard->board = NULL;

        CloseFont(curBoard->draw_font);

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

void board_draw_update(int bPartialUpdate)
{
    int r, c, i, x, y;

    assert( curBoard != NULL );

    SetFont(curBoard->draw_font, BLACK);

    for (r=0; r<curBoard->size; r++) {
        for (c=0; c<curBoard->size; c++) {
            i = c * curBoard->size + r;
            x = c * curBoard->draw_elemSize;
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
            }

        }
    }
    /*
    if (bPartialUpdate)
        PartialUpdate(0, curBoard->draw_offset_y, curBoard->draw_elemSize * curBoard->size, curBoard->draw_elemSize * curBoard->size);
    */
}


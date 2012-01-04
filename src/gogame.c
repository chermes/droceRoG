/* Go game
 *
 * Author: Christoph Hermes (hermes@hausmilbe.net)
 */

#include "gogame.h"

#include <stdlib.h>
#include <assert.h>

#include <sgftree.h>
#include <inkview.h>

#include "goboard.h"

/******************************************************************************/

typedef struct {
    char *name;
    char *rank;
} Player;

typedef struct {
    Player black;
    Player white;
    int boardSize;
    char *komi;
    int handicap;
    char *date;
    char *result;
    int time;
    char *overtime;
    char *ruleset;
} GameInfo;

typedef struct {
    int fontSize;
    int fontSpace;
    ifont *font_ttf;
} DrawProperties;

/******************************************************************************/

static SGFTree *gameTree = NULL; /* game tree */
static GameInfo gameInfo;

static DrawProperties drawProperties;

static char *str_unknown = "unknown";

/******************************************************************************/

#define GET_CHAR_PROP(name__, ref__) \
    if (!sgfGetCharProperty(gameTree->root, name__, &ref__)) \
        ref__ = str_unknown;

/******************************************************************************/

void readGameInfo();
void initDrawProperties();

/******************************************************************************/

int gogame_new_from_file(const char *filename)
{/*{{{*/
    gogame_cleanup();

    gameTree = (SGFTree *) malloc(sizeof(SGFTree));
    sgftree_clear(gameTree); /* set node pointers to NULL */

    if (!sgftree_readfile(gameTree, filename))
        return 0;

    readGameInfo();
    initDrawProperties();

    board_new(gameInfo.boardSize, drawProperties.fontSize * 2 + drawProperties.fontSpace * 3);

    return 1;
}/*}}}*/

void gogame_cleanup()
{/*{{{*/
    if (gameTree != NULL) {
        /* free SGF info */
        sgfFreeNode(gameTree->root); /* recursively free the sgf tree */
        sgftree_clear(gameTree);
        free(gameTree);
        gameTree = NULL;

        /* cleanup other game info */
        gameInfo.black.name = NULL;
        gameInfo.black.rank = NULL;
        gameInfo.white.name = NULL;
        gameInfo.white.rank = NULL;
        gameInfo.boardSize  = 19;
        gameInfo.komi       = NULL;
        gameInfo.handicap   = 0;
        gameInfo.date       = NULL;
        gameInfo.result     = NULL;
        gameInfo.time       = 0;
        gameInfo.overtime   = NULL;
        gameInfo.ruleset    = NULL;

        /* cleanup draw properties */
        drawProperties.fontSize = 12;
        CloseFont(drawProperties.font_ttf);
        drawProperties.font_ttf = NULL;

        /* cleanup go board */
        board_cleanup();
    }
}/*}}}*/

void initDrawProperties()
{/*{{{*/
    drawProperties.fontSize  = (int) ((double)ScreenWidth() / 600.0 * 12.0);
    drawProperties.fontSpace = (int) ((double)ScreenWidth() / 600.0 * 4.0);
    drawProperties.font_ttf = OpenFont("DejaVuSerif", drawProperties.fontSize, 1);
}/*}}}*/

void gogame_draw_fullrepaint()
{/*{{{*/
    char msg[1024];

    assert( gameTree != NULL );

    ClearScreen();

    /* draw title */
    SetFont(drawProperties.font_ttf, BLACK);
    snprintf( msg, sizeof(msg),
        "Black: %s [%s], White: %s [%s], Date: %s, Result = %s",
        gameInfo.black.name, gameInfo.black.rank, gameInfo.white.name, gameInfo.white.rank, 
        gameInfo.date, gameInfo.result);
    DrawString(2, drawProperties.fontSpace, msg);
    snprintf( msg, sizeof(msg),
        "Time: %d min (%s), Komi: %s, Handicap: %d, Ruleset: %s",
        gameInfo.time / 60, gameInfo.overtime,
        gameInfo.komi, gameInfo.handicap, gameInfo.ruleset);
    DrawString(2, drawProperties.fontSpace*2+drawProperties.fontSize, msg);

    /* draw go board, if an SGF is loaded */
    if (gameTree != NULL)
        board_draw_update(0);

    FullUpdate();

}/*}}}*/

void gogame_printGameInfo()
{/*{{{*/
    assert(gameTree != NULL);

    fprintf(stderr, "GAME INFO   Black: %s [%s], White: %s [%s]\n", gameInfo.black.name, gameInfo.black.rank, gameInfo.white.name, gameInfo.white.rank);
    fprintf(stderr, "            Board size %d x %d\n", gameInfo.boardSize, gameInfo.boardSize);
    fprintf(stderr, "            Result = %s, Date = %s\n", gameInfo.result, gameInfo.date);
    fprintf(stderr, "            Komi = %s, Handicap = %d\n", gameInfo.komi, gameInfo.handicap);
    fprintf(stderr, "            Ruleset = %s, Time = %d min, Overtime = %s\n", gameInfo.ruleset, gameInfo.time/60, gameInfo.overtime);
}/*}}}*/

void readGameInfo()
{/*{{{*/
    assert(gameTree != NULL);

    GET_CHAR_PROP("PB", gameInfo.black.name);

    GET_CHAR_PROP("PB", gameInfo.black.name);
    GET_CHAR_PROP("BR", gameInfo.black.rank);
    GET_CHAR_PROP("PW", gameInfo.white.name);
    GET_CHAR_PROP("WR", gameInfo.white.rank);

    if (!sgfGetIntProperty(gameTree->root, "SZ", &gameInfo.boardSize))
        gameInfo.boardSize = 19;
    GET_CHAR_PROP("KM", gameInfo.komi);
    if (!sgfGetIntProperty(gameTree->root, "HA", &gameInfo.handicap))
        gameInfo.handicap = 0;
    GET_CHAR_PROP("DT", gameInfo.date);
    GET_CHAR_PROP("RE", gameInfo.result);
    if (!sgfGetIntProperty(gameTree->root, "TM", &gameInfo.time))
        gameInfo.time = 0;
    GET_CHAR_PROP("OT", gameInfo.overtime);
    GET_CHAR_PROP("RU", gameInfo.ruleset);
}/*}}}*/


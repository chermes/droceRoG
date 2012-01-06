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
static SGFNode *curNode = NULL; /* current node in the game tree */

static GameInfo gameInfo;

static DrawProperties drawProperties;

static char *str_unknown = "unknown";

/******************************************************************************/

#define GET_CHAR_PROP(name__, ref__) \
    if (!sgfGetCharProperty(gameTree->root, name__, &ref__)) \
        ref__ = str_unknown;

#define ENC_SGFPROP(c1_, c2_) ((short)( c1_ | c2_ << 8 ))

/******************************************************************************/

void readGameInfo();
void initDrawProperties();
void test_readSGF();
void debug_msg(char *s);
void apply_sgf_cmds_to_board();

/******************************************************************************/

void debug_msg(char *s) 
{/*{{{*/
    ifont *times12;

    times12 = OpenFont("DejaVuSerif", 12, 1);
    FillArea(350, 770, 250, 20, WHITE);
    SetFont(times12, BLACK);
    DrawString(350, 770, s);
    PartialUpdate(350, 770, 250, 20);
    CloseFont(times12);
}/*}}}*/

int gogame_new_from_file(const char *filename)
{/*{{{*/
    gogame_cleanup();

    gameTree = (SGFTree *) malloc(sizeof(SGFTree));
    if (gameTree == NULL)
        return 1;
    sgftree_clear(gameTree); /* set node pointers to NULL */

    if (!sgftree_readfile(gameTree, filename)) {
        gogame_cleanup();
        return 2;
    }
    curNode = gameTree->root;

    readGameInfo();
    initDrawProperties();

    board_new(gameInfo.boardSize, drawProperties.fontSize * 2 + drawProperties.fontSpace * 3);

    apply_sgf_cmds_to_board();
    /* test_readSGF(); */

    return 0;
}/*}}}*/

void gogame_cleanup()
{/*{{{*/
    if (gameTree != NULL) {
        /* free SGF info */
        sgfFreeNode(gameTree->root); /* recursively free the sgf tree */
        sgftree_clear(gameTree);
        free(gameTree);
        gameTree = NULL;
        curNode = NULL;

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

void test_readSGF()
{/*{{{*/
    SGFNode *cur = NULL;
    SGFProperty *prop = NULL;
    int r, c;

    /* test: list all properties in the main path, ignoring children */
    for (cur = gameTree->root; cur; cur = cur->child) {
        /* list all properties */
        for (prop = cur->props; prop; prop = prop->next) {
            fprintf(stderr, "%c%c[%s] ",
                prop->name & 255, prop->name >> 8, prop->value );
        }
        fprintf(stderr, "\n");

        for (prop = cur->props; prop; prop = prop->next) {
            r = prop->value[1] - 'a';
            c = prop->value[0] - 'a';
            switch (prop->name) {
                case ENC_SGFPROP('A', 'W'):
                    board_placeStone(r, c, BOARD_WHITE, 0);
                    break;

                case ENC_SGFPROP('A', 'B'):
                    board_placeStone(r, c, BOARD_BLACK, 0);
                    break;

                case ENC_SGFPROP('B', ' '):
                    board_placeStone(r, c, BOARD_BLACK, 1);
                    break;

                case ENC_SGFPROP('W', ' '):
                    board_placeStone(r, c, BOARD_WHITE, 1);
                    break;
            }
        }
    }
}/*}}}*/

void gogame_draw_fullrepaint()
{/*{{{*/
    char msg[1024];
    ifont *default_ttf;

    ClearScreen();

    if (gameTree != NULL) {
        /* draw title */
        SetFont(drawProperties.font_ttf, BLACK);
        snprintf( msg, sizeof(msg),
            "Black: %s [%s], White: %s [%s], Date: %s, Result: %s",
            gameInfo.black.name, gameInfo.black.rank, gameInfo.white.name, gameInfo.white.rank, 
            gameInfo.date, gameInfo.result);
        DrawString(2, drawProperties.fontSpace, msg);
        snprintf( msg, sizeof(msg),
            "Time: %d min (%s), Komi: %s, Handicap: %d, Ruleset: %s",
            gameInfo.time / 60, gameInfo.overtime,
            gameInfo.komi, gameInfo.handicap, gameInfo.ruleset);
        DrawString(2, drawProperties.fontSpace*2+drawProperties.fontSize, msg);
    } else {
        default_ttf = OpenFont("DejaVuSerif", 12, 1);
        SetFont(default_ttf, BLACK);
        DrawString(5, 20, "droceRoG - Go Game Record Viewer");
        DrawString(5, 40, "Author: Christoph Hermes (hermes<at>hausmilbe<dot>net)");
        DrawString(5, 70, "Please open a file by pressing the Menu symbol on the right side.");
        CloseFont(default_ttf);
    }

    /* draw go board, if an SGF is loaded */
    if (gameTree != NULL)
        board_draw_update(0);

    FullUpdate();

}/*}}}*/

void gogame_draw_update()
{/*{{{*/
    if (gameTree != NULL)
        board_draw_update(1);
}/*}}}*/

void gogame_printGameInfo()
{/*{{{*/
    if (gameTree == NULL)
        return;

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

void gogame_move_forward()
{/*{{{*/
    if (gameTree == NULL)
        return;

    /* do nothing, if no continuation in this variation exists */
    if (!curNode->child) 
        return;
    curNode = curNode->child;

    apply_sgf_cmds_to_board();

}/*}}}*/

void apply_sgf_cmds_to_board()
{/*{{{*/
    SGFProperty *prop = NULL;
    int r, c;

    assert(gameTree != NULL);

    /* for all properties in this move */
    for (prop = curNode->props; prop; prop = prop->next) {
        r = prop->value[1] - 'a';
        c = prop->value[0] - 'a';
        switch (prop->name) {
            case ENC_SGFPROP('A', 'W'):
                board_placeStone(r, c, BOARD_WHITE, 0);
                break;

            case ENC_SGFPROP('A', 'B'):
                board_placeStone(r, c, BOARD_BLACK, 0);
                break;

            case ENC_SGFPROP('B', ' '):
                board_placeStone(r, c, BOARD_BLACK, 1);
                break;

            case ENC_SGFPROP('W', ' '):
                board_placeStone(r, c, BOARD_WHITE, 1);
                break;
        }
    }
}/*}}}*/

void gogame_move_back()
{/*{{{*/
    if (gameTree == NULL)
        return;
}/*}}}*/

void gogame_moveVar_down()
{/*{{{*/
    if (gameTree == NULL)
        return;
}/*}}}*/

void gogame_moveVar_up()
{/*{{{*/
    if (gameTree == NULL)
        return;
}/*}}}*/


/* droceRoG - SGF file selector
 *
 * Author: Christoph Hermes (hermes<AT>hausmilbe<DOT>net)
 */

#include "fileselector.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <inkview.h>

/******************************************************************************/

typedef struct TOC_Elem_s {
    struct TOC_Elem_s *next;
    tocentry toc;
    char full_fname[256];
    unsigned int isDir:1;
} TOC_Elem;

/******************************************************************************/

void (*cb_update_fun)(char *filename) = NULL;

static TOC_Elem *lst;

static tocentry *contents = NULL;

/******************************************************************************/

TOC_Elem *readFileList(char *dirname, int lvl);
TOC_Elem *tocElem_new();
void tocElem_free(TOC_Elem *elem);
int tocElem_getNumInList(TOC_Elem *elem);

/******************************************************************************/

void entry_selected(int page) 
{/*{{{*/
    TOC_Elem *cur;

    // fprintf(stderr, "fileselector.c: page %d selected.\n", page);

    /* find selected entry */
    cur = lst;
    while (cur && cur->toc.page != page)
        cur = cur->next;
    assert(cur != NULL);
    if (cb_update_fun != NULL)
        (*cb_update_fun)(cur->full_fname);

    /* free list */
    while (lst != NULL) {
        cur = lst;
        lst = lst->next;

        tocElem_free(cur);
    }

    /* free contents */
    if (contents != NULL)
        free(contents);

}/*}}}*/

void fileselector_chooseFile(void (*cb_update)(char *filename))
{/*{{{*/
    int current_page = 1;
    TOC_Elem *cur;
    int i;
    int numElems;

    cb_update_fun = cb_update;

    /* read directory */
    lst = readFileList(FLASHDIR, 0);

    /* add increasing number to toc list */
    i = 0;
    for (cur = lst; cur; cur = cur->next) {
        // fprintf(stderr, "list elem: %s [lvl: %d]\n", cur->full_fname, cur->toc.level);
        cur->toc.page = i;
        cur->toc.position = (long long) i;
        i += 1;
    }

    numElems = tocElem_getNumInList(lst);

    /* free list and return if only one element is found */
    if (numElems <= 1) {
        tocElem_free(lst);
        return;
    }

    /* build and populate contents list */
    contents = (tocentry *) malloc(sizeof(tocentry) * (numElems - 1));
    cur = lst->next;
    for (i=0; i<numElems-1; i++) {
        contents[i].level = cur->toc.level;
        contents[i].page = cur->toc.page;
        contents[i].position = cur->toc.position;
        contents[i].text = cur->toc.text;

        cur = cur->next;
    }

    // for (i=0; i<numElems-1; i++) {
        // fprintf(stderr, "contents[%d] = (lvl: %d, text: %s, page: %d, pos: %d)\n", 
                // i, contents[i].level, contents[i].text,
                // contents[i].page, (int) contents[i].position);
    // }
    // fprintf(stderr, "ready building up content list\n");
    // fflush(stderr);

    OpenContents(contents, numElems-1, current_page, (iv_tochandler) entry_selected);

    // fprintf(stderr, "finished OpenContents\n");
}/*}}}*/

int is_SGF_filename(char *fname)
{/*{{{*/
    assert(fname);

    return strstr(fname, ".sgf") > (char *)NULL;
}/*}}}*/

char* get_filename_in_path(char *path)
{/*{{{*/
    unsigned int i;
    int lastOcc = 0;

    assert(path);

    for (i=0; i<strlen(path); i++) {
        if (path[i] == '/')
            lastOcc = i + 1;
    }

    return path + lastOcc;
}/*}}}*/

TOC_Elem *readFileList(char *dirname, int lvl)
{/*{{{*/
    DIR *dir;
    struct dirent *dir_content;
    char fullpath[256];
    TOC_Elem *subdir, *curElem, *curList;
    
    dir = iv_opendir(dirname); /* root directory */
    if (!dir) {
        fprintf(stderr, "[ERROR] Could not open directory %s", dirname);
        return NULL;
    }

    /* add current directory to toc list */
    curElem = tocElem_new();
    snprintf(curElem->full_fname, sizeof(curElem->full_fname), "%s", dirname);
    curElem->isDir = 1;
    curElem->toc.level = lvl;
    /* point only to filename */
    curElem->toc.text = get_filename_in_path(curElem->full_fname);
    curList = curElem; /* save beginning */

    while ( (dir_content = iv_readdir(dir)) ) {
        /* ignore ".." and "." directories */
        if (!iv_strcmp(dir_content->d_name, ".") || !iv_strcmp(dir_content->d_name, ".."))
            continue;

        /* recursively move down one dir level */
        if (dir_content->d_type & DT_DIR) {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, dir_content->d_name);
            subdir = readFileList(fullpath, lvl + 1);

            /* ignore subdirectory if only one element exists (the dir name itself) */
            if (tocElem_getNumInList(subdir) <= 1) {
                tocElem_free(subdir);
                continue;
            }

            /* add subdirectory lists to curElem */
            curElem->next = subdir;
            while (curElem->next)   /* move to end of list */
                curElem = curElem->next;
        }

        /* ignore non-sgf files */
        if (dir_content->d_type & DT_REG && is_SGF_filename(dir_content->d_name)) {
            /* add file element to list */
            curElem->next = tocElem_new();
            curElem = curElem->next;
            snprintf(curElem->full_fname, sizeof(curElem->full_fname),
                     "%s/%s",
                     dirname, dir_content->d_name);
            curElem->isDir = 0;
            curElem->toc.level = lvl + 1;
            /* point only to filename */
            curElem->toc.text = get_filename_in_path(curElem->full_fname);
        }
            

        // fprintf(stderr, "dir content: %s/%s [lvl: %d] [isFile: %d] [isDir: %d]\n",
                // dirname, dir_content->d_name, lvl,
                // dir_content->d_type & DT_REG, dir_content->d_type & DT_DIR);
    }

    iv_closedir( dir );

    return curList;
}/*}}}*/

TOC_Elem *tocElem_new()
{/*{{{*/
    TOC_Elem *elem;

    elem = (TOC_Elem *) malloc(sizeof(TOC_Elem));

    elem->next = NULL;
    elem->isDir = 0;
    elem->toc.level = 0;
    elem->toc.page = 0;
    elem->toc.position = 0;
    elem->toc.text = NULL;

    return elem;
}/*}}}*/

void tocElem_free(TOC_Elem *elem)
{/*{{{*/
    if (elem != NULL)
        free(elem);
}/*}}}*/

int tocElem_getNumInList(TOC_Elem *elem)
{/*{{{*/
    int num = 0;

    if (!elem)
        return -1;

    while (elem) {
        num += 1;
        elem = elem->next;
    }

    return num;
}/*}}}*/


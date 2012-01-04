#INCLUDE_DIRECTORIES(${GNUGo_SOURCE_DIR}/utils)


########### sgf library ###############

SET(sgf_STAT_SRCS
    sgf_utils.c
    sgfnode.c
    sgftree.c
    )

ADD_LIBRARY(sgf STATIC ${sgf_STAT_SRCS})

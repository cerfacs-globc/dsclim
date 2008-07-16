/*! \file utils.h
    \brief Include file for utilities library.
*/
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#ifndef UTILS_H
#define UTILS_H

void alloc_error(void);
void banner(char *pgm, char *verstat, char *type);

#endif


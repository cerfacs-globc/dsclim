/*! \file utils.h
    \brief Include file for utilities library.
*/
#ifndef UTILS_H
#define UTILS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

void alloc_error(void);
void banner(char *pgm, char *verstat, char *type);

#endif


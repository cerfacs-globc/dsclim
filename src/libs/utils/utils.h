/*! \file utils.h
    \brief Include file for utilities library.
*/
#ifndef UTILS_H
#define UTILS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions */
#define _GNU_SOURCE

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

/** TRUE value macro is 1. */
#define TRUE 1
/** FALSE value macro is 0. */
#define FALSE 0

void alloc_error(char *filename, int line);
void banner(char *pgm, char *verstat, char *type);

#endif


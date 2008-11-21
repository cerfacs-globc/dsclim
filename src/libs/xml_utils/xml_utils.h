/*! \file xml_utils.h
    \brief Include file for XML utilities library.
*/
#ifndef XML_UTIL_H
#define XML_UTIL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions */
#define _GNU_SOURCE

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
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
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

/** TRUE value macro is 1. */
#define TRUE 1
/** FALSE value macro is 0. */
#define FALSE 0

#include <utils.h>

#include <libxml/parser.h>
#include <libxml/xpath.h>

/** XML structure to deal with DOM using XPath. */
typedef struct {
  char *file; /**< XML filename. */
  xmlDocPtr doc; /**< Pointer to XML document. */
  xmlNodePtr root; /**< Pointer to first node of DOM. */
  xmlXPathContextPtr ctxt; /**< Pointer to XPath context. */
} xmlConfig_t;


void xml_free_config(xmlConfig_t *conf);
xmlConfig_t *xml_load_config(char *filename);
xmlNodePtr xml_get_node_by_xpath(xmlConfig_t *conf, char *path);
xmlChar *xml_get_setting(xmlConfig_t *conf, char *path);
void xml_print_setting(xmlConfig_t *conf, char *path);

#endif

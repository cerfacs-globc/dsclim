# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

noinst_LTLIBRARIES = libxml_utils.la
libxml_utils_la_SOURCES = xml_utils.h xml_free_config.c xml_get_node_by_xpath.c xml_get_setting.c xml_load_config.c xml_print_setting.c
libxml_utils_la_CPPFLAGS = -I${top_srcdir}/src/libs/utils $(XML_CPPFLAGS)
libxml_utils_la_LIBADD = ${top_srcdir}/src/libs/utils/libutils.la $(XML_LIBS) -ludunits -lm

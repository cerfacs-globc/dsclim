# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

include $(top_srcdir)/doxygen-include.am

ACLOCAL_AMFLAGS = -I m4

#EXTRA_DIST = reconf configure $(DX_CONFIG) doc/html
EXTRA_DIST = reconf configure $(DX_CONFIG)
SUBDIRS = src/libs src tests .

MOSTLYCLEANFILES = $(DX_CLEANFILES)

CLEANFILES = $(DIST_ARCHIVES)

MAINTAINERCLEANFILES = \
  Makefile \
  Makefile.in \
  aclocal.m4 \
  autom4te.cache \
  config.log \
  config.status \
  configure \
  install-sh \
  missing

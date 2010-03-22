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
DISTCHECK_CONFIGURE_FLAGS = --prefix=/home/page/codes/local --with-zlib=/contrex/local64 --with-szlib=/contrex/local64 --with-hdf5=/contrex/local64 --with-netcdf=/contrex/local64 --with-udunits=/contrex/local64 --disable-doxygen 

#EXTRA_DIST = reconf configure $(DX_CONFIG) doc/html
EXTRA_DIST = configure $(DX_CONFIG) etc doc/html
SUBDIRS = m4 src/libs src tests .

confsdir=${pkgdatadir}
confs_DATA=etc/configuration_example_grid.xml etc/configuration_example_nongrid.xml

#htmldocdir=${pkgdatadir}/doc/html
#htmldoc_DATA = doc/html/*

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

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(datadir)/$(PACKAGE)/doc/html
	$(INSTALL_DATA) $(srcdir)/doc/html/* $(DESTDIR)$(datadir)/$(PACKAGE)/doc/html
uninstall-local:
	rm $(DESTDIR)$(datadir)/$(PACKAGE)/doc/html/*
	-rmdir $(DESTDIR)$(datadir)/$(PACKAGE)/doc/html

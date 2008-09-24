# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

noinst_LTLIBRARIES = libio.la
libio_la_SOURCES = io.h read_netcdf_dims_3d.c read_netcdf_var_3d.c handle_netcdf_error.c
libio_la_CPPFLAGS = -I${top_srcdir}/src/libs/utils $(NCDF_CPPFLAGS)
libio_la_LIBADD = ${top_srcdir}/src/libs/utils/libutils.la $(NCDF_LIBS)

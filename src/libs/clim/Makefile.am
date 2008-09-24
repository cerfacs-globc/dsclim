# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

noinst_LTLIBRARIES = libclim.la
libclim_la_SOURCES = clim.h clim_daily_tserie_climyear.c remove_seasonal_cycle.c
libclim_la_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/filter
libclim_la_LIBADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/filter/libfilter.la

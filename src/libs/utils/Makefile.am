# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

noinst_LTLIBRARIES = libutils.la
libutils_la_SOURCES = utils.h alloc_error.c banner.c alloc_mmap_float.c alloc_mmap_double.c alloc_mmap_int.c alloc_mmap_longint.c alloc_mmap_shortint.c data_to_gregorian_cal.c utCalendar_cal.h utCalendar_cal.c get_calendar.c get_calendar_ts.c mean_variance_field_spatial.c sub_period_common.c extract_subdomain.c extract_subperiod.c
libutils_la_CPPFLAGS = $(GSL_CFLAGS)
libutils_la_LIBADD = $(GSL_LIBS) -ludunits -lm

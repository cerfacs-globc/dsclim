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
libutils_la_SOURCES = utils.h alloc_mmap_float.c alloc_mmap_double.c alloc_mmap_int.c alloc_mmap_longint.c alloc_mmap_shortint.c data_to_gregorian_cal.c utCalendar2_cal.h utCalendar2_cal.c get_calendar.c get_calendar_ts.c change_date_origin.c mean_variance_field_spatial.c sub_period_common.c extract_subdomain.c extract_subperiod_months.c mask_region.c mask_points.c mean_field_spatial.c covariance_fields_spatial.c time_mean_variance_field_2d.c normalize_field.c normalize_field_2d.c comparf.c distance_point.c find_str_value.c alt_to_press.c spechum_to_hr.c calc_etp_mf.c get_filename_ext.c
libutils_la_CPPFLAGS = -I${top_srcdir}/src/libs/misc -I${top_srcdir}/src $(GSL_CFLAGS) $(UDUNITS_CPPFLAGS)
libutils_la_LIBADD = ../misc/libmisc.la $(GSL_LIBS) $(UDUNITS_LIBS) -lm

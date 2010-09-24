# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

SUBDIRS=.

bin_PROGRAMS = dsclim
dsclim_SOURCES = dsclim.h constants.h dsclim.c load_conf.c write_learning_fields.c write_regression_fields.c read_large_scale_fields.c read_learning_obs_eof.c read_learning_rea_eof.c read_large_scale_eof.c remove_clim.c read_field_subdomain_period.c read_learning_fields.c read_regression_points.c read_mask.c read_obs_period.c find_the_days.c compute_secondary_large_scale_diff.c merge_seasons.c merge_seasonal_data.c merge_seasonal_data_i.c output_downscaled_analog.c read_analog_data.c save_analog_data.c free_main_data.c wt_downscaling.c wt_learning.c 
dsclim_CPPFLAGS = -I${top_srcdir}/src/libs/misc -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/classif -I${top_srcdir}/src/libs/pceof -I${top_srcdir}/src/libs/clim -I${top_srcdir}/src/libs/filter -I${top_srcdir}/src/libs/regress -I${top_srcdir}/src/libs/xml_utils -I${top_srcdir}/src/libs/io -I. $(XML_CPPFLAGS) $(GSL_CFLAGS) $(NCDF_CPPFLAGS)
dsclim_LDADD = libs/misc/libmisc.la libs/utils/libutils.la libs/classif/libclassif.la libs/pceof/libpceof.la libs/clim/libclim.la libs/filter/libfilter.la libs/regress/libregress.la libs/xml_utils/libxml_utils.la libs/io/libio.la $(XML_LIBS) $(GSL_LIBS) $(NCDF_LIBS)

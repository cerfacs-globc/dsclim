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
dsclim_SOURCES = dsclim.c load_conf.c wt_downscaling.c read_large_scale_fields.c read_large_scale_eof.c remove_clim.c
dsclim_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/classif -I${top_srcdir}/src/libs/pceof -I${top_srcdir}/src/libs/clim -I${top_srcdir}/src/libs/filter -I${top_srcdir}/src/libs/regress -I${top_srcdir}/src/libs/xml_utils -I${top_srcdir}/src/libs/io -I. $(XML_CPPFLAGS) $(GSL_CFLAGS) $(NCDF_CPPFLAGS)
dsclim_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/classif/libclassif.la ${top_srcdir}/src/libs/pceof/libpceof.la ${top_srcdir}/src/libs/clim/libclim.la ${top_srcdir}/src/libs/filter/libfilter.la ${top_srcdir}/src/libs/regress/libregress.la ${top_srcdir}/src/libs/xml_utils/libxml_utils.la ${top_srcdir}/src/libs/io/libio.la $(XML_LIBS) $(GSL_LIBS) $(NCDF_LIBS)

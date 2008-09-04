# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS = testfilter testrandomu testclassif testbestclassif testbestclassif_realdata testregress testcalendar testcalendar_val testudunits

testfilter_SOURCES = testfilter.c
testfilter_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/filter
testfilter_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/filter/libfilter.la

testrandomu_SOURCES = testrandomu.c
testrandomu_CPPFLAGS = -I${top_srcdir}/src/libs/utils $(GSL_CFLAGS)
testrandomu_LDADD = ${top_srcdir}/src/libs/utils/libutils.la $(GSL_LIBS)

testregress_SOURCES = testregress.c
testregress_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/regress $(GSL_CFLAGS)
testregress_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/regress/libregress.la $(GSL_LIBS)

testclassif_SOURCES = testclassif.c
testclassif_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/classif $(GSL_CFLAGS)
testclassif_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/classif/libclassif.la $(GSL_LIBS)

testbestclassif_SOURCES = testbestclassif.c
testbestclassif_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/classif $(GSL_CFLAGS)
testbestclassif_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/classif/libclassif.la $(GSL_LIBS)

testbestclassif_realdata_SOURCES = testbestclassif_realdata.c
testbestclassif_realdata_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/classif $(GSL_CFLAGS) $(NCDF_CPPFLAGS)
testbestclassif_realdata_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/classif/libclassif.la $(GSL_LIBS) $(NCDF_LIBS)

testcalendar_SOURCES = testcalendar.c
testcalendar_CPPFLAGS = -I${top_srcdir}/src/libs/utils $(NCDF_CPPFLAGS) $(UDUNITS_CPPFLAGS)
testcalendar_LDADD = ${top_srcdir}/src/libs/utils/libutils.la $(NCDF_LIBS) $(UDUNITS_LIBS)

testcalendar_val_SOURCES = testcalendar_val.c
testcalendar_val_CPPFLAGS = -I${top_srcdir}/src/libs/utils $(UDUNITS_CPPFLAGS)
testcalendar_val_LDADD = ${top_srcdir}/src/libs/utils/libutils.la $(UDUNITS_LIBS)

testudunits_SOURCES = testudunits.c
testudunits_CPPFLAGS = $(UDUNITS_CPPFLAGS)
testudunits_LDADD = $(UDUNITS_LIBS) $(LIBS)

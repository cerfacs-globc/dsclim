# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS = testfilter testrandomu testclassif testbestclassif

testfilter_SOURCES = testfilter.c
testfilter_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/filter
testfilter_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/filter/libfilter.la

testrandomu_SOURCES = testrandomu.c
testrandomu_CPPFLAGS = -I${top_srcdir}/src/libs/utils $(GSL_CFLAGS)
testrandomu_LDADD = ${top_srcdir}/src/libs/utils/libutils.la $(GSL_LIBS) -lgslcblas

testclassif_SOURCES = testclassif.c
testclassif_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/classif $(GSL_CFLAGS)
testclassif_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/classif/libclassif.la $(GSL_LIBS) -lgslcblas

testbestclassif_SOURCES = testbestclassif.c
testbestclassif_CPPFLAGS = -I${top_srcdir}/src/libs/utils -I${top_srcdir}/src/libs/classif $(GSL_CFLAGS)
testbestclassif_LDADD = ${top_srcdir}/src/libs/utils/libutils.la ${top_srcdir}/src/libs/classif/libclassif.la $(GSL_LIBS) -lgslcblas

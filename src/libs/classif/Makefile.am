# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

noinst_LTLIBRARIES = libclassif.la
libclassif_la_SOURCES = classif.h class_days_pc_clusters.c generate_clusters.c best_clusters.c mean_variance_dist_clusters.c dist_clusters_normctrl.c
libclassif_la_CPPFLAGS = -I${top_srcdir}/src/libs/misc $(GSL_CFLAGS)
libclassif_la_LIBADD = ../misc/libmisc.la $(GSL_LIBS) -lm

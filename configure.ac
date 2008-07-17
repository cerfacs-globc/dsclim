# Copyright (C) 2008 Christian Page <christian.page@cerfacs.fr>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# initial information about the project
AC_INIT([dsclim],
        [0.1],
        [Christian Page christian.page@cerfacs.fr],
        [dsclim])

AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)

# check if the source folder is correct
AC_CONFIG_SRCDIR(src/dsclim.c)

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h math.h])
AC_TYPE_SIGNAL

#DX_HTML_FEATURE(ON)
#DX_CHM_FEATURE(OFF)
#DX_CHI_FEATURE(OFF)
#DX_MAN_FEATURE(OFF)
#DX_RTF_FEATURE(OFF)
#DX_XML_FEATURE(OFF)
#DX_PDF_FEATURE(OFF)
#DX_PS_FEATURE(OFF)
#DX_INIT_DOXYGEN(dsclim, doxygen.cfg)

AM_INIT_AUTOMAKE([dist-bzip2])

#AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_DISABLE_SHARED

AC_CONFIG_FILES([
   Makefile
   README
   doc/Makefile
   m4/Makefile 
   src/lib/Makefile
   src/lib/utils/Makefile
   src/lib/filter/Makefile
   src/lib/classif/Makefile
   src/lib/clim/Makefile
   src/Makefile
   tests/Makefile
])

AC_OUTPUT

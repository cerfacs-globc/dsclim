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

# get the flags
CFLAGS="${CFLAGS=}"
# this macro is used to get the arguments supplied
# to the configure script (./configure --enable-debug)
# Check if we have enable debug support.
AC_MSG_CHECKING(whether to enable debugging)
debug_default="yes"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging
                       [default=$debug_default]],, enable_debug=$debug_default)
# Yes, shell scripts can be used
if test "x$enable_debug" = "xyes"; then
         CFLAGS="$CFLAGS -g -DDEBUG"
AC_MSG_RESULT(yes)
else
CFLAGS="$CFLAGS -O3"
AC_MSG_RESULT(no)
fi

# Example to add --with option
#AC_ARG_WITH([readline],
#            [AS_HELP_STRING([--with-readline],
#            [support fancy command line editing @<:@default=check@:>@])],
#            [],
#            [with_readline=check])
#          
#          LIBREADLINE=
#          AS_IF([test "x$with_readline" != xno],
#            [AC_CHECK_LIB([readline], [main],
#              [AC_SUBST([LIBREADLINE], ["-lreadline -lncurses"])
#               AC_DEFINE([HAVE_LIBREADLINE], [1],
#                         [Define if you have libreadline])
#              ],
#              [if test "x$with_readline" != xcheck; then
#                 AC_MSG_FAILURE(
#                   [--with-readline was given, but test for readline failed])
#
#              ], -lncurses)])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h math.h libgen.h string.h signal.h sys/types.h stdio.h time.h fcntl.h unistd.h sys/stat.h])
AC_TYPE_SIGNAL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

AX_PATH_GSL(1.0, [], [])

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

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
        [1.5.12],
        [Christian Page christian.page@cerfacs.fr],
        [dsclim])

AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)

# check if the source folder is correct
AC_CONFIG_SRCDIR(src/dsclim.c)

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

#
AC_CONFIG_MACRO_DIR([m4])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h math.h libgen.h string.h signal.h sys/types.h stdio.h time.h fcntl.h unistd.h sys/stat.h sys/mman.h errno.h])
AC_TYPE_SIGNAL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Get the flags
CFLAGS="${CFLAGS=}"
CPPFLAGS="${CPPFLAGS=}"
LDFLAGS="${LDFLAGS=}"

#
# Utility stuff
#
# Did they want debugging?
AC_MSG_CHECKING(for debugging level)
AC_ARG_WITH(debugging, [  --with-debugging[=num]  enable debugging output, num is an integer 0-9], [
if test "$withval" = "yes"; then
  withval=4
else
:
fi
if test "$withval" != "no"; then 
  echo "$withval"
  AC_DEFINE_UNQUOTED(DEBUG, $withval, [Debugging level to compile in.])
  CFLAGS="$CFLAGS -g -Wall -Werror"
else
  echo "no debugging"
  AC_DEFINE_UNQUOTED(DEBUG, 0, [Debugging level to compile in.])
  CFLAGS="$CFLAGS -O3 -Wall -Werror"
fi], [echo "0"
AC_DEFINE_UNQUOTED(DEBUG, 0, [Debugging level to compile in.])
])

# Check for math functions
AC_MSG_CHECKING(for libm)
AC_MSG_RESULT($libm_dir)
AC_CHECK_HEADER(math.h,passed=1,passed=0)
AC_CHECK_LIB(m,ceil,passed=1,passed=0)
AC_MSG_CHECKING(if libm library is complete)
if test $passed -gt 0
then
  AC_DEFINE(HAVE_LIBM,1,Define if you have LIBM library)
  AC_MSG_RESULT(yes)
fi

# Check udunits library
AC_ARG_WITH([udunits],
        [AC_HELP_STRING([--with-udunits=UDUNITS],
        [use specified udunits installation directory])],
        udunits_dir=$withval, udunits_dir='system')

if test $udunits_dir != 'system'
then
  CPPFLAGS="$CPPFLAGS -I${udunits_dir}/include"
  LDFLAGS="$LDFLAGS -L${udunits_dir}/lib"
fi

#AC_MSG_CHECKING(for udunits 2.x)
#AC_MSG_RESULT($udunits_dir)
#AC_CHECK_HEADER(udunits2.h,passed=1,passed=0)
#AC_CHECK_LIB(udunits2,utInit,passed=1,passed=0,-lm)
#AC_MSG_CHECKING(if udunits 2.x support package is complete)

#if test $passed -gt 0
#then
#  UDUNITS_LIBS='-ludunits2 -lm'
#  AC_SUBST(UDUNITS_LIBS)
#  UDUNITS_CPPFLAGS='-I${udunits_dir}/include'
#  AC_SUBST(UDUNITS_CPPFLAGS)
#  UDUNITS_LDFLAGS='-L${udunits_dir}/lib'
#  AC_SUBST(UDUNITS_CPPFLAGS)
#  AC_DEFINE(HAVE_UDUNITS2,1,Define if you have UDUNITS 2.x library)
#  AC_MSG_RESULT(yes)
#else
#  AC_MSG_RESULT(no)
  AC_MSG_CHECKING(for udunits 1.x)
  AC_MSG_RESULT($udunits_dir)
  AC_CHECK_HEADER(udunits.h,passed=1,passed=0)
  AC_CHECK_LIB(udunits,utInit,passed=1,passed=0,-lm)
  AC_MSG_CHECKING(if udunits 1.x support package is complete)

  if test $passed -gt 0
  then
    UDUNITS_LIBS='-ludunits -lm'
    AC_SUBST(UDUNITS_LIBS)
    UDUNITS_CPPFLAGS='-I${udunits_dir}/include'
    AC_SUBST(UDUNITS_CPPFLAGS)
    UDUNITS_LDFLAGS='-L${udunits_dir}/lib'
    AC_SUBST(UDUNITS_CPPFLAGS)
    AC_DEFINE(HAVE_UDUNITS,1,Define if you have UDUNITS 1.x library)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_NOTICE([
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  CONFIGURATION ERROR: UDUNITS 1.x or 2.x required.
  UDUNITS installation is not working or missing.

  To fix this problem you have the following options.
  1) Check if you have UDUNITS installed in a path different from your system path.
     In this case you can provide your UDUNITS installation path to the configure
     through the option --with-udunits="your_udunits_path"
  2) With your package manager, install the UDUNITS library.
  3) Source code for UDUNITS can be found at http://www.unidata.ucar.edu/udunits/.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        ])
    exit
  fi
#fi

# Check zlib library
AC_ARG_WITH([zlib],
        [AC_HELP_STRING([--with-zlib=ZLIB],
        [use specified zlib installation directory])],
        zlib_dir=$withval, zlib_dir='system')

if test $zlib_dir != 'system'
then
  CPPFLAGS="$CPPFLAGS -I${zlib_dir}/include"
  LDFLAGS="$LDFLAGS -L${zlib_dir}/lib"
fi

AC_MSG_CHECKING(for zlib)
AC_MSG_RESULT($zlib_dir)
AC_CHECK_HEADER(zlib.h,passed=1,passed=0)
AC_CHECK_LIB(z,deflate,passed=1,passed=0)
AC_MSG_CHECKING(if zlib support package is complete)

if test $passed -gt 0
then
  AC_DEFINE(HAVE_ZLIB,1,Define if you have ZLIB library)
  AC_MSG_RESULT(yes)
else
  AC_MSG_NOTICE([
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  CONFIGURATION ERROR: ZLIB required.
  ZLIB installation is not working or missing.

  To fix this problem you have the following options.
  1) Check if you have ZLIB installed in a path different from your system path.
     In this case you can provide your ZLIB installation path to the configure
     through the option --with-zlib="your_zlib_path"
  2) With your package manager, install the ZLIB library.
  3) Source code for ZLIB can be found at http://www.zlib.net/.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      ])
  exit
fi

# Check szlib library
AC_ARG_WITH([szlib],
        [AC_HELP_STRING([--with-szlib=SZLIB],
        [use specified szlib installation directory])],
        szlib_dir=$withval, szlib_dir='system')

if test $szlib_dir != 'system'
then
  CPPFLAGS="$CPPFLAGS -I${szlib_dir}/include"
  LDFLAGS="$LDFLAGS -L${szlib_dir}/lib"
fi

AC_MSG_CHECKING(for szlib)
AC_MSG_RESULT()
AC_CHECK_HEADER(szlib.h,passed=1,passed=0)
AC_CHECK_LIB(sz,SZ_Decompress,passed=1,passed=0)
AC_MSG_CHECKING(if szlib support package is complete)

if test $passed -gt 0
then
  AC_DEFINE(HAVE_SZLIB,1,Define if you have SZLIB library)
  AC_MSG_RESULT(yes)
else
  AC_MSG_NOTICE([
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  CONFIGURATION ERROR: SZLIB required.
  SZLIB installation is not working or missing.

  The HDF5 Library includes a predefined compression filter that 
  uses the extended-Rice lossless compression algorithm for chunked 
  datasets. For more information about Szip compression and license 
  terms, see http://hdfgroup.org/doc_resource/SZIP/.

  To fix this problem you have the following options.
  1) Check if you have SZLIB installed in a path different from your system path.
     In this case you can provide your SZLIB installation path to the configure
     through the option --with-szlib="your_szlib_path"
  2) Precompiled Szip binaries for each supported platform and a source 
     tar file can be found at ftp://ftp.hdfgroup.org/lib-external/szip/.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      ])
  exit
fi

# Check HDF5 library
AC_ARG_WITH([hdf5],
        [AC_HELP_STRING([--with-hdf5=HDF5],
        [use specified hdf5 installation directory])],
        hdf5_dir=$withval, hdf5_dir='system')

if test $hdf5_dir != 'system'
then
  CPPFLAGS="$CPPFLAGS -I${hdf5_dir}/include"
  LDFLAGS="$LDFLAGS -L${hdf5_dir}/lib"
fi

AC_MSG_CHECKING(for hdf5)
AC_MSG_RESULT()
AC_CHECK_HEADER(hdf5.h,passed=1,passed=0)
AC_CHECK_LIB(hdf5,H5check_version,passed=1,passed=0,-lhdf5_hl -lsz -lz -lm)
AC_MSG_CHECKING(if hdf5 support package is complete)

if test $passed -gt 0
then
  AC_DEFINE(HAVE_HDF5LIB,1,Define if you have HDF5 library)
  AC_MSG_RESULT(yes)
else
  AC_MSG_NOTICE([
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  CONFIGURATION ERROR: HDF5 is required.
  Hdf5 installation is not working or missing.

  To fix this problem you have the following options.
  1) Check if you have HDF5 installed in a path different from your system path.
     In this case you can provide your HDF5 installation path to the configure
     through the option --with-hdf5="your_hdf5_path"
  2) Download HDF5 from ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4/ and install it.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      ])
  exit
fi

# Check NetCDF library
AC_ARG_WITH([netcdf],
        [AC_HELP_STRING([--with-netcdf=NETCDF],
        [use specified netcdf installation directory])],
        netcdf_dir=$withval, netcdf_dir='system')

if test $netcdf_dir != 'system'
then
  CPPFLAGS=" -I${netcdf_dir}/include $CPPFLAGS"
  LDFLAGS=" -L${netcdf_dir}/lib $LDFLAGS"
fi

AC_MSG_CHECKING(for netcdf)
AC_MSG_RESULT()
AC_CHECK_HEADER(netcdf.h,passed=1,passed=0)
AC_CHECK_LIB(netcdf,nc_create,passed=1,passed=0,-lhdf5_hl -lhdf5 -lsz -lz -lm)
AC_MSG_CHECKING(if netcdf support package is complete)

if test $passed -gt 0
then
  NCDF_LIBS='-lnetcdf -lhdf5_hl -lhdf5 -lsz -lz -lm'
  AC_SUBST(NCDF_LIBS)
  NCDF_CPPFLAGS='-I${netcdf_dir}/include'
  AC_SUBST(NCDF_CPPFLAGS)
  NCDF_LDFLAGS='-L${netcdf_dir}/lib'
  AC_SUBST(NCDF_CPPFLAGS)
  AC_DEFINE(HAVE_NETCDFLIB,1,Define if you have NETCDF library)
  AC_MSG_RESULT(yes)
else
  AC_MSG_NOTICE([
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  CONFIGURATION ERROR: NetCDF 4.0 or greater required.
  NetCDF installation is not working or missing.

  To fix this problem you have the following options.
  1) Check if you have netcdf installed in a path different from your system path.
     In this case you can provide your netcdf installation path to the configure
     through the option --with-netcdf="your_netcdf_path"
  2) Download netcdf from http://www.unidata.ucar.edu/software/netcdf/ and install it.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      ])
  exit
fi

AX_PATH_GSL(1.0, [], [])

# Checl libxml2 library
AM_PATH_XML2([], [], [AC_MSG_ERROR([*** cannot find the libxml2 library ])])
AC_SUBST(XML_CPPFLAGS)
AC_SUBST(XML_LIBS)

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

#AC_ARG_ENABLE([doxygen],
#  [AS_HELP_STRING([--disable-doxygen],
#    [disable doc generation])])
#AM_CONDITIONAL([ENABLE_DOXYGEN], [test "x$enable_doxygen" != xno]) 

# Checks for optional programs.
AC_PROG_TRY_DOXYGEN

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(dsclim, doxygen.cfg, doc)

AM_INIT_AUTOMAKE([dist-bzip2])

AC_PROG_LIBTOOL
AC_DISABLE_SHARED

AC_CONFIG_FILES([
   Makefile
   README
   m4/Makefile 
   src/libs/Makefile
   src/libs/misc/Makefile
   src/libs/utils/Makefile
   src/libs/filter/Makefile
   src/libs/classif/Makefile
   src/libs/clim/Makefile
   src/libs/regress/Makefile
   src/libs/xml_utils/Makefile
   src/libs/io/Makefile
   src/libs/pceof/Makefile
   src/Makefile
   tests/Makefile
])

AC_OUTPUT

echo "
  ($PACKAGE_NAME) version $PACKAGE_VERSION
  Prefix.........: $prefix
  Debug Build....: $debug
  C Compiler.....: $CC $CFLAGS $CPPFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
"

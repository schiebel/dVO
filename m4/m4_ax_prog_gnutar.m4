
#
# SYNOPSIS
#
#   AX_PROG_GNUTAR(PROG,path)
#
# DESCRIPTION
#
#   look for GNU tar in path (optional) and assign the fully qualifed path to PROG
#
# LICENSE
#
#   Copyright (C) 2014 Associated Universities, Inc. Washington DC, USA.
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AX_PROG_GNUTAR], [dnl
  m4_if( [$2], [],dnl
         AC_CACHE_CHECK([for gnu tar], [ac_cv_path_GNUTAR],
                        [AC_PATH_PROGS_FEATURE_CHECK( [GNUTAR], [gnutar gtar tar],
                                                      [[gtarout=`$ac_path_GNUTAR --version | head -1 | sed -e 's|.*\(GNU tar\).*|\1|'`
                                                        test "$gtarout" = 'GNU tar' && ac_cv_path_GNUTAR=$ac_path_GNUTAR ac_path_GNUTAR_found=:]],
                                                      [AC_MSG_ERROR([could not find gnu tar])])]),dnl
         AC_CACHE_CHECK([for gnu tar], [ac_cv_path_GNUTAR],dnl
                        [AC_PATH_PROGS_FEATURE_CHECK( [GNUTAR], [gnutar gtar tar],
                                                      [[gtarout=`$ac_path_GNUTAR --version | head -1 | sed -e 's|.*\(GNU tar\).*|\1|'`
                                                        test "$gtarout" = 'GNU tar' && ac_cv_path_GNUTAR=$ac_path_GNUTAR ac_path_GNUTAR_found=:]],
                                                      [AC_MSG_ERROR([could not find gnu tar])],$2)]))
  $1="$ac_cv_path_GNUTAR"
])

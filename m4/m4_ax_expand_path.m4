#
# SYNOPSIS
#
#   AX_EXPAND_PATH(path,var)
#
# DESCRIPTION
#
#   expand the path to a fully qualified path and assign it to var
#
# LICENSE
#
#   Copyright (C) 2014 Associated Universities, Inc. Washington DC, USA.
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AX_EXPAND_PATH], [dnl
  AC_PREREQ([2.50])dnl
  m4_if([$1], [], [m4_fatal([first argument of AX_EXPAND_PATH missing])])dnl
  m4_if([$2], [],[m4_fatal([second argument of AX_EXPAND_PATH missing])])dnl
  ac_success=no
  if test -d $1; then
    $2=`cd $1 && pwd`
    ac_success=yes
  else
    AC_MSG_ERROR([expanded path ($1) is not a directory])
  fi
])

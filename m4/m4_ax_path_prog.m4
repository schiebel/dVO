#
# SYNOPSIS
#
#   AX_EXPAND_PATH(PROG,prog,path)
#
# DESCRIPTION
#
#   look for prog in path and assign the fully qualifed path to PROG
#
# LICENSE
#
#   Copyright (C) 2014 Associated Universities, Inc. Washington DC, USA.
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AX_PATH_PROG], [dnl
  m4_if( [$3], [], [AC_PATH_PROG([$1], [$2], [])], [AC_PATH_PROG([$1], [$2], [], [$3])])dnl
  if test -z "${$1}"; then
    AC_MSG_ERROR([unable to find $2])
  fi
])

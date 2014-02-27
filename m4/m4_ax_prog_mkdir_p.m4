# Copyright (C) 1992-1996, 1998-2012 Free Software Foundation, Inc.
# Copyright (C) 2014 Associated Universities, Inc. Washington DC, USA.
#
# This file was *originally* part of Autoconf.  This program is
# free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Under Section 7 of GPL version 3, you are granted additional
# permissions described in the Autoconf Configure Script Exception,
# version 3.0, as published by the Free Software Foundation.
#
# You should have received a copy of the GNU General Public License
# and a copy of the Autoconf Configure Script Exception along with
# this program; see the files COPYINGv3 and COPYING.EXCEPTION
# respectively.  If not, see <http://www.gnu.org/licenses/>.

# This macro is a modified version of the AC_PROG_MKDIR_P standard
# autoconf macro. It was modified not to require the existence of
# the old install.sh, etc. scripts.

# Originial written by David MacKenzie, with help from
# Franc,ois Pinard, Karl Berry, Richard Pixley, Ian Lance Taylor,
# Roland McGrath, Noah Friedman, david d zuhn, and many others
# modified by Darrell Schiebel to no longer require install.sh
# et al.

# AX_PROG_MKDIR_P
# ---------------
# Check whether `mkdir -p' is known to be thread-safe, and fall back to
# install-sh -d otherwise.
#
# Automake 1.8 used `mkdir -m 0755 -p --' to ensure that directories
# created by `make install' are always world readable, even if the
# installer happens to have an overly restrictive umask (e.g. 077).
# This was a mistake.  There are at least two reasons why we must not
# use `-m 0755':
#   - it causes special bits like SGID to be ignored,
#   - it may be too restrictive (some setups expect 775 directories).
#
# Do not use -m 0755 and let people choose whatever they expect by
# setting umask.
#
# We cannot accept any implementation of `mkdir' that recognizes `-p'.
# Some implementations (such as Solaris 8's) are vulnerable to race conditions:
# if a parallel make tries to run `mkdir -p a/b' and `mkdir -p a/c'
# concurrently, both version can detect that a/ is missing, but only
# one can create it and the other will error out.  Consequently we
# restrict ourselves to known race-free implementations.
#
# Automake used to define mkdir_p as `mkdir -p .', in order to
# allow $(mkdir_p) to be used without argument.  As in
#   $(mkdir_p) $(somedir)
# where $(somedir) is conditionally defined.  However we don't do
# that for MKDIR_P.
#  1. before we restricted the check to GNU mkdir, `mkdir -p .' was
#     reported to fail in read-only directories.  The system where this
#     happened has been forgotten.
#  2. in practice we call $(MKDIR_P) on directories such as
#       $(MKDIR_P) "$(DESTDIR)$(somedir)"
#     and we don't want to create $(DESTDIR) if $(somedir) is empty.
#     To support the latter case, we have to write
#       test -z "$(somedir)" || $(MKDIR_P) "$(DESTDIR)$(somedir)"
#     so $(MKDIR_P) always has an argument.
#     We will have better chances of detecting a missing test if
#     $(MKDIR_P) complains about missing arguments.
#  3. $(MKDIR_P) is named after `mkdir -p' and we don't expect this
#     to accept no argument.
#  4. having something like `mkdir .' in the output is unsightly.
#
# On NextStep and OpenStep, the `mkdir' command does not
# recognize any option.  It will interpret all options as
# directories to create.
AC_DEFUN([AX_PROG_MKDIR_P],
[AC_MSG_CHECKING([for a thread-safe mkdir -p])
m4_if( [$1], [],dnl
       [m4_fatal([invalid first argument `$1' to AX_PROG_MKDIR_P])] )
if test -z "${$1}"; then
  AC_CACHE_VAL([ax_cv_path_mkdir],
    [_AS_PATH_WALK([$PATH$PATH_SEPARATOR/bin$PATH_SEPARATOR/opt/sfw/bin],
      [for ac_prog in mkdir gmkdir; do
	 for ac_exec_ext in '' $ac_executable_extensions; do
	   AS_EXECUTABLE_P(["$as_dir/$ac_prog$ac_exec_ext"]) || continue
	   case `"$as_dir/$ac_prog$ac_exec_ext" --version 2>&1` in #(
	     'mkdir (GNU coreutils) '* | \
	     'mkdir (coreutils) '* | \
	     'mkdir (fileutils) '4.1*)
	       ax_cv_path_mkdir=$as_dir/$ac_prog$ac_exec_ext
	       break 3;;
	   esac
	 done
       done])])
  test -d ./--version && rmdir ./--version
  if test "${ax_cv_path_mkdir+set}" = set; then
    $1="$ax_cv_path_mkdir -p"
  else
    AC_MSG_ERROR([could not find safe mkdir executable])
  fi
fi
dnl status.m4 does special magic for MKDIR_P instead of AC_SUBST,
dnl to get relative names right.  However, also AC_SUBST here so
dnl that Automake versions before 1.10 will pick it up (they do not
dnl trace AC_SUBST_TRACE).
dnl FIXME: Remove this once we drop support for Automake < 1.10.
dnl AC_SUBST([MKDIR_P])dnl
AC_MSG_RESULT([${$1}])
])

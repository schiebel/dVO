#
# LICENSE
#
#   Copyright (C) 2014 Associated Universities, Inc. Washington DC, USA.
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

### library directories to check: "" /lib64 /usr/lib64 /opt/local/lib /usr/local/lib /lib /usr/lib
### include directories to check: "" /usr/include/dbus-1.0 /opt/local/include/dbus-1.0
AC_DEFUN([AX_LIB_DBUS], [

    AC_ARG_WITH( [dbus-libdir],
                 AS_HELP_STRING([--with-dbus-libdir=DIR],[specify directory for DBus libraries]),
                 [ if test -d "$withval"; then
                       dbus_libdir="$withval"
                   else
                       AC_MSG_ERROR([--with-dbus-libdir expected directory name])
                   fi ])

    AC_CACHE_VAL( ax_cv_lib_version_dbus, [
        AC_CACHE_VAL( ax_cv_lib_include_path_dbus, [
            AC_CACHE_CHECK( [for DBus library path], ax_cv_lib_path_dbus, [
                orig_LDFLAGS="${LDFLAGS}"
                orig_LIBS="${LIBS}"
                for version in 1; do
                    LIBS="${orig_LIBS} -ldbus-${version}"
                    for dir in $dbus_libdir /lib64 /usr/lib64 /lib /usr/lib /opt/local/lib64 /usr/local/lib64 /opt/local/lib /usr/local/lib ; do
                        if test -d "${dir}"; then
                            if test -z "${dir}"; then
                                LDFLAGS="${orig_LDFLAGS}"
                            else
                                LDFLAGS="${orig_LDFLAGS} -L${dir}"
                            fi
                            AC_TRY_LINK( [ #include <stdio.h>
                                           extern "C" void dbus_get_version(int*,int*,int*);],
                                         [ int major,minor,micro;
                                           dbus_get_version(&major,&minor,&micro);],
                                         [ax_cv_lib_path_dbus="${dir}"],
                                         [ax_cv_lib_path_dbus="no"] )
                            if test "${ax_cv_lib_path_dbus}" != "no" -a \
                                    -d "${ax_cv_lib_path_dbus}/dbus-${version}.0/include" ; then
							    if test "${ax_cv_static_link}" = yes; then
									if test -e ${ax_cv_lib_path_dbus}/libdbus-${version}.a ; then
										ax_cv_lib_version_dbus="${version}"
										ax_cv_lib_include_path_dbus="${ax_cv_lib_path_dbus}/dbus-${version}.0/include"
										break
									else 
										AC_MSG_NOTICE([skipping ${ax_cv_lib_path_dbus}, no static library])
									fi
								else
									ax_cv_lib_version_dbus="${version}"
									ax_cv_lib_include_path_dbus="${ax_cv_lib_path_dbus}/dbus-${version}.0/include"
									break
								fi
                            fi
                        fi
                    done
                done
                LDFLAGS="${orig_LDFLAGS}"
                LIBS="${orig_LIBS}"])
        ])
    ])

    AC_ARG_WITH( [dbus-incdir],
                 AS_HELP_STRING([--with-dbus-include=DIR],[specify directory for DBus include files]),
                 [ if test -d "$withval"; then
                       dbus_incdir="$withval"
                   else
                       AC_MSG_ERROR([--with-dbus-include expected directory name])
                   fi ])

    AC_CACHE_CHECK( [for DBus include path], ax_cv_include_path_dbus, [
        orig_CPPFLAGS="${CPPFLAGS}"
        for dir in $dbus_incdir /usr/include /opt/local/include /usr/local/include ; do
            incdir="${dir}/dbus-${ax_cv_lib_version_dbus}.0"
            if test -d "${incdir}"; then
                CPPFLAGS="${orig_LDFLAGS} -I${ax_cv_lib_include_path_dbus} -I${incdir}"
                AC_TRY_LINK( [#include <dbus/dbus.h>],
                             [ ],
                             [ax_cv_include_path_dbus="${incdir}"],
                             [ax_cv_include_path_dbus="no"] )
                if test "${ax_cv_include_path_dbus}" != "no"; then
                    break
                fi
            fi
        done
        CPPFLAGS="${orig_CPPFLAGS}"
    ])
    if test "${ax_cv_include_path_dbus}" != "no" -a \
            "${ax_cv_lib_include_path_dbus}" != "no" -a \
            "${ax_cv_lib_path_dbus}" != "no"; then
		CPPFLAGS="$CPPFLAGS -I${ax_cv_lib_include_path_dbus} -I${ax_cv_include_path_dbus}"
		if test "${ax_cv_static_link}" = yes; then
			LIBS="${LIBS} ${ax_cv_lib_path_dbus}/libdbus-${version}.a"
		else
			LDFLAGS="${LDFLAGS} -L${ax_cv_lib_path_dbus}"
			LIBS="${LIBS} -ldbus-${ax_cv_lib_version_dbus}"
		fi
    else
        AC_MSG_ERROR([could not identify DBus installation])
    fi
])

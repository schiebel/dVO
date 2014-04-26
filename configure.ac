AC_INIT([DBus VO Service],[0.0.1],[drs@nrao.edu],[dVO],[http://casa.nrao.edu/])
AC_PREREQ([2.59])
AC_CONFIG_MACRO_DIRS([m4])

dnl setup cannonical host variables without requiring
dnl install.sh et al. as AC_CANONICAL_HOST does
AX_CANONICAL_SPLIT(host,`./config.guess`)

AX_EXPAND_PATH($srcdir,root)
AC_MSG_NOTICE([package root is $PKG_ROOT])
AC_MSG_NOTICE([host os is ${host_os}])
MODIFIEDPATH=$root/bin$PATH_SEPARATOR$PATH
AC_MSG_NOTICE([path is $MODIFIEDPATH])

AC_LANG(C++)
AC_PROG_CXX([clang++ c++ g++])
AX_CXX_COMPILE_STDCXX_11
AX_LIB_STDCXX

dnl ax_save_PATH="$PATH"
dnl if test "$CXX"; then
dnl     if test "`dirname $CXX`" != "."; then
dnl         PATH="`dirname $CXX`${PATH_SEPARATOR}${PATH}"
dnl     else
dnl         as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
dnl         for as_dir in $PATH
dnl         do
dnl             IFS=$as_save_IFS
dnl             test -z "$as_dir" && as_dir=.
dnl             if test -x "${as_dir}/${CXX}"; then
dnl                 PATH="${as_dir}${PATH_SEPARATOR}${PATH}"
dnl                 break
dnl             fi
dnl         done
dnl         IFS=$as_save_IFS
dnl     fi
dnl fi
AC_PROG_CC([clang cc gcc])
dnl PATH="$ax_save_PATH"

AX_PATH_PROG([DBUS_XML2CPP],[dbuspp-xml2cpp],[$MODIFIEDPATH])
AX_PROG_GNUTAR([TAR],[$MODIFIEDPATH])
dnl AC_PROG_MKDIR_P
AX_PROG_MKDIR_P([MKDIR])

AX_LIB_DBUS
LIBCURL_CHECK_CONFIG( [yes],[7.15.5],[
                      CPPFLAGS="${CPPFLAGS} ${LIBCURL_CPPFLAGS}"
                      LDFLAGS="${LDFLAGS} ${LIBCURL}"],
                      AC_MSG_ERROR([could not locate libcurl installation]) )
dnl AX_CHECK_LIBXML2( [2.6.26], [
dnl                       CPPFLAGS="${CPPFLAGS} ${LIBXML2_CPPFLAGS}"
dnl                       LDFLAGS="${LDFLAGS} ${LIBXML2_LDFLAGS}"
dnl                       LIBS="${LIBS} ${LIBXML2_LIBS}"],
dnl                       AC_MSG_ERROR([could not locate libxml2 installation]) )
AX_PATH_XML2( [2.6.26], [
              CPPFLAGS="${CPPFLAGS} ${XML_CPPFLAGS}"
              LIBS="${LIBS} ${XML_LIBS}"],
              AC_MSG_ERROR([could not locate libxml2 installation]) )

case $host_os in
	darwin*|macosx*)
		LDFLAGS="${LDFLAGS} -framework IOKit -framework CoreFoundation"
		;;
esac

AC_SUBST(SYS_LIBS)

AC_SUBST(DBUS_XML2CPP)
AC_SUBST(TAR)
AC_SUBST(MKDIR)
AC_CONFIG_HEADERS([include/dvo/config.h])
AC_CONFIG_FILES([mk/rules.mk])
AC_OUTPUT

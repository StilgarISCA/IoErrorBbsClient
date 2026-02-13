dnl Process this file with autoconf to produce a configure script.
AC_INIT([IoErrorBbsClient],[2.3.10-Stilgar])
AC_CONFIG_SRCDIR([filter.c])
AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_ARG_WITH(ssl,
   [  --with-ssl=PATH         use OpenSSL (no BBSs support this yet) ],
   [
   	if test "x$withval" != "xno" ; then
   		trussldir=$withval
   	fi
   ]
)
AC_ARG_ENABLE(titlebar, [  --disable-titlebar      disable xterm/NeXT titlebar (default is enabled)])
AC_ARG_ENABLE(xlog, [  --enable-xlog           enable logging express messages (doesn't work yet)])
AC_ARG_ENABLE(save-password, [  --enable-save-password  allow passwords to be saved])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED(HOSTTYPE,"$host",[Define to the canonical host])

dnl Checks for libraries.
dnl Replace `main' with a function in -linet:
AC_CHECK_LIB(inet, main)
dnl Replace `main' with a function in -lnsl:
AC_CHECK_LIB(nsl, main)
dnl Replace `main' with a function in -lresolv:
AC_CHECK_LIB(resolv, main)
dnl Replace `main' with a function in -lseq:
AC_CHECK_LIB(seq, main)
dnl Replace `main' with a function in -lsocket:
AC_CHECK_LIB(socket, main)
dnl Replace `main' with a function in -lutil:
AC_CHECK_LIB(util, main)

PLATFORM_OBJS="unix.o"

# Some target-specific stuff
case "$host_os" in
netbsd*)
   need_dash_r=1;
   ;;
solaris*)
   need_dash_r=1;
   ;;
cygwin* | mingw*)
   AC_DEFINE(USE_CYGWIN,[1],[Define this if building in the Cygwin environment])
#	PLATFORM_OBJS="${PLATFORM_OBJS} windows.o"
   CFLAGS="$CFLAGS -I/usr/include/w32api"
   ;;
esac

if test x$with_ssl = xyes; then
   # The big search for OpenSSL
   saved_LIBS="$LIBS"
   saved_LDFLAGS="$LDFLAGS"
   saved_CFLAGS="$CFLAGS"
   if test "x$prefix" != "xNONE"; then
   	tryssldir="$tryssldir $prefix"
   fi
   AC_CACHE_CHECK([for OpenSSL], ac_cv_openssldir, [
   	for ssldir in $tryssldir "" /usr /usr/local/openssl /usr/lib/openssl /usr/local/ssl /usr/lib/ssl /usr/local /usr/pkg /opt /opt/openssl ; do
   		CFLAGS="$saved_CFLAGS"
   		LDFLAGS="$saved_LDFLAGS"
   		LIBS="$saved_LIBS -lssl -lcrypto"

   		# Skip directories if they don't exist
   		if test ! -z "$ssldir" -a ! -d "$ssldir" ; then
   			continue;
   		fi
   		if test ! -z "$ssldir" -a "x$ssldir" != "x/usr"; then
   			# Try to use $ssldir/lib if it exists, otherwise
   			# $ssldir
   			if test -d "$ssldir/lib" ; then
   				LDFLAGS="-L$ssldir/lib $saved_LDFLAGS"
   				if test ! -z "$need_dash_r" ; then
   					LDFLAGS="-R$ssldir/lib $LDFLAGS"
   				fi
   			else
   				LDFLAGS="-L$ssldir $saved_LDFLAGS"
   				if test ! -z "$need_dash_r" ; then
   					LDFLAGS="-R$ssldir $LDFLAGS"
   				fi
   			fi
   			# Try to use $ssldir/include if it exists,
   			# otherwise $ssldir
   			if test -d "$ssldir/include" ; then
   				CFLAGS="-I$ssldir/include $saved_CFLAGS"
   			else
   				CFLAGS="-I$ssldir $saved_CFLAGS"
   			fi
   		fi

   		# Basic test to check for compatible version and
   		# correct linking *does not* test for RSA - that
   		# comes later.
   		AC_RUN_IFELSE(
   			[AC_LANG_SOURCE([[
#include <string.h>
#include <openssl/rand.h>
int main(void)
{
   char a[2048];
   memset(a, 0, sizeof(a));
   RAND_add(a, sizeof(a), sizeof(a));
   return(RAND_status() <= 0);
}
   			]])],
   			[
   				found_crypto=1
   				break;
   			], [], []
   		)

   		if test ! -z "$found_crypto" ; then
   			break;
   		fi
   	done

   	if test -z "$ssldir" ; then
   		ssldir="(system)"
   	fi

   	if test ! -z "$found_crypto" ; then
   		ac_cv_openssldir=$ssldir
   	else
   		ac_cv_openssldir="no"
   	fi
   ])
   LIBS="$saved_LIBS"
   LDFLAGS="$saved_LDFLAGS"
   CFLAGS="$saved_CFLAGS"
else
   ac_cv_openssldir=no
fi

if test "x$ac_cv_openssldir" != "xno" ; then
   AC_DEFINE(HAVE_OPENSSL,[1],[Define this if we have OpenSSL])
   LIBS="$saved_LIBS -lssl -lcrypto"
   dnl Need to recover ssldir - test above runs in subshell
   ssldir=$ac_cv_openssldir
   if test ! -z "$ssldir" -a "x$ssldir" != "x/usr" -a "x$ssldir" != "x(system)"; then
   	# Try to use $ssldir/lib if it exists, otherwise
   	# $ssldir
   	if test -d "$ssldir/lib" ; then
   		LDFLAGS="-L$ssldir/lib $saved_LDFLAGS"
   		if test ! -z "$need_dash_r" ; then
   			LDFLAGS="-R$ssldir/lib $LDFLAGS"
   		fi
   	else
   		LDFLAGS="-L$ssldir $saved_LDFLAGS"
   		if test ! -z "$need_dash_r" ; then
   			LDFLAGS="-R$ssldir $LDFLAGS"
   		fi
   	fi
   	# Try to use $ssldir/include if it exists, otherwise
   	# $ssldir
   	if test -d "$ssldir/include" ; then
   		CFLAGS="-I$ssldir/include $saved_CFLAGS"
   	else
   		CFLAGS="-I$ssldir $saved_CFLAGS"
   	fi
   fi
fi


if test x$enable_save_password = xyes; then
   AC_DEFINE(ENABLE_SAVE_PASSWORD,[1],[Define this if you want to enable saving passwords])
fi

if test x$enable_titlebar != xno; then
   AC_DEFINE(ENABLE_TITLEBAR,[1],[Define this if you want to use xterm/NeXT titlebars])
fi

if test x$enable_xlog = xyes; then
   AC_DEFINE(ENABLE_XLOG,[1],[Define this to enable X message logging])
fi

dnl Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(sgtty.h sys/ioctl.h time.h sys/time.h termio.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(select socket)


AC_SUBST(PLATFORM_OBJS)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

if test x$enable_save_password = xyes; then
   cat <<SPT;

  *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING ***

    You have chosen to enable saving your password.  This option provides
    NO SECURITY WHATSOEVER!  Anyone with access to your configuration file
    may be able to recover your saved BBS password.  This includes anyone
    with physical access to your computer, not to mention system crackers.
    ISCABBS policy provides that you will be responsible for any activity
    from your account.  I will leave it up to you as to whether you wish
    to incur this risk.  If you don't, please re-run configure without the
    --enable-save-password option.

SPT

fi

dnl Process this file with autoconf to produce a configure script.
AC_INIT([IoErrorBbsClient],[2.3.10-Stilgar])
AC_CONFIG_SRCDIR([main.c])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CANONICAL_HOST

AC_ARG_WITH(ssl,
   [  --with-ssl=PATH         use OpenSSL (no BBSs support this yet) ],
   [
   	if test "x$withval" != "xno" ; then
   		tryssldir=$withval
   	fi
   ]
)
AC_ARG_ENABLE(save-password, [  --enable-save-password  allow passwords to be saved])
AC_ARG_ENABLE(native-optimizations,
   [  --disable-native-optimizations  disable macOS host-specific tuning flags],
   [],
   [enable_native_optimizations=yes])
AC_ARG_ENABLE(release-build,
   [  --enable-release-build         build an optimized release binary],
   [],
   [enable_release_build=no])
AC_ARG_ENABLE(universal-binary,
   [  --enable-universal-binary       build a universal macOS binary for Apple Silicon and Intel],
   [],
   [enable_universal_binary=no])

dnl Checks for programs.
if test "x$CC" = "x"; then
   case "$host_os" in
   darwin*)
      AC_CHECK_PROGS([DEFAULT_CLANG], [clang], [no])
      if test "x$DEFAULT_CLANG" != "xno"; then
         CC="$DEFAULT_CLANG"
      fi
      ;;
   esac
fi

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_CHECK_TOOL([DSYMUTIL], [dsymutil], [no])
AC_CHECK_TOOL([STRIP], [strip], [no])
AC_DEFINE_UNQUOTED(HOSTTYPE,"$host",[Define to the canonical host])
AC_DEFINE_UNQUOTED([BUILD_VERSION], ["$PACKAGE_VERSION"],
   [Define to the configured release version string])

compilerFamily="Unknown"
isClangCompiler=no
isAppleClangCompiler=no
AC_MSG_CHECKING([whether $CC is Clang])
AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([[
#ifndef __clang__
#error not clang
#endif
   ]], [[]])],
   [
      AC_MSG_RESULT([yes])
      isClangCompiler=yes
      compilerFamily="Clang"
      AC_MSG_CHECKING([whether $CC is Apple Clang])
      AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[
#ifndef __apple_build_version__
#error not apple clang
#endif
         ]], [[]])],
         [
            AC_MSG_RESULT([yes])
            isAppleClangCompiler=yes
            compilerFamily="Apple Clang"
         ],
         [AC_MSG_RESULT([no])]
      )
   ],
   [
      AC_MSG_RESULT([no])
      compilerFamily="GCC-compatible"
   ]
)

OPTIMIZATION_CFLAGS=""
LTO_CFLAGS=""
LTO_LDFLAGS=""
STACK_PROTECTOR_CFLAGS=""
WARNING_CFLAGS=""
SANITIZER_CFLAGS=""
SANITIZER_LDFLAGS=""
UNIVERSAL_CFLAGS=""
UNIVERSAL_LDFLAGS=""

if test "x$ac_env_CFLAGS_set" != "xset"; then
   if test "x$enable_release_build" = "xyes"; then
      CFLAGS="-O2 -g -DNDEBUG -UDEBUG"
   else
      CFLAGS="-g -Og -DDEBUG -UNDEBUG"
   fi
fi

for warningFlag in "-Wall" "-Wextra" "-Wformat=2" "-Wformat-security" "-Werror=format-security";
do
   saved_CFLAGS="$CFLAGS"
   CFLAGS="$CFLAGS $warningFlag"
   AC_MSG_CHECKING([whether $CC accepts $warningFlag])
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
      [
         AC_MSG_RESULT([yes])
         WARNING_CFLAGS="$WARNING_CFLAGS $warningFlag"
      ],
      [AC_MSG_RESULT([no])]
   )
   CFLAGS="$saved_CFLAGS"
done

if test "x$enable_release_build" = "xyes" && test "x$enable_universal_binary" != "xyes"; then
   ltoFlags="-flto"
   saved_CFLAGS="$CFLAGS"
   saved_LDFLAGS="$LDFLAGS"
   CFLAGS="$CFLAGS $ltoFlags"
   LDFLAGS="$LDFLAGS $ltoFlags"
   AC_MSG_CHECKING([whether $CC supports release LTO flags])
   AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
      [
         AC_MSG_RESULT([yes])
         LTO_CFLAGS="$ltoFlags"
         LTO_LDFLAGS="$ltoFlags"
      ],
      [AC_MSG_RESULT([no])]
   )
   CFLAGS="$saved_CFLAGS"
   LDFLAGS="$saved_LDFLAGS"
fi

for stackProtectorFlag in "-fstack-protector-strong" "-fstack-protector";
do
   saved_CFLAGS="$CFLAGS"
   CFLAGS="$CFLAGS $stackProtectorFlag"
   AC_MSG_CHECKING([whether $CC accepts $stackProtectorFlag])
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
      [
         AC_MSG_RESULT([yes])
         STACK_PROTECTOR_CFLAGS="$stackProtectorFlag"
         CFLAGS="$saved_CFLAGS"
         break
      ],
      [AC_MSG_RESULT([no])]
   )
   CFLAGS="$saved_CFLAGS"
done

if test "x$enable_release_build" != "xyes" && test "x$enable_universal_binary" != "xyes"; then
   sanitizerCflags="-fsanitize=address,undefined -fno-omit-frame-pointer"
   sanitizerLdflags="-fsanitize=address,undefined"
   saved_CFLAGS="$CFLAGS"
   saved_LDFLAGS="$LDFLAGS"
   CFLAGS="$CFLAGS $sanitizerCflags"
   LDFLAGS="$LDFLAGS $sanitizerLdflags"
   AC_MSG_CHECKING([whether $CC supports development sanitizer flags])
   AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
      [
         AC_MSG_RESULT([yes])
         SANITIZER_CFLAGS="$sanitizerCflags"
         SANITIZER_LDFLAGS="$sanitizerLdflags"
      ],
      [
         AC_MSG_RESULT([no])
         AC_MSG_WARN([development sanitizer flags are unavailable with this compiler; continuing without them])
      ]
   )
   CFLAGS="$saved_CFLAGS"
   LDFLAGS="$saved_LDFLAGS"
fi

case "$host_os" in
darwin*)
if test "x$enable_universal_binary" = "xyes"; then
   universalFlags="-arch arm64 -arch x86_64"
   saved_CFLAGS="$CFLAGS"
   saved_LDFLAGS="$LDFLAGS"
   CFLAGS="$CFLAGS $universalFlags"
   LDFLAGS="$LDFLAGS $universalFlags"
   AC_MSG_CHECKING([whether $CC supports macOS universal binary flags])
   AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
      [
         AC_MSG_RESULT([yes])
         UNIVERSAL_CFLAGS="$universalFlags"
         UNIVERSAL_LDFLAGS="$universalFlags"
      ],
      [
         AC_MSG_RESULT([no])
         AC_MSG_ERROR([--enable-universal-binary requires compiler support for -arch arm64 -arch x86_64])
      ]
   )
   CFLAGS="$saved_CFLAGS"
   LDFLAGS="$saved_LDFLAGS"
elif test "x$enable_native_optimizations" = "xyes"; then
   if test "x$ac_env_CFLAGS_set" != "xset"; then
      case "$host_cpu" in
      arm64|aarch64)
         for optimizationFlag in "-mcpu=native" "-mcpu=apple-m1"; do
            saved_CFLAGS="$CFLAGS"
            CFLAGS="$CFLAGS $optimizationFlag"
            AC_MSG_CHECKING([whether $CC accepts $optimizationFlag])
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
               [
                  AC_MSG_RESULT([yes])
                  OPTIMIZATION_CFLAGS="$OPTIMIZATION_CFLAGS $optimizationFlag"
                  CFLAGS="$saved_CFLAGS"
                  break
               ],
               [AC_MSG_RESULT([no])]
            )
            CFLAGS="$saved_CFLAGS"
         done
         ;;
      x86_64|amd64)
         for optimizationFlag in "-march=native" "-mtune=native"; do
            saved_CFLAGS="$CFLAGS"
            CFLAGS="$CFLAGS $optimizationFlag"
            AC_MSG_CHECKING([whether $CC accepts $optimizationFlag])
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
               [
                  AC_MSG_RESULT([yes])
                  OPTIMIZATION_CFLAGS="$OPTIMIZATION_CFLAGS $optimizationFlag"
               ],
               [AC_MSG_RESULT([no])]
            )
            CFLAGS="$saved_CFLAGS"
         done
         ;;
      esac
   fi
fi
;;
esac

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

#Some target - specific stuff
case "$host_os" in
netbsd*)
   need_dash_r=1;
   ;;
solaris*)
   need_dash_r=1;
   ;;
esac

if test x$with_ssl = xyes; then
#The big search for OpenSSL
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

#Skip directories if they don't exist
   		if test ! -z "$ssldir" -a ! -d "$ssldir" ; then
   			continue;
   		fi
   		if test ! -z "$ssldir" -a "x$ssldir" != "x/usr"; then
#Try to use $ssldir / lib if it exists, otherwise
#$ssldir
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
#Try to use $ssldir / include if it exists,
#otherwise $ssldir
   			if test -d "$ssldir/include" ; then
   				CFLAGS="-I$ssldir/include $saved_CFLAGS"
   			else
   				CFLAGS="-I$ssldir $saved_CFLAGS"
   			fi
   		fi

#Basic test to check for compatible version and
#correct linking *does not *test for RSA - that
#comes later.
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
#Try to use $ssldir / lib if it exists, otherwise
#$ssldir
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
#Try to use $ssldir / include if it exists, otherwise
#$ssldir
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

dnl Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(sgtty.h sys/ioctl.h time.h sys/time.h termio.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(select socket)

AC_PATH_PROG([PKG_CONFIG], [pkg-config], [no])
CMOCKA_CFLAGS=""
CMOCKA_LIBS=""
have_cmocka=no
if test "x$PKG_CONFIG" != "xno"; then
   if $PKG_CONFIG --exists cmocka; then
      have_cmocka=yes
      CMOCKA_CFLAGS=`$PKG_CONFIG --cflags cmocka`
      CMOCKA_LIBS=`$PKG_CONFIG --libs cmocka`
   fi
fi
AM_CONDITIONAL([HAVE_CMOCKA], [test "x$have_cmocka" = "xyes"])
if test "x$have_cmocka" = "xyes"; then
   AC_DEFINE([HAVE_CMOCKA], [1], [Define if cmocka is available for unit tests])
fi
AC_SUBST([CMOCKA_CFLAGS])
AC_SUBST([CMOCKA_LIBS])
AC_SUBST([DSYMUTIL])
AC_SUBST([LTO_CFLAGS])
AC_SUBST([LTO_LDFLAGS])
AC_SUBST([OPTIMIZATION_CFLAGS])
AC_SUBST([RELEASE_BUILD_ENABLED], [$enable_release_build])
AC_SUBST([STACK_PROTECTOR_CFLAGS])
AC_SUBST([WARNING_CFLAGS])
AC_SUBST([SANITIZER_CFLAGS])
AC_SUBST([SANITIZER_LDFLAGS])
AC_SUBST([STRIP])
AC_SUBST([UNIVERSAL_CFLAGS])
AC_SUBST([UNIVERSAL_LDFLAGS])
AC_SUBST([HOST_TRIPLE], [$host])

if test "x$enable_release_build" = "xyes"; then
   buildMode="release"
   optimizationMode="release"
else
   buildMode="dev"
   optimizationMode="development"
fi

if test "x$LTO_CFLAGS" != "x"; then
   optimizationMode="$optimizationMode + lto"
fi

if test "x$SANITIZER_CFLAGS" != "x"; then
   sanitizerMode="enabled"
else
   sanitizerMode="disabled"
fi

if test "x$OPTIMIZATION_CFLAGS" != "x"; then
   nativeOptimizationMode="enabled"
else
   nativeOptimizationMode="disabled"
fi

if test "x$enable_universal_binary" = "xyes"; then
   universalMode="enabled"
else
   universalMode="disabled"
fi

if test "x$STACK_PROTECTOR_CFLAGS" != "x"; then
   stackProtectorMode="$STACK_PROTECTOR_CFLAGS"
else
   stackProtectorMode="disabled"
fi

if test "x$ac_cv_openssldir" != "xno"; then
   sslMode="enabled"
else
   sslMode="disabled"
fi

if test "x$enable_save_password" = "xyes"; then
   savePasswordMode="enabled"
else
   savePasswordMode="disabled"
fi

if test "x$have_cmocka" = "xyes"; then
   cmockaMode="found"
else
   cmockaMode="not found"
fi

compilerSummary="$CC ($compilerFamily)"

AC_DEFINE_UNQUOTED([BUILD_COMPILER], ["$compilerSummary"],
   [Define to the selected compiler summary])
AC_DEFINE_UNQUOTED([BUILD_MODE], ["$buildMode"],
   [Define to the selected build mode])
AC_DEFINE_UNQUOTED([BUILD_OPTIMIZATION_MODE], ["$optimizationMode"],
   [Define to the selected optimization mode])
AC_DEFINE_UNQUOTED([BUILD_UNIVERSAL_MODE], ["$universalMode"],
   [Define to the universal binary mode])
AC_DEFINE_UNQUOTED([BUILD_SSL_MODE], ["$sslMode"],
   [Define to the OpenSSL support mode])
AC_DEFINE_UNQUOTED([BUILD_SAVE_PASSWORD_MODE], ["$savePasswordMode"],
   [Define to the saved password support mode])
AC_DEFINE_UNQUOTED([BUILD_CMOCKA_MODE], ["$cmockaMode"],
   [Define to the cmocka availability mode])
AC_DEFINE_UNQUOTED([BUILD_SANITIZER_MODE], ["$sanitizerMode"],
   [Define to the sanitizer mode])
AC_DEFINE_UNQUOTED([BUILD_NATIVE_OPTIMIZATION_MODE], ["$nativeOptimizationMode"],
   [Define to the native optimization mode])
AC_DEFINE_UNQUOTED([BUILD_STACK_PROTECTOR_MODE], ["$stackProtectorMode"],
   [Define to the stack protector mode])

AC_SUBST(PLATFORM_OBJS)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile bbs.1])
AC_OUTPUT

cat <<CFG

Configuration summary:
  Compiler: $compilerSummary
  Host: $host
  Build mode: $buildMode
  Optimization mode: $optimizationMode
  Universal binary: $universalMode
  OpenSSL support: $sslMode
  Save password support: $savePasswordMode
  CMocka: $cmockaMode
  Sanitizers: $sanitizerMode
  Native optimizations: $nativeOptimizationMode
  Stack protector: $stackProtectorMode

CFG

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

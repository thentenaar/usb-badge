dnl
dnl usb-badge
dnl Copyright (C) 2009-2016 Tim Hentenaar.
dnl
dnl This code is licenced under the Simplified BSD License.
dnl See the LICENSE file for details.
dnl

AC_PREREQ([2.63])
AC_INIT([usb-badge], [1.2], [http://github.com/thentenaar/usb-badge])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign])
LT_INIT
AC_LANG([C])

dnl Check for cc, libtool, and headers
AC_PROG_CC
AC_PROG_LIBTOOL
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h])

dnl Check compiler characteristics
AC_C_CONST
AC_TYPE_SIZE_T

dnl Check for hidapi
BUILD_LIBHIDAPI=no
HIDAPI_OS=unknown
HIDAPI_TARGET=
HIDAPI_SUBDIR=

AC_CANONICAL_HOST
AS_CASE([$host_os],
	[*linux*],[
		HIDAPI_OS=linux
		AC_CHECK_HEADERS([hidapi/hidapi.h])
		AS_IF([test "$HAVE_HIDAPI_HIDAPI_H" == "no" ], [
			BUILD_HIDAPI=yes
		],[
			AC_CHECK_LIB([hidapi-libusb], [hid_enumerate], [
				HIDAPI_TARGET=-libusb
			],[
				AC_CHECK_LIB([hidapi-hidraw], [hid_enumerate], [
					HIDAPI_TARGET=-hidraw
					AC_MSG_WARN([the listen command may not work with hidraw.])
				],[ BUILD_HIDAPI=yes ])
			])
		])

		AS_IF([test "$BUILD_HIDAPI" == "yes" ],[
			AC_CHECK_LIB([usb-1.0], [libusb_init], [
				HIDAPI_TARGET=-libusb
				HIDAPI_OS=libusb
			],[
				HIDAPI_TARGET=-linux
			])
		])

		AM_CONDITIONAL([UDEV], [true])
	],
	[*darwin*],[ HIDAPI_OS=mac     ],
	[*bsd*],   [ HIDAPI_OS=libusb  ],
	[*mingw*], [ HIDAPI_OS=windows ],
	[*cygwin*],[ HIDAPI_OS=windows ],
	[ AC_MSG_WARN([unknown OS: $host_os]) ]
)

dnl libhidapi is simply libhidapi, except on Linux
AS_IF([test "x$HIDAPI_TARGET" != "xlinux" &&
       test "x$HIDAPI_TARGET" != "xlibusb"],[
		AC_CHECK_HEADERS([hidapi/hidapi.h])
		AS_IF([test "$HAVE_HIDAPI_HIDAPI_H" == "no" ],[
			BUILD_HIDAPI=yes
		],[
			AC_CHECK_LIB([hidapi], [hid_enumerate])
			AS_IF([test "$HAVE_LIBHIDAPI" == "no" ],[
				BUILD_HIDAPI=yes
			])
		])
])

AM_CONDITIONAL([BUILD_HIDAPI], [test "$BUILD_HIDAPI" == "yes"])
AS_IF([test "$BUILD_HIDAPI" == "yes"],[
	AC_CONFIG_SUBDIRS([hidapi])
	HIDAPI_SUBDIR=hidapi
])

AC_SUBST([HIDAPI_SUBDIR])
AC_SUBST([HIDAPI_TARGET])
AC_SUBST([HIDAPI_OS])

dnl Check for GTK+-2.x
AC_ARG_ENABLE([gui],
	[AS_HELP_STRING(
		[--enable-gui],
		[enable the GTK+ GUI])],
	[enable_gtk=$enableval],
	[enable_gtk=yes]
)

AS_IF([test "$enable_gtk" != "no"],[
	PKG_CHECK_MODULES([GTK2], [gtk+-2.0])
	AC_SUBST([GTK2_CFLAGS])
	AC_SUBST([GTK2_LIBS])

	AS_IF([test "x$GTK2_LIBS" == "x"],[
		AC_MSG_WARN([GTK+ 2.x is required to built the GUI])
		enable_gtk=no
	])
])
AM_CONDITIONAL([BUILD_GUI], [test "$enable_gtk" == "yes"])

dnl Tighten up CFLAGS
CFLAGS="-O2 -D_XOPEN_SOURCE=500 -ansi"
AX_STRICT_CFLAGS
CFLAGS="$CFLAGS -Werror"

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

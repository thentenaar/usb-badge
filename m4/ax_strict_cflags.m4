dnl
dnl usb-badge - m4 Macros
dnl Copyright (C) 2009-2016 Tim Hentenaar.
dnl
dnl This code is licenced under the Simplified BSD License.
dnl See the LICENSE file for details.
dnl
dnl SYNOPSIS
dnl
dnl AX_STRICT_CFLAGS - Set strict compiler flags
dnl
dnl DESCRIPTION
dnl
dnl This macro checks a number of warning options for GCC and
dnl Clang, and adds supoprted options to CFLAGS. -Werror doesn't
dnl get added here, simply because having -Werror and -Wstrict-prototypes
dnl will cause compilations from autoconf to fail.
dnl

AC_DEFUN([AX_STRICT_CFLAGS],[
	AX_REQUIRE_DEFINED([AX_APPEND_COMPILE_FLAGS])
	AX_REQUIRE_DEFINED([AX_CHECK_COMPILE_FLAG])
	AC_LANG_PUSH([C])

	dnl Clang uses -Q, GCC doesn't support it.
	AX_CHECK_COMPILE_FLAG([-Werror -Qunused-arguments],dnl
	                      [ax_cc_clang="yes"],dnl
	                      [ax_cc_clang="no"])

	AS_IF([test "$ax_cc_clang" == "yes"],[
		AX_APPEND_COMPILE_FLAGS([ dnl
			-pedantic dnl
			-Weverything dnl
			-Wno-padded dnl
			-Wno-switch-enum dnl
			-Wno-missing-variable-declarations dnl
			-Wno-disabled-macro-expansion dnl
			-Wno-missing-noreturn dnl
			-Wno-documentation dnl
			-Wno-format-nonliteral dnl
			-Wno-long-long dnl
			-Wno-format dnl
			-Qunused-arguments dnl
		])
	],[	dnl Assume GCC
		AX_APPEND_COMPILE_FLAGS([ dnl
			-pedantic dnl
			-Wall dnl
			-W dnl
			-Wconversion dnl
			-Wstrict-prototypes dnl
			-Wmissing-prototypes dnl
			-Wmissing-declarations dnl
			-Wnested-externs dnl
			-Wshadow dnl
			-Wcast-align dnl
			-Wwrite-strings dnl
			-Wcomment dnl
			-Wcast-qual dnl
			-Wredundant-decls dnl
			-Wbad-function-cast dnl
		])

		dnl -Wformat-security was included in gcc 3.0.4
		AX_APPEND_COMPILE_FLAGS([-Wformat-security])

		dnl -Wc90-c99-compat was introduced in gcc 5
		AX_APPEND_COMPILE_FLAGS([-Wc90-c99-compat])
	])

	AC_LANG_POP([C])
]) dnl AX_STRICT_CFLAGS

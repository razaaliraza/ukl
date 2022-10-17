# Unikernel Linux
# Copyright (C) 2018-2022 Red Hat Inc., Boston University,
# Ali Raza, Tommy Unger, Eric Munson, Richard W.M. Jones.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

AC_INIT([UKL], 1.0.0)

dnl This must be used very early else you will get
dnl "warning: AC_RUN_IFELSE was called before AC_USE_SYSTEM_EXTENSIONS"
m4_ifdef([AC_USE_SYSTEM_EXTENSIONS],[],
         [m4_define([AC_USE_SYSTEM_EXTENSIONS],[])])
AC_USE_SYSTEM_EXTENSIONS

dnl NB: Do not [quote] this parameter.
AM_INIT_AUTOMAKE(foreign)

dnl Check for basic C environment.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_CPP
AC_SYS_LARGEFILE

AC_C_PROTOTYPES
test "x$U" != "x" && AC_MSG_ERROR([Compiler not ANSI compliant])

AM_PROG_CC_C_O

dnl Misc programs.
AC_PROG_SED

dnl supermin (required)
AC_CHECK_PROG([SUPERMIN],[supermin],[supermin],[no])
test "x$SUPERMIN" = "xno" && AC_MSG_ERROR(['supermin' program is required])

dnl Select the target program.
AC_MSG_CHECKING([which program to compile])
AC_ARG_WITH([program],
    [AS_HELP_STRING([--with-program],
        [set which program to compile @<:@default=hello@:>@])],
        [PROGRAM="$withval"],
        [PROGRAM="hello"])
AC_MSG_RESULT([$PROGRAM])
AC_SUBST([PROGRAM])

dnl Control glibc bypass.
AC_MSG_CHECKING([if we should enable glibc bypass])
AC_ARG_ENABLE([bypass],
    [AS_HELP_STRING([--enable-bypass],
        [enable glibc bypass (UKL_BP) @<:@default=no@:>@])],
        [ENABLE_BYPASS="$enableval"],
        [ENABLE_BYPASS="no"])
AM_CONDITIONAL([ENABLE_BYPASS], [test "x$ENABLE_BYPASS" = "xyes"])
AC_MSG_RESULT([$ENABLE_BYPASS])

dnl Control UKL features.
AC_MSG_CHECKING([if we should use same stack])
AC_ARG_ENABLE([same-stack],
    [AS_HELP_STRING([--enable-same-stack],
        [enable same stack (CONFIG_UKL_SAME_STACK) @<:@default=no@:>@])],
        [ENABLE_SAME_STACK="$enableval"],
        [ENABLE_SAME_STACK="no"])
AM_CONDITIONAL([ENABLE_SAME_STACK], [test "x$ENABLE_SAME_STACK" = "xyes"])
AC_MSG_RESULT([$ENABLE_SAME_STACK])

AC_MSG_CHECKING([if we should use ret instead of iret])
AC_ARG_ENABLE([use-ret],
    [AS_HELP_STRING([--enable-use-ret],
        [use ret instead of iret (CONFIG_UKL_USE_RET) @<:@default=no@:>@])],
        [ENABLE_USE_RET="$enableval"],
        [ENABLE_USE_RET="no"])
AM_CONDITIONAL([ENABLE_USE_RET], [test "x$ENABLE_USE_RET" = "xyes"])
AC_MSG_RESULT([$ENABLE_USE_RET])

AC_MSG_CHECKING([if we should use IST for PF])
AC_ARG_ENABLE([use-ist-pf],
    [AS_HELP_STRING([--enable-use-ist-pf],
        [use IST for PF instead of DF (CONFIG_UKL_USE_IST_PF) @<:@default=no@:>@])],
        [ENABLE_USE_IST_PF="$enableval"],
        [ENABLE_USE_IST_PF="no"])
AM_CONDITIONAL([ENABLE_USE_IST_PF], [test "x$ENABLE_USE_IST_PF" = "xyes"])
AC_MSG_RESULT([$ENABLE_USE_IST_PF])

AC_MSG_CHECKING([if we should create an afterspace])
AC_ARG_ENABLE([afterspace],
    [AS_HELP_STRING([--enable-afterspace],
        [enable afterspace (CONFIG_UKL_CREATE_AFTERSPACE) @<:@default=no@:>@])],
        [ENABLE_AFTERSPACE="$enableval"],
        [ENABLE_AFTERSPACE="no"])
AM_CONDITIONAL([ENABLE_AFTERSPACE], [test "x$ENABLE_AFTERSPACE" = "xyes"])
AC_MSG_RESULT([$ENABLE_AFTERSPACE])

AC_MSG_CHECKING([if we should enable deep shortcuts])
AC_ARG_ENABLE([shortcuts],
    [AS_HELP_STRING([--enable-shortcuts],
	[enable deep kernel shortcuts @<:@default=no@:>@])],
	[ENABLE_SHORTCUT="$enableval"],
	[ENABLE_SHORTCUT="no"])
AM_CONDITIONAL([ENABLE_SHORTCUT], [test "x$ENABLE_SHORTCUT" = "xyes"])
AC_MSG_RESULT([$ENABLE_SHORTCUT])

dnl Produce output files.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile \
                 dnsmasq/Makefile \
                 hello/Makefile \
                 memcached/Makefile \
		 mpc-bench/Makefile \
		 fio/Makefile \
                 redis/Makefile])
AC_OUTPUT

PHP_ARG_WITH(icu4c, for ICU4C support,
[  --with-icu4c             Include ICU4C support])

if test "$PHP_ICU4C" != "no"; then
  dnl Check for ICU4C library
  AC_MSG_CHECKING(for ICU4C library)
  
  dnl Use pkg-config to find ICU4C
  PKG_CHECK_MODULES([ICU], [icu-uc icu-i18n], [
    PHP_EVAL_INCLINE($ICU_CFLAGS)
    PHP_EVAL_LIBLINE($ICU_LIBS, ICU4C_SHARED_LIBADD)
    AC_DEFINE(HAVE_ICU4C, 1, [Have ICU4C support])
    AC_MSG_RESULT(yes)
  ], [
    dnl Fallback to manual search
    AC_MSG_RESULT(no pkg-config, trying manual search)
    
    dnl Search for ICU4C headers
    for i in /usr /usr/local /opt/local; do
      if test -r "$i/include/unicode/ubrk.h"; then
        ICU4C_INCDIR=$i/include
        break
      fi
    done
    
    if test -z "$ICU4C_INCDIR"; then
      AC_MSG_ERROR([Cannot find ICU4C headers. Please install libicu-dev or similar package.])
    fi
    
    dnl Search for ICU4C libraries
    for i in /usr /usr/local /opt/local; do
      if test -r "$i/lib/libicuuc.so" -o -r "$i/lib/libicuuc.dylib" -o -r "$i/lib/libicuuc.a"; then
        ICU4C_LIBDIR=$i/lib
        break
      fi
    done
    
    if test -z "$ICU4C_LIBDIR"; then
      AC_MSG_ERROR([Cannot find ICU4C libraries. Please install libicu-dev or similar package.])
    fi
    
    PHP_ADD_INCLUDE($ICU4C_INCDIR)
    PHP_ADD_LIBRARY_WITH_PATH(icuuc, $ICU4C_LIBDIR, ICU4C_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(icui18n, $ICU4C_LIBDIR, ICU4C_SHARED_LIBADD)
    
    AC_DEFINE(HAVE_ICU4C, 1, [Have ICU4C support])
    AC_MSG_RESULT(yes)
  ])
  
  PHP_SUBST(ICU4C_SHARED_LIBADD)
  PHP_NEW_EXTENSION(icu4c, icu4c.c icu4c_iterator.c, $ext_shared)
fi
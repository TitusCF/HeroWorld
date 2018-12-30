# Check for -fvisibility and the related __attribute__s

AC_DEFUN([CF_CHECK_VISIBILITY],
[
	AC_MSG_CHECKING([whether the C compiler supports -fvisibility=hidden and the visibility __attribute__])
	saved_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -fvisibility=hidden -Werror"
	AC_RUN_IFELSE([AC_LANG_SOURCE([[
__attribute__((visibility("hidden")))
int t1(void)
{
    return 0;
}

__attribute__((visibility("default")))
int t2(void)
{
    return 0;
}

int main(void)
{
    t1();
    t2();
    return 0;
}
		]])],[
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_VISIBILITY], 1, [Define if __attribute__((visibility)) is supported])
			CFLAGS="$saved_CFLAGS -fvisibility=hidden"
		],[
			AC_MSG_RESULT([no])
			CFLAGS="$saved_CFLAGS"
		],[
			AC_MSG_RESULT([skipped because cross compiling])
	])
])

dnl suppfile.m4 -- Generate Valgrind suppressions.
changequote([,])dnl
define([suppress],[dnl
{
    $1: $2
    Memcheck:Leak
    ...
    fun:$1
}])dnl
include(gst.supp)
suppress(_dl_init)
suppress(gst_init_check)

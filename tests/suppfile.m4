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
{
   libasound:
   Memcheck:Leak
   match-leak-kinds: possible
   fun:malloc
   obj:/usr/lib/libasound.so.2.0.0
   ...
}
{
   libasound:
   Memcheck:Leak
   match-leak-kinds: possible
   ...
   fun:calloc
   obj:/usr/lib/libasound.so.2.0.0
   ...
}
{
   libasound:
   Memcheck:Leak
   match-leak-kinds: possible
   ...
   fun:strdup
   obj:/usr/lib/libasound.so.2.0.0
   ...
}
suppress(_dl_init)

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
suppress(jack_client_open)
{
   libasound:
   Memcheck:Leak
   match-leak-kinds: possible
   fun:malloc
   obj:/usr/lib/libasound.so.*
   ...
}
{
   libasound:
   Memcheck:Leak
   match-leak-kinds: possible
   ...
   fun:calloc
   obj:/usr/lib/libasound.so.*
   ...
}
{
   libasound:
   Memcheck:Leak
   match-leak-kinds: possible
   ...
   fun:strdup
   obj:/usr/lib/libasound.so.*
   ...
}

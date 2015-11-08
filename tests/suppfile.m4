dnl suppfile.m4 -- Generate Valgrind suppressions.
changequote([,])dnl
define([suppress],[dnl
{
    $1: $2
    Memcheck:Leak
    ...
    fun:$1
}])dnl

suppress(_dl_init)
suppress(g_array_append_vals)
suppress(g_type_add_interface_static)
suppress(g_type_class_ref)
suppress(g_type_register_fundamental)
suppress(g_type_register_static)
suppress(g_type_register_static_simple)

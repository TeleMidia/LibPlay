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
suppress(g_option_context_parse)
suppress(g_type_create_instance)
suppress(g_type_register_static)
suppress(gst_init)
suppress(gst_init_check)
suppress(gst_update_registry)
suppress(g_type_class_ref)

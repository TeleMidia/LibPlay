#include <gst/gst.h>
#include <string.h>

static GMainLoop *loop;

static void
cb_need_data (GstElement *appsrc,
    guint       unused_size,
    gpointer    user_data)
{
  static gboolean white = FALSE;
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  guint size;
  GstFlowReturn ret;
  const char *text = "HELLO WORLD";

  size = strlen (text) * sizeof (char);

  buffer = gst_buffer_new_allocate (NULL, size, NULL);

  gst_buffer_fill (buffer, 0, text, size);

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

  timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
  gst_buffer_unref (buffer);

  if (ret != GST_FLOW_OK) {
    /* something wrong, stop pushing */
    g_main_loop_quit (loop);
  }
}

gint
main (gint   argc,
    gchar *argv[])
{
  GstElement *pipeline, *appsrc, *render, *capsfilter, *videoconvert, *videosink;

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* setup pipeline */
  pipeline = gst_pipeline_new ("pipeline");
  appsrc = gst_element_factory_make ("appsrc", "source");
  render = gst_element_factory_make ("textrender", "render");
  capsfilter = gst_element_factory_make ("capsfilter", "filter");
  videoconvert = gst_element_factory_make ("videoconvert", "convert");
  videosink = gst_element_factory_make ("xvimagesink", "videosink");

  /* setup */
  g_object_set (G_OBJECT (appsrc), "caps",
      gst_caps_from_string ("text/x-raw, format=(string)utf8"), 
      "format", GST_FORMAT_TIME, NULL);

  g_object_set (G_OBJECT (capsfilter), "caps",
      gst_caps_new_simple ("video/x-raw",
        "width", G_TYPE_INT, 400,
        "height", G_TYPE_INT, 400,
        NULL), NULL);

  g_object_set (G_OBJECT (render), "valignment", 1, NULL);

  gst_bin_add_many (GST_BIN (pipeline), appsrc, render, capsfilter, videoconvert, 
      videosink, NULL);
  g_assert (gst_element_link (appsrc, render));
  g_assert (gst_element_link (render, capsfilter));
  g_assert (gst_element_link (capsfilter, videoconvert));
  g_assert (gst_element_link (videoconvert, videosink));

  /* setup appsrc */
  g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), NULL);

  /* play */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  g_main_loop_run (loop);

  /* clean up */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));
  g_main_loop_unref (loop);

  return 0;
}



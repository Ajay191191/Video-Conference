#include <string.h>
#include <math.h>

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gdk/gdkx.h>
#include "player-xml.h"

#define AUDIO_CAPS "application/x-rtp,media=(string)audio,clock-rate=(int)8000,encoding-name=(string)PCMA"

#define AUDIO_DEPAY "rtppcmadepay"
#define AUDIO_DEC   "alawdec"
#define AUDIO_SINK  "autoaudiosink"

#define DEST_HOST "127.0.0.1"

GstElement *audiodepay;

static void
pad_added_cb (GstElement * rtpbin, GstPad * new_pad, gpointer data)
{
  GstPad *sinkpad;
  GstPadLinkReturn lres;
  g_print ("new payload on pad: %s\n", GST_PAD_NAME (new_pad));
  sinkpad = gst_element_get_static_pad (audiodepay, "sink");
  g_assert (sinkpad);
  lres = gst_pad_link (new_pad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
   
  gst_object_unref (sinkpad);
}

gint client_audio_stream(int rtp_src,int rtcp_src,int rtcp_sink)
{
  GstElement *rtpbin, *rtpsrc, *rtcpsrc, *rtcpsink;

  GstElement  *audiodec, *audiores, *audioconv, *audiosink;

  GMainLoop *loop;
  GstCaps *caps;
  gboolean res;
  GstPadLinkReturn lres;
  GstPad *srcpad, *sinkpad;

  
  RTP_SRC_A=rtp_src;
  RTCP_SRC_A=rtcp_src;
  RTCP_SINK_A=rtcp_sink;

   

  pipelineAC = gst_pipeline_new ("Client");
  g_assert (pipelineAC);

  rtpsrc = gst_element_factory_make ("udpsrc", "rtpsrc");
  g_assert (rtpsrc);
  g_object_set (rtpsrc, "port", RTP_SRC_A, NULL);
  caps = gst_caps_from_string (AUDIO_CAPS);
  g_object_set (rtpsrc, "caps", caps, NULL);
  gst_caps_unref (caps);

  rtcpsrc = gst_element_factory_make ("udpsrc", "rtcpsrc");
  g_assert (rtcpsrc);
  g_object_set (rtcpsrc, "port", RTCP_SRC_A, NULL);

  rtcpsink = gst_element_factory_make ("udpsink", "rtcpsink");
  g_assert (rtcpsink);
  g_object_set (rtcpsink, "port", RTCP_SINK_A, "host", DEST_HOST, NULL);
  /* no need for synchronisation or preroll on the RTCP sink */
  g_object_set (rtcpsink, "async", FALSE, "sync", FALSE, NULL);



  gst_bin_add_many (GST_BIN (pipelineAC), rtpsrc, rtcpsrc, rtcpsink, NULL);

  /* the depayloading and decoding */
  audiodepay = gst_element_factory_make (AUDIO_DEPAY, "audiodepay");
  g_assert (audiodepay);
  audiodec = gst_element_factory_make (AUDIO_DEC, "audiodec");
  g_assert (audiodec);
  /* the audio playback and format conversion */
  audioconv = gst_element_factory_make ("audioconvert", "audioconv");
  g_assert (audioconv);
  audiores = gst_element_factory_make ("audioresample", "audiores");
  g_assert (audiores);
  audiosink = gst_element_factory_make (AUDIO_SINK, "audiosink");
  g_assert (audiosink);


  gst_bin_add_many (GST_BIN (pipelineAC), audiodepay, audiodec, audioconv,audiores, audiosink, NULL);


  res = gst_element_link_many (audiodepay, audiodec, audioconv, audiores,audiosink, NULL);
  g_assert (res == TRUE);


  /* the rtpbin element */
  rtpbin = gst_element_factory_make ("gstrtpbin", "rtpbin");
  g_assert (rtpbin);

  gst_bin_add (GST_BIN (pipelineAC), rtpbin);


  srcpad = gst_element_get_static_pad (rtpsrc, "src");
  sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtp_sink_0");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (srcpad);

  srcpad = gst_element_get_static_pad (rtcpsrc, "src");
  sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtcp_sink_0");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (srcpad);
  gst_object_unref (sinkpad);

  srcpad = gst_element_get_request_pad (rtpbin, "send_rtcp_src_0");
  sinkpad = gst_element_get_static_pad (rtcpsink, "sink");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (sinkpad);



  

  g_signal_connect (rtpbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

  
  g_print ("starting receiver pipeline\n");
  


  return 0;
}

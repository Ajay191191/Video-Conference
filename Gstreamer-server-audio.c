
#include <string.h>
#include <math.h>

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gdk/gdkx.h>
#include "player-xml.h"

#define DEST_HOST "127.0.0.1"

/* #define AUDIO_SRC  "alsasrc" */
#define AUDIO_SRC  "alsasrc"

/* the encoder and payloader elements */
#define AUDIO_ENC  "alawenc"
#define AUDIO_PAY  "rtppcmapay"


gint server_audio_stream(int rtp_src,int rtcp_src,int rtcp_sink)
{
  GstElement *audiosrc, *audioconv, *audiores, *audioenc, *audiopay;


  GstElement *rtpbin, *rtpsink, *rtcpsink, *rtcpsrc;

  GMainLoop *loop;
  GstPad *srcpad, *sinkpad;


    RTP_SRC_A=rtp_src;
  RTCP_SRC_A=rtcp_src;
  RTCP_SINK_A=rtcp_sink;

  /* the pipeline to hold everything */
  pipelineAudio = gst_pipeline_new ("ServerAudio");
  g_assert (pipelineAudio);

  /* the audio capture and format conversion */
  audiosrc = gst_element_factory_make (AUDIO_SRC, "audiosrc");
  g_assert (audiosrc);
  audioconv = gst_element_factory_make ("audioconvert", "audioconv");
  g_assert (audioconv);
  audiores = gst_element_factory_make ("audioresample", "audiores");
  g_assert (audiores);
  /* the encoding and payloading */
  audioenc = gst_element_factory_make (AUDIO_ENC, "audioenc");
  g_assert (audioenc);
  audiopay = gst_element_factory_make (AUDIO_PAY, "audiopay");
  g_assert (audiopay);

  gst_bin_add_many (GST_BIN (pipelineAudio), audiosrc, audioconv, audiores,
      audioenc, audiopay, NULL);

  if (!gst_element_link_many (audiosrc, audioconv, audiores, audioenc,
          audiopay, NULL)) {
    g_error ("Failed to link audiosrc, audioconv, audioresample, "
        "audio encoder and audio payloader");
  }

  // the rtpbin element 
  rtpbin = gst_element_factory_make ("gstrtpbin", "rtpbin");
  g_assert (rtpbin);

  gst_bin_add (GST_BIN (pipelineAudio), rtpbin);

  // the udp sinks and source we will use for RTP and RTCP 
  rtpsink = gst_element_factory_make ("udpsink", "rtpsink");
  g_assert (rtpsink);
  g_object_set (rtpsink, "port", RTP_SRC_A, "host", DEST_HOST, NULL);

  rtcpsink = gst_element_factory_make ("udpsink", "rtcpsink");
  g_assert (rtcpsink);
  g_object_set (rtcpsink, "port", RTCP_SRC_A, "host", DEST_HOST, NULL);
  g_object_set (rtcpsink, "async", FALSE, "sync", FALSE, NULL);

  rtcpsrc = gst_element_factory_make ("udpsrc", "rtcpsrc");
  g_assert (rtcpsrc);
  g_object_set (rtcpsrc, "port", RTCP_SINK_A, NULL);

  


  gst_bin_add_many (GST_BIN (pipelineAudio), rtpsink, rtcpsink, rtcpsrc, NULL);

  // RTP sinkpad for session 0 
  sinkpad = gst_element_get_request_pad (rtpbin, "send_rtp_sink_0");
  srcpad = gst_element_get_static_pad (audiopay, "src");
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    g_error ("Failed to link audio payloader to rtpbin");
  gst_object_unref (srcpad);

  // RTP srcpad
  srcpad = gst_element_get_static_pad (rtpbin, "send_rtp_src_0");
  sinkpad = gst_element_get_static_pad (rtpsink, "sink");
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    g_error ("Failed to link rtpbin to rtpsink");
  gst_object_unref (srcpad);
  gst_object_unref (sinkpad);

  // RTCP srcpad for sending RTCP to the receiver 
  srcpad = gst_element_get_request_pad (rtpbin, "send_rtcp_src_0");
  sinkpad = gst_element_get_static_pad (rtcpsink, "sink");
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    g_error ("Failed to link rtpbin to rtcpsink");
  gst_object_unref (sinkpad);	

  // request an RTCP sinkpad for session 0 
  srcpad = gst_element_get_static_pad (rtcpsrc, "src");
  sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtcp_sink_0");
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    g_error ("Failed to link rtcpsrc to rtpbin");
  gst_object_unref (srcpad);

  return 0;
}

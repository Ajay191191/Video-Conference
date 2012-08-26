/*
 * Copyright 2012
 *  @author: Ajay <Ajay191191@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * 
 */


#include <string.h>
#include <math.h>

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gdk/gdkx.h>
#include "player-xml.h"

#define VIDEO_CAPS "application/x-rtp, media=(string)video, clock-rate=(int)90000,encoding-name=(string)VP8-DRAFT-IETF-01,width=320, height=240"

#define DEST_HOST "192.168.1.5"

GstElement *videodepay;

static void
pad_added_cb (GstElement * rtpbin, GstPad * new_pad, gpointer data)
{
  GstPad *sinkpad;
  GstPadLinkReturn lres;
  
  g_print ("new payload on pad: %s\n", GST_PAD_NAME (new_pad));
  sinkpad = gst_element_get_static_pad (videodepay, "sink");
  g_assert (sinkpad);
  lres = gst_pad_link (new_pad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
   
  gst_object_unref (sinkpad);
}

gint client_video_stream(int rtp_src,int rtcp_src,int rtcp_sink)
{
  GstElement *rtpbin;
  GstElement *rtpvsrc, *rtcpvsrc, *rtcpvsink;

  GstElement  *videodec, *videosink;  
  GstElement *queue;

  
  GstCaps *caps;
  gboolean res;
  GstPadLinkReturn lres;
  GstPad *srcpad, *sinkpad;

  RTP_SRC_V=rtp_src;
  RTCP_SRC_V=rtcp_src;
  RTCP_SINK_V=rtcp_sink;


  pipelineVC = gst_pipeline_new ("Client");
  g_assert (pipelineVC);


  rtpvsrc = gst_element_factory_make ("udpsrc", "rtpvsrc");
  g_assert (rtpvsrc);
  g_object_set (rtpvsrc, "port", RTP_SRC_V, NULL);
  caps = gst_caps_from_string (VIDEO_CAPS);
  g_object_set (rtpvsrc, "caps", caps, NULL);
  gst_caps_unref (caps);

  rtcpvsrc = gst_element_factory_make ("udpsrc", "rtcpvsrc");
  g_assert (rtcpvsrc);
  g_object_set (rtcpvsrc, "port", RTCP_SRC_V, NULL);

  rtcpvsink = gst_element_factory_make ("udpsink", "rtcpvsink");
  g_assert (rtcpvsink);		
  g_object_set (rtcpvsink, "port", RTCP_SINK_V, "host", DEST_HOST, NULL);
  g_object_set (rtcpvsink, "async", FALSE, "sync", FALSE, NULL);



  gst_bin_add_many (GST_BIN (pipelineVC), rtpvsrc, rtcpvsrc, rtcpvsink, NULL);

  //Video
  videodepay = gst_element_factory_make ("rtpvp8depay", "videodepay");
  g_assert (videodepay);
  videodec = gst_element_factory_make ("vp8dec", "videodec");
  g_assert (videodec);
  videosink = gst_element_factory_make ("xvimagesink", "videosink");
  g_assert (videosink);
  g_object_set(videosink,"sync",FALSE,NULL);
  queue = gst_element_factory_make ("queue","queue");
  g_assert(queue);

  /* add depayloading and playback to the pipelineVC and link */
  gst_bin_add_many (GST_BIN (pipelineVC), videodepay,queue, videodec, videosink, NULL);


  res = gst_element_link_many (videodepay,queue, videodec, videosink, NULL);
  g_assert (res == TRUE);


  /* the rtpbin element */
  rtpbin = gst_element_factory_make ("gstrtpbin", "rtpbin");
  g_assert (rtpbin);
  g_object_set(rtpbin,"latency",10,NULL);

  gst_bin_add (GST_BIN (pipelineVC), rtpbin);





  //Video
  srcpad = gst_element_get_static_pad (rtpvsrc, "src");
  sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtp_sink_1");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (srcpad);

  // RTCP sinkpad in session 1 
  srcpad = gst_element_get_static_pad (rtcpvsrc, "src");
  sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtcp_sink_1");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (srcpad);
  gst_object_unref (sinkpad);

  // RTCP srcpad for sending RTCP 
  srcpad = gst_element_get_request_pad (rtpbin, "send_rtcp_src_1");
  sinkpad = gst_element_get_static_pad (rtcpvsink, "sink");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (sinkpad);


  //Directing xvideo to Drawing Area
  if (GST_IS_X_OVERLAY (videosink))
  {
            gst_x_overlay_set_window_handle(GST_X_OVERLAY(videosink), GPOINTER_TO_UINT(GINT_TO_POINTER(GDK_WINDOW_XWINDOW(remote_video->window))));
  }
  
  

  g_signal_connect (rtpbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

  
  g_print ("starting receiver pipelineVC\n");

  return 0;
}

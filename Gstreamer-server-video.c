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

#define DEST_HOST "192.168.1.5"

#define VIDEO_CAPS "video/x-raw-yuv, width=320, height=240"
/* #define AUDIO_SRC  "alsasrc" */
#define AUDIO_SRC  "alsasrc"

/* the encoder and payloader elements */
#define AUDIO_ENC  "alawenc"
#define AUDIO_PAY  "rtppcmapay"


gint server_video_stream(int rtp_src,int rtcp_src,int rtcp_sink)
{
  GstElement *videosrc, *videoenc, *ffmpeg,*capsfilter, *videopay;
  GstElement *tee, *queueCl, *queueRtp,*videosink;
  GstElement *bin,*binR;
  GstCaps *caps;

  GstElement *rtpbin;
  GstElement *rtpvsink, *rtcpvsink, *rtcpvsrc;

  GMainLoop *loop;
  GstPad *srcpad, *sinkpad;
  
  RTP_SRC_V=rtp_src;
  RTCP_SRC_V=rtcp_src;
  RTCP_SINK_V=rtcp_sink;


  pipelineVideo = gst_pipeline_new ("Server");
  g_assert (pipelineVideo);

 
  /* the video capture and format conversion */
  videosrc = gst_element_factory_make ("v4l2src", "videosrc");
  g_assert (videosrc);
  g_object_set(videosrc,"device","/dev/video1" ,NULL);
  ffmpeg = gst_element_factory_make ("ffmpegcolorspace", "ffmpegcolorspace") ;
  capsfilter = gst_element_factory_make("capsfilter","capsfilter");

  caps = gst_caps_from_string (VIDEO_CAPS);
  g_object_set(capsfilter,"caps",caps,NULL);

  //Tee
  tee = gst_element_factory_make("tee","tee");
  g_assert(tee);

  queueCl = gst_element_factory_make("queue","queueCl");
  g_assert(queueCl);
  g_object_set(queueCl,"leaky",2,NULL);
  g_object_set(queueCl,"max-size-time", 10*GST_SECOND,NULL);

  queueRtp = gst_element_factory_make("queue","queueRtp");
  g_assert(queueRtp);
  g_object_set(queueRtp,"leaky",2,NULL);
  g_object_set(queueRtp,"max-size-time", 10*GST_SECOND,NULL);

  videosink = gst_element_factory_make("xvimagesink","videosink");
  g_assert(videosink);
  g_object_set(videosink,"sync",FALSE,NULL);


  //Directing xvideo to Drawing Area
  if (GST_IS_X_OVERLAY (videosink))
  {
            gst_x_overlay_set_window_handle(GST_X_OVERLAY(videosink), GPOINTER_TO_UINT(GINT_TO_POINTER(GDK_WINDOW_XWINDOW(client_video->window))));
  }


  bin = gst_bin_new ("Sub bin1");
  binR = gst_bin_new ("Sub bin2");

  /* the encoding and payloading */
  videoenc = gst_element_factory_make ("vp8enc", "videoenc");
  g_assert (videoenc);
  g_object_set(videoenc,"speed",7,"threads",4,NULL);
  videopay = gst_element_factory_make ("rtpvp8pay", "videopay");
  g_assert (videopay);
  //g_object_set(videopay,"max-ptime",10,NULL);

  gst_bin_add_many (GST_BIN (pipelineVideo), videosrc,ffmpeg,capsfilter,tee,videopay,NULL);

  if (!gst_element_link_many (videosrc,ffmpeg,capsfilter,tee,NULL)) {
    g_error ("Failed to link video,"
        "video encoder and video payloader");
  }

  GstPad *sinkpadvideo, *srcpadvideo;
  GstPadLinkReturn ret;
   //gst_element_set_state (bin, GST_STATE_NULL);
  //Linking !
  gst_bin_add_many (GST_BIN (bin), queueCl, videosink, NULL);
  

  if(!gst_element_link_many(queueCl,videosink,NULL) ) 
	g_error("Cannot link queueCl,videosink");

  sinkpadvideo = gst_element_get_static_pad(queueCl, "sink");
  gst_element_add_pad(bin, gst_ghost_pad_new("videosink", sinkpadvideo));
  gst_object_unref(GST_OBJECT(sinkpadvideo));
 // gst_element_set_state(bin, GST_STATE_PAUSED);
  tee = gst_bin_get_by_name (GST_BIN(pipelineVideo), "tee");
  srcpadvideo = gst_element_get_request_pad(tee, "src%d");
  sinkpadvideo = gst_element_get_pad(bin, "videosink");
  gst_bin_add(GST_BIN(pipelineVideo), bin);
  if(gst_pad_link(srcpadvideo, sinkpadvideo) != GST_PAD_LINK_OK)
	g_error("Cannot link srcpadvide,sinkpadvideo queuecl");
   //gst_element_set_state(bin, GST_STATE_PLAYING);
  //gst_element_set_state(pipeline,GST_STATE_PLAYING);

  //Linking RTP !
  gst_bin_add_many (GST_BIN (binR), queueRtp, videoenc, NULL);

  if(!gst_element_link_many(queueRtp,videoenc,NULL))
	g_error("Canno link queuertp ,videoenc");

  sinkpadvideo = gst_element_get_static_pad(queueRtp, "sink");
  gst_element_add_pad(binR, gst_ghost_pad_new("videosink", sinkpadvideo));
  gst_object_unref(GST_OBJECT(sinkpadvideo));
  //gst_element_set_state(binR, GST_STATE_PAUSED);
  tee = gst_bin_get_by_name (GST_BIN(pipelineVideo), "tee");
  srcpadvideo = gst_element_get_request_pad(tee, "src%d");
  sinkpadvideo = gst_element_get_pad(binR, "videosink");
  gst_bin_add(GST_BIN(pipelineVideo), binR);
  if(gst_pad_link(srcpadvideo, sinkpadvideo)!=GST_PAD_LINK_OK)
	g_error("Cannot link srcpd,sinkpad queuertp");
 // gst_element_set_state(binR, GST_STATE_PLAYING);
  //gst_element_set_state(pipelineVideo,GST_STATE_PLAYING);

  sinkpadvideo = gst_element_get_static_pad(videopay, "sink");
  gst_pad_set_active(sinkpadvideo,TRUE);
  //g_print("Pad: %d",gst_pad_is_active(sinkpadvideo));
//  gst_element_add_pad(pipelineVideo, gst_ghost_pad_new("videopay", sinkpadvideo));
 // gst_object_unref(GST_OBJECT(sinkpadvideo));
  srcpadvideo = gst_element_get_static_pad(videoenc, "src");  
  
  GstPad *ghost;
  ghost= gst_ghost_pad_new("videopaysrc",srcpadvideo);
  gst_pad_set_active(ghost,TRUE);
  gst_element_add_pad(binR,ghost);
  gst_object_unref(GST_OBJECT(srcpadvideo));
  srcpadvideo=gst_element_get_pad(binR,"videopaysrc");
 
    //gst_pad_set_active(srcpadvideo,TRUE);
//   g_print("Pad: %d",gst_pad_is_active(srcpadvideo));
//  sinkpadvideo = gst_element_get_pad(pipelineVideo, "videopay");
  gst_pad_link(srcpadvideo, sinkpadvideo) ;
//	g_error("Cannot link srcpad,sinkpad videoenc,pay");



 /* sinkpadvideo = gst_element_get_static_pad(videopay, "sink");
  srcpadvideo = gst_element_get_static_pad(videoenc, "src");  
 if( gst_pad_link(srcpadvideo, sinkpadvideo) !=GST_PAD_LINK_OK)
	g_error("Cannot link srcpad,sinkpad videoenc,pay");
*/
  // the rtpbin element 
  rtpbin = gst_element_factory_make ("gstrtpbin", "rtpbin");
  g_assert (rtpbin);

  gst_bin_add (GST_BIN (pipelineVideo), rtpbin);

  //Video
  rtpvsink = gst_element_factory_make ("udpsink", "rtpvsink");
  g_assert (rtpvsink);
  g_object_set (rtpvsink, "port", RTP_SRC_V, "host", DEST_HOST, "sync",FALSE,NULL);

  rtcpvsink = gst_element_factory_make ("udpsink", "rtcpvsink");
  g_assert (rtcpvsink);
  g_object_set (rtcpvsink, "port", RTCP_SRC_V, "host", DEST_HOST, NULL);
  g_object_set (rtcpvsink, "async", FALSE, "sync", FALSE, NULL);

  rtcpvsrc = gst_element_factory_make ("udpsrc", "rtcpvsrc");
  g_assert (rtcpvsrc);
  g_object_set (rtcpvsrc, "port", RTCP_SINK_V, NULL);





 
  gst_bin_add_many (GST_BIN (pipelineVideo), rtpvsink, rtcpvsink, rtcpvsrc, NULL);

 
  //Video
  // RTP sinkpad for session 0 
  sinkpad = gst_element_get_request_pad (rtpbin, "send_rtp_sink_1");
  srcpad = gst_element_get_static_pad (videopay, "src");
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    g_error ("Failed to link video payloader to rtpbin");
  gst_object_unref (srcpad);

  // RTP srcpad 
  srcpad = gst_element_get_static_pad (rtpbin, "send_rtp_src_1");
  sinkpad = gst_element_get_static_pad (rtpvsink, "sink");
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    g_error ("Failed to link rtpbin to rtpvsink");
  gst_object_unref (srcpad);
  gst_object_unref (sinkpad);

  // RTCP srcpad for sending RTCP to the receiver 
  srcpad = gst_element_get_request_pad (rtpbin, "send_rtcp_src_1");
  sinkpad = gst_element_get_static_pad (rtcpvsink, "sink");
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    g_error ("Failed to link rtpbin to rtcpvsink");
  gst_object_unref (sinkpad);

  // request an RTCP sinkpad for session 1 
  srcpad = gst_element_get_static_pad (rtcpvsrc, "src");
  sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtcp_sink_1");
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    g_error ("Failed to link rtcpvsrc to rtpbin");
  gst_object_unref (srcpad);

  return 0;
}

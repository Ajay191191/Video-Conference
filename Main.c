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

#include <gtk/gtk.h>
#include <glib.h>


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <gst/gst.h>

#include "player-xml.h"



int rtp_src_v, rtcp_src_v,rtcp_sink_v;
int rtp_src_v_c, rtcp_src_v_c,rtcp_sink_v_c;
int mutex=0;

int rtp_src_a, rtcp_src_a,rtcp_sink_a;
int rtp_src_a_c, rtcp_src_a_c,rtcp_sink_a_c;


gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  return(FALSE);
}




void destroy(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}



static gpointer
thread_func( gpointer data )
{
    while( TRUE )
    {
	sleep( 3 );
	if(stream_on && mutex<1){
       
	mutex++;
        client_video_stream(rtp_src_v_c, rtcp_src_v_c,rtcp_sink_v_c);
        client_audio_stream(rtp_src_a_c, rtcp_src_a_c,rtcp_sink_a_c);	
	gst_element_set_state (pipelineVC, GST_STATE_PLAYING);
	gst_element_set_state (pipelineAC, GST_STATE_PLAYING);
        }
    }

    return( NULL );
}



void on_click(GtkWidget *widget, gpointer data) {
    

    stream_on=TRUE;   
   //Dynamic ports
   const gchar *text = gtk_entry_get_text( port );
   rtp_src_v=atoi(text);
   rtcp_src_v=rtp_src_v+1;
   rtcp_sink_v=rtp_src_v+4;
   rtp_src_a=atoi(text)+100;
   rtcp_src_v=rtp_src_v+101;
   rtcp_sink_v=rtp_src_v+104;
   
      const gchar *textC = gtk_entry_get_text( portC );
   rtp_src_v_c=atoi(textC);
   rtcp_src_v_c=rtp_src_v_c+1;
   rtcp_sink_v_c=rtp_src_v_c+4;
  rtp_src_a_c=atoi(textC)+100;
   rtcp_src_a_c=rtp_src_v_c+101;
   rtcp_sink_a_c=rtp_src_v_c+104;

   server_video_stream(rtp_src_v, rtcp_src_v,rtcp_sink_v);
   server_audio_stream(rtp_src_a, rtcp_src_a,rtcp_sink_a);
   
	gst_element_set_state (pipelineVideo, GST_STATE_PLAYING);
	gst_element_set_state (pipelineAudio, GST_STATE_PLAYING);
   /*gdk_threads_enter();
   if(!g_thread_create( thread_func, NULL,FALSE, &t_error ))
    {
		g_printerr ("Failed to create YES thread: %s\n", t_error->message);
	        return 1;
    }
   /gdk_threads_leave();
    */

}


int main(int argc, char *argv[]) {


     gdk_threads_init();

    /* Do stuff as usual */
    gtk_init( &argc, &argv );
    gtk_init(&argc, &argv);
    gst_init (&argc, &argv);
	GMutex* lock;
	GList *list;
	

  
    t_error=NULL;
    builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, "Video-Conference.glade", NULL)) {
        g_print("Error opening file");
        return 0;
    }

    

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    vBox = GTK_WIDGET(gtk_builder_get_object(builder, "vbox1"));
    hBox = GTK_WIDGET(gtk_builder_get_object(builder, "hbox1"));
    menu = GTK_MENU_BAR(gtk_builder_get_object(builder, "menubar1"));
    client_video = GTK_WIDGET(gtk_builder_get_object(builder, "user_drawingareaClient"));
    remote_video = GTK_WIDGET(gtk_builder_get_object(builder, "user_drawingareaRemote"));
    aspectFrameClient = GTK_WIDGET(gtk_builder_get_object(builder, "user_frame"));
    aspectFrameRemote = GTK_WIDGET(gtk_builder_get_object(builder, "user_frame1"));
    label_remote = GTK_WIDGET(gtk_builder_get_object(builder, "frame_label"));
    label_client = GTK_WIDGET(gtk_builder_get_object(builder, "frame_label1"));
    button = GTK_WIDGET(gtk_builder_get_object(builder, "button"));
    port = GTK_ENTRY(gtk_builder_get_object(builder, "Port"));
    portC = GTK_ENTRY(gtk_builder_get_object(builder, "PortC"));
    gtk_signal_connect(GTK_OBJECT (window), "delete_event",
		     GTK_SIGNAL_FUNC(delete_event), NULL);
          
  gtk_signal_connect(GTK_OBJECT (window), "destroy",
		     GTK_SIGNAL_FUNC(destroy), NULL);
    gtk_signal_connect(GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC(on_click), NULL);

          
 
/*
    //Ports
    //Video
    RTP_SRC_V=8002;
    RTCP_SRC_V=8003;
    RTCP_SINK_V=8007;

    //Audio
    RTP_SRC_A=5002;
    RTCP_SRC_A=5003;
    RTCP_SINK_A=5007;
*/



if(!g_thread_create( thread_func, NULL,FALSE, &t_error ))
    {
		g_printerr ("Failed to create YES thread: %s\n", t_error->message);
	        return 1;
    }
    
   
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(G_OBJECT(builder));

    gtk_widget_show_all(window);
    gdk_threads_enter();
   
    gtk_main();
    

    gdk_threads_leave();

    exit(0);
}


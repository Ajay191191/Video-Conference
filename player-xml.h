
#ifndef PLAYER_XML_H
#define PLAYER_XML_H


GtkBuilder *builder; 
GtkWidget *window;
GtkWidget *vBox;
GtkWidget *hBox;

GtkMenuBar *menu;
GtkMenuItem *FileItem;
GtkMenu *MenuFile;
GtkImageMenuItem *openMenu;
GtkImageMenuItem *quitMenu;
GtkWidget *client_video,*remote_video;
GtkWidget *aspectFrameClient, *aspectFrameRemote;
GtkWidget *label_remote,*label_client;
GtkWidget *button;
GtkEntry *port,*portC;

char *filename;
gboolean stream_on;

GstElement *pipelineVideo,*pipelineVC,*pipelineAudio,*pipelineAC;
GMainLoop *loopCl,*loopSv;

GThread   *thread;
GError    *t_error ;


gint on_window_destroy (GtkObject *object, GdkEvent *event,gpointer user_data);
gint client_video_stream(int rtp_src,int rtcp_src,int rtcp_sink);
gint server_video_stream(int rtp_src,int rtcp_src,int rtcp_sink);

//Video
int RTP_SRC_V,RTCP_SRC_V,RTCP_SINK_V;

//Audio
int RTP_SRC_A,RTCP_SRC_A,RTCP_SINK_A;




#endif /* PLAYER_XML_H */

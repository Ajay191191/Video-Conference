CC=gcc

EXTRA_WARNINGS=-Wall -W -Wformat-nonliteral -Wcast-align -Wpointer-arith \
	       -Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes \
	       -Wmissing-declarations -Winline -Wundef -Wnested-externs -Wcast-qual \
	       -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal -pedantic -ansi -std=c99 -export-dynamic

GST_LIBS=`pkg-config --libs gstreamer-0.10` -lgstinterfaces-0.10
GST_CFLAGS=`pkg-config --cflags gstreamer-0.10`
GTK_LIBS=`pkg-config --libs gtk+-2.0 gthread-2.0`
GTK_CFLAGS=`pkg-config --cflags gtk+-2.0 gthread-2.0`

CFLAGS=-ggdb $(EXTRA_WARNINGS)

BINS=gst-player

all: $(BINS)

gst-player:  Main.c Gstreamer-server-video.c Gstreamer-client-video.c
	 $(CC) $+ $(CFLAGS) $(GTK_CFLAGS) $(GTK_LIBS) $(GST_CFLAGS) $(GST_LIBS) -o $@ -I.

clean:
	rm -rf $(BINS)

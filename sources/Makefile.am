NULL =

noinst_LTLIBRARIES = libsources.la libsourcesimpl.la

libsources_la_SOURCES = 		\
	rb-source.c 			\
	rb-source.h			\
	rb-streaming-source.c		\
	rb-streaming-source.h		\
	rb-source-group.c		\
	rb-source-group.h		\
	rb-source-search.c		\
	rb-source-search.h		\
	$(NULL)

libsourcesimpl_la_SOURCES =		\
	rb-sourcelist.c			\
	rb-sourcelist.h			\
	rb-sourcelist-model.c		\
	rb-sourcelist-model.h		\
	rb-browser-source.c		\
	rb-browser-source.h		\
	rb-library-source.c		\
	rb-library-source.h		\
	rb-podcast-source.c		\
	rb-podcast-source.h		\
	rb-removable-media-source.c	\
	rb-removable-media-source.h	\
	rb-playlist-source.c            \
	rb-playlist-source.h		\
	rb-playlist-xml.h		\
	rb-auto-playlist-source.c	\
	rb-auto-playlist-source.h	\
	rb-static-playlist-source.c	\
	rb-static-playlist-source.h	\
	rb-play-queue-source.c		\
	rb-play-queue-source.h		\
	rb-missing-files-source.c	\
	rb-missing-files-source.h	\
	rb-import-errors-source.c	\
	rb-import-errors-source.h	\
	rb-source-search-basic.c	\
	rb-source-search-basic.h	\
	$(NULL)

INCLUDES =						\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DG_LOG_DOMAIN=\"Rhythmbox\"		 	\
	-I$(top_srcdir) 				\
	-I$(top_srcdir)/lib 				\
	-I$(top_builddir)/lib 				\
	-I$(top_srcdir)/rhythmdb			\
	-I$(top_srcdir)/metadata 			\
	-I$(top_srcdir)/widgets 			\
	-I$(top_srcdir)/library 			\
	-I$(top_srcdir)/player	 			\
	-I$(top_srcdir)/iradio				\
	-I$(top_srcdir)/podcast				\
	-I$(top_srcdir)/shell				\
	-I$(top_srcdir)/backends			\
	-I$(top_srcdir)/plugins				\
	-DPIXMAP_DIR=\""$(datadir)/pixmaps"\"		\
	-DSHARE_DIR=\"$(pkgdatadir)\"                   \
	-DDATADIR=\""$(datadir)"\"			\
	$(TOTEM_PLPARSER_CFLAGS)			\
	$(HAL_CFLAGS)					\
	$(RHYTHMBOX_CFLAGS)				\
	$(NO_STRICT_ALIASING_CFLAGS)

libsources_la_LDFLAGS = -export-dynamic
libsourcesimpl_la_LDFLAGS = -export-dynamic


NULL =

plugindir = $(PLUGINDIR)/iradio
plugin_LTLIBRARIES = libiradio.la

libiradio_la_SOURCES = \
	rb-iradio-plugin.c					\
	$(top_srcdir)/sources/rb-iradio-source.c		\
	$(top_srcdir)/sources/rb-iradio-source.h		\
	$(top_srcdir)/iradio/rb-new-station-dialog.c		\
	$(top_srcdir)/iradio/rb-new-station-dialog.h		\
	$(top_srcdir)/iradio/rb-station-properties-dialog.c	\
	$(top_srcdir)/iradio/rb-station-properties-dialog.h

libiradio_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libiradio_la_LIBADD = 					\
	$(top_builddir)/lib/librb.la			\
	$(top_builddir)/sources/libsources.la

INCLUDES = 						\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DG_LOG_DOMAIN=\"Rhythmbox\"		 	\
	-I$(top_srcdir) 				\
	-I$(top_srcdir)/lib                        	\
	-I$(top_srcdir)/metadata                       	\
	-I$(top_srcdir)/player                       	\
	-I$(top_srcdir)/rhythmdb                       	\
	-I$(top_srcdir)/widgets                    	\
	-I$(top_srcdir)/sources                    	\
	-I$(top_srcdir)/iradio                    	\
	-I$(top_srcdir)/podcast                    	\
	-I$(top_srcdir)/plugins				\
	-I$(top_srcdir)/shell				\
	-I$(top_srcdir)/backends			\
	-DPIXMAP_DIR=\""$(datadir)/pixmaps"\"		\
	-DSHARE_DIR=\"$(pkgdatadir)\"                   \
	-DDATADIR=\""$(datadir)"\"			\
	$(RHYTHMBOX_CFLAGS)				\
	$(WARN_CFLAGS)					\
	$(TOTEM_PLPARSER_CFLAGS)			\
	-D_XOPEN_SOURCE -D_BSD_SOURCE

gladedir = $(datadir)/rhythmbox/glade
glade_DATA =

plugin_in_files = iradio.rb-plugin.desktop.in

%.rb-plugin: %.rb-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

BUILT_SOURCES =							\
	$(plugin_in_files:.rb-plugin.desktop.in=.rb-plugin) 	\
	$(NULL)

plugin_DATA = 			\
	$(BUILT_SOURCES)	\
	$(NULL)

EXTRA_DIST = 			\
	$(glade_DATA)		\
	$(plugin_in_files)	\
	iradio-ui.xml		\
	$(NULL)

CLEANFILES = 			\
	$(BUILT_SOURCES)	\
	$(NULL)

DISTCLEANFILES =		\
	$(BUILT_SOURCES)	\
	$(NULL)

plugindir = $(PLUGINDIR)
plugin_LTLIBRARIES = libsample-vala.la
#noinst_LTLIBRARIES = libsample-vala.la

nodist_libsample_vala_la_SOURCES = \
	rb-sample-vala-plugin.c				\
	rb-sample-vala-plugin.h

libsample_vala_la_SOURCES = \
	rb-sample-vala-plugin.vala
	
libsample_vala_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libsample_vala_la_CFLAGS = $(WNOERROR_CFLAGS)

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
	-DPIXMAP_DIR=\""$(datadir)/pixmaps"\"		\
	-DSHARE_DIR=\"$(pkgdatadir)\"                   \
	-DDATADIR=\""$(datadir)"\"			\
	$(RHYTHMBOX_CFLAGS)				\
	-D_XOPEN_SOURCE -D_BSD_SOURCE


gladedir = $(datadir)/rhythmbox/glade
glade_DATA =

plugin_in_files = sample-vala.rb-plugin.in

%.rb-plugin: %.rb-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po); $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

%.c %.h: %.vala
	$(VALAC) -C --vapidir=$(top_srcdir)/bindings/vala --pkg=rhythmdb --pkg=rb $^
	
plugin_DATA = $(plugin_in_files:.rb-plugin.in=.rb-plugin)

EXTRA_DIST = $(glade_DATA) $(plugin_in_files)

CLEANFILES = $(plugin_DATA) $(libsample_vala_la_SOURCES:.vala=.h) $(libsample_vala_la_SOURCES:.vala=.c)
DISTCLEANFILES = $(CLEANFILES)

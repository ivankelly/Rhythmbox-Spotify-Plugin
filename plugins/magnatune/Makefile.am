# Magnatune Store Python Plugin

SUBDIRS = magnatune

plugindir = $(PLUGINDIR)/magnatune

%.rb-plugin: %.rb-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache
plugin_DATA = $(plugin_in_files:.rb-plugin.in=.rb-plugin)


plugin_in_files = magnatune.rb-plugin.in

glade_DATA =	\
		magnatune-loading.glade			\
		magnatune-purchase.glade		\
		magnatune-prefs.glade			\
		magnatune_logo_color_small.png		\
		magnatune_logo_color_tiny.png

themedir = $(pkgdatadir)/icons/hicolor
size = 24x24
context = places
icondir = $(themedir)/$(size)/$(context)
icon_DATA = icons/hicolor/$(size)/$(context)/magnatune.png

gladedir = $(plugindir)
EXTRA_DIST = $(plugin_in_files) $(glade_DATA) $(icon_DATA)
CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)


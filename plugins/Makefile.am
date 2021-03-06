## arch-tag: Automake rules for Rhythmbox's plugin system

SUBDIRS = 						\
	audiocd						\
	audioscrobbler					\
	generic-player					\
	iradio						\
	mmkeys						\
	power-manager					\
	sample						\
	visualizer

if WITH_LIRC
SUBDIRS += lirc
endif

if ENABLE_PYTHON
SUBDIRS += 						\
	pythonconsole					\
	sample-python					\
	artdisplay					\
	lyrics						\
	magnatune					\
	jamendo						\
	coherence					\
	dontreallyclose					\
	im-status					\
	rb
endif

if ENABLE_VALA
SUBDIRS +=						\
	sample-vala
endif

if USE_IPOD
SUBDIRS += ipod
endif

if HAVE_LIBBRASERO_MEDIA
SUBDIRS += brasero-disc-recorder
else
if HAVE_NAUTILUS_BURN
SUBDIRS += cd-recorder
endif # HAVE_NAUTILUS_BURN
endif # HAVE_LIBBRASERO_MEDIA

if USE_DAAP
SUBDIRS += daap
endif

if USE_MTP
SUBDIRS += mtpdevice
endif

if ENABLE_FM_RADIO
SUBDIRS += fmradio
endif

if WITH_SPOTIFY
SUBDIRS += rbspotify
endif


noinst_LTLIBRARIES = librbplugins.la

librbplugins_la_SOURCES =				\
	rb-plugin.h					\
	rb-plugin.c					\
	rb-module.c					\
	rb-module.h					\
	rb-plugin-manager.h				\
	rb-plugin-manager.c				\
	rb-plugins-engine.h				\
	rb-plugins-engine.c

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-DG_LOG_DOMAIN=\"Rhythmbox\"		 	\
	-I$(top_srcdir) 				\
	-I$(top_srcdir)/lib 				\
	-I$(top_builddir)/lib 				\
	-I$(top_srcdir)/player				\
	-I$(top_srcdir)/metadata			\
	-I$(top_srcdir)/rhythmdb			\
	-I$(top_srcdir)/sources				\
	-I$(top_srcdir)/library				\
	-I$(top_srcdir)/iradio				\
	-I$(top_srcdir)/shell				\
	-I$(top_srcdir)/widgets                    	\
	$(RHYTHMBOX_CFLAGS)				\
	-DRB_PLUGIN_DIR=\"$(PLUGINDIR)\"

if ENABLE_PYTHON
librbplugins_la_SOURCES += \
	rb-python-module.c		\
	rb-python-module.h		\
	rb-python-plugin.c		\
	rb-python-plugin.h

INCLUDES += \
	$(NO_STRICT_ALIASING_CFLAGS)	\
	$(PYGTK_CFLAGS)			\
	$(PYTHON_CFLAGS)
endif


librbplugins_la_LDFLAGS = -export-dynamic


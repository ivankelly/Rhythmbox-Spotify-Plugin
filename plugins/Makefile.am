## arch-tag: Automake rules for Rhythmbox's plugin system

SUBDIRS = sample

if WITH_AUDIOSCROBBLER
SUBDIRS += audioscrobbler
endif

if ENABLE_PYTHON
SUBDIRS += pythonconsole sample-python
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
	-I$(top_srcdir)/remote				\
	-I$(top_builddir)/remote			\
	$(WARN_CFLAGS)					\
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

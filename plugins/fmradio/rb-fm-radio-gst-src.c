/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 */

#include "config.h"

#include <string.h>

#include "rb-debug.h"

#include <gst/gst.h>

#define RB_TYPE_FM_RADIO_SRC (rb_fm_radio_src_get_type())
#define RB_FM_RADIO_SRC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),RB_TYPE_FM_RADIO_SRC,RBFMRadioSrc))
#define RB_FM_RADIO_SRC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),RB_TYPE_FM_RADIO_SRC,RBFMRadioSrcClass))
#define RB_IS_LASTFM_SRC(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),RB_TYPE_FM_RADIO_SRC))
#define RB_IS_LASTFM_SRC_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass),RB_TYPE_FM_RADIO_SRC))

typedef struct _RBFMRadioSrc RBFMRadioSrc;
typedef struct _RBFMRadioSrcClass RBFMRadioSrcClass;

struct _RBFMRadioSrc
{
	GstBin parent;

	GstElement *audiotestsrc;
	GstPad *ghostpad;
};

struct _RBFMRadioSrcClass
{
	GstBinClass parent_class;
};

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
	GST_PAD_SRC,
	GST_PAD_ALWAYS,
	GST_STATIC_CAPS_ANY);

static GstElementDetails rb_fm_radio_src_details =
GST_ELEMENT_DETAILS ("RB Silence Source",
	"Source/File",
	"Outputs buffers of silence",
	"James Henstridge <james@jamesh.id.au>");

static void rb_fm_radio_src_uri_handler_init (gpointer g_iface,
					      gpointer iface_data);

static void
_do_init (GType fmradio_src_type)
{
	static const GInterfaceInfo urihandler_info = {
		rb_fm_radio_src_uri_handler_init,
		NULL,
		NULL
	};

	g_type_add_interface_static (fmradio_src_type, GST_TYPE_URI_HANDLER,
				     &urihandler_info);
}

GST_BOILERPLATE_FULL (RBFMRadioSrc, rb_fm_radio_src,
		      GstBin, GST_TYPE_BIN, _do_init);

static void
rb_fm_radio_src_base_init (gpointer g_class)
{
	GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);
	gst_element_class_add_pad_template (element_class,
		gst_static_pad_template_get (&srctemplate));
	gst_element_class_set_details (element_class, &rb_fm_radio_src_details);
}

static void
rb_fm_radio_src_init (RBFMRadioSrc *src, RBFMRadioSrcClass *klass)
{
	GstPad *pad;

	rb_debug ("creating rb silence src element");

	src->audiotestsrc = gst_element_factory_make ("audiotestsrc", NULL);
	gst_bin_add (GST_BIN (src), src->audiotestsrc);
	gst_object_ref (src->audiotestsrc);

	/* set the audiotestsrc to generate silence (wave type 4) */
	g_object_set (src->audiotestsrc,
		      "wave", 4,
		      NULL);

	pad = gst_element_get_pad (src->audiotestsrc, "src");
	src->ghostpad = gst_ghost_pad_new ("src", pad);
	gst_element_add_pad (GST_ELEMENT (src), src->ghostpad);
	gst_object_ref (src->ghostpad);
	gst_object_unref (pad);
	
}

static void
rb_fm_radio_src_finalize (GObject *object)
{
	RBFMRadioSrc *src = RB_FM_RADIO_SRC (object);

	if (src->ghostpad)
		gst_object_unref (src->ghostpad);
	if (src->audiotestsrc)
		gst_object_unref (src->audiotestsrc);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
rb_fm_radio_src_class_init (RBFMRadioSrcClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = rb_fm_radio_src_finalize;
}


/* URI handler interface */

static guint
rb_fm_radio_src_uri_get_type (void)
{
	return GST_URI_SRC;
}

static gchar **
rb_fm_radio_src_uri_get_protocols (void)
{
	static gchar *protocols[] = {"xrbsilence", NULL};
	return protocols;
}

static const gchar *
rb_fm_radio_src_uri_get_uri (GstURIHandler *handler)
{
	return "xrbsilence:///";
}

static gboolean
rb_fm_radio_src_uri_set_uri (GstURIHandler *handler,
			   const gchar *uri)
{
	if (g_str_has_prefix (uri, "xrbsilence://") == FALSE)
		return FALSE;

	return TRUE;
}

static void
rb_fm_radio_src_uri_handler_init (gpointer g_iface,
				gpointer iface_data)
{
	GstURIHandlerInterface *iface = (GstURIHandlerInterface *) g_iface;

	iface->get_type = rb_fm_radio_src_uri_get_type;
	iface->get_protocols = rb_fm_radio_src_uri_get_protocols;
	iface->get_uri = rb_fm_radio_src_uri_get_uri;
	iface->set_uri = rb_fm_radio_src_uri_set_uri;
}

static gboolean
plugin_init (GstPlugin *plugin)
{
	gboolean ret = gst_element_register (plugin, "rbsilencesrc", GST_RANK_PRIMARY, RB_TYPE_FM_RADIO_SRC);
	return ret;
}

GST_PLUGIN_DEFINE_STATIC (GST_VERSION_MAJOR,
			  GST_VERSION_MINOR,
			  "rbsilencesrc",
			  "element to output silence",
			  plugin_init,
			  VERSION,
			  "GPL",
			  PACKAGE,
			  "");
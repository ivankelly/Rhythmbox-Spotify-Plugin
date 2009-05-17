/*
 * Copyright/Licensing information.
 */

/* inclusion guard */
#ifndef __RBSPOTIFYPLUGIN_H__
#define __RBSPOTIFYPLUGIN_H__

#include <glib-object.h>
#include <spotify/api.h>
#include <pthread.h>

/*
 * Potentially, include other headers on which this header depends.
 */
#include "rb-shell.h"
#include "rb-browser-source.h"
#include "rb-plugin.h"


G_BEGIN_DECLS


#define RBSPOTIFYPLUGIN_TYPE		(rb_spotify_plugin_get_type ())
#define RBSPOTIFYPLUGIN(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), RBSPOTIFYPLUGIN_TYPE, RBSpotifyPlugin))
#define RBSPOTIFYPLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), RBSPOTIFYPLUGIN_TYPE, RBSpotifyPluginClass))
#define IS_RBSPOTIFYPLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), RBSPOTIFYPLUGIN_TYPE))
#define IS_RBSPOTIFYPLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE ((k), RBSPOTIFYPLUGIN_TYPE))
#define RBSPOTIFYPLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), RBSPOTIFYPLUGIN_TYPE, RBSpotifyPluginClass))

typedef struct
{
     sp_session *sess;
     pthread_t notify_thread;
     GtkWidget *preferences;
     GtkWidget *config_widget;

     GtkWidget *username_entry;
     GtkWidget *username_label;
     GtkWidget *password_entry;
     GtkWidget *password_label;
} RBSpotifyPluginPrivate;

typedef struct
{
     RBPlugin parent;
     RBSpotifyPluginPrivate *priv;
} RBSpotifyPlugin;

typedef struct
{
     RBPluginClass parent_class;
} RBSpotifyPluginClass;

GType	rb_spotify_plugin_get_type		(void) G_GNUC_CONST;

G_END_DECLS

#endif

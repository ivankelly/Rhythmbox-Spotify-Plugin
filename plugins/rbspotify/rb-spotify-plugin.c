/*
 * rb-sample-plugin.h
 * 
 * Copyright (C) 2002-2005 - Paolo Maggi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * The Rhythmbox authors hereby grant permission for non-GPL compatible
 * GStreamer plugins to be used and distributed together with GStreamer
 * and Rhythmbox. This permission is above and beyond the permissions granted
 * by the GPL license by which Rhythmbox is covered. If you modify this code
 * you may extend this exception to your version of the code, but you are not
 * obligated to do so. If you do not wish to do so, delete this exception
 * statement from your version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#include <string.h> /* For strlen */

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>

#include <gconf/gconf-value.h>
#include <time.h>

#include "eel-gconf-extensions.h"
#include "rb-plugin.h"
#include "rb-debug.h"
#include "rb-shell.h"
#include "rb-dialog.h"

#include "rb-preferences.h"

#include "rb-spotify-plugin.h"
#include "rb-spotify-source.h"
#include "rb-spotify-src.h"
#include "rb-glade-helpers.h"
#include "audio.h"

extern const char g_appkey[];

/// The size of the application key.
extern const size_t g_appkey_size;

/// Synchronization mutex for the main thread
pthread_mutex_t g_notify_mutex;
// Synchronization condition variable for the main thread
pthread_cond_t g_notify_cond;
static int g_notify_do;

audio_fifo_t g_audio_fifo;

/**
 * The session callbacks
 */
static void spcb_logged_in(sp_session *sess, sp_error error);
static void spcb_notify_main_thread(sp_session *sess);
extern int spcb_music_delivery(sp_session *sess, const sp_audioformat *format, const void *frames, int num_frames);
static void spcb_metadata_updated(sp_session *sess);
static void spcb_play_token_lost(sp_session *sess);
static void spcb_connection_error(sp_session *session, sp_error error);
static void spcb_message_to_user(sp_session *session, const char *message);

void* notification_routine(void *s);

void rb_spotify_username_entry_focus_out_event_cb (GtkWidget *widget, RBSpotifyPlugin *spotify);
void rb_spotify_username_entry_activate_cb (GtkEntry *entry, RBSpotifyPlugin *spotify);
void rb_spotify_password_entry_focus_out_event_cb (GtkWidget *widget, RBSpotifyPlugin *spotify);
void rb_spotify_password_entry_activate_cb (GtkEntry *entry, RBSpotifyPlugin *spotify);

/**
 * This callback is called for log messages.
 *
 * @sa sp_session_callbacks#log_message
 */
static void log_message(sp_session *session, const char *data)
{
	fprintf(stderr, "log_message: %s\n", data);
}

static sp_session_callbacks session_callbacks = {
	.logged_in = &spcb_logged_in,
	.notify_main_thread = &spcb_notify_main_thread,
	.music_delivery = &spcb_music_delivery,
	.metadata_updated = &spcb_metadata_updated,
	.play_token_lost = &spcb_play_token_lost,
	.log_message = &log_message,
	.connection_error = &spcb_connection_error,
	.message_to_user = &spcb_message_to_user,
	
};

/**
 * The session configuration. Note that application_key_size is an external, so
 * we set it in main() instead.
 */
static sp_session_config spconfig = {
	.api_version = SPOTIFY_API_VERSION,
	.cache_location = "tmp",
	.settings_location = "tmp",
	.application_key = g_appkey,
	.application_key_size = 0, // Set in main()
	.user_agent = "testexamples",
	.callbacks = &session_callbacks,
	NULL,
};

G_MODULE_EXPORT GType register_rb_plugin (GTypeModule *module);




static void rb_spotify_plugin_init (RBSpotifyPlugin *plugin);
static void rb_spotify_plugin_finalize (GObject *object);
static void impl_activate (RBPlugin *plugin, RBShell *shell);
static void impl_deactivate (RBPlugin *plugin, RBShell *shell);
static GtkWidget* impl_create_configure_dialog (RBPlugin *plugin);

RB_PLUGIN_REGISTER(RBSpotifyPlugin, rb_spotify_plugin)
#define RB_SPOTIFY_PLUGIN_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), RBSPOTIFYPLUGIN_TYPE, RBSpotifyPluginPrivate))


static void
rb_spotify_plugin_class_init (RBSpotifyPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	RBPluginClass *plugin_class = RB_PLUGIN_CLASS (klass);

	object_class->finalize = rb_spotify_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->create_configure_dialog = impl_create_configure_dialog;
	
	g_type_class_add_private (object_class, sizeof (RBSpotifyPluginPrivate));
}

static void
rb_spotify_plugin_init (RBSpotifyPlugin *plugin)
{
	plugin->priv = RB_SPOTIFY_PLUGIN_GET_PRIVATE (plugin);

	rb_debug ("RBSpotifyPlugin initialising");
}

static void
rb_spotify_plugin_finalize (GObject *object)
{
/*
	RBSamplePlugin *plugin = RB_SAMPLE_PLUGIN (object);
*/
	rb_debug ("RBSpotifyPlugin finalising");

	G_OBJECT_CLASS (rb_spotify_plugin_parent_class)->finalize (object);
}



void* notification_routine(void *s)
{
     fprintf(stderr, "Start notification");
     sp_session *sp = (sp_session*)s;
     int next_timeout = 0;

     
     pthread_mutex_lock(&g_notify_mutex);
     
     for (;;) {
	  if (next_timeout == 0) {
	       while(!g_notify_do)
		    pthread_cond_wait(&g_notify_cond, &g_notify_mutex);
	  } else {
	       struct timespec ts;

	       clock_gettime(0, &ts);
	       
	       ts.tv_sec += next_timeout / 1000;
	       ts.tv_nsec += (next_timeout % 1000) * 1000000;
	       
	       pthread_cond_timedwait(&g_notify_cond, &g_notify_mutex, &ts);
	  }

	  g_notify_do = 0;
	  pthread_mutex_unlock(&g_notify_mutex);

	  do {
	       sp_session_process_events(sp, &next_timeout);
	  } while (next_timeout == 0);

	  pthread_mutex_lock(&g_notify_mutex);
     }
}

static void
impl_activate (RBPlugin *plugin,
	       RBShell *shell)
{
//	rb_error_dialog (NULL, _("Spotify Plugin"), "Spotify plugin activated, with shell %p", shell);

     
	RBSpotifySource *source;
	RhythmDBEntryType type;
	RhythmDB *db;
	char *entry_type_name, *username, *password;
	int err;
	RBSpotifyPluginPrivate *pprivate = RB_SPOTIFY_PLUGIN_GET_PRIVATE(plugin);
	
	pthread_mutex_init(&g_notify_mutex, NULL);
	pthread_cond_init(&g_notify_cond, NULL);

	audio_fifo_init(&g_audio_fifo);

	spconfig.application_key_size = g_appkey_size;
	err = sp_session_init(&spconfig, &pprivate->sess);

	
	if (err != SP_ERROR_OK) {
	     rb_error_dialog (NULL, _("Spotify Plugin"), "Error initialising spotify session");
	     pprivate->sess = NULL;
	     return;
	}
	fprintf(stderr, "err: %x", err);

	err = pthread_create(&pprivate->notify_thread, 0, notification_routine, pprivate->sess);
	fprintf(stderr, "Thread created");
	if (err != 0)
	{
	     fprintf(stderr, "Error creating notification thread %x\n", err);
	     return;
	}

	username = eel_gconf_get_string (CONF_SPOTIFY_USERNAME);
	password = eel_gconf_get_string (CONF_SPOTIFY_PASSWORD);
	if (username == NULL || password == NULL) {
	     rb_error_dialog (NULL, _("Spotify Plugin"), "Username and password not set.");
	     return;
	}
	
	err = sp_session_login(pprivate->sess, username, password);
	fprintf(stderr, "err: %x", err);

	rbspotifysrc_set_plugin(plugin);
	
	g_object_get (shell, "db", &db, NULL);
	entry_type_name = g_strdup_printf ("spotify");
	type = rhythmdb_entry_register_type (db, entry_type_name);
	g_free (entry_type_name);
	type->save_to_disk = FALSE;
	type->category = RHYTHMDB_ENTRY_NORMAL;
//	type->get_playback_uri = (RhythmDBEntryStringFunc) rb_daap_source_get_playback_uri;
	g_object_unref (db);

//	icon = rb_daap_plugin_get_icon (RB_DAAP_PLUGIN (plugin), password_protected, FALSE);
	source = (RBSpotifySource*)RB_SOURCE (g_object_new (RBSPOTIFYSOURCE_TYPE,
					  "name", "spotify",
					  "entry-type", type,
					  "shell", shell,
					  "visibility", TRUE,
//					  "sorting-key", CONF_STATE_SORTING,
					  "source-group", RB_SOURCE_GROUP_SHARED,
					  "plugin", RB_PLUGIN (plugin),
					  NULL));

	source->priv->sess = pprivate->sess;
	source->priv->db = db;
	source->priv->type = type;
	
	rb_shell_register_entry_type_for_source (shell, (RBSource*)source,	 type);

	rb_shell_append_source (shell, (RBSource*)source, NULL);

//	return source;
}

static void
impl_deactivate	(RBPlugin *plugin,
		 RBShell *shell)
{
     //rb_error_dialog (NULL, _("Spotify Plugin"), "Spotify plugin deactivated");
}

static void
preferences_response_cb (GtkWidget *dialog, gint response, RBPlugin *plugin)
{
	gtk_widget_hide (dialog);
}


void
rb_spotify_username_entry_focus_out_event_cb (GtkWidget *widget,
					      RBSpotifyPlugin *spotify)
{
     eel_gconf_set_string (CONF_SPOTIFY_USERNAME,
			   gtk_entry_get_text (GTK_ENTRY (widget)));
}

void
rb_spotify_username_entry_activate_cb (GtkEntry *entry,
				       RBSpotifyPlugin *spotify)
{
     RBSpotifyPluginPrivate *pprivate = RB_SPOTIFY_PLUGIN_GET_PRIVATE(spotify);
     gtk_widget_grab_focus (pprivate->password_entry);
}

void
rb_spotify_password_entry_focus_out_event_cb (GtkWidget *widget,
					      RBSpotifyPlugin *spotify)
{
     eel_gconf_set_string (CONF_SPOTIFY_PASSWORD,
			   gtk_entry_get_text (GTK_ENTRY (widget)));
}

/**
   knicked from audio scrobbler
*/
static GtkWidget*
impl_create_configure_dialog (RBPlugin *plugin)
{
     RBSpotifyPluginPrivate *pprivate = RB_SPOTIFY_PLUGIN_GET_PRIVATE(plugin);
     
     if (pprivate->preferences == NULL) {
	  char* t;
	  
	  if (pprivate->config_widget == NULL) {
	       GladeXML *xml;
	       char *gladefile;

	       gladefile = rb_plugin_find_file (plugin, "spotify-prefs.glade");
	       g_assert (gladefile != NULL);
	       xml = rb_glade_xml_new (gladefile, "spotify_vbox", plugin);
	       g_free (gladefile);

	       pprivate->config_widget = glade_xml_get_widget (xml, "spotify_vbox");
	       pprivate->username_entry = glade_xml_get_widget (xml, "username_entry");
	       pprivate->username_label = glade_xml_get_widget (xml, "username_label");
	       pprivate->password_entry = glade_xml_get_widget (xml, "password_entry");
	       pprivate->password_label = glade_xml_get_widget (xml, "password_label");
	
	       g_object_unref (G_OBJECT (xml));

	  }

	  t = eel_gconf_get_string (CONF_SPOTIFY_USERNAME);
	  gtk_entry_set_text (GTK_ENTRY (pprivate->username_entry),
			      t ? t : "");
	  t = eel_gconf_get_string (CONF_SPOTIFY_USERNAME);
	  gtk_entry_set_text (GTK_ENTRY (pprivate->password_entry),
			      t ? t : "");

	  pprivate->preferences = gtk_dialog_new_with_buttons (_("Spotify Preferences"),
							     NULL,
							     GTK_DIALOG_DESTROY_WITH_PARENT,
							     GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
							     NULL);
	  gtk_dialog_set_has_separator (GTK_DIALOG (pprivate->preferences), FALSE);
	  gtk_container_set_border_width (GTK_CONTAINER (pprivate->preferences), 5);
	  gtk_window_set_resizable (GTK_WINDOW (pprivate->preferences), FALSE);

	  g_signal_connect (G_OBJECT (pprivate->preferences),
			    "response",
			    G_CALLBACK (preferences_response_cb),
			    plugin);
	  gtk_widget_hide_on_delete (pprivate->preferences);
	  
	  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pprivate->preferences)->vbox), pprivate->config_widget);
     }
     
     gtk_widget_show_all (pprivate->preferences);
     return pprivate->preferences;
}

void spcb_logged_in(sp_session *sess, sp_error error)
{
     fprintf(stderr, "Spotify logged in\n");
}

void spcb_notify_main_thread(sp_session *sess)
{
     fprintf(stderr, "Spotify notify\n");
     pthread_mutex_lock(&g_notify_mutex);
     g_notify_do = 1;
     pthread_cond_signal(&g_notify_cond);
     pthread_mutex_unlock(&g_notify_mutex);
}


void spcb_metadata_updated(sp_session *sess)
{
     fprintf(stderr, "Spotify metadata updated\n");
}

void spcb_play_token_lost(sp_session *sess)
{
     fprintf(stderr, "Spotify play token lost\n");
}

void spcb_connection_error(sp_session *session, sp_error error)
{
     fprintf(stderr, "Spotify connection error %x\n", error);
}

void spcb_message_to_user(sp_session *session, const char *message)
{
     fprintf(stderr, "Spotify message to user %s\n", message);
}

#include "rb-spotify-source.h"


#include <string.h> /* For strlen */

#include <glib/gi18n-lib.h>


G_DEFINE_TYPE (RBSpotifySource, rbspotifysource, RB_TYPE_BROWSER_SOURCE);

void rbspotifysource_search(RBSource *source, RBSourceSearch *search, const char *cur_text, const char *new_text);

void 	rbspotify_search_complete_cb (sp_search *result, void *userdata);

static void rbspotifysource_init(RBSpotifySource *self)
{
     self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
					       RBSPOTIFYSOURCE_TYPE,
					       RBSpotifySourcePrivate);
     self->priv->sess = NULL;
     self->priv->db = NULL;
}

static void rbspotifysource_class_init (RBSpotifySourceClass *klass)
{
     //GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  //gobject_class->dispose = maman_bar_dispose;
  //gobject_class->finalize = maman_bar_finalize;
     RBSourceClass *source_class = RB_SOURCE_CLASS (klass);
     g_type_class_add_private (klass, sizeof (RBSpotifySourcePrivate));

     source_class->impl_search = rbspotifysource_search;
}

static void
entry_set_string_prop (RhythmDB        *db,
		       RhythmDBEntry   *entry,
		       RhythmDBPropType propid,
		       const char      *str)
{
	GValue value = {0,};
	const gchar *tmp;

	if (str == NULL || *str == '\0' || !g_utf8_validate (str, -1, NULL)) {
		tmp = _("Unknown");
	} else {
		tmp = str;
	}

	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, tmp);
	rhythmdb_entry_set (RHYTHMDB (db), entry, propid, &value);
	g_value_unset (&value);
}

void rbspotify_search_complete_cb (sp_search *result, void *userdata)
{
     int i =0;
     RBSpotifySource *self = RBSPOTIFYSOURCE (userdata);
     fprintf(stderr, "Result callback\n");
     if (!result) return;
     
     if (result && SP_ERROR_OK == sp_search_error(result))
     {
	  rhythmdb_entry_delete_by_type(self->priv->db, self->priv->type);
	  rhythmdb_commit (self->priv->db);
	  fprintf(stderr, "Num tracks: %x\n", sp_search_num_tracks(result));
	  for (i = 0; i < sp_search_num_tracks(result) && i < 200; ++i)
	  {
	       sp_track* track = sp_search_track(result, i);
	       sp_link* link = sp_link_create_from_track(track, 0);
	       char lstr[100];
	       int duration = sp_track_duration(track);

	       RhythmDBEntry *entry = NULL;
	       sp_link_as_string(link, lstr, 100);
	       entry = rhythmdb_entry_new (self->priv->db, self->priv->type, lstr);
	       
	       entry_set_string_prop (self->priv->db, entry, RHYTHMDB_PROP_TITLE, sp_track_name(track));
	       
	       /* album */
	       entry_set_string_prop (self->priv->db, entry, RHYTHMDB_PROP_ALBUM, sp_album_name(sp_track_album(track)));
	       
	       /* artist */
	       entry_set_string_prop (self->priv->db, entry, RHYTHMDB_PROP_ARTIST, sp_artist_name(sp_track_artist(track, 0)));
	       
	       /* genre */
	       //entry_set_string_prop (self->priv->db, entry, RHYTHMDB_PROP_GENRE, sp_track_);
	       
	       GValue value = {0,};
	       /* length */
	       g_value_init (&value, G_TYPE_ULONG);
	       g_value_set_ulong (&value,(gulong) duration/1000);
	       rhythmdb_entry_set (self->priv->db, entry, RHYTHMDB_PROP_DURATION, &value);
	       g_value_unset (&value);
	  }
	  rhythmdb_commit (self->priv->db);
     }
     sp_search_release(result);
}

void rbspotifysource_search(RBSource *source, RBSourceSearch *search, const char *cur_text, const char *new_text)
{
     fprintf(stderr, "cur: %s new: %s\n", cur_text, new_text);
     RBSpotifySource *self = RBSPOTIFYSOURCE (source);

     if (self == NULL || self->priv->sess == NULL || cur_text == NULL)
	  return;
     /*sp_search *s =*/ sp_search_create(self->priv->sess, cur_text, 0, 100, rbspotify_search_complete_cb, self);
}


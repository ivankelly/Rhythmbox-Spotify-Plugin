/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Copyright (C) 2002 Jorn Baayen <jorn@nl.linux.org>
 *  Copyright (C) 2003 Colin Walters <walters@gnome.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <config.h>
#include <gtk/gtk.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libxml/tree.h>

#include "rb-auto-playlist-source.h"
#include "rb-util.h"
#include "rb-debug.h"
#include "rb-stock-icons.h"
#include "rb-playlist-xml.h"

static GObject *rb_auto_playlist_source_constructor (GType type, guint n_construct_properties,
						      GObjectConstructParam *construct_properties);
/* source methods */
static GdkPixbuf *impl_get_pixbuf (RBSource *source);
static gboolean impl_show_popup (RBSource *source);
static gboolean impl_receive_drag (RBSource *asource, GtkSelectionData *data);

/* playlist methods */
static void impl_save_contents_to_xml (RBPlaylistSource *source,
				       xmlNodePtr node);

static void rb_auto_playlist_source_songs_sort_order_changed_cb (RBEntryView *view, 
								 RBAutoPlaylistSource *source); 

#define AUTO_PLAYLIST_SOURCE_POPUP_PATH "/AutoPlaylistSourcePopup"

typedef struct _RBAutoPlaylistSourcePrivate RBAutoPlaylistSourcePrivate;

struct _RBAutoPlaylistSourcePrivate
{
	gboolean query_resetting;
};

G_DEFINE_TYPE (RBAutoPlaylistSource, rb_auto_playlist_source, RB_TYPE_PLAYLIST_SOURCE)
#define RB_AUTO_PLAYLIST_SOURCE_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), RB_TYPE_AUTO_PLAYLIST_SOURCE, RBAutoPlaylistSourcePrivate))

static void
rb_auto_playlist_source_class_init (RBAutoPlaylistSourceClass *klass)
{
	gint size;

	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	RBSourceClass *source_class = RB_SOURCE_CLASS (klass);
	RBPlaylistSourceClass *playlist_class = RB_PLAYLIST_SOURCE_CLASS (klass);

	object_class->constructor = rb_auto_playlist_source_constructor;

	source_class->impl_get_pixbuf = impl_get_pixbuf;
	source_class->impl_can_cut = (RBSourceFeatureFunc) rb_false_function;
	source_class->impl_can_delete = (RBSourceFeatureFunc) rb_false_function;
	source_class->impl_receive_drag = impl_receive_drag;
	source_class->impl_show_popup = impl_show_popup;

	playlist_class->impl_save_contents_to_xml = impl_save_contents_to_xml;
	
	gtk_icon_size_lookup (GTK_ICON_SIZE_LARGE_TOOLBAR, &size, NULL);
	klass->pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
						  GNOME_MEDIA_AUTO_PLAYLIST,
						  size,
						  0, NULL);
	
	g_type_class_add_private (klass, sizeof (RBAutoPlaylistSourcePrivate));
}

static void
rb_auto_playlist_source_init (RBAutoPlaylistSource *source)
{
}

static GObject *
rb_auto_playlist_source_constructor (GType type, guint n_construct_properties,
				      GObjectConstructParam *construct_properties)
{
	RBEntryView *songs;
	RBAutoPlaylistSource *source;
	GObjectClass *parent_class = G_OBJECT_CLASS (rb_auto_playlist_source_parent_class);

	source = RB_AUTO_PLAYLIST_SOURCE (
			parent_class->constructor (type, n_construct_properties, construct_properties));

	songs = rb_source_get_entry_view (RB_SOURCE (source));
	g_signal_connect_object (G_OBJECT (songs), "sort-order-changed",
				 G_CALLBACK (rb_auto_playlist_source_songs_sort_order_changed_cb), source, 0);
	
	return G_OBJECT (source);
}

RBSource *
rb_auto_playlist_source_new (RBShell *shell, const char *name, gboolean local)
{
	if (name == NULL)
		name = "";

	return RB_SOURCE (g_object_new (RB_TYPE_AUTO_PLAYLIST_SOURCE,
					"name", name,
					"shell", shell,
					"is-local", local,
					"entry-type", RHYTHMDB_ENTRY_TYPE_SONG,
					NULL));
}

RBSource *	
rb_auto_playlist_source_new_from_xml (RBShell *shell, xmlNodePtr node)
{
	RBAutoPlaylistSource *source = RB_AUTO_PLAYLIST_SOURCE (rb_auto_playlist_source_new (shell, NULL, TRUE));

	xmlNodePtr child;
	xmlChar *tmp;
	GPtrArray *query;
	gint limit_count = 0, limit_mb = 0, limit_time = 0;
	gchar *sort_key = NULL;
	gint sort_direction = 0;

	child = node->children;
	while (xmlNodeIsText (child))
		child = child->next;

	query = rhythmdb_query_deserialize (rb_playlist_source_get_db (RB_PLAYLIST_SOURCE (source)), 
					    child);

	tmp = xmlGetProp (node, RB_PLAYLIST_LIMIT_COUNT);
	if (!tmp) /* Backwards compatibility */
		tmp = xmlGetProp (node, RB_PLAYLIST_LIMIT);
	if (tmp) {
		limit_count = atoi ((char*) tmp);
		g_free (tmp);
	}
	tmp = xmlGetProp (node, RB_PLAYLIST_LIMIT_SIZE);
	if (tmp) {
		limit_mb = atoi ((char*) tmp);
		g_free (tmp);
	}
	tmp = xmlGetProp (node, RB_PLAYLIST_LIMIT_TIME);
	if (tmp) {
		limit_time = atoi ((char*) tmp);
		g_free (tmp);
	}

	sort_key = (gchar*) xmlGetProp (node, RB_PLAYLIST_SORT_KEY);
	if (sort_key && *sort_key) {
		tmp = xmlGetProp (node, RB_PLAYLIST_SORT_DIRECTION);
		if (tmp) {
			sort_direction = atoi ((char*) tmp);
			g_free (tmp);
		}
	} else {
		g_free (sort_key);
		sort_key = NULL;
		sort_direction = 0;
	}

	rb_auto_playlist_source_set_query (source, query,
					    limit_count,
					    limit_mb,
					    limit_time,
					    sort_key,
					    sort_direction);
	g_free (sort_key);

	return RB_SOURCE (source);
}

static GdkPixbuf *
impl_get_pixbuf (RBSource *asource)
{
	RBAutoPlaylistSourceClass *klass = RB_AUTO_PLAYLIST_SOURCE_GET_CLASS (asource);
	return klass->pixbuf;
}

static gboolean
impl_show_popup (RBSource *source)
{
	_rb_source_show_popup (source, AUTO_PLAYLIST_SOURCE_POPUP_PATH);
	return TRUE;
}

static RhythmDBPropType
rb_auto_playlist_source_drag_atom_to_prop (GdkAtom smasher)
{
	if (smasher == gdk_atom_intern ("text/x-rhythmbox-album", TRUE))
		return RHYTHMDB_PROP_ALBUM;
	else if (smasher == gdk_atom_intern ("text/x-rhythmbox-artist", TRUE))
		return RHYTHMDB_PROP_ARTIST;
	else if (smasher == gdk_atom_intern ("text/x-rhythmbox-genre", TRUE))
		return RHYTHMDB_PROP_GENRE;
	else {
		g_assert_not_reached ();
		return 0;
	}
}

static gboolean
impl_receive_drag (RBSource *asource, GtkSelectionData *data)
{
	RBAutoPlaylistSource *source = RB_AUTO_PLAYLIST_SOURCE (asource);
        
	GPtrArray *subquery = NULL;
	gchar **names;
	guint propid;
	int i;
	RhythmDB *db;

	/* ignore URI lists */
	if (data->type == gdk_atom_intern ("text/uri-list", TRUE))
		return TRUE;

	names = g_strsplit ((char *)data->data, "\r\n", 0);
	propid = rb_auto_playlist_source_drag_atom_to_prop (data->type);
	g_object_get (G_OBJECT (asource), "db", &db, NULL);

	for (i=0; names[i]; i++) {
		if (subquery == NULL) 
			subquery = rhythmdb_query_parse (db,
							 RHYTHMDB_QUERY_PROP_EQUALS,
							 propid,
							 names[i],
							 RHYTHMDB_QUERY_END);
		else
			rhythmdb_query_append (db,
					       subquery,
					       RHYTHMDB_QUERY_DISJUNCTION,
					       RHYTHMDB_QUERY_PROP_EQUALS,
					       propid,
					       names[i],
					       RHYTHMDB_QUERY_END);
	}

	g_strfreev (names);

	if (subquery) {
		RhythmDBEntryType qtype;
		GPtrArray *query;
		
		g_object_get (G_OBJECT (source), "entry-type", &qtype, NULL);
		if (qtype == -1)
			qtype = RHYTHMDB_ENTRY_TYPE_SONG;

		query = rhythmdb_query_parse (db,
					      RHYTHMDB_QUERY_PROP_EQUALS,
					      RHYTHMDB_PROP_TYPE,
					      qtype,
					      RHYTHMDB_QUERY_SUBQUERY,
					      subquery,
					      RHYTHMDB_QUERY_END);
		rb_auto_playlist_source_set_query (RB_AUTO_PLAYLIST_SOURCE (source), query, 0, 0, 0, NULL, 0);
	}

	g_object_unref (G_OBJECT (db));

	return TRUE;
}

static void 
impl_save_contents_to_xml (RBPlaylistSource *psource, xmlNodePtr node)
{
	GPtrArray *query;
	guint max_count, max_size_mb, max_time;
	const gchar *sort_key;
	gint sort_direction;
	gchar *temp_str;
	RBAutoPlaylistSource *source = RB_AUTO_PLAYLIST_SOURCE (psource);

	xmlSetProp (node, RB_PLAYLIST_TYPE, RB_PLAYLIST_AUTOMATIC);

	rb_auto_playlist_source_get_query (source,
					    &query,
					    &max_count, &max_size_mb, &max_time,
					    &sort_key, &sort_direction);
	temp_str = g_strdup_printf ("%d", max_count);
	xmlSetProp (node, RB_PLAYLIST_LIMIT_COUNT, BAD_CAST temp_str);
	g_free (temp_str);
	temp_str = g_strdup_printf ("%d", max_size_mb);
	xmlSetProp (node, RB_PLAYLIST_LIMIT_SIZE, BAD_CAST temp_str);
	g_free (temp_str);
	temp_str = g_strdup_printf ("%d", max_time);
	xmlSetProp (node, RB_PLAYLIST_LIMIT_TIME, BAD_CAST temp_str);
	g_free (temp_str);

	if (sort_key && *sort_key) {
		xmlSetProp (node, RB_PLAYLIST_SORT_KEY, BAD_CAST sort_key);
		temp_str = g_strdup_printf ("%d", sort_direction);
		xmlSetProp (node, RB_PLAYLIST_SORT_DIRECTION, BAD_CAST temp_str);
		g_free (temp_str);
	}

	rhythmdb_query_serialize (rb_playlist_source_get_db (psource), query, node);
}

static void
rb_auto_playlist_source_do_query (RBAutoPlaylistSource *source,
				   GPtrArray *query,
				   guint limit_count,
				   guint limit_mb,
				   guint limit_time)
{
	RhythmDBQueryModel *model;
	RBPlaylistSource *psource = RB_PLAYLIST_SOURCE (source);
	RhythmDB *db = rb_playlist_source_get_db (psource);

	model = g_object_new (RHYTHMDB_TYPE_QUERY_MODEL,
			      "db", db,
			      "max-count", limit_count,
			      "max-size", limit_mb, 
			      "max-time", limit_time, 
			      NULL);

	rhythmdb_do_full_query_async_parsed (db, GTK_TREE_MODEL (model), query);
	rb_playlist_source_set_query_model (psource, model);
}

void
rb_auto_playlist_source_set_query (RBAutoPlaylistSource *source,
				    GPtrArray *query,
				    guint limit_count,
				    guint limit_mb,
				    guint limit_time,
				    const char *sort_key,
				    gint sort_direction)
{
	RBAutoPlaylistSourcePrivate *priv = RB_AUTO_PLAYLIST_SOURCE_GET_PRIVATE (source);
	RBEntryView *songs = rb_source_get_entry_view (RB_SOURCE (source));

	priv->query_resetting = TRUE;

	/* playlists that aren't limited, with a particular sort order, are user-orderable */
	rb_entry_view_set_columns_clickable (songs, (limit_count == 0 && limit_mb == 0));
	rb_entry_view_set_sorting_order (songs, sort_key, sort_direction);

	rb_auto_playlist_source_do_query (source, query, limit_count, limit_mb, limit_time);
	rhythmdb_query_free (query);
	priv->query_resetting = FALSE;
}

void
rb_auto_playlist_source_get_query (RBAutoPlaylistSource *source,
				    GPtrArray **query,
				    guint *limit_count,
				    guint *limit_mb,
				    guint *limit_time,
				    const char **sort_key,
				    gint *sort_direction)
{
	RBEntryView *songs = rb_source_get_entry_view (RB_SOURCE (source));
	RhythmDBQueryModel *model = rb_playlist_source_get_query_model (RB_PLAYLIST_SOURCE (source));

	g_object_get (G_OBJECT (model),
		      "query", query,
		      "max-count", limit_count,
		      "max-size", limit_mb,
		      "max-time", limit_time,
		      NULL);

	rb_entry_view_get_sorting_order (songs, sort_key, sort_direction);
}

static void
rb_auto_playlist_source_songs_sort_order_changed_cb (RBEntryView *view, RBAutoPlaylistSource *source)
{
	RBAutoPlaylistSourcePrivate *priv = RB_AUTO_PLAYLIST_SOURCE_GET_PRIVATE (source);

	/* don't process this if we are in the middle of setting a query */
	if (priv->query_resetting)
		return;
	rb_debug ("sort order changed");

	rb_entry_view_resort_model (view);
}

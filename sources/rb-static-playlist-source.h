/*
 *  Copyright (C) 2002 Jorn Baayen <jorn@nl.linux.org>
 *  Copyright (C) 2003,2004 Colin Walters <walters@redhat.com>
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

/* 
 * Header for static playlist class
 */

#ifndef __RB_STATIC_PLAYLIST_SOURCE_H
#define __RB_STATIC_PLAYLIST_SOURCE_H

#include "rb-playlist-source.h"
#include "rhythmdb.h"

G_BEGIN_DECLS

#define RB_TYPE_STATIC_PLAYLIST_SOURCE         (rb_static_playlist_source_get_type ())
#define RB_STATIC_PLAYLIST_SOURCE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), RB_TYPE_STATIC_PLAYLIST_SOURCE, RBStaticPlaylistSource))
#define RB_STATIC_PLAYLIST_SOURCE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), RB_TYPE_STATIC_PLAYLIST_SOURCE, RBStaticPlaylistSourceClass))
#define RB_IS_STATIC_PLAYLIST_SOURCE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), RB_TYPE_STATIC_PLAYLIST_SOURCE))
#define RB_IS_STATIC_PLAYLIST_SOURCE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), RB_TYPE_STATIC_PLAYLIST_SOURCE))
#define RB_STATIC_PLAYLIST_SOURCE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), RB_TYPE_STATIC_PLAYLIST_SOURCE, RBStaticPlaylistSourceClass))

typedef struct
{
	RBPlaylistSource parent;
} RBStaticPlaylistSource;

typedef struct
{
	RBPlaylistSourceClass parent;
	GdkPixbuf *pixbuf;
} RBStaticPlaylistSourceClass;

GType		rb_static_playlist_source_get_type 	(void);

RBSource *	rb_static_playlist_source_new		(RBShell *shell,
							 const char *name,
							 gboolean local,
							 RhythmDBEntryType entry_type);

RBSource *	rb_static_playlist_source_new_from_xml	(RBShell *shell,
							 xmlNodePtr node);

void		rb_static_playlist_source_add_entry	(RBStaticPlaylistSource *source,
						 	 RhythmDBEntry *entry);

void		rb_static_playlist_source_remove_entry	(RBStaticPlaylistSource *source,
							 RhythmDBEntry *entry);

void		rb_static_playlist_source_add_location	(RBStaticPlaylistSource *source,
							 const char *location);

void            rb_static_playlist_source_add_locations (RBStaticPlaylistSource *source,
							 GList *locations);

void		rb_static_playlist_source_remove_location(RBStaticPlaylistSource *source,
						 	 const char *location);

G_END_DECLS

#endif /* __RB_STATIC_PLAYLIST_SOURCE_H */

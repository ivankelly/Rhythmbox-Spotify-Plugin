/*
 *  arch-tag: Header for Nokia 770 source object
 *
 *  Copyright (C) 2006 James Livingston  <doclivingston@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  The Rhythmbox authors hereby grant permission for non-GPL compatible
 *  GStreamer plugins to be used and distributed together with GStreamer
 *  and Rhythmbox. This permission is above and beyond the permissions granted
 *  by the GPL license by which Rhythmbox is covered. If you modify this code
 *  you may extend this exception to your version of the code, but you are not
 *  obligated to do so. If you do not wish to do so, delete this exception
 *  statement from your version.
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

#ifndef __RB_NOKIA770_SOURCE_H
#define __RB_NOKIA770_SOURCE_H

#include "rb-shell.h"
#include "rb-generic-player-source.h"
#include "rhythmdb.h"

G_BEGIN_DECLS

#define RB_TYPE_NOKIA770_SOURCE         (rb_nokia770_source_get_type ())
#define RB_NOKIA770_SOURCE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), RB_TYPE_NOKIA770_SOURCE, RBNokia770Source))
#define RB_NOKIA770_SOURCE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), RB_TYPE_NOKIA770_SOURCE, RBNokia770SourceClass))
#define RB_IS_NOKIA770_SOURCE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), RB_TYPE_NOKIA770_SOURCE))
#define RB_IS_NOKIA770_SOURCE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), RB_TYPE_NOKIA770_SOURCE))
#define RB_NOKIA770_SOURCE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), RB_TYPE_NOKIA770_SOURCE, RBNokia770SourceClass))

typedef struct
{
	RBGenericPlayerSource parent;
} RBNokia770Source;

typedef struct
{
	RBGenericPlayerSourceClass parent;
} RBNokia770SourceClass;

RBRemovableMediaSource *	rb_nokia770_source_new		(RBShell *shell, GMount *mount);
GType				rb_nokia770_source_get_type	(void);
GType				rb_nokia770_source_register_type (GTypeModule *module);

gboolean			rb_nokia770_is_mount_player	(GMount *mount);

G_END_DECLS

#endif /* __RB_NOKIA770_SOURCE_H */

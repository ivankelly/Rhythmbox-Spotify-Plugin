/*
 *  arch-tag: Header for Rhythmbox removable media management object
 *
 *  Copyright (C) 2005 James Livingston <jrl@ids.org.au>
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

#ifndef __RB_REMOVABLE_MEDIA_MANAGER_H
#define __RB_REMOVABLE_MEDIA_MANAGER_H

#include "rb-source.h"
#include "rhythmdb.h"
#include "rb-shell.h"
#include "rb-sourcelist.h"

G_BEGIN_DECLS

#define RB_TYPE_REMOVABLE_MEDIA_MANAGER         (rb_removable_media_manager_get_type ())
#define RB_REMOVABLE_MEDIA_MANAGER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), RB_TYPE_REMOVABLE_MEDIA_MANAGER, RBRemovableMediaManager))
#define RB_REMOVABLE_MEDIA_MANAGER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), RB_TYPE_REMOVABLE_MEDIA_MANAGER, RBRemovableMediaManagerClass))
#define RB_IS_REMOVABLE_MEDIA_MANAGER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), RB_TYPE_REMOVABLE_MEDIA_MANAGER))
#define RB_IS_REMOVABLE_MEDIA_MANAGER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), RB_TYPE_REMOVABLE_MEDIA_MANAGER))
#define RB_REMOVABLE_MEDIA_MANAGER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), RB_TYPE_REMOVABLE_MEDIA_MANAGER, RBRemovableMediaManagerClass))

typedef struct
{
	GObject parent;
} RBRemovableMediaManager;

typedef struct
{
	GObjectClass parent_class;

	/* signals */
	void	(*medium_added) (RBSource *source);
} RBRemovableMediaManagerClass;


RBRemovableMediaManager* rb_removable_media_manager_new		(RBShell *shell, RBSourceList *sourcelist);
GType			rb_removable_media_manager_get_type	(void);

gboolean		rb_removable_media_manager_load_media	(RBRemovableMediaManager *mgr);

G_END_DECLS

#endif /* __RB_REMOVABLE_MEDIA_MANAGER_H */
/*
 *  arch-tag: Header file for Rhythmbox Audioscrobbler support
 *
 *  Copyright (C) 2005 Alex Revo <xiphoiadappendix@gmail.com>
 *					   Ruben Vermeersch <ruben@Lambda1.be>
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

#ifndef __RB_AUDIOSCROBBLER_H
#define __RB_AUDIOSCROBBLER_H

G_BEGIN_DECLS

#include <glib.h>

#include "eel-gconf-extensions.h"
#include "rhythmdb.h"
#include "rb-shell.h"
#include "rb-shell-player.h"
#include "rb-source.h"

#define RB_TYPE_AUDIOSCROBBLER			(rb_audioscrobbler_get_type ())
#define RB_AUDIOSCROBBLER(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), RB_TYPE_AUDIOSCROBBLER, RBAudioscrobbler))
#define RB_AUDIOSCROBBLER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), RB_TYPE_AUDIOSCROBBLER, RBAudioscrobblerClass))
#define RB_IS_AUDIOSCROBBLER(o)			(G_TYPE_CHECK_INSTANCE_TYPE ((o), RB_TYPE_AUDIOSCROBBLER))
#define RB_IS_AUDIOSCROBBLER_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE ((k), RB_TYPE_AUDIOSCROBBLER))
#define RB_AUDIOSCROBBLER_GET_CLASS(o)		(G_TYPE_INSTANCE_GET_CLASS ((o), RB_TYPE_AUDIOSCROBBLER, RBAudioscrobblerClass))


typedef struct _RBAudioscrobblerPrivate RBAudioscrobblerPrivate;

typedef struct
{
	GObject parent;

	RBAudioscrobblerPrivate *priv;
} RBAudioscrobbler;

typedef struct
{
	GObjectClass parent_class;
} RBAudioscrobblerClass;


GType			rb_audioscrobbler_get_type (void);

RBAudioscrobbler *	rb_audioscrobbler_new (RBShellPlayer *shell_player);

GtkWidget *		rb_audioscrobbler_get_config_widget (RBAudioscrobbler *audioscrobbler);

void			rb_audioscrobbler_username_entry_changed_cb (GtkEntry *entry,
								     RBAudioscrobbler *audioscrobbler);
void			rb_audioscrobbler_username_entry_activate_cb (GtkEntry *entry,
								      RBAudioscrobbler *audioscrobbler);

void			rb_audioscrobbler_password_entry_changed_cb (GtkEntry *entry,
								     RBAudioscrobbler *audioscrobbler);
void			rb_audioscrobbler_password_entry_activate_cb (GtkEntry *entry,
								      RBAudioscrobbler *audioscrobbler);

void			rb_audioscrobbler_enabled_check_changed_cb (GtkCheckButton *button,
								    RBAudioscrobbler *audioscrobbler);


G_END_DECLS

#endif
/* 
 *  Copyright (C) 2003 Jeffrey Yasskin <jyasskin@mail.utexas.edu>
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

#ifndef __RB_PLAY_ORDER_QUEUE_H
#define __RB_PLAY_ORDER_QUEUE_H

#include "rb-play-order.h"

#include "rb-shell-player.h"

G_BEGIN_DECLS

#define RB_TYPE_QUEUE_PLAY_ORDER         (rb_queue_play_order_get_type ())
#define RB_QUEUE_PLAY_ORDER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), RB_TYPE_QUEUE_PLAY_ORDER, RBQueuePlayOrder))
#define RB_QUEUE_PLAY_ORDER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), RB_TYPE_QUEUE_PLAY_ORDER, RBQueuePlayOrderClass))
#define RB_IS_QUEUE_PLAY_ORDER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), RB_TYPE_QUEUE_PLAY_ORDER))
#define RB_IS_QUEUE_PLAY_ORDER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), RB_TYPE_QUEUE_PLAY_ORDER))
#define RB_QUEUE_PLAY_ORDER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), RB_TYPE_QUEUE_PLAY_ORDER, RBQueuePlayOrderClass))

typedef struct
{
	RBPlayOrder parent;
} RBQueuePlayOrder;

typedef struct
{
	RBPlayOrderClass parent_class;
} RBQueuePlayOrderClass;

GType			rb_queue_play_order_get_type	(void);

RBPlayOrder *		rb_queue_play_order_new	(RBShellPlayer *player);

G_END_DECLS

#endif /* __RB_PLAY_ORDER_QUEUE_H */
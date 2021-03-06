/*
 *  arch-tag: Header for linear, looping navigation method
 *
 *  Copyright (C) 2003 Jeffrey Yasskin <jyasskin@mail.utexas.edu>
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

#ifndef __RB_PLAY_ORDER_LINEAR_LOOP_H
#define __RB_PLAY_ORDER_LINEAR_LOOP_H

#include "rb-play-order.h"

#include "rb-shell-player.h"

G_BEGIN_DECLS

#define RB_TYPE_LINEAR_PLAY_ORDER_LOOP         (rb_linear_play_order_loop_get_type ())
#define RB_LINEAR_PLAY_ORDER_LOOP(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), RB_TYPE_LINEAR_PLAY_ORDER_LOOP, RBLinearPlayOrderLoop))
#define RB_LINEAR_PLAY_ORDER_LOOP_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), RB_TYPE_LINEAR_PLAY_ORDER_LOOP, RBLinearPlayOrderLoopClass))
#define RB_IS_LINEAR_PLAY_ORDER_LOOP(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), RB_TYPE_LINEAR_PLAY_ORDER_LOOP))
#define RB_IS_LINEAR_PLAY_ORDER_LOOP_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), RB_TYPE_LINEAR_PLAY_ORDER_LOOP))
#define RB_LINEAR_PLAY_ORDER_LOOP_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), RB_TYPE_LINEAR_PLAY_ORDER_LOOP, RBLinearPlayOrderLoopClass))

typedef struct
{
	RBPlayOrder parent;
} RBLinearPlayOrderLoop;

typedef struct
{
	RBPlayOrderClass parent_class;
} RBLinearPlayOrderLoopClass;

GType			rb_linear_play_order_loop_get_type	(void);

RBPlayOrder *		rb_linear_play_order_loop_new	(RBShellPlayer *player);

G_END_DECLS

#endif /* __RB_PLAY_ORDER_LINEAR_LOOP_H */

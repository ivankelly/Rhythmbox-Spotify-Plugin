/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 *  arch-tag: Implementation of rating renderer object
 *
 *  Copyright (C) 2002 Olivier Martin <olive.martin@gmail.com>
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

#include "config.h"

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "rb-rating.h"
#include "rb-rating-helper.h"
#include "rb-stock-icons.h"
#include "rb-cut-and-paste-code.h"

/* Offset at the beggining of the widget */
#define X_OFFSET 0

/* Vertical offset */
#define Y_OFFSET 2

static void rb_rating_class_init (RBRatingClass *class);
static void rb_rating_init (RBRating *label);
static void rb_rating_finalize (GObject *object);
static void rb_rating_get_property (GObject *object,
				    guint param_id,
				    GValue *value,
				    GParamSpec *pspec);
static void rb_rating_set_property (GObject *object,
				    guint param_id,
				    const GValue *value,
				    GParamSpec *pspec);
static void rb_rating_realize (GtkWidget *widget);
static void rb_rating_size_request (GtkWidget *widget,
				    GtkRequisition *requisition);
static gboolean rb_rating_expose (GtkWidget *widget,
				  GdkEventExpose *event);
static gboolean rb_rating_focus (GtkWidget *widget, GtkDirectionType direction);
static gboolean rb_rating_set_rating_cb (RBRating *rating, gdouble score);
static gboolean rb_rating_adjust_rating_cb (RBRating *rating, gdouble adjust);
static gboolean rb_rating_button_press_cb (GtkWidget *widget,
					   GdkEventButton *event);

struct _RBRatingPrivate
{
	double rating;
	RBRatingPixbufs *pixbufs;
};

G_DEFINE_TYPE (RBRating, rb_rating, GTK_TYPE_WIDGET)
#define RB_RATING_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), RB_TYPE_RATING, RBRatingPrivate))

/**
 * SECTION:rb-rating
 * @short_description: widget for displaying song ratings
 *
 * This widget displays a rating (0-5 stars) and allows the user to
 * alter the rating by clicking.
 */

enum
{
	PROP_0,
	PROP_RATING
};

enum
{
	RATED,
	SET_RATING,
	ADJUST_RATING,
	LAST_SIGNAL
};

static guint rb_rating_signals[LAST_SIGNAL] = { 0 };

static void
rb_rating_class_init (RBRatingClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class;
	GtkBindingSet *binding_set;

	widget_class = (GtkWidgetClass*) klass;

	object_class->finalize = rb_rating_finalize;
	object_class->get_property = rb_rating_get_property;
	object_class->set_property = rb_rating_set_property;

	widget_class->realize = rb_rating_realize;
	widget_class->expose_event = rb_rating_expose;
	widget_class->size_request = rb_rating_size_request;
	widget_class->button_press_event = rb_rating_button_press_cb;
	widget_class->focus = rb_rating_focus;

	klass->set_rating = rb_rating_set_rating_cb;
	klass->adjust_rating = rb_rating_adjust_rating_cb;

	/**
	 * RBRating:rating:
	 *
	 * The rating displayed in the widget, as a floating point value
	 * between 0.0 and 5.0.
	 */
	rb_rating_install_rating_property (object_class, PROP_RATING);

	/**
	 * RBRating::rated:
	 * @rating: the #RBRating
	 * @score: the new rating
	 *
	 * Emitted when the user changes the rating.
	 */
	rb_rating_signals[RATED] =
		g_signal_new ("rated",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (RBRatingClass, rated),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__DOUBLE,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_DOUBLE);
	rb_rating_signals[SET_RATING] =
		g_signal_new ("set-rating",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (RBRatingClass, set_rating),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__DOUBLE,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_DOUBLE);
	rb_rating_signals[ADJUST_RATING] =
		g_signal_new ("adjust-rating",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (RBRatingClass, adjust_rating),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__DOUBLE,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_DOUBLE);

	binding_set = gtk_binding_set_by_class (klass);
	gtk_binding_entry_add_signal (binding_set, GDK_Home, 0, "set-rating", 1, G_TYPE_DOUBLE, 0.0);
	gtk_binding_entry_add_signal (binding_set, GDK_End, 0, "set-rating", 1, G_TYPE_DOUBLE, (double)RB_RATING_MAX_SCORE);

	gtk_binding_entry_add_signal (binding_set, GDK_equal, 0, "adjust-rating", 1, G_TYPE_DOUBLE, 1.0);
	gtk_binding_entry_add_signal (binding_set, GDK_plus, 0, "adjust-rating", 1, G_TYPE_DOUBLE, 1.0);
	gtk_binding_entry_add_signal (binding_set, GDK_KP_Add, 0, "adjust-rating", 1, G_TYPE_DOUBLE, 1.0);
	gtk_binding_entry_add_signal (binding_set, GDK_Right, 0, "adjust-rating", 1, G_TYPE_DOUBLE, 1.0);
	gtk_binding_entry_add_signal (binding_set, GDK_KP_Right, 0, "adjust-rating", 1, G_TYPE_DOUBLE, 1.0);
	
	gtk_binding_entry_add_signal (binding_set, GDK_minus, 0, "adjust-rating", 1, G_TYPE_DOUBLE, -1.0);
	gtk_binding_entry_add_signal (binding_set, GDK_KP_Subtract, 0, "adjust-rating", 1, G_TYPE_DOUBLE, -1.0);
	gtk_binding_entry_add_signal (binding_set, GDK_Left, 0, "adjust-rating", 1, G_TYPE_DOUBLE, -1.0);
	gtk_binding_entry_add_signal (binding_set, GDK_KP_Left, 0, "adjust-rating", 1, G_TYPE_DOUBLE, -1.0);
	
	g_type_class_add_private (klass, sizeof (RBRatingPrivate));
}

static void
rb_rating_init (RBRating *rating)
{
	rating->priv = RB_RATING_GET_PRIVATE (rating);

	/* create the needed icons */
	rating->priv->pixbufs = rb_rating_pixbufs_new ();
	
	rb_rating_set_accessible_name (GTK_WIDGET (rating), 0.0);
}

static void
rb_rating_finalize (GObject *object)
{
	RBRating *rating;

	rating = RB_RATING (object);

	if (rating->priv->pixbufs != NULL) {
		rb_rating_pixbufs_free (rating->priv->pixbufs);
	}

	G_OBJECT_CLASS (rb_rating_parent_class)->finalize (object);
}

static void
rb_rating_get_property (GObject *object,
			guint param_id,
			GValue *value,
			GParamSpec *pspec)
{
	RBRating *rating = RB_RATING (object);

	switch (param_id) {
	case PROP_RATING:
		g_value_set_double (value, rating->priv->rating);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
rb_rating_set_rating (RBRating *rating, gdouble value)
{
	/* clip to the value rating range */
	if (value > RB_RATING_MAX_SCORE) {
		value = RB_RATING_MAX_SCORE;
	} else if (value < 0.0) {
		value = 0.0;
	}

	rating->priv->rating = value;

	/* update accessible object name */
	rb_rating_set_accessible_name (GTK_WIDGET (rating), value);

	gtk_widget_queue_draw (GTK_WIDGET (rating));
}


static void
rb_rating_set_property (GObject *object,
			guint param_id,
			const GValue *value,
			GParamSpec *pspec)
{
	RBRating *rating = RB_RATING (object);

	switch (param_id) {
	case PROP_RATING:
		rb_rating_set_rating (rating, g_value_get_double (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

/**
 * rb_rating_new:
 *
 * Return value: a new #RBRating widget.
 */
RBRating *
rb_rating_new ()
{
	RBRating *rating;

	rating = g_object_new (RB_TYPE_RATING, NULL);

	g_return_val_if_fail (rating->priv != NULL, NULL);

	return rating;
}

static void
rb_rating_realize (GtkWidget *widget)
{
	GdkWindowAttr attributes;
	int attributes_mask;

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED | GTK_CAN_FOCUS);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_FOCUS_CHANGE_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
	widget->style = gtk_style_attach (widget->style, widget->window);

	gdk_window_set_user_data (widget->window, widget);

	gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
}

static void
rb_rating_size_request (GtkWidget *widget,
			GtkRequisition *requisition)
{
	int icon_size;

	g_return_if_fail (requisition != NULL);

	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &icon_size, NULL);

	requisition->width = RB_RATING_MAX_SCORE * icon_size + X_OFFSET;
	requisition->height = icon_size + Y_OFFSET * 2;
}

static gboolean
rb_rating_expose (GtkWidget *widget,
		  GdkEventExpose *event)
{
	gboolean ret;
	RBRating *rating;
	int x = 0;
	int y = 0;
	int width;
	int height;
	int focus_width;

	g_return_val_if_fail (RB_IS_RATING (widget), FALSE);
	if (GTK_WIDGET_DRAWABLE (widget) == FALSE) {
		return FALSE;
	}

	ret = FALSE;
	rating = RB_RATING (widget);

	gdk_drawable_get_size (widget->window, &width, &height);

	gtk_widget_style_get (widget, "focus-line-width", &focus_width, NULL);
	if (GTK_WIDGET_HAS_FOCUS (widget)) {
		x += focus_width;
		y += focus_width;
		width -= 2 * focus_width;
		height -= 2 * focus_width;
	}

	gtk_paint_flat_box (widget->style, widget->window,
			    GTK_STATE_NORMAL, GTK_SHADOW_IN,
			    NULL, widget, "entry_bg", x, y,
			    width, height);

	/* draw the stars */
	if (rating->priv->pixbufs != NULL) {
		ret = rb_rating_render_stars (widget,
					      widget->window,
					      rating->priv->pixbufs,
					      0, 0,
					      X_OFFSET, Y_OFFSET,
					      rating->priv->rating,
					      FALSE);
	}

	return ret;
}

static gboolean
rb_rating_button_press_cb (GtkWidget *widget,
			   GdkEventButton *event)
{
	int mouse_x, mouse_y;
	double new_rating;
	RBRating *rating;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (RB_IS_RATING (widget), FALSE);

	rating = RB_RATING (widget);

	gtk_widget_get_pointer (widget, &mouse_x, &mouse_y);

	new_rating = rb_rating_get_rating_from_widget (widget, mouse_x,
						       widget->allocation.width,
						       rating->priv->rating);

	if (new_rating > -0.0001) {
		g_signal_emit (G_OBJECT (rating),
			       rb_rating_signals[RATED],
			       0, new_rating);
	}

	gtk_widget_grab_focus (widget);

	return FALSE;
}

static gboolean
rb_rating_set_rating_cb (RBRating *rating, gdouble score)
{
	rb_rating_set_rating (rating, score);
	return TRUE;
}

static gboolean
rb_rating_adjust_rating_cb (RBRating *rating, gdouble adjust)
{
	rb_rating_set_rating (rating, rating->priv->rating + adjust);
	return TRUE;
}

static gboolean
rb_rating_focus (GtkWidget *widget, GtkDirectionType direction)
{
	return (GTK_WIDGET_CLASS (rb_rating_parent_class))->focus (widget, direction);
}

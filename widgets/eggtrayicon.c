/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* eggtrayicon.c
 * Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301  USA.
 */

#include <config.h>
#include <string.h>
#include <libintl.h>

#include "eggtrayicon.h"
#include "rb-stock-icons.h"
#include "rb-file-helpers.h"

#include <gdkconfig.h>
#include <gtk/gtk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#endif
#ifdef HAVE_NOTIFY
#include <libnotify/notify.h>
#endif

#ifndef EGG_COMPILATION
#ifndef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)
#define N_(x) x
#endif
#else
#define _(x) x
#define N_(x) x
#endif

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define SYSTEM_TRAY_ORIENTATION_HORZ 0
#define SYSTEM_TRAY_ORIENTATION_VERT 1

enum {
  PROP_0,
  PROP_ORIENTATION
};

#ifdef HAVE_NOTIFY
struct _Notify {
  NotifyNotification *handle;
};
#endif

static GtkPlugClass *parent_class = NULL;

static void egg_tray_icon_init (EggTrayIcon *icon);
static void egg_tray_icon_class_init (EggTrayIconClass *klass);

static void egg_tray_icon_get_property (GObject    *object,
					guint       prop_id,
					GValue     *value,
					GParamSpec *pspec);

static void egg_tray_icon_realize   (GtkWidget *widget);
static void egg_tray_icon_unrealize (GtkWidget *widget);

static void egg_tray_icon_add (GtkContainer *container,
			       GtkWidget    *widget);

#ifdef GDK_WINDOWING_X11
static void egg_tray_icon_update_manager_window    (EggTrayIcon *icon,
						    gboolean     dock_if_realized);
static void egg_tray_icon_manager_window_destroyed (EggTrayIcon *icon);
#endif

GType
egg_tray_icon_get_type (void)
{
  static GType our_type = 0;

  if (our_type == 0)
    {
      static const GTypeInfo our_info =
      {
	sizeof (EggTrayIconClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) egg_tray_icon_class_init,
	NULL, /* class_finalize */
	NULL, /* class_data */
	sizeof (EggTrayIcon),
	0,    /* n_preallocs */
	(GInstanceInitFunc) egg_tray_icon_init
      };

      our_type = g_type_register_static (GTK_TYPE_PLUG, "EggTrayIcon", &our_info, 0);
    }

  return our_type;
}

static void
egg_tray_icon_init (EggTrayIcon *icon)
{
  icon->stamp = 1;
  icon->orientation = GTK_ORIENTATION_HORIZONTAL;
#ifdef HAVE_NOTIFY
  icon->notify = g_new0 (Notify, 1);
#endif
  gtk_widget_add_events (GTK_WIDGET (icon), GDK_PROPERTY_CHANGE_MASK);
}

static void
egg_tray_icon_class_init (EggTrayIconClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;
  GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
  GtkContainerClass *container_class = (GtkContainerClass *)klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->get_property = egg_tray_icon_get_property;

  widget_class->realize   = egg_tray_icon_realize;
  widget_class->unrealize = egg_tray_icon_unrealize;

  container_class->add = egg_tray_icon_add;

  g_object_class_install_property (gobject_class,
				   PROP_ORIENTATION,
				   g_param_spec_enum ("orientation",
						      _("Orientation"),
						      _("The orientation of the tray."),
						      GTK_TYPE_ORIENTATION,
						      GTK_ORIENTATION_HORIZONTAL,
						      G_PARAM_READABLE));

#if defined (GDK_WINDOWING_X11)
  /* Nothing */
#elif defined (GDK_WINDOWING_WIN32)
  g_warning ("Port eggtrayicon to Win32");
#else
  g_warning ("Port eggtrayicon to this GTK+ backend");
#endif
}

static void
egg_tray_icon_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  EggTrayIcon *icon = EGG_TRAY_ICON (object);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, icon->orientation);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

#ifdef GDK_WINDOWING_X11

static void
egg_tray_icon_get_orientation_property (EggTrayIcon *icon)
{
  Display *xdisplay;
  Atom type;
  int format;
  union {
	gulong *prop;
	guchar *prop_ch;
  } prop = { NULL };
  gulong nitems;
  gulong bytes_after;
  int error, result;

  g_assert (icon->manager_window != None);

  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));

  gdk_error_trap_push ();
  type = None;
  result = XGetWindowProperty (xdisplay,
			       icon->manager_window,
			       icon->orientation_atom,
			       0, G_MAXLONG, FALSE,
			       XA_CARDINAL,
			       &type, &format, &nitems,
			       &bytes_after, &(prop.prop_ch));
  error = gdk_error_trap_pop ();

  if (error || result != Success)
    return;

  if (type == XA_CARDINAL)
    {
      GtkOrientation orientation;

      orientation = (prop.prop [0] == SYSTEM_TRAY_ORIENTATION_HORZ) ?
					GTK_ORIENTATION_HORIZONTAL :
					GTK_ORIENTATION_VERTICAL;

      if (icon->orientation != orientation)
	{
	  icon->orientation = orientation;

	  g_object_notify (G_OBJECT (icon), "orientation");
	}
    }

  if (prop.prop)
    XFree (prop.prop);
}

static GdkFilterReturn
egg_tray_icon_manager_filter (GdkXEvent *xevent, GdkEvent *event, gpointer user_data)
{
  EggTrayIcon *icon = user_data;
  XEvent *xev = (XEvent *)xevent;

  if (xev->xany.type == ClientMessage &&
      xev->xclient.message_type == icon->manager_atom &&
      xev->xclient.data.l[1] == icon->selection_atom)
    {
      egg_tray_icon_update_manager_window (icon, TRUE);
    }
  else if (xev->xany.window == icon->manager_window)
    {
      if (xev->xany.type == PropertyNotify &&
	  xev->xproperty.atom == icon->orientation_atom)
	{
	  egg_tray_icon_get_orientation_property (icon);
	}
      if (xev->xany.type == DestroyNotify)
	{
	  egg_tray_icon_manager_window_destroyed (icon);
	}
    }
  return GDK_FILTER_CONTINUE;
}

#endif

static void
egg_tray_icon_unrealize (GtkWidget *widget)
{
#ifdef GDK_WINDOWING_X11
  EggTrayIcon *icon = EGG_TRAY_ICON (widget);
  GdkWindow *root_window;

  if (icon->manager_window != None)
    {
      GdkWindow *gdkwin;

      gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (widget),
                                              icon->manager_window);

      gdk_window_remove_filter (gdkwin, egg_tray_icon_manager_filter, icon);
    }

  root_window = gdk_screen_get_root_window (gtk_widget_get_screen (widget));

  gdk_window_remove_filter (root_window, egg_tray_icon_manager_filter, icon);

  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    (* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
#endif

#ifdef HAVE_NOTIFY

  if (EGG_TRAY_ICON (widget)->notify->handle) {
    notify_notification_close (EGG_TRAY_ICON (widget)->notify->handle, NULL);
  }

  g_free (EGG_TRAY_ICON (widget)->notify);
#endif
}

#ifdef GDK_WINDOWING_X11

static void
egg_tray_icon_send_manager_message (EggTrayIcon *icon,
				    long         message,
				    Window       window,
				    long         data1,
				    long         data2,
				    long         data3)
{
  XClientMessageEvent ev;
  Display *display;

  ev.type = ClientMessage;
  ev.window = window;
  ev.message_type = icon->system_tray_opcode_atom;
  ev.format = 32;
  ev.data.l[0] = gdk_x11_get_server_time (GTK_WIDGET (icon)->window);
  ev.data.l[1] = message;
  ev.data.l[2] = data1;
  ev.data.l[3] = data2;
  ev.data.l[4] = data3;

  display = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));

  gdk_error_trap_push ();
  XSendEvent (display,
	      icon->manager_window, False, NoEventMask, (XEvent *)&ev);
  XSync (display, False);
  gdk_error_trap_pop ();
}

static void
egg_tray_icon_send_dock_request (EggTrayIcon *icon)
{
  egg_tray_icon_send_manager_message (icon,
				      SYSTEM_TRAY_REQUEST_DOCK,
				      icon->manager_window,
				      gtk_plug_get_id (GTK_PLUG (icon)),
				      0, 0);
}

static void
egg_tray_icon_update_manager_window (EggTrayIcon *icon,
				     gboolean     dock_if_realized)
{
  Display *xdisplay;

  if (icon->manager_window != None)
    return;

  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));

  XGrabServer (xdisplay);

  icon->manager_window = XGetSelectionOwner (xdisplay,
					     icon->selection_atom);

  if (icon->manager_window != None)
    XSelectInput (xdisplay,
		  icon->manager_window, StructureNotifyMask|PropertyChangeMask);

  XUngrabServer (xdisplay);
  XFlush (xdisplay);

  if (icon->manager_window != None)
    {
      GdkWindow *gdkwin;

      gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (GTK_WIDGET (icon)),
					      icon->manager_window);

      gdk_window_add_filter (gdkwin, egg_tray_icon_manager_filter, icon);

      if (dock_if_realized && GTK_WIDGET_REALIZED (icon))
	egg_tray_icon_send_dock_request (icon);

      egg_tray_icon_get_orientation_property (icon);
    }
}

static gboolean
transparent_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  gdk_window_clear_area (widget->window, event->area.x, event->area.y,
			 event->area.width, event->area.height);
  return FALSE;
}

static void
make_transparent_again (GtkWidget *widget, GtkStyle *previous_style,
			gpointer user_data)
{
  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
}

static void
make_transparent (GtkWidget *widget, gpointer user_data)
{
  if (GTK_WIDGET_NO_WINDOW (widget) || GTK_WIDGET_APP_PAINTABLE (widget))
    return;

  gtk_widget_set_app_paintable (widget, TRUE);
  gtk_widget_set_double_buffered (widget, FALSE);
  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
  g_signal_connect (widget, "expose_event",
		    G_CALLBACK (transparent_expose_event), NULL);
  g_signal_connect_after (widget, "style_set",
			  G_CALLBACK (make_transparent_again), NULL);
}	

static void
egg_tray_icon_manager_window_destroyed (EggTrayIcon *icon)
{
  GdkWindow *gdkwin;

  g_return_if_fail (icon->manager_window != None);

  gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (GTK_WIDGET (icon)),
					  icon->manager_window);

  gdk_window_remove_filter (gdkwin, egg_tray_icon_manager_filter, icon);

  icon->manager_window = None;

  egg_tray_icon_update_manager_window (icon, TRUE);
}

#endif

gboolean
egg_tray_icon_have_manager (EggTrayIcon *icon)
{
  GtkPlug * plug = GTK_PLUG (icon);

  if (plug->socket_window)
    return TRUE;
  else
    return FALSE;
}

static void
egg_tray_icon_realize (GtkWidget *widget)
{
#ifdef GDK_WINDOWING_X11
  EggTrayIcon *icon = EGG_TRAY_ICON (widget);
  GdkScreen *screen;
  GdkDisplay *display;
  Display *xdisplay;
  char buffer[256];
  GdkWindow *root_window;

  if (GTK_WIDGET_CLASS (parent_class)->realize)
    GTK_WIDGET_CLASS (parent_class)->realize (widget);

  make_transparent (widget, NULL);

  screen = gtk_widget_get_screen (widget);
  display = gdk_screen_get_display (screen);
  xdisplay = gdk_x11_display_get_xdisplay (display);

  /* Now see if there's a manager window around */
  g_snprintf (buffer, sizeof (buffer),
	      "_NET_SYSTEM_TRAY_S%d",
	      gdk_screen_get_number (screen));

  icon->selection_atom = XInternAtom (xdisplay, buffer, False);

  icon->manager_atom = XInternAtom (xdisplay, "MANAGER", False);

  icon->system_tray_opcode_atom = XInternAtom (xdisplay,
						   "_NET_SYSTEM_TRAY_OPCODE",
						   False);

  icon->orientation_atom = XInternAtom (xdisplay,
					"_NET_SYSTEM_TRAY_ORIENTATION",
					False);

  egg_tray_icon_update_manager_window (icon, FALSE);
  egg_tray_icon_send_dock_request (icon);

  root_window = gdk_screen_get_root_window (screen);

  /* Add a root window filter so that we get changes on MANAGER */
  gdk_window_add_filter (root_window,
			 egg_tray_icon_manager_filter, icon);
#endif
}

static void
egg_tray_icon_add (GtkContainer *container, GtkWidget *widget)
{
  g_signal_connect (widget, "realize",
		    G_CALLBACK (make_transparent), NULL);
  GTK_CONTAINER_CLASS (parent_class)->add (container, widget);
}

EggTrayIcon *
egg_tray_icon_new_for_screen (GdkScreen *screen, const char *name)
{
  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);

  return g_object_new (EGG_TYPE_TRAY_ICON, "screen", screen, "title", name, NULL);
}

EggTrayIcon*
egg_tray_icon_new (const gchar *name)
{
  return g_object_new (EGG_TYPE_TRAY_ICON, "title", name, NULL);
}

guint
egg_tray_icon_send_message (EggTrayIcon *icon,
			    gint         timeout,
			    const gchar *message_markup,
			    gint         len)
{
  g_return_val_if_fail (EGG_IS_TRAY_ICON (icon), 0);
  g_return_val_if_fail (timeout >= 0, 0);
  g_return_val_if_fail (message_markup != NULL, 0);

#ifdef HAVE_NOTIFY
  egg_tray_icon_notify (icon, timeout, _("Notification"), NULL, message_markup);
#endif

  return 1;
}

void
egg_tray_icon_cancel_message (EggTrayIcon *icon,
			      guint        id)
{
  g_return_if_fail (EGG_IS_TRAY_ICON (icon));

#ifdef HAVE_NOTIFY
  if (icon->notify->handle)
  {
    notify_notification_close (icon->notify->handle, NULL);
  }
#endif
}

GtkOrientation
egg_tray_icon_get_orientation (EggTrayIcon *icon)
{
  g_return_val_if_fail (EGG_IS_TRAY_ICON (icon), GTK_ORIENTATION_HORIZONTAL);

  return icon->orientation;
}

void
egg_tray_icon_notify (EggTrayIcon *icon,
		      guint timeout,
		      const char *primary_markup,
		      GdkPixbuf *pixbuf,
		      const char *secondary_markup)
{
#ifdef HAVE_NOTIFY
  GtkRequisition size;
  int x;
  int y;

  if (!notify_is_initted ())
    if (!notify_init ("rhythmbox"))
      return;

  if (primary_markup == NULL)
    {
      primary_markup = "";
    }
  if (secondary_markup == NULL)
    {
      secondary_markup = "";
    }

  if (icon->notify->handle == NULL)
    {
      icon->notify->handle = notify_notification_new (primary_markup,
						      secondary_markup,
						      NULL,
						      GTK_WIDGET (icon));
    }
  else
    {
      notify_notification_update (icon->notify->handle,
				  primary_markup,
				  secondary_markup,
				  NULL);
      notify_notification_attach_to_widget (icon->notify->handle,
					    GTK_WIDGET (icon));
    }

  notify_notification_set_timeout (icon->notify->handle, timeout);

  if (pixbuf == NULL)
    {
      GtkIconTheme *theme;
      gint size;

      theme = gtk_icon_theme_get_default ();
      gtk_icon_size_lookup (GTK_ICON_SIZE_DIALOG, &size, NULL);
      pixbuf = gtk_icon_theme_load_icon (theme,
	  				 RB_APP_ICON,
					 size,
					 0,
					 NULL);
      if (pixbuf != NULL)
	{
	  notify_notification_set_icon_from_pixbuf (icon->notify->handle, pixbuf);
	  g_object_unref (pixbuf);
	}
    }
  else
    {
      notify_notification_set_icon_from_pixbuf (icon->notify->handle, pixbuf);
    }

  gdk_window_get_origin (GTK_WIDGET (icon)->window, &x, &y);
  gtk_widget_size_request (GTK_WIDGET (icon), &size);
  x += size.width / 2;
  y += size.height;
  notify_notification_set_hint_int32 (icon->notify->handle, "x", x);
  notify_notification_set_hint_int32 (icon->notify->handle, "y", y);

  if (! notify_notification_show (icon->notify->handle, NULL))
    {
      g_warning ("failed to send notification (%s)", primary_markup);
    }

  return;
#endif
}

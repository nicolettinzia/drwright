/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002-2003 Richard Hult <richard@imendo.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <libgnomeui/gnome-stock-icons.h>
#include <libgnomeui/gnome-client.h>
#include <gconf/gconf-client.h>
#include "drwright.h"
#include "drw-intl.h"
#include "drw-break-window.h"
#include "drw-monitor.h"
#include "drw-preferences.h"
#include "eggtrayicon.h"

#define BLINK_TIMEOUT 200
#define BLINK_TIMEOUT_MIN 120
#define BLINK_TIMEOUT_FACTOR 100

#define POPUP_ITEM_ENABLED 1

typedef enum {
	STATE_START,
	STATE_IDLE,
	STATE_TYPE,
	STATE_WARN_TYPE,
	STATE_WARN_IDLE,
	STATE_BREAK_SETUP,
	STATE_BREAK,
	STATE_BREAK_DONE_SETUP,
	STATE_BREAK_DONE
} DrwState;

struct _DrWright {
	/* Widgets. */
	GtkWidget      *break_window;

	DrwMonitor     *monitor;

	GtkItemFactory *popup_factory;
	
	DrwState        state;
	GTimer         *timer;
	GTimer         *idle_timer;

	gint            last_elapsed_time;
	
	gboolean        is_active;
	
	/* Time settings. */
	gint            type_time;
	gint            break_time;
	gint            warn_time;

	gboolean        use_warning_window;

	gboolean        enabled;

	guint           clock_timeout_id;
	guint           blink_timeout_id;

	gboolean        blink_on;

	EggTrayIcon    *icon;
	GtkWidget      *icon_image;
	GtkWidget      *icon_event_box;
	GtkTooltips    *tooltips;

	GdkPixbuf      *neutral_bar;
	GdkPixbuf      *red_bar;
	GdkPixbuf      *green_bar;
	GdkPixbuf      *disabled_bar;
	GdkPixbuf      *composite_bar;

	GtkWidget      *warn_dialog;
};

static void     activity_detected_cb    (DrwMonitor     *monitor,
					 DrWright       *drwright);
static gboolean maybe_change_state      (DrWright       *drwright);
static gboolean update_tooltip          (DrWright       *drwright);
static gboolean icon_button_press_cb    (GtkWidget      *widget,
					 GdkEventButton *event,
					 DrWright       *drwright);
static void     break_window_destroy_cb (GtkWidget      *window,
					 DrWright       *dr);
static void     popup_enabled_cb        (gpointer        callback_data,
					 guint           action,
					 GtkWidget      *widget);
static void     popup_preferences_cb    (gpointer        callback_data,
					 guint           action,
					 GtkWidget      *widget);
static void     popup_quit_cb           (gpointer        callback_data,
					 guint           action,
					 GtkWidget      *widget);
static void     popup_about_cb          (gpointer        callback_data,
					 guint           action,
					 GtkWidget      *widget);
static gchar *  item_factory_trans_cb   (const gchar    *path,
					 gpointer        data);
static void     init_tray_icon          (DrWright       *dr);


#define GIF_CB(x) ((GtkItemFactoryCallback)(x))

static GtkItemFactoryEntry popup_items[] = {
	{ N_("/_Enabled"),      NULL, GIF_CB (popup_enabled_cb),     POPUP_ITEM_ENABLED, "<ToggleItem>", NULL },
	{ "/sep1",              NULL, 0,                             0,                  "<Separator>",  NULL },
	{ N_("/_Preferences"),  NULL, GIF_CB (popup_preferences_cb), 0,                  "<StockItem>",  GTK_STOCK_PREFERENCES },
	{ N_("/_About"),        NULL, GIF_CB (popup_about_cb),       0,                  "<StockItem>",  GNOME_STOCK_ABOUT },
	{ "/sep2",              NULL, 0,                             0,                  "<Separator>",  NULL },
	{ N_("/_Remove Icon"),  "",   GIF_CB (popup_quit_cb),        0,                  "<StockItem>",  GTK_STOCK_REMOVE },
};

GConfClient *client = NULL;
gboolean debug;
gboolean debug_break_time;


static void
sanity_check_preferences (DrWright *dr)
{
	dr->type_time = MAX (dr->type_time, 10);
	dr->warn_time = CLAMP (dr->warn_time, 5, dr->type_time - 5);

	if (debug) {
		dr->type_time = 5;
		dr->warn_time = 4;
		dr->break_time = 10;

		debug_break_time = dr->break_time;
	}
}

static void
update_warning_dialog (DrWright *dr)
{
	gint   elapsed_time, min, sec;
	gchar *str;

	if (!dr->warn_dialog) {
		return;
	}
	
	elapsed_time = g_timer_elapsed (dr->timer, NULL);

	sec = dr->type_time - elapsed_time;
	min = sec / 60;
	sec -= min * 60;

	min = MAX (0, min);
	sec = MAX (0, sec);

	if (min > 0 || sec > 0) {
		str = g_strdup_printf ("%s\n"
				       "<span size=\"10000\"><b>%d:%02d</b></span>",
				       _("Break coming up!"),
				       min, sec);
	} else {
		str = g_strdup (_("Break!"));
	}		
	
	g_object_set (GTK_MESSAGE_DIALOG (dr->warn_dialog)->label,
		      "use-markup", TRUE,
		      "wrap", TRUE,
		      "label", str,
		      NULL);
	
	g_free (str);
}

static gboolean
warn_dialog_delete_event (GtkWidget *dialog)
{
	gtk_widget_hide (dialog);
	return TRUE;
}

static void
warn_dialog_response (GtkWidget *dialog)
{
	gtk_widget_hide (dialog);
}

static void
hide_warning_dialog (DrWright *dr)
{
	if (dr->warn_dialog) {
		gtk_widget_hide (dr->warn_dialog);
		return;
	}
}

static void
show_warning_dialog (DrWright *dr)
{
	if (!dr->use_warning_window) {
		return;
	}
	
	if (dr->warn_dialog) {
		gtk_window_present (GTK_WINDOW (dr->warn_dialog));
		return;
	}
	
	dr->warn_dialog = gtk_message_dialog_new (NULL,
						  0,
						  GTK_MESSAGE_INFO,
						  GTK_BUTTONS_CLOSE,
						  " ");
	
	update_warning_dialog (dr);

	g_signal_connect (dr->warn_dialog,
			  "delete_event",
			  G_CALLBACK (warn_dialog_delete_event),
			  NULL);

	g_signal_connect (dr->warn_dialog,
			  "response",
			  G_CALLBACK (warn_dialog_response),
			  NULL);
	
	gtk_widget_show (dr->warn_dialog);
}

static void
update_icon (DrWright *dr)
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *tmp_pixbuf;
	gint       width, height;
	gfloat     r;
	gint       offset;
	gboolean   set_pixbuf;

	if (!dr->enabled) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (dr->icon_image), dr->disabled_bar);
		return;
	}
	
	tmp_pixbuf = gdk_pixbuf_copy (dr->neutral_bar);

	width = gdk_pixbuf_get_width (tmp_pixbuf);
	height = gdk_pixbuf_get_height (tmp_pixbuf);

	set_pixbuf = TRUE;

	if (dr->state == STATE_BREAK) {
		r = (float) (dr->break_time - g_timer_elapsed (dr->timer, NULL)) / (float) dr->break_time;
	}
	else if (dr->state == STATE_BREAK_DONE) {
		r = 0;
	} else {
		r = (float) g_timer_elapsed (dr->timer, NULL) / (float) dr->type_time;
	}
	
	offset = CLAMP ((height - 0) * (1.0 - r), 1, height - 0);
	
	switch (dr->state) {
	case STATE_WARN_TYPE:
	case STATE_WARN_IDLE:
		pixbuf = dr->red_bar;
		set_pixbuf = FALSE;
		break;

	case STATE_BREAK_SETUP:
	case STATE_BREAK:
		pixbuf = dr->red_bar;
		break;

	default:
		pixbuf = dr->green_bar;
	}		
	
	gdk_pixbuf_composite (pixbuf,
			      tmp_pixbuf,
			      0,
			      offset,
			      width,
			      height - offset,
			      0,
			      0,
			      1.0,
			      1.0,
			      GDK_INTERP_BILINEAR,
			      255);
	
	if (set_pixbuf) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (dr->icon_image), tmp_pixbuf);
	}
	
	if (dr->composite_bar) {
		g_object_unref (dr->composite_bar);
	}

	dr->composite_bar = tmp_pixbuf;
}

static gboolean
blink_timeout_cb (DrWright *dr)
{
	gfloat r;
	gint   timeout;
	
	r = (dr->type_time - g_timer_elapsed (dr->timer, NULL)) / dr->warn_time;
	timeout = BLINK_TIMEOUT + BLINK_TIMEOUT_FACTOR * r;

	if (timeout < BLINK_TIMEOUT_MIN) {
		timeout = BLINK_TIMEOUT_MIN;
	}
	
	if (dr->blink_on || timeout == 0) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (dr->icon_image), dr->composite_bar);
	} else {
		gtk_image_set_from_pixbuf (GTK_IMAGE (dr->icon_image), dr->neutral_bar);
	}
	
	dr->blink_on = !dr->blink_on;

	if (timeout) {
		dr->blink_timeout_id = g_timeout_add (timeout,
						      (GSourceFunc) blink_timeout_cb,
						      dr);
	} else {
		dr->blink_timeout_id = 0;
	}
		
	return FALSE;
}

static void
start_blinking (DrWright *dr)
{
	if (!dr->blink_timeout_id) {
		show_warning_dialog (dr);
		dr->blink_on = TRUE;
		blink_timeout_cb (dr);
	}

	/*gtk_widget_show (GTK_WIDGET (dr->icon));*/
}

static void
stop_blinking (DrWright *dr)
{
	hide_warning_dialog (dr);

	if (dr->blink_timeout_id) {
		g_source_remove (dr->blink_timeout_id);
		dr->blink_timeout_id = 0;
	}

	/*gtk_widget_hide (GTK_WIDGET (dr->icon));*/
}

static gboolean
maybe_change_state (DrWright *dr)
{
	gint elapsed_time;
	gint elapsed_idle_time;

	if (debug) {
		g_timer_reset (dr->idle_timer);
	}
	
	elapsed_time = g_timer_elapsed (dr->timer, NULL);
	elapsed_idle_time = g_timer_elapsed (dr->idle_timer, NULL);

	if (elapsed_time > dr->last_elapsed_time + dr->warn_time) {
		/* If the timeout is delayed by the amount of warning time, then
		 * we must have been suspended or stopped, so we just start
		 * over.
		 */
		dr->state = STATE_START;
	}

	update_warning_dialog (dr);
	
	switch (dr->state) {
	case STATE_START:
		if (dr->break_window) {
			gtk_widget_destroy (dr->break_window);
			dr->break_window = NULL;
		}

		gtk_image_set_from_pixbuf (GTK_IMAGE (dr->icon_image), dr->neutral_bar);

		g_timer_start (dr->timer);
		g_timer_start (dr->idle_timer);

		if (dr->enabled) {
			dr->state = STATE_IDLE;
		}
		
		update_tooltip (dr);
		stop_blinking (dr);
		break;

	case STATE_IDLE:
		if (elapsed_idle_time >= dr->break_time) {
			g_timer_start (dr->timer);
			g_timer_start (dr->idle_timer);
		} else if (dr->is_active) {
			dr->state = STATE_TYPE;
		}
		break;

	case STATE_TYPE:
		if (elapsed_time >= dr->type_time - dr->warn_time) {
			dr->state = STATE_WARN_TYPE;

			start_blinking (dr);
		} else if (elapsed_time >= dr->type_time) {
			dr->state = STATE_BREAK_SETUP;
		}
		else if (!dr->is_active) {
			dr->state = STATE_IDLE;
			g_timer_start (dr->idle_timer);
		}
		break;

	case STATE_WARN_TYPE:
		if (elapsed_time >= dr->type_time) {
			dr->state = STATE_BREAK_SETUP;
		}
		else if (!dr->is_active) {
			dr->state = STATE_WARN_IDLE;
		}
		break;

	case STATE_WARN_IDLE:
		if (elapsed_idle_time >= dr->break_time) {
			dr->state = STATE_BREAK_DONE_SETUP;
		}
		else if (dr->is_active) {
			dr->state = STATE_WARN_TYPE;
		}
		
		break;
		
	case STATE_BREAK_SETUP:
		stop_blinking (dr);
		gtk_image_set_from_pixbuf (GTK_IMAGE (dr->icon_image), dr->red_bar);

		g_timer_start (dr->timer);

		dr->break_window = drw_break_window_new ();

		g_signal_connect (dr->break_window,
				  "destroy",
				  G_CALLBACK (break_window_destroy_cb),
				  dr);
		
		gtk_widget_show (dr->break_window);

		dr->state = STATE_BREAK;
		break;
	       
	case STATE_BREAK:
		if (elapsed_time >= dr->break_time) {
			dr->state = STATE_BREAK_DONE_SETUP;
		}
		break;

	case STATE_BREAK_DONE_SETUP:
		stop_blinking (dr);
		gtk_image_set_from_pixbuf (GTK_IMAGE (dr->icon_image), dr->green_bar);

		dr->state = STATE_BREAK_DONE;
		break;
			
	case STATE_BREAK_DONE:
		if (dr->is_active) {
			dr->state = STATE_START;
			if (dr->break_window) {
				gtk_widget_destroy (dr->break_window);
				dr->break_window = NULL;
			}
		}
		break;
	}

	dr->is_active = FALSE;
	dr->last_elapsed_time = elapsed_time;
	
	update_icon (dr);
		
	return TRUE;
}

static gboolean update_tooltip (DrWright *dr)
{
	gint   elapsed_time, min;
	gchar *str;

	if (!dr->enabled) {
		gtk_tooltips_set_tip (GTK_TOOLTIPS (dr->tooltips),
				      dr->icon_event_box,
				      _("Disabled"), _("Disabled"));
		return TRUE;
	}
	
	elapsed_time = g_timer_elapsed (dr->timer, NULL);

	min = ceil ((dr->type_time - elapsed_time) / 60.0);

	if (min > 1) {
		str = g_strdup_printf (_("%d minutes until the next break"), min);
	} else {
		str = g_strdup_printf (_("One minute until the next break"));
	}
	
	gtk_tooltips_set_tip (GTK_TOOLTIPS (dr->tooltips),
			      dr->icon_event_box,
			      str, str);

	g_free (str);

	return TRUE;
}

static void
activity_detected_cb (DrwMonitor *monitor,
		      DrWright   *dr)
{
	dr->is_active = TRUE;
	g_timer_start (dr->idle_timer);
}

static void
gconf_notify_cb (GConfClient *client,
		 guint        cnxn_id,
		 GConfEntry  *entry,
		 gpointer     user_data)
{
	DrWright *dr = user_data;
	
	if (!strcmp (entry->key, "/apps/drwright/type_time")) {
		if (entry->value->type == GCONF_VALUE_INT) {
			dr->type_time = 60 * gconf_value_get_int (entry->value);
			dr->state = STATE_START;
		}
	}
	else if (!strcmp (entry->key, "/apps/drwright/warn_time")) {
		if (entry->value->type == GCONF_VALUE_INT) {
			dr->warn_time = 60 * gconf_value_get_int (entry->value);
			dr->state = STATE_START;
		}
	}
	else if (!strcmp (entry->key, "/apps/drwright/break_time")) {
		if (entry->value->type == GCONF_VALUE_INT) {
			dr->break_time = 60 * gconf_value_get_int (entry->value);
			dr->state = STATE_START;
		}
	}
	else if (!strcmp (entry->key, "/apps/drwright/use_warning_window")) {
		if (entry->value->type == GCONF_VALUE_BOOL) {
			dr->use_warning_window = gconf_value_get_bool (entry->value);
			hide_warning_dialog (dr);
		}
	}
	else if (!strcmp (entry->key, "/apps/drwright/enabled")) {
		if (entry->value->type == GCONF_VALUE_BOOL) {
			dr->enabled = gconf_value_get_bool (entry->value);
			dr->state = STATE_START;

			update_tooltip (dr);
		}
	}
		
	maybe_change_state (dr);
}

static void
popup_enabled_cb (gpointer   callback_data,
		  guint      action,
		  GtkWidget *widget)
{
	DrWright  *dr = callback_data;
	GtkWidget *item;

	item = gtk_item_factory_get_widget_by_action (dr->popup_factory,
						      POPUP_ITEM_ENABLED);

	gconf_client_set_bool (client, "/apps/drwright/enabled",
			       gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)),
			       NULL);	
}

static void
popup_preferences_cb (gpointer   callback_data,
		      guint      action,
		      GtkWidget *widget)
{
	drw_preferences_new ();
}

static void
popup_quit_cb (gpointer   callback_data,
	       guint      action,
	       GtkWidget *widget)
{
	GtkWidget *dialog;
	gchar     *str;
	gint       response;
	GnomeClient *client;

	str = g_strdup_printf ("<b>%s</b>\n%s",
			       _("Quit DrWright?"),			       
			       _("Don't forget to take regular breaks."));
	
	dialog = gtk_message_dialog_new (NULL,
					 0,
					 GTK_MESSAGE_QUESTION,
					 GTK_BUTTONS_YES_NO,
					 str);

	g_free (str);
	
	g_object_set (GTK_MESSAGE_DIALOG (dialog)->label,
		      "use-markup", TRUE,
		      "wrap", TRUE,
		      NULL);
	
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response != GTK_RESPONSE_DELETE_EVENT) {
		gtk_widget_destroy (dialog);
	}
	
	if (response == GTK_RESPONSE_YES) {
		client = gnome_master_client ();
		gnome_client_set_restart_style (client, GNOME_RESTART_NEVER);
		
		gtk_main_quit ();
	}
}

static void
popup_about_cb (gpointer   callback_data,
		guint      action,
		GtkWidget *widget)
{
	static GtkWidget *about_window;
	GtkWidget        *vbox;
	GtkWidget        *label;
	GdkPixbuf        *icon;
	gchar            *markup;

	if (about_window) {
		gtk_window_present (GTK_WINDOW (about_window));
		return;
	}
	
	about_window = gtk_dialog_new ();

	g_object_add_weak_pointer (G_OBJECT (about_window), (gpointer *) &about_window);
	
	gtk_dialog_add_button (GTK_DIALOG (about_window),
			       GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_set_default_response (GTK_DIALOG (about_window),
					 GTK_RESPONSE_OK);
	
	gtk_window_set_title (GTK_WINDOW (about_window), _("About DrWright"));
	icon = NULL; /*gdk_pixbuf_new_from_file (IMAGEDIR "/bar.png", NULL);*/
	if (icon != NULL) {
		gtk_window_set_icon (GTK_WINDOW (about_window), icon);
		g_object_unref (icon);
	}
	
	gtk_window_set_resizable (GTK_WINDOW (about_window), FALSE);
	gtk_window_set_position (GTK_WINDOW (about_window), 
				 GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_type_hint (GTK_WINDOW (about_window), 
				  GDK_WINDOW_TYPE_HINT_DIALOG);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (about_window)->vbox), vbox, FALSE, FALSE, 0);

	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	markup = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">DrWright " VERSION "</span>\n\n"
				  "%s\n\n"
				  "<span size=\"small\">%s</span>\n"
				  "<span size=\"small\">%s</span>\n",
				  _("A computer break reminder."),
				  _("Written by Richard Hult &lt;rhult@imendo.com&gt;"),
				  _("Eye candy added by Anders Carlsson"));
	gtk_label_set_markup (GTK_LABEL (label), markup);
	g_free (markup);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
	
	gtk_widget_show_all (about_window);
	gtk_dialog_run (GTK_DIALOG (about_window));
	gtk_widget_destroy (about_window);
}

static void
popup_menu_position_cb (GtkMenu  *menu,
			gint     *x,
			gint     *y,
			gboolean *push_in,
			gpointer  data)
{
	GtkWidget      *w = data;
	GtkRequisition  requisition;
	gint            wx, wy;

	g_return_if_fail (w != NULL);

	gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

	gdk_window_get_origin (w->window, &wx, &wy);

	if (*x < wx)
		*x = wx;
	else if (*x > wx + w->allocation.width)
		*x = wx + w->allocation.width;

	if (*x + requisition.width > gdk_screen_width())
		*x = gdk_screen_width() - requisition.width;

	if (*y < wy)
		*y = wy;
	 else if (*y > wy + w->allocation.height)
		*y = wy + w->allocation.height;

	if (*y + requisition.height > gdk_screen_height())
		*y = gdk_screen_height() - requisition.height;

	*push_in = TRUE;
}

static gboolean
icon_button_press_cb (GtkWidget      *widget,
		      GdkEventButton *event,
		      DrWright       *dr)
{
	gint       seconds;
	gint       minutes;
	GtkWidget *menu;

	if (debug && event->button == 1) {
		seconds = g_timer_elapsed (dr->timer, NULL);
		minutes = seconds / 60;
		seconds -= minutes * 60;
		g_print ("\nType: %d:%02d\n", minutes, seconds);
		
		seconds = g_timer_elapsed (dr->idle_timer, NULL);
		minutes = seconds / 60;
		seconds -= minutes * 60;
		g_print ("Idle: %d:%02d\n", minutes, seconds);

		return FALSE;
	}
	if (event->button == 3) {
		menu = gtk_item_factory_get_widget (dr->popup_factory, "");

		gtk_menu_popup (GTK_MENU (menu),
				NULL,
				NULL,
				popup_menu_position_cb,
				dr->icon,
				event->button,
				event->time);

		return TRUE;
	}
	
	return FALSE;
}

static void
break_window_destroy_cb (GtkWidget *window,
			 DrWright  *dr)
{
	dr->state = STATE_BREAK_DONE_SETUP;
	dr->break_window = NULL;

	maybe_change_state (dr);
}

static char *
item_factory_trans_cb (const gchar *path,
		       gpointer     data)
{
	return _((gchar*) path);
}

static void
icon_event_box_destroy_cb (GtkWidget *widget,
			   DrWright  *dr)
{
	gtk_widget_destroy (GTK_WIDGET (dr->icon));
	init_tray_icon (dr);
}
	
static void
init_tray_icon (DrWright *dr)
{
	dr->icon = egg_tray_icon_new (_("Break reminder"));

	dr->icon_event_box = gtk_event_box_new ();
	dr->icon_image = gtk_image_new_from_pixbuf (dr->neutral_bar);
	gtk_container_add (GTK_CONTAINER (dr->icon_event_box), dr->icon_image);
		
	gtk_widget_add_events (GTK_WIDGET (dr->icon), GDK_BUTTON_PRESS_MASK);
	gtk_container_add (GTK_CONTAINER (dr->icon), dr->icon_event_box);
	gtk_widget_show_all (GTK_WIDGET (dr->icon));
	
	update_tooltip (dr);
	update_icon (dr);

	g_signal_connect (dr->icon,
			  "button_press_event",
			  G_CALLBACK (icon_button_press_cb),
			  dr);

	g_signal_connect (dr->icon,
			  "destroy",
			  G_CALLBACK (icon_event_box_destroy_cb),
			  dr);
}

DrWright *
drwright_new (void)
{
	DrWright  *dr;
	GtkWidget *item;

        dr = g_new0 (DrWright, 1);

	client = gconf_client_get_default ();

	gconf_client_add_dir (client,
			     "/apps/drwright",
			      GCONF_CLIENT_PRELOAD_NONE,
			      NULL);

	gconf_client_notify_add (client, "/apps/drwright",
				 gconf_notify_cb,
				 dr,
				 NULL,
				 NULL);
	
	dr->type_time = 60 * gconf_client_get_int (
		client, "/apps/drwright/type_time", NULL);
	
	dr->warn_time = 60 * gconf_client_get_int (
		client, "/apps/drwright/warn_time", NULL);
	
	dr->break_time = 60 * gconf_client_get_int (
		client, "/apps/drwright/break_time", NULL);

	dr->use_warning_window = gconf_client_get_bool (
		client, "/apps/drwright/use_warning_window", NULL);
	
	dr->enabled = gconf_client_get_bool (
		client,
		"/apps/drwright/enabled",
		NULL);

	sanity_check_preferences (dr);
	
	dr->popup_factory = gtk_item_factory_new (GTK_TYPE_MENU,
						      "<main>",
						      NULL);
	gtk_item_factory_set_translate_func (dr->popup_factory,
					     item_factory_trans_cb,
					     NULL,
					     NULL);
	
	gtk_item_factory_create_items (dr->popup_factory,
				       G_N_ELEMENTS (popup_items),
				       popup_items,
				       dr);

	item = gtk_item_factory_get_widget_by_action (dr->popup_factory,
						      POPUP_ITEM_ENABLED);
	
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), dr->enabled);

	dr->timer = g_timer_new ();
	dr->idle_timer = g_timer_new ();
	
	dr->state = STATE_START;

	dr->monitor = drw_monitor_new ();

	g_signal_connect (dr->monitor,
			  "activity",
			  G_CALLBACK (activity_detected_cb),
			  dr);

	dr->neutral_bar = gdk_pixbuf_new_from_file (IMAGEDIR "/bar.png", NULL);
	dr->red_bar = gdk_pixbuf_new_from_file (IMAGEDIR "/bar-red.png", NULL);
	dr->green_bar = gdk_pixbuf_new_from_file (IMAGEDIR "/bar-green.png", NULL);
	dr->disabled_bar = gdk_pixbuf_new_from_file (IMAGEDIR "/bar-disabled.png", NULL);

	dr->tooltips = gtk_tooltips_new ();

	init_tray_icon (dr);
	
	g_timeout_add (15*1000,
		       (GSourceFunc) update_tooltip,
		       dr);
	g_timeout_add (500,
		       (GSourceFunc) maybe_change_state,
		       dr);

	return dr;
}
       

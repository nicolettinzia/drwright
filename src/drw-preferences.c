/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
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
#include <string.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include "drw-intl.h"

typedef struct {
	gint       type_time;
	gint       break_time;
	gint       warn_time;

	gboolean   allow_unlock;
	gchar     *unlock_phrase;
	
	guint      notify_id;
	
	GtkWidget *type_sb;
	GtkWidget *break_sb;
	GtkWidget *warn_sb;

	GtkWidget *unlock_cb;
	GtkWidget *unlock_entry;
} DrwPreferences;

extern GConfClient *client;

static void
type_time_changed_cb (GtkSpinButton  *sb,
		      DrwPreferences *prefs)
{
	gint t;
	
	t = gtk_spin_button_get_value_as_int (sb);
	
	gconf_client_set_int (client, "/apps/drwright/type_time", t, NULL);
}

static void
break_time_changed_cb (GtkSpinButton  *sb,
		       DrwPreferences *prefs)
{
	gint t;

	t = gtk_spin_button_get_value_as_int (sb);
	
	gconf_client_set_int (client, "/apps/drwright/break_time", t, NULL);
}

static void
warn_time_changed_cb (GtkSpinButton  *sb,
		      DrwPreferences *prefs)
{
	gint t;

	t = gtk_spin_button_get_value_as_int (sb);
	
	gconf_client_set_int (client, "/apps/drwright/warn_time", t, NULL);
}

static void
allow_unlock_toggled_cb (GtkToggleButton *tb,
			   DrwPreferences  *prefs)
{
	gboolean a;
	
	a = gtk_toggle_button_get_active (tb);
	
	gconf_client_set_bool (client, "/apps/drwright/allow_unlock", a, NULL);
}

static void
unlock_phrase_changed_cb (GtkEntry       *entry,
			  DrwPreferences *prefs)
{
	const gchar *str;

	str = gtk_entry_get_text (entry);
	
	gconf_client_set_string (client, "/apps/drwright/unlock_phrase", str, NULL);
}

static void
type_time_update (DrwPreferences *prefs)
{
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (prefs->type_sb),
				   prefs->type_time);
}

static void
break_time_update (DrwPreferences *prefs)
{
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (prefs->break_sb),
				   prefs->break_time);
}

static void
warn_time_update (DrwPreferences *prefs)
{
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (prefs->warn_sb),
				   prefs->warn_time);
}

static void
allow_unlock_update (DrwPreferences *prefs)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->unlock_cb),
				      prefs->allow_unlock);

	gtk_widget_set_sensitive (prefs->unlock_entry, prefs->allow_unlock);
}

static void
unlock_phrase_update (DrwPreferences *prefs)
{
	gchar *str;
	
	str = prefs->unlock_phrase;
	if (!str) {
		str = "";
	}
	
	gtk_entry_set_text (GTK_ENTRY (prefs->unlock_entry),
			    str);
}

static void
gconf_notify_cb (GConfClient *client,
		 guint        cnxn_id,
		 GConfEntry  *entry,
		 gpointer     user_data)
{
	DrwPreferences *prefs;

	prefs = user_data;
	
	if (!strcmp (entry->key, "/apps/drwright/type_time")) {
		if (entry->value->type == GCONF_VALUE_INT) {
			prefs->type_time = gconf_value_get_int (entry->value);
			type_time_update (prefs);
		}
	}
	else if (!strcmp (entry->key, "/apps/drwright/break_time")) {
		if (entry->value->type == GCONF_VALUE_INT) {
			prefs->break_time = gconf_value_get_int (entry->value);
			break_time_update (prefs);
		}
	}
	else if (!strcmp (entry->key, "/apps/drwright/warn_time")) {
		if (entry->value->type == GCONF_VALUE_INT) {
			prefs->warn_time = gconf_value_get_int (entry->value);
			warn_time_update (prefs);
		}
	}
	else if (!strcmp (entry->key, "/apps/drwright/allow_unlock")) {
		if (entry->value->type == GCONF_VALUE_BOOL) {
			prefs->allow_unlock = gconf_value_get_bool (entry->value);
			allow_unlock_update (prefs);
		}
	}
	else if (!strcmp (entry->key, "/apps/drwright/unlock_phrase")) {
		if (entry->value->type == GCONF_VALUE_STRING) {
			g_free (prefs->unlock_phrase);
			prefs->unlock_phrase = g_strdup (gconf_value_get_string (entry->value));
			unlock_phrase_update (prefs);
		}
	}
}

static void
window_destroy_cb (GtkWidget      *widget,
		   DrwPreferences *prefs)
{
	gconf_client_notify_remove (client, prefs->notify_id);

	g_free (prefs->unlock_phrase);
	g_free (prefs);
}		  

DrwPreferences *
drw_preferences_new (void)
{
	DrwPreferences *prefs;
	GladeXML       *glade;
	GtkWidget      *dialog;
//	GtkWidget      *widget;
	GtkSizeGroup   *group;

	prefs = g_new0 (DrwPreferences, 1);

	prefs->notify_id = gconf_client_notify_add (client, "/apps/drwright",
						    gconf_notify_cb,
						    prefs,
						    NULL,
						    NULL);
	
	prefs->type_time = gconf_client_get_int (
		client, "/apps/drwright/type_time", NULL);
	
	prefs->break_time = gconf_client_get_int (
		client, "/apps/drwright/break_time", NULL);
	
	prefs->warn_time = gconf_client_get_int (
		client, "/apps/drwright/warn_time", NULL);
	
	prefs->allow_unlock = gconf_client_get_bool (
		client, "/apps/drwright/allow_unlock", NULL);
	
	prefs->unlock_phrase = g_strdup (gconf_client_get_string (
		client, "/apps/drwright/unlock_phrase", NULL));
	
	glade = glade_xml_new (GLADEDIR "/drw-preferences.glade",
			       NULL,
			       NULL);

	dialog = glade_xml_get_widget (glade, "dialog");

	prefs->type_sb = glade_xml_get_widget (glade, "type_spinbutton");
	type_time_update (prefs);
	g_signal_connect (prefs->type_sb,
			  "value-changed",
			  G_CALLBACK (type_time_changed_cb),
			  prefs);
	
	prefs->break_sb = glade_xml_get_widget (glade, "break_spinbutton");
	break_time_update (prefs);
	g_signal_connect (prefs->break_sb,
			  "value-changed",
			  G_CALLBACK (break_time_changed_cb),
			  prefs);

	prefs->warn_sb = glade_xml_get_widget (glade, "warn_spinbutton");
	warn_time_update (prefs);
	g_signal_connect (prefs->warn_sb,
			  "value-changed",
			  G_CALLBACK (warn_time_changed_cb),
			  prefs);

	prefs->unlock_entry = glade_xml_get_widget (glade, "unlock_entry");
	unlock_phrase_update (prefs);
	g_signal_connect (prefs->unlock_entry,
			  "changed",
			  G_CALLBACK (unlock_phrase_changed_cb),
			  prefs);

	prefs->unlock_cb = glade_xml_get_widget (glade, "unlock_checkbutton");
	allow_unlock_update (prefs);
	g_signal_connect (prefs->unlock_cb,
			  "toggled",
			  G_CALLBACK (allow_unlock_toggled_cb),
			  prefs);

	g_signal_connect (dialog,
			  "response",
			  G_CALLBACK (gtk_widget_destroy),
			  prefs);

	g_signal_connect (dialog,
			  "destroy",
			  G_CALLBACK (window_destroy_cb),
			  prefs);

	group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "type_pad_label"));
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "warn_pad_label"));
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "break_pad_label"));
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "unlock_pad_label"));
	g_object_unref (group);

	group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "type_label"));
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "warn_label"));
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "break_label"));
	g_object_unref (group);

	group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "type_spinbutton"));
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "warn_spinbutton"));
	gtk_size_group_add_widget (group, glade_xml_get_widget (glade, "break_spinbutton"));
	g_object_unref (group);

	gtk_widget_show (dialog);

	g_object_unref (glade);
	
	return prefs;
}


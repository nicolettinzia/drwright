/*
 * Copyright (C) 2010 Intel, Inc
 * Copyright © 2010 Christian Persch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Thomas Wood <thomas.wood@intel.com>
 *         Christian Persch <chpe@gnome.org>
 */

#include <config.h>

#include "drw-cc-panel.h"

#define DRW_SETTINGS_SCHEMA_ID "org.gnome.settings-daemon.plugins.typing-break"

G_DEFINE_DYNAMIC_TYPE (DrwCcPanel, drw_cc_panel, CC_TYPE_PANEL)

#if 0
#define DRW_PANEL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DRW_TYPE_CC_PANEL, DrwCcPanelPrivate))

struct _DrwCcPanelPrivate
{
  gpointer dummy;
};
#endif

static void
drw_cc_panel_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
drw_cc_panel_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
drw_cc_panel_dispose (GObject *object)
{
  G_OBJECT_CLASS (drw_cc_panel_parent_class)->dispose (object);
}

static void
drw_cc_panel_finalize (GObject *object)
{
  G_OBJECT_CLASS (drw_cc_panel_parent_class)->finalize (object);
}

static void
drw_cc_panel_class_init (DrwCcPanelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  /* g_type_class_add_private (klass, sizeof (DrwCcPanelPrivate)); */

  object_class->get_property = drw_cc_panel_get_property;
  object_class->set_property = drw_cc_panel_set_property;
  object_class->dispose = drw_cc_panel_dispose;
  object_class->finalize = drw_cc_panel_finalize;
}

static void
drw_cc_panel_class_finalize (DrwCcPanelClass *klass)
{
}

static void
drw_cc_panel_init (DrwCcPanel *self)
{
  GtkBuilder *builder;
  GError *error = NULL;
  GtkWidget *widget;
  GSettings *settings;

  /* priv = self->priv = DRW_CC_PANEL_GET_PRIVATE (self); */

  builder = gtk_builder_new ();
  gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);

  if (!gtk_builder_add_from_file (builder, BUILDERDIR "/drwright-panel.ui", &error))
    g_error ("%s", error->message);

  widget = (GtkWidget *) gtk_builder_get_object (builder, "typing-break-preferences-box");
  gtk_widget_reparent (widget, GTK_WIDGET (self));

  widget = (GtkWidget *) gtk_builder_get_object (builder, "toplevel");
  gtk_widget_destroy (widget);

  /* Now connect the settings */
  settings = g_settings_new (DRW_SETTINGS_SCHEMA_ID);

  g_settings_bind (settings, "enabled",
                   gtk_builder_get_object (builder, "break-enabled-checkbutton"),
                   "active",
                   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (settings, "enabled",
                   gtk_builder_get_object (builder, "inner-box"),
                   "sensitive",
                   G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "type-time",
                   gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "work-interval-spinbutton"))),
                   "value",
                   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (settings, "break-time",
                   gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "break-interval-spinbutton"))),
                   "value",
                   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (settings, "allow-postpone",
                   gtk_builder_get_object (builder, "allow-postpone-checkbutton"),
                   "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_object_unref (settings);
  g_object_unref (builder);
}

void
drw_cc_panel_register (GIOModule *module)
{
  drw_cc_panel_register_type (G_TYPE_MODULE (module));
  g_io_extension_point_implement (CC_SHELL_PANEL_EXTENSION_POINT,
                                  DRW_TYPE_CC_PANEL,
                                  "typing-break", 0);
}
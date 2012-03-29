/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2003 Richard Hult <richard@imendio.com>
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
#include <unistd.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include "drw-utils.h"

static GdkPixbuf *
create_tile_pixbuf (GdkPixbuf    *dest_pixbuf,
		    GdkPixbuf    *src_pixbuf,
		    GdkRectangle *field_geom,
		    guint         alpha,
		    GdkColor     *bg_color) 
{
	gboolean need_composite;
	gboolean use_simple;
	gdouble  cx, cy;
	gdouble  colorv;
	gint     pwidth, pheight;

	need_composite = (alpha < 255 || gdk_pixbuf_get_has_alpha (src_pixbuf));
	use_simple = (dest_pixbuf == NULL);

	if (dest_pixbuf == NULL)
		dest_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
					      FALSE, 8,
					      field_geom->width, field_geom->height);

	if (need_composite && use_simple)
		colorv = ((bg_color->red & 0xff00) << 8) |
			(bg_color->green & 0xff00) |
			((bg_color->blue & 0xff00) >> 8);
	else
		colorv = 0;

	pwidth = gdk_pixbuf_get_width (src_pixbuf);
	pheight = gdk_pixbuf_get_height (src_pixbuf);

	for (cy = 0; cy < field_geom->height; cy += pheight) {
		for (cx = 0; cx < field_geom->width; cx += pwidth) {
			if (need_composite && !use_simple)
				gdk_pixbuf_composite (src_pixbuf, dest_pixbuf,
						      cx, cy,
						      MIN (pwidth, field_geom->width - cx), 
						      MIN (pheight, field_geom->height - cy),
						      cx, cy,
						      1.0, 1.0,
						      GDK_INTERP_BILINEAR,
						      alpha);
			else if (need_composite && use_simple)
				gdk_pixbuf_composite_color (src_pixbuf, dest_pixbuf,
							    cx, cy,
							    MIN (pwidth, field_geom->width - cx), 
							    MIN (pheight, field_geom->height - cy),
							    cx, cy,
							    1.0, 1.0,
							    GDK_INTERP_BILINEAR,
							    alpha,
							    65536, 65536, 65536,
							    colorv, colorv);
			else
				gdk_pixbuf_copy_area (src_pixbuf,
						      0, 0,
						      MIN (pwidth, field_geom->width - cx),
						      MIN (pheight, field_geom->height - cy),
						      dest_pixbuf,
						      cx, cy);
		}
	}

	return dest_pixbuf;
}

static gboolean
window_draw_event (GtkWidget      *widget,
		   cairo_t        *context,
		   gpointer        data)
{
	cairo_t         *cr;
	cairo_surface_t *surface;
	int              width;
	int              height;

	cairo_set_operator (context, CAIRO_OPERATOR_SOURCE);
	gtk_window_get_size (GTK_WINDOW (widget), &width, &height);

	surface = cairo_surface_create_similar (cairo_get_target (context),
						CAIRO_CONTENT_COLOR_ALPHA,
						width,
						height);

	if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS) {
		goto done;
	}

	cr = cairo_create (surface);
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) {
		goto done;
	}
	cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	cairo_paint (cr);

	/* draw a box */
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 0.5);
	cairo_fill (cr);

	cairo_destroy (cr);

	cairo_set_source_surface (context, surface, 0, 0);
	cairo_paint (context);

 done:
	if (surface != NULL) {
		cairo_surface_destroy (surface);
	}

	return FALSE;
}

static void
set_surface_background (GtkWidget *window)
{
	GdkScreen    *screen;
	GdkPixbuf    *pixbuf, *tile_pixbuf;
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	GdkRectangle  rect;
	GdkColor      color;
	gint          width, height;
        cairo_t      *cr;

	gtk_widget_realize (window);

	screen = gtk_widget_get_screen (window);
	width = gdk_screen_get_width (screen);
	height = gdk_screen_get_height (screen);

	pixbuf = gdk_pixbuf_new_from_resource ("/org/gnome/drwright/ocean-stripes.png", NULL);

	rect.x = 0;
	rect.y = 0;
	rect.width = width;
	rect.height = height;

	color.red = 0;
	color.blue = 0;
	color.green = 0;

	tile_pixbuf = create_tile_pixbuf (NULL,
					  pixbuf,
					  &rect,
					  155,
					  &color);

	g_object_unref (pixbuf);

	cr = gdk_cairo_create (gtk_widget_get_window (window));
	surface = cairo_surface_create_similar (cairo_get_target (cr),
						CAIRO_CONTENT_COLOR_ALPHA,
						width,
						height);
	cairo_destroy (cr);

        cr = cairo_create (surface);
        gdk_cairo_set_source_pixbuf (cr, tile_pixbuf, 0, 0);
        cairo_paint (cr);
        cairo_destroy (cr);
	g_object_unref (tile_pixbuf);

	pattern = cairo_pattern_create_for_surface (surface);
	cairo_surface_destroy (surface);

	gdk_window_set_background_pattern (gtk_widget_get_window (window), pattern);
	cairo_pattern_destroy (pattern);
}

void
drw_setup_background (GtkWidget *window)
{
	GdkScreen    *screen;
	GdkVisual    *visual;
	gboolean      is_composited;

	screen = gtk_widget_get_screen (window);
	visual = gdk_screen_get_rgba_visual (screen);
	if (visual == NULL) {
                visual = gdk_screen_get_system_visual (screen);
	}

	if (visual != NULL && gdk_screen_is_composited (screen)) {
		gtk_widget_set_visual (GTK_WIDGET (window), visual);
		is_composited = TRUE;
	} else {
		is_composited = FALSE;
	}

	if (is_composited) {
		g_signal_connect (window, "draw", G_CALLBACK (window_draw_event), window);
	} else {
		set_surface_background (window);
	}
}

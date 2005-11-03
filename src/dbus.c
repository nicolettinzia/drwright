/*
 * Copyright (C) 2003-2004 Richard Hult <richard@imendio.com>
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
#include <gtk/gtkmain.h>
#include "dbus.h"

static void              server_unregistered_func (DBusConnection *connection,
						   gpointer        user_data);
static DBusHandlerResult server_message_func      (DBusConnection *connection,
						   DBusMessage    *message,
						   gpointer        user_data);


static DBusConnection *bus_conn;

static DBusObjectPathVTable
server_vtable = {
	server_unregistered_func,
	server_message_func,
	NULL,
};


static gboolean
dbus_init_service (void)
{
	DBusError error;

	if (bus_conn)
		return TRUE;
  
	dbus_error_init (&error);
	bus_conn = dbus_bus_get (DBUS_BUS_SESSION, &error);
  
	if (!bus_conn) {
		g_warning ("Failed to connect to the D-BUS daemon: %s", error.message);
		dbus_error_free (&error);
		return FALSE;
	}
  
	dbus_bus_request_name (bus_conn, DRWRIGHT_DBUS_SERVICE, 0, &error);
	if (dbus_error_is_set (&error)) {
		g_warning ("Failed to acquire drwright service");
		dbus_error_free (&error);
		return FALSE;
	}

	if (!dbus_connection_register_object_path (bus_conn,
						   "/org/gnome/TypingMonitor",
						   &server_vtable,
						   NULL)) {
		g_warning ("Failed to register server object with the D-BUS bus daemon");
		return FALSE;
	}
  
	dbus_connection_setup_with_g_main (bus_conn, NULL);
  
	return TRUE;
}

static void
server_unregistered_func (DBusConnection *connection, void *user_data)
{
}

static DBusHandlerResult
server_message_func (DBusConnection *connection,
		     DBusMessage    *message,
		     gpointer        user_data)
{
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

gboolean
dbus_emit_break (gboolean started)
{
	DBusMessage *message;

	if (!dbus_init_service ()) {
		return FALSE;
	}

	message = dbus_message_new_signal (DRWRIGHT_DBUS_OBJECT,
					   DRWRIGHT_DBUS_INTERFACE,
					   started ? DRWRIGHT_DBUS_BREAK_STARTED: DRWRIGHT_DBUS_BREAK_FINISHED);
      
	dbus_connection_send (bus_conn, message, NULL);
  
	dbus_message_unref (message);

	return TRUE;
}


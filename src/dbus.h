/*
 * Copyright (C) 2004 Richard Hult <richard@imendio.com>
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

#ifndef __DBUS_H__
#define __DBUS_H__

#include <dbus/dbus-glib-lowlevel.h>

#define DRWRIGHT_DBUS_SERVICE "org.gnome.TypingMonitor"

#define DRWRIGHT_DBUS_INTERFACE "org.gnome.TypingMonitor"
#define DRWRIGHT_DBUS_OBJECT "/org/gnome/TypingMonitor"
#define DRWRIGHT_DBUS_BREAK_STARTED "BreakStarted"
#define DRWRIGHT_DBUS_BREAK_FINISHED "BreakFinished"

#endif /* __DBUS_H__ */

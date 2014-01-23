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
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DR_WRIGHT_H__
#define __DR_WRIGHT_H__

#define DRW_SETTINGS_SCHEMA_ID "org.gnome.settings-daemon.plugins.typing-break"

typedef struct _DrWright DrWright;

DrWright *drwright_new (void);

gboolean  drwright_can_lock_screen (DrWright *dr);

#endif /* __DR_WRIGHT_H__ */

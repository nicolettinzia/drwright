/*
 * Copyright (C) 2010 Intel, Inc
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Thomas Wood <thomas.wood@intel.com>
 */

#ifndef DRW_CC_PANEL_H
#define DRW_CC_PANEL_H

#include <glib-object.h>

G_BEGIN_DECLS

#define DRW_TYPE_CC_PANEL drw_cc_panel_get_type()

#define DRW_CC_PANEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  DRW_TYPE_CC_PANEL, DrwCcPanel))

#define DRW_CC_PANEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  DRW_TYPE_CC_PANEL, DrwCcPanelClass))

#define DRW_IS_CC_PANEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  DRW_TYPE_CC_PANEL))

#define DRW_IS_CC_PANEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  DRW_TYPE_CC_PANEL))

#define DRW_CC_PANEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  DRW_TYPE_CC_PANEL, DrwCcPanelClass))

typedef struct _DrwCcPanel DrwCcPanel;
typedef struct _DrwCcPanelClass DrwCcPanelClass;
typedef struct _DrwCcPanelPrivate DrwCcPanelPrivate;

GType drw_cc_panel_get_type (void) G_GNUC_CONST;

void  drw_cc_panel_register_type (GTypeModule *module);

G_END_DECLS

#endif /* DRW_CC_PANEL_H */

/* json-gvariant.h - JSON GVariant integration for JSON-C
 *
 * Derived from JSON-GLib and adapted to JSON-C
 * Copyright (C) 2007  OpenedHand Ltd.
 * Copyright (C) 2009  Intel Corp.
 * Copyright (C) 2013  Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   Emanuele Aina <emanuele.aina@collabora.com>
 *   Eduardo Lima Mitev  <elima@igalia.com>
 */

#ifndef __JSON_GVARIANT_H__
#define __JSON_GVARIANT_H__

#include <glib.h>

G_BEGIN_DECLS

GVariant * json_gvariant_deserialize_data (const gchar  *json,
                                           gssize        length,
                                           const gchar  *signature,
                                           GError      **error);

G_END_DECLS

#endif /* __JSON_GVARIANT_H__ */

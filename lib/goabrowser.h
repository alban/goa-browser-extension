/*
 * This file is part of the goa-browser-extension.
 * Copyright (C) Collabora Ltd. 2013
 *
 * Author: Emanuele Aina <emanuele.aina@collabora.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GOABROWSER_H
#define GOABROWSER_H

#include <glib.h>

G_BEGIN_DECLS

void goabrowser_login_detected (const gchar *domain,
                                const gchar *userid);

G_END_DECLS

#endif /* GOABROWSER_H */

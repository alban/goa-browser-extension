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

#include <glib-object.h>
#define GOA_API_IS_SUBJECT_TO_CHANGE
#include <goa/goa.h>

G_BEGIN_DECLS

#define GOABROWSER_TYPE_OBJECT            (goabrowser_object_get_type ())
#define GOABROWSER_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOABROWSER_TYPE_OBJECT, GoaBrowserObject))
#define GOABROWSER_IS_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOABROWSER_TYPE_OBJECT))
#define GOABROWSER_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GOABROWSER_TYPE_OBJECT, GoaBrowserObjectClass))
#define GOABROWSER_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOABROWSER_TYPE_OBJECT))
#define GOABROWSER_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GOABROWSER_TYPE_OBJECT, GoaBrowserObjectClass))

typedef struct _GoaBrowserObjectClass     GoaBrowserObjectClass;
typedef struct _GoaBrowserObject          GoaBrowserObject;
typedef struct _GoaBrowserObjectPrivate   GoaBrowserObjectPrivate;

struct _GoaBrowserObjectClass
{
    GObjectClass parent_instance;
};

struct _GoaBrowserObject {
    GObject                  parent_instance;
    GoaBrowserObjectPrivate *priv;
};

GType             goabrowser_object_get_type        (void) G_GNUC_CONST;
GoaBrowserObject *goabrowser_object_new             (GoaClient *client);
void              goabrowser_object_login_detected  (GoaBrowserObject *self,
                                                     const gchar      *domain,
                                                     const gchar      *userid);

G_END_DECLS

#endif /* GOABROWSER_H */

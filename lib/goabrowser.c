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

#include "goabrowser.h"

enum
{
    PROP_0,
    PROP_GOA_CLIENT,
    PROP_LAST
};

static GParamSpec *obj_props[PROP_LAST];

struct _GoaBrowserObjectPrivate {
    GoaClient *goa;
};

G_DEFINE_TYPE (GoaBrowserObject, goabrowser_object, G_TYPE_OBJECT)

static void
goabrowser_object_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  GoaBrowserObject *self = GOABROWSER_OBJECT (object);

  switch (property_id)
    {
      case PROP_GOA_CLIENT:
        self->priv->goa = GOA_CLIENT (g_value_dup_object (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
goabrowser_object_get_property (GObject      *object,
                                guint         property_id,
                                GValue       *value,
                                GParamSpec   *pspec)
{
  GoaBrowserObject *self = GOABROWSER_OBJECT (object);

  switch (property_id)
    {
      case PROP_GOA_CLIENT:
        g_value_take_object (value, self->priv->goa);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
goabrowser_object_class_init (GoaBrowserObjectClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GoaBrowserObjectPrivate));

  gobject_class->set_property = goabrowser_object_set_property;
  gobject_class->get_property = goabrowser_object_get_property;

  obj_props[PROP_GOA_CLIENT] =
    g_param_spec_object ("goa-client",
                         "GOA Client",
                         "The client used to talk with the GNOME Online Accounts daemon",
                         GOA_TYPE_CLIENT,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (gobject_class, PROP_LAST, obj_props);
}

static void
goabrowser_object_init (GoaBrowserObject *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GOABROWSER_TYPE_OBJECT,
                                            GoaBrowserObjectPrivate);
}

GoaBrowserObject *
goabrowser_object_new (GoaClient *client)
{
  return g_object_new (GOABROWSER_TYPE_OBJECT, "goa-client", client, NULL);
}

void
goabrowser_object_login_detected (GoaBrowserObject *self,
                                  const gchar      *domain,
                                  const gchar      *userid)
{
  g_debug ("%s() domain:%s userid:%s", G_STRFUNC, domain, userid);
}

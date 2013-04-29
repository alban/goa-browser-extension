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

#include "json-gvariant.h"

enum
{
    PROP_0,
    PROP_GOA_CLIENT,
    PROP_LAST
};

static GParamSpec *obj_props[PROP_LAST];

struct _GoaBrowserObjectPrivate {
    GoaClient *goa;
    GList *accounts;
};

G_DEFINE_TYPE (GoaBrowserObject, goabrowser_object, G_TYPE_OBJECT)

static void
on_account_added (GoaClient *client,
                  GoaObject *object,
                  gpointer   user_data)
{
  GoaBrowserObject *self = GOABROWSER_OBJECT (user_data);
  GoaBrowserObjectPrivate *priv = self->priv;

  g_debug ("%s()", G_STRFUNC);
  priv->accounts = g_list_prepend (priv->accounts, g_object_ref (object));
}

static void
on_account_removed (GoaClient *client,
                    GoaObject *object,
                    gpointer   user_data)
{
  GoaBrowserObject *self = GOABROWSER_OBJECT (user_data);
  GoaBrowserObjectPrivate *priv = self->priv;

  g_debug ("%s()", G_STRFUNC);
  priv->accounts = g_list_remove (priv->accounts, object);
  g_object_unref (object);
}


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
        g_signal_connect (self->priv->goa, "account-added", G_CALLBACK (on_account_added), self);
        g_signal_connect (self->priv->goa, "account-removed", G_CALLBACK (on_account_removed), self);
        self->priv->accounts = goa_client_get_accounts (self->priv->goa);
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
        g_value_set_object (value, self->priv->goa);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
goabrowser_object_dispose (GObject *object)
{
  GoaBrowserObject *self = GOABROWSER_OBJECT (object);
  GoaBrowserObjectPrivate *priv = self->priv;

  g_clear_object (&priv->goa);

  G_OBJECT_CLASS (goabrowser_object_parent_class)->dispose (object);
}

static void
goabrowser_object_finalize (GObject *object)
{
  GoaBrowserObject *self = GOABROWSER_OBJECT (object);
  GoaBrowserObjectPrivate *priv = self->priv;

  g_list_free_full (priv->accounts, g_object_unref);

  G_OBJECT_CLASS (goabrowser_object_parent_class)->finalize (object);
}

static void
goabrowser_object_class_init (GoaBrowserObjectClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GoaBrowserObjectPrivate));

  gobject_class->set_property = goabrowser_object_set_property;
  gobject_class->get_property = goabrowser_object_get_property;
  gobject_class->dispose = goabrowser_object_dispose;
  gobject_class->finalize = goabrowser_object_finalize;

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
  g_return_val_if_fail (client != NULL, NULL);
  return g_object_new (GOABROWSER_TYPE_OBJECT, "goa-client", client, NULL);
}

void
goabrowser_object_login_detected (GoaBrowserObject *self,
                                  gchar            *collected_data_json)
{
  static const gchar *app_id = "org.gnome.ControlCenter";
  static const gchar *action_id = "launch-panel";
  GError *error = NULL;
  GVariant *params, *preseed = NULL, *v;
  GVariantBuilder *builder = NULL;
  GApplication *gnomecc = NULL;
  g_debug ("%s()", G_STRFUNC);
  g_debug ("%s() collected data:\n%s", G_STRFUNC, collected_data_json);

  preseed = json_gvariant_deserialize_data (collected_data_json, -1, NULL, &error);
  if (preseed == NULL)
    {
      g_warning ("Unable to parse the request for the creation of a new GNOME Online Account: %s",
          error->message);
      g_error_free (error);
      goto out;
    }

  g_debug ("%s() requesting new account creation", G_STRFUNC);

  builder = g_variant_builder_new (G_VARIANT_TYPE ("av"));
  g_variant_builder_add (builder, "v", g_variant_new_string ("add"));
  g_variant_builder_add (builder, "v", g_variant_new ("a{sv}")); /* Flags, unused */
  v = g_variant_lookup_value (preseed, "provider", G_VARIANT_TYPE_STRING);
  g_variant_builder_add (builder, "v", v);
  g_variant_builder_add (builder, "v", preseed);
  params = g_variant_new ("(s@av)", "online-accounts", g_variant_builder_end (builder));

  gnomecc = g_application_new (app_id, G_APPLICATION_IS_LAUNCHER);
  if (!g_application_register (gnomecc, NULL, &error))
    {
      g_warning ("Failed to register launcher for %s: %s (%s, %d)", app_id,
          error->message, g_quark_to_string (error->domain), error->code);
      g_error_free (error);
      goto out;
    }
  g_debug ("%s() activating action '%s'", G_STRFUNC, action_id);
  g_action_group_activate_action (G_ACTION_GROUP (gnomecc), action_id, params);
out:
  g_clear_object (&gnomecc);
}

const GList *
goabrowser_object_list_accounts (GoaBrowserObject *self)
{
  GoaBrowserObjectPrivate *priv = self->priv;

  return priv->accounts;
}

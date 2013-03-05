/*
 * This file is part of the gnome-online-account-browser-plugin.
 * Copyright (C) Canonical Ltd. 2012
 * Copyright (C) Collabora Ltd. 2013
 *
 * Author:
 *   Emanuele Aina <emanuele.aina@collabora.com>
 *
 * Based on webaccounts-browser-plugin by:
 *   Alberto Mardegan <alberto.mardegan@canonical.com>
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
#include "object.h"

#include <string.h>
#include <glib.h>
#include <json.h>

typedef struct {
    NPObject object;
    NPP instance;
    NPObject *window;
    GoaBrowserObject *goa;
} GoaBrowserObjectWrapper;

#define METHODS                           \
  METHOD (loginDetected, login_detected)  \
  /* */

/* Method wrapper prototypes */
#define METHOD(name, symbol)                                              \
  static gboolean goabrowser_##symbol##_wrapper (NPObject        *object, \
                                                 const NPVariant *args,   \
                                                 uint32_t         argc,   \
                                                 NPVariant       *result);
METHODS
#undef METHOD

/* Method identifiers declaration */
#define METHOD(name, symbol) static NPIdentifier symbol##_id;
METHODS
#undef METHOD

static void
init_identifiers (void)
{
    static gboolean initialized = 0;
    if (g_atomic_int_compare_and_exchange (&initialized, 0, 1))
      {
        g_debug ("%s() initializing", G_STRFUNC);
        /* Method identifiers initialization */
#define METHOD(name, symbol) symbol##_id = NPN_GetStringIdentifier(#name);
        METHODS
#undef METHOD
      }
}

static gboolean
variant_to_string (const NPVariant *variant,
                   gchar **string)
{
    if (G_UNLIKELY (!NPVARIANT_IS_STRING (*variant)))
      return FALSE;
    *string = g_strndup (NPVARIANT_TO_STRING (*variant).UTF8Characters,
                         NPVARIANT_TO_STRING (*variant).UTF8Length);
    return TRUE;
}

static NPObject *
NPClass_Allocate (NPP instance, NPClass *aClass)
{
    GoaBrowserObjectWrapper *object = g_new0 (GoaBrowserObjectWrapper, 1);
    init_identifiers ();
    return (NPObject *)object;
}

static void
NPClass_Deallocate (NPObject *npobj)
{
    GoaBrowserObjectWrapper *wrapper = (GoaBrowserObjectWrapper*)npobj;
    NPN_ReleaseObject (wrapper->window);
    g_clear_object (&wrapper->goa);
    g_free (wrapper);
}

static void
NPClass_Invalidate (NPObject *npobj)
{
}

static bool
NPClass_HasMethod (NPObject *npobj, NPIdentifier id)
{
#define METHOD(name, symbol) (id == (symbol##_id)) ||
    /* expands to (id == login_detected_id) || ... || FALSE; */
    return METHODS FALSE;
#undef METHOD
}


static bool
NPClass_Invoke (NPObject *npobj, NPIdentifier id,
                const NPVariant *args, uint32_t argc, NPVariant *result)
{
    g_return_val_if_fail (npobj != NULL, FALSE);

    VOID_TO_NPVARIANT (*result);

#define METHOD(name, symbol)                                      \
    if (id == symbol##_id)                                        \
      return goabrowser_##symbol##_wrapper (npobj, args, argc, result);
    METHODS
#undef METHOD

    return FALSE;
}

static bool
NPClass_InvokeDefault (NPObject *npobj, const NPVariant *args, uint32_t argc,
                       NPVariant *result)
{
    return FALSE;
}

static bool
NPClass_HasProperty (NPObject *npobj, NPIdentifier name)
{
    return FALSE;
}

static bool
NPClass_GetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
    return FALSE;
}


static bool
NPClass_SetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
    return FALSE;
}


static bool
NPClass_RemoveProperty (NPObject *npobj, NPIdentifier name)
{
    return FALSE;
}


static bool
NPClass_Enumerate (NPObject *npobj, NPIdentifier **identifier, uint32_t *count)
{
    return FALSE;
}


static bool
NPClass_Construct (NPObject *npobj, const NPVariant *args, uint32_t argc,
                   NPVariant *result)
{
    return FALSE;
}

static NPClass js_object_class = {
    .structVersion = NP_CLASS_STRUCT_VERSION,
    .allocate = NPClass_Allocate,
    .deallocate = NPClass_Deallocate,
    .invalidate = NPClass_Invalidate,
    .hasMethod = NPClass_HasMethod,
    .invoke = NPClass_Invoke,
    .invokeDefault = NPClass_InvokeDefault,
    .hasProperty = NPClass_HasProperty,
    .getProperty = NPClass_GetProperty,
    .setProperty = NPClass_SetProperty,
    .removeProperty = NPClass_RemoveProperty,
    .enumerate = NPClass_Enumerate,
    .construct = NPClass_Construct
};

NPObject *
goabrowser_create_plugin_object (NPP instance, NPObject *window, GoaClient *client)
{
    NPObject *object = NPN_CreateObject (instance, &js_object_class);
    GoaBrowserObjectWrapper *wrapper = (GoaBrowserObjectWrapper*)object;
    g_return_val_if_fail (wrapper != NULL, NULL);
    g_debug ("%s()", G_STRFUNC);
    wrapper->instance = instance;
    wrapper->window = NPN_RetainObject (window);
    wrapper->goa = goabrowser_object_new (client);
    return object;
}

static gboolean
is_valid_json (const gchar* data)
{
    enum json_tokener_error error = json_tokener_success;
    json_object *json = json_tokener_parse_verbose (data, &error);
    if (G_UNLIKELY (error != json_tokener_success))
      {
        g_debug ("%s() failed to parse argument #1 (collectedData) as JSON: %s", G_STRFUNC,
                 json_tokener_error_desc(error));
        return FALSE;
    }
    json_object_put (json);
    return TRUE;
}

static gboolean
goabrowser_login_detected_wrapper (NPObject *object,
                                   const NPVariant *args,
                                   uint32_t argc,
                                   NPVariant *result)
{
    GoaBrowserObjectWrapper *wrapper = (GoaBrowserObjectWrapper*)object;
    gchar *collected_data = NULL;
    gboolean success = TRUE;

    g_debug ("%s()", G_STRFUNC);

    if (G_UNLIKELY (argc < 1 || !variant_to_string (&args[0], &collected_data) || collected_data == NULL))
      {
        g_debug ("%s() JSON-encoded string expected for argument #1 (collectedData)", G_STRFUNC);
        success = FALSE;
        goto out;
    }

    if (G_UNLIKELY (!is_valid_json (collected_data)))
      {
        success = FALSE;
        goto out;
      }

    goabrowser_object_login_detected (wrapper->goa, collected_data);
out:
    g_free (collected_data);
    return success;
}


/* json-gvariant.h - JSON GVariant integration for JSON-C
 *
 * Derived from JSON-GLib and adapted to JSON-C
 * Copyright (C) 2007  OpenedHand Ltd.
 * Copyright (C) 2013  Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Author:
 *   Emanuele Aina <emanuele.aina@collabora.com>
 *   Eduardo Lima Mitev  <elima@igalia.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <glib/gi18n-lib.h>

#include "json-gvariant.h"

#include <json.h>
#include <glib-object.h>
#include <gio/gio.h>

/* custom extension to the GVariantClass enumeration to differentiate
 * a single dictionary entry from an array of dictionary entries
 */
#define JSON_G_VARIANT_CLASS_DICTIONARY 'c'

static GVariant * json_to_gvariant_recurse (json_object   *json_node,
                                            const gchar  **signature,
                                            GError       **error);

/* ========================================================================== */
/* JSON to GVariant */
/* ========================================================================== */

static GVariantClass
json_to_gvariant_get_next_class (json_object  *json_node,
                                 const gchar **signature)
{
  if (signature == NULL)
    {
      GVariantClass class = 0;

      switch (json_object_get_type (json_node))
        {
        case json_type_boolean:
          class = G_VARIANT_CLASS_BOOLEAN;
          break;

        case json_type_int:
          class = G_VARIANT_CLASS_INT64;
          break;

        case json_type_double:
          class = G_VARIANT_CLASS_DOUBLE;
          break;

        case json_type_string:
          class = G_VARIANT_CLASS_STRING;
          break;

        case json_type_array:
          class = G_VARIANT_CLASS_ARRAY;
          break;

        case json_type_object:
          class = JSON_G_VARIANT_CLASS_DICTIONARY;
          break;

        case json_type_null:
          class = G_VARIANT_CLASS_MAYBE;
          break;
        }

      return class;
    }
  else
    {
      if ((*signature)[0] == G_VARIANT_CLASS_ARRAY &&
          (*signature)[1] == G_VARIANT_CLASS_DICT_ENTRY)
        return JSON_G_VARIANT_CLASS_DICTIONARY;
      else
        return (*signature)[0];
    }
}

static gboolean
json_node_assert_type (json_object     *json_node,
                       json_type        type,
                       GType            sub_type,
                       GError         **error)
{
  if (!json_object_is_type (json_node, type))
    {
      /* translators: the '%s' is the type name */
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVALID_DATA,
                   _("Unexpected type '%s' in JSON node"),
                   json_type_to_name (json_object_get_type (json_node)));
      return FALSE;
    }
  else
    {
      return TRUE;
    }
}

static void
json_to_gvariant_foreach_add (gpointer data, gpointer user_data)
{
  GVariantBuilder *builder = (GVariantBuilder *) user_data;
  GVariant *child = (GVariant *) data;

  g_variant_builder_add_value (builder, child);
}

static void
json_to_gvariant_foreach_free (gpointer data, gpointer user_data)
{
  GVariant *child = (GVariant *) data;

  g_variant_unref (child);
}

static GVariant *
json_to_gvariant_build_from_glist (GList *list, const gchar *signature)
{
  GVariantBuilder *builder;
  GVariant *result;

  builder = g_variant_builder_new (G_VARIANT_TYPE (signature));

  g_list_foreach (list, json_to_gvariant_foreach_add, builder);
  result = g_variant_builder_end (builder);

  g_variant_builder_unref (builder);

  return result;
}

static GVariant *
json_to_gvariant_tuple (json_object  *json_node,
                        const gchar **signature,
                        GError      **error)
{
  GVariant *variant = NULL;
  gint i;
  GList *children = NULL;
  gboolean roll_back = FALSE;
  const gchar *initial_signature;

  initial_signature = *signature;
  (*signature)++;
  i = 1;
  while ((*signature)[0] != ')' && (*signature)[0] != '\0')
    {
      json_object *json_child;
      GVariant *variant_child;

      if (i - 1 >= json_object_array_length (json_node))
        {
          g_set_error_literal (error,
                               G_IO_ERROR,
                               G_IO_ERROR_INVALID_DATA,
                               _("Missing elements in JSON array to conform to a tuple"));
          roll_back = TRUE;
          break;
        }

      json_child = json_object_array_get_idx (json_node, i - 1);

      variant_child = json_to_gvariant_recurse (json_child, signature, error);
      if (variant_child != NULL)
        {
          children = g_list_append (children, variant_child);
        }
      else
        {
          roll_back = TRUE;
          break;
        }

      i++;
    }

  if (! roll_back)
    {
      if ( (*signature)[0] != ')')
        {
          g_set_error_literal (error,
                               G_IO_ERROR,
                               G_IO_ERROR_INVALID_DATA,
                               _("Missing closing symbol ')' in the GVariant tuple type"));
          roll_back = TRUE;
        }
      else if (json_object_array_length (json_node) >= i)
        {
          g_set_error_literal (error,
                               G_IO_ERROR,
                               G_IO_ERROR_INVALID_DATA,
                               _("Unexpected extra elements in JSON array"));
          roll_back = TRUE;
        }
      else
        {
          gchar *tuple_type;

          tuple_type = g_strndup (initial_signature,
                                  (*signature) - initial_signature + 1);

          variant = json_to_gvariant_build_from_glist (children, tuple_type);

          g_free (tuple_type);
        }
    }

  if (roll_back)
    g_list_foreach (children, json_to_gvariant_foreach_free, NULL);

  g_list_free (children);

  return variant;
}

static gchar *
signature_get_next_complete_type (const gchar **signature)
{
  GVariantClass class;
  const gchar *initial_signature;
  gchar *result;

  /* here it is assumed that 'signature' is a valid type string */

  initial_signature = *signature;
  class = (*signature)[0];

  if (class == G_VARIANT_CLASS_TUPLE || class == G_VARIANT_CLASS_DICT_ENTRY)
    {
      gchar stack[256] = {0};
      guint stack_len = 0;

      do
        {
          if ( (*signature)[0] == G_VARIANT_CLASS_TUPLE)
            {
              stack[stack_len] = ')';
              stack_len++;
            }
          else if ( (*signature)[0] == G_VARIANT_CLASS_DICT_ENTRY)
            {
              stack[stack_len] = '}';
              stack_len++;
            }

          (*signature)++;

          if ( (*signature)[0] == stack[stack_len - 1])
            stack_len--;
        }
      while (stack_len > 0);

      (*signature)++;
    }
  else if (class == G_VARIANT_CLASS_ARRAY || class == G_VARIANT_CLASS_MAYBE)
    {
      gchar *tmp_sig;

      (*signature)++;
      tmp_sig = signature_get_next_complete_type (signature);
      g_free (tmp_sig);
    }
  else
    {
      (*signature)++;
    }

  result = g_strndup (initial_signature, (*signature) - initial_signature);

  return result;
}

static GVariant *
json_to_gvariant_maybe (json_object  *json_node,
                        const gchar **signature,
                        GError      **error)
{
  GVariant *variant = NULL;
  GVariant *value;
  gchar *maybe_signature;

  if (signature)
    {
      (*signature)++;
      maybe_signature = signature_get_next_complete_type (signature);
    }
  else
    {
      maybe_signature = g_strdup ("v");
    }

  if (json_object_is_type (json_node, json_type_null))
    {
      variant = g_variant_new_maybe (G_VARIANT_TYPE (maybe_signature), NULL);
    }
  else
    {
      const gchar *tmp_signature;

      tmp_signature = maybe_signature;
      value = json_to_gvariant_recurse (json_node,
                                        &tmp_signature,
                                        error);

      if (value != NULL)
        variant = g_variant_new_maybe (G_VARIANT_TYPE (maybe_signature), value);
    }

  g_free (maybe_signature);

  /* compensate the (*signature)++ call at the end of 'recurse()' */
  if (signature)
    (*signature)--;

  return variant;
}

static GVariant *
json_to_gvariant_array (json_object  *json_node,
                        const gchar **signature,
                        GError      **error)
{
  GVariant *variant = NULL;
  GList *children = NULL;
  gboolean roll_back = FALSE;
  const gchar *orig_signature;
  gchar *child_signature;

  if (signature != NULL)
    {
      orig_signature = *signature;

      (*signature)++;
      child_signature = signature_get_next_complete_type (signature);
    }
  else
    child_signature = g_strdup ("v");

  if (json_object_array_length (json_node) > 0)
    {
      gint i;
      guint len;

      len = json_object_array_length (json_node);
      for (i = 0; i < len; i++)
        {
          json_object *json_child;
          GVariant *variant_child;
          const gchar *tmp_signature;

          json_child = json_object_array_get_idx (json_node, i);

          tmp_signature = child_signature;
          variant_child = json_to_gvariant_recurse (json_child,
                                                    &tmp_signature,
                                                    error);
          if (variant_child != NULL)
            {
              children = g_list_append (children, variant_child);
            }
          else
            {
              roll_back = TRUE;
              break;
            }
        }
    }

  if (!roll_back)
    {
      gchar *array_signature;

      if (signature)
        array_signature = g_strndup (orig_signature, (*signature) - orig_signature);
      else
        array_signature = g_strdup ("av");

      variant = json_to_gvariant_build_from_glist (children, array_signature);

      g_free (array_signature);

      /* compensate the (*signature)++ call at the end of 'recurse()' */
      if (signature)
        (*signature)--;
    }
  else
    g_list_foreach (children, json_to_gvariant_foreach_free, NULL);

  g_list_free (children);
  g_free (child_signature);

  return variant;
}

static GVariant *
gvariant_simple_from_string (const gchar    *st,
                             GVariantClass   class,
                             GError        **error)
{
  GVariant *variant = NULL;
  gchar *nptr = NULL;

  errno = 0;

  switch (class)
    {
    case G_VARIANT_CLASS_BOOLEAN:
      if (g_strcmp0 (st, "true") == 0)
        variant = g_variant_new_boolean (TRUE);
      else if (g_strcmp0 (st, "false") == 0)
        variant = g_variant_new_boolean (FALSE);
      else
        errno = 1;
      break;

    case G_VARIANT_CLASS_BYTE:
      variant = g_variant_new_byte (g_ascii_strtoll (st, &nptr, 10));
      break;

    case G_VARIANT_CLASS_INT16:
      variant = g_variant_new_int16 (g_ascii_strtoll (st, &nptr, 10));
      break;

    case G_VARIANT_CLASS_UINT16:
      variant = g_variant_new_uint16 (g_ascii_strtoll (st, &nptr, 10));
      break;

    case G_VARIANT_CLASS_INT32:
      variant = g_variant_new_int32 (g_ascii_strtoll (st, &nptr, 10));
      break;

    case G_VARIANT_CLASS_UINT32:
      variant = g_variant_new_uint32 (g_ascii_strtoull (st, &nptr, 10));
      break;

    case G_VARIANT_CLASS_INT64:
      variant = g_variant_new_int64 (g_ascii_strtoll (st, &nptr, 10));
      break;

    case G_VARIANT_CLASS_UINT64:
      variant = g_variant_new_uint64 (g_ascii_strtoull (st, &nptr, 10));
      break;

    case G_VARIANT_CLASS_HANDLE:
      variant = g_variant_new_handle (strtol (st, &nptr, 10));
      break;

    case G_VARIANT_CLASS_DOUBLE:
      variant = g_variant_new_double (g_ascii_strtod (st, &nptr));
      break;

    case G_VARIANT_CLASS_STRING:
    case G_VARIANT_CLASS_OBJECT_PATH:
    case G_VARIANT_CLASS_SIGNATURE:
      variant = g_variant_new_string (st);
      break;

    default:
      g_assert_not_reached ();
      break;
    }

  if (errno != 0 || nptr == st)
    {
      g_set_error_literal (error,
                           G_IO_ERROR,
                           G_IO_ERROR_INVALID_DATA,
                           _("Invalid string value converting to GVariant"));
      if (variant != NULL)
        {
          g_variant_unref (variant);
          variant = NULL;
        }
    }

  return variant;
}

static void
parse_dict_entry_signature (const gchar **signature,
                            gchar       **entry_signature,
                            gchar       **key_signature,
                            gchar       **value_signature)
{
  const gchar *tmp_sig;

  if (signature != NULL)
    *entry_signature = signature_get_next_complete_type (signature);
  else
    *entry_signature = g_strdup ("{sv}");

  tmp_sig = (*entry_signature) + 1;
  *key_signature = signature_get_next_complete_type (&tmp_sig);
  *value_signature = signature_get_next_complete_type (&tmp_sig);
}

static GVariant *
json_to_gvariant_dict_entry (json_object  *json_node,
                             const gchar **signature,
                             GError      **error)
{
  GVariant *variant = NULL;

  gchar *entry_signature;
  gchar *key_signature;
  gchar *value_signature;
  const gchar *tmp_signature;

  struct lh_entry *member;

  const gchar *json_member;
  json_object *json_value;
  GVariant *variant_member;
  GVariant *variant_value;
  struct json_object_iter iter;
  int count = 0;

  json_object_object_foreachC (json_node, iter)
    count++;

  if (count != 1)
    {
      g_set_error_literal (error,
                           G_IO_ERROR,
                           G_IO_ERROR_INVALID_DATA,
                           _("A GVariant dictionary entry expects a JSON object with exactly one member"));
      return NULL;
    }

  parse_dict_entry_signature (signature,
                              &entry_signature,
                              &key_signature,
                              &value_signature);

  member = json_object_get_object(json_node)->head;

  json_member = member->k;
  variant_member = gvariant_simple_from_string (json_member,
                                                key_signature[0],
                                                error);
  if (variant_member != NULL)
    {
      json_value = json_object_object_get (json_node, json_member);

      tmp_signature = value_signature;
      variant_value = json_to_gvariant_recurse (json_value,
                                                &tmp_signature,
                                                error);

      if (variant_value != NULL)
        {
          GVariantBuilder *builder;

          builder = g_variant_builder_new (G_VARIANT_TYPE (entry_signature));
          g_variant_builder_add_value (builder, variant_member);
          g_variant_builder_add_value (builder, variant_value);
          variant = g_variant_builder_end (builder);

          g_variant_builder_unref (builder);
        }
    }

  g_free (value_signature);
  g_free (key_signature);
  g_free (entry_signature);

  /* compensate the (*signature)++ call at the end of 'recurse()' */
  if (signature)
    (*signature)--;

  return variant;
}

static GVariant *
json_to_gvariant_dictionary (json_object  *json_node,
                             const gchar **signature,
                             GError      **error)
{
  GVariant *variant = NULL;
  gboolean roll_back = FALSE;

  gchar *dict_signature;
  gchar *entry_signature;
  gchar *key_signature;
  gchar *value_signature;
  const gchar *tmp_signature;

  GVariantBuilder *builder;
  GList *members;
  GList *member;

  if (signature != NULL)
    (*signature)++;

  parse_dict_entry_signature (signature,
                              &entry_signature,
                              &key_signature,
                              &value_signature);

  dict_signature = g_strdup_printf ("a%s", entry_signature);

  builder = g_variant_builder_new (G_VARIANT_TYPE (dict_signature));

  json_object_object_foreach (json_node, json_member, json_value)
    {
      GVariant *variant_member;
      GVariant *variant_value;

      variant_member = gvariant_simple_from_string (json_member,
                                                    key_signature[0],
                                                    error);
      if (variant_member == NULL)
        {
          roll_back = TRUE;
          break;
        }

      tmp_signature = value_signature;
      variant_value = json_to_gvariant_recurse (json_value,
                                                &tmp_signature,
                                                error);

      if (variant_value != NULL)
        {
          g_variant_builder_open (builder, G_VARIANT_TYPE (entry_signature));
          g_variant_builder_add_value (builder, variant_member);
          g_variant_builder_add_value (builder, variant_value);
          g_variant_builder_close (builder);
        }
      else
        {
          roll_back = TRUE;
          break;
        }
    }

  if (! roll_back)
    variant = g_variant_builder_end (builder);

  g_variant_builder_unref (builder);
  g_free (value_signature);
  g_free (key_signature);
  g_free (entry_signature);
  g_free (dict_signature);

  /* compensate the (*signature)++ call at the end of 'recurse()' */
  if (signature != NULL)
    (*signature)--;

  return variant;
}

static GVariant *
json_to_gvariant_recurse (json_object   *json_node,
                          const gchar  **signature,
                          GError       **error)
{
  GVariant *variant = NULL;
  GVariantClass class;

  class = json_to_gvariant_get_next_class (json_node, signature);

  if (class == JSON_G_VARIANT_CLASS_DICTIONARY)
    {
      if (json_node_assert_type (json_node, json_type_object, 0, error))
        variant = json_to_gvariant_dictionary (json_node, signature, error);

      goto out;
    }

  switch (class)
    {
    case G_VARIANT_CLASS_BOOLEAN:
      if (json_node_assert_type (json_node, json_type_boolean, 0, error))
        variant = g_variant_new_boolean (json_object_get_boolean (json_node));
      break;

    case G_VARIANT_CLASS_BYTE:
      if (json_node_assert_type (json_node, json_type_int, 0, error))
        variant = g_variant_new_byte (json_object_get_int (json_node));
      break;

    case G_VARIANT_CLASS_INT16:
      if (json_node_assert_type (json_node, json_type_int, 0, error))
        variant = g_variant_new_int16 (json_object_get_int (json_node));
      break;

    case G_VARIANT_CLASS_UINT16:
      if (json_node_assert_type (json_node, json_type_int, 0, error))
        variant = g_variant_new_uint16 (json_object_get_int (json_node));
      break;

    case G_VARIANT_CLASS_INT32:
      if (json_node_assert_type (json_node, json_type_int, 0, error))
        variant = g_variant_new_int32 (json_object_get_int (json_node));
      break;

    case G_VARIANT_CLASS_UINT32:
      if (json_node_assert_type (json_node, json_type_int, 0, error))
        variant = g_variant_new_uint32 (json_object_get_int (json_node));
      break;

    case G_VARIANT_CLASS_INT64:
      if (json_node_assert_type (json_node, json_type_int, 0, error))
        variant = g_variant_new_int64 (json_object_get_int (json_node));
      break;

    case G_VARIANT_CLASS_UINT64:
      if (json_node_assert_type (json_node, json_type_int, 0, error))
        variant = g_variant_new_uint64 (json_object_get_int (json_node));
      break;

    case G_VARIANT_CLASS_HANDLE:
      if (json_node_assert_type (json_node, json_type_int, 0, error))
        variant = g_variant_new_handle (json_object_get_int (json_node));
      break;

    case G_VARIANT_CLASS_DOUBLE:
      if (json_node_assert_type (json_node, json_type_double, 0, error))
        variant = g_variant_new_double (json_object_get_double (json_node));
      break;

    case G_VARIANT_CLASS_STRING:
      if (json_node_assert_type (json_node, json_type_string, 0, error))
        variant = g_variant_new_string (json_object_get_string (json_node));
      break;

    case G_VARIANT_CLASS_OBJECT_PATH:
      if (json_node_assert_type (json_node, json_type_string, 0, error))
        variant = g_variant_new_object_path (json_object_get_string (json_node));
      break;

    case G_VARIANT_CLASS_SIGNATURE:
      if (json_node_assert_type (json_node, json_type_string, 0, error))
        variant = g_variant_new_signature (json_object_get_string (json_node));
      break;

    case G_VARIANT_CLASS_VARIANT:
      variant = g_variant_new_variant (json_to_gvariant_recurse (json_node,
                                                                 NULL,
                                                                 error));
      break;

    case G_VARIANT_CLASS_MAYBE:
      variant = json_to_gvariant_maybe (json_node, signature, error);
      break;

    case G_VARIANT_CLASS_ARRAY:
      if (json_node_assert_type (json_node, json_type_array, 0, error))
        variant = json_to_gvariant_array (json_node, signature, error);
      break;

    case G_VARIANT_CLASS_TUPLE:
      if (json_node_assert_type (json_node, json_type_array, 0, error))
        variant = json_to_gvariant_tuple (json_node, signature, error);
      break;

    case G_VARIANT_CLASS_DICT_ENTRY:
      if (json_node_assert_type (json_node, json_type_object, 0, error))
        variant = json_to_gvariant_dict_entry (json_node, signature, error);
      break;

    default:
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVALID_DATA,
                   _("GVariant class '%c' not supported"), class);
      break;
    }

out:
  if (signature)
    (*signature)++;

  return variant;
}

static GVariant *
json_gvariant_deserialize (json_object *json_node,
                           const gchar  *signature,
                           GError      **error)
{
  g_return_val_if_fail (json_node != NULL, NULL);

  if (signature != NULL && ! g_variant_type_string_is_valid (signature))
    {
      g_set_error_literal (error,
                           G_IO_ERROR,
                           G_IO_ERROR_INVALID_ARGUMENT,
                           _("Invalid GVariant signature"));
      return NULL;
    }

  return json_to_gvariant_recurse (json_node, signature ? &signature : NULL, error);
}

GVariant *
json_gvariant_deserialize_data (const gchar  *json,
                                gssize        length,
                                const gchar  *signature,
                                GError      **error)
{
  GVariant *variant = NULL;
  enum json_tokener_error err = json_tokener_success;
  json_object *json_node;
  
  json_node = json_tokener_parse_verbose (json, &err);
  if (G_UNLIKELY (err != json_tokener_success))
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVALID_DATA,
                   _("JSON data is malformed:%s"), json_tokener_error_desc(err));
      return FALSE;
    }
  variant = json_gvariant_deserialize (json_node, signature, error);
  json_object_put (json_node);

  return variant;
}

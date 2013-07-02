/*
 * This file is part of the gnome-online-account-browser-plugin.
 * Copyright (C) Canonical Ltd. 2012
 * Copyright (C) Intel Corporation. 2013
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

#define GOA_API_IS_SUBJECT_TO_CHANGE
#include <goa/goa.h>

#ifndef GOABROWSER_NPAPI_OBJECT_H
#define GOABROWSER_NPAPI_OBJECT_H

#include "npapi-headers/headers/npapi.h"
#include "npapi-headers/headers/npruntime.h"

NPObject *goabrowser_create_plugin_object (NPP instance, NPObject* window, GoaClient *goa);

#endif /* GOABROWSER_NPAPI_OBJECT_H */

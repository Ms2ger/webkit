/*
 * Copyright (C) 2018 Igalia S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "WebKitURIUtilities.h"

#include <WebCore/GUniquePtrSoup.h>
#include <WebCore/URL.h>
#include <WebCore/URLHelpers.h>
#include <libsoup/soup.h>
#include <mutex>
#include <unicode/uidna.h>
#include <wtf/HashMap.h>

/**
 * SECTION: WebKitURIUtilities
 * @Short_description: Utility functions to manipulate URIs
 * @Title: WebKitURIUtilities
 **/

/**
 * webkit_uri_for_display:
 * @uri: the URI to be converted
 *
 * Use this function to format a URI for display. The URIs used internally by
 * WebKit may contain percent-encoded characters or Punycode, which are not
 * generally suitable to display to users. This function provides protection
 * against IDN homograph attacks so in some cases the host part of the returned
 * URI may be in Punycode if the safety check fails.
 *
 * Returns: (transfer full): @uri suitable for display.
 **/
gchar* webkit_uri_for_display(const gchar* uri)
{
    g_return_val_if_fail(uri, nullptr);

    auto coreURI = WebCore::URL(WebCore::URL(), String::fromUTF8(uri));
    if (!coreURI.isValid())
        return g_strdup(uri);

    // Remove password and percent-decode host name.
    coreURI.setPass(emptyString());
    auto soupURI = coreURI.createSoupURI();
    if (!soupURI.get()->host)
        return g_strdup(uri);

    GUniquePtr<gchar> percentDecodedHostChars(soup_uri_decode(soupURI.get()->host));
    auto percentDecodedHost = String::fromUTF8(percentDecodedHostChars.get());
    // Handle Unicode characters in the host name.
    uint32_t IDNScriptWhiteList[(USCRIPT_CODE_LIMIT + 31) / 32] = {};
    bool error = false;
    auto convertedHostName = WebCore::URLHelpers::ICUConvertHostName(percentDecodedHost, false, IDNScriptWhiteList, &error);
    if (error)
        return g_strdup(uri);

    if (convertedHostName.isNull()) {
        convertedHostName = percentDecodedHost;
    }

    g_free(soupURI.get()->host);
    soupURI.get()->host = g_strdup(convertedHostName.utf8().data());

    // Now, decode any percent-encoded characters in the URI. If there are null
    // characters or escaped slashes, this returns nullptr, so just display the
    // encoded URI in that case.
    GUniquePtr<char> percentEncodedURI(soup_uri_to_string(soupURI.get(), FALSE));
    char* decodedURI = g_uri_unescape_string(percentEncodedURI.get(), "/");
    return decodedURI ? decodedURI : g_strdup(percentEncodedURI.get());
}

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
#include <wtf/glib/GRefPtr.h>

/**
 * SECTION: WebKitURIUtilities
 * @Short_description: Utility functions to manipulate URIs
 * @Title: WebKitURIUtilities
 **/

static WebCore::ICUConvertHostnameWhitelist IDNScriptWhiteList;

static void readIDNScriptWhiteListFile()
{
#if PLATFORM(GTK)
    const char* path = "/org/webkitgtk/resources/IDNScriptWhiteList";
#elif PLATFORM(WPE)
    const char* path = "/org/webkitwpe/resources/IDNScriptWhiteList";
#else
#error "Unsupported plaform"
#endif

    GRefPtr<GBytes> gdata = adoptGRef(g_resources_lookup_data(path, G_RESOURCE_LOOKUP_FLAGS_NONE, nullptr));
    if (!gdata)
        return;

    const char* data = static_cast<const char*>(g_bytes_get_data(gdata.get(), nullptr));
    if (!data)
        return;

    // Read a word at a time.
    // Allow comments, starting with # character to the end of the line.
    while (1) {
        // Skip a comment if present.
        int offset = 0;
        if (sscanf(data, " #%*[^\n\r]%*[\n\r]%n", &offset) == EOF)
            break;
        data += offset;

        // Read a script name if present.
        char word[33];
        offset = 0;
        int result = sscanf(data, " %32[^# \t\n\r]%*[\n\r]%n", word, &offset);
        if (result == EOF)
            break;
        data += offset;

        if (result == 1) {
            // Got a word, map to script code and put it into the array.
            int32_t script = u_getPropertyValueEnum(UCHAR_SCRIPT, word);
            if (script >= 0 && script < USCRIPT_CODE_LIMIT) {
                size_t index = script / 32;
                uint32_t mask = 1 << (script % 32);
                IDNScriptWhiteList[index] |= mask;
            }
        }
    }
}

static void loadIDNScriptWhiteList()
{
    static std::once_flag flag;
    std::call_once(flag, readIDNScriptWhiteListFile);
}

/**
 * webkit_uri_for_display:
 * @uri: the URI to be converted
 *
 * Use this function to format a URI for display. The URIs used internally by
 * WebKit may contain percent-encoded characters or Punycode, which are not
 * generally suitable to display to users. This function provides protection
 * against IDN homograph attacks, so in some cases the host part of the returned
 * URI may be in Punycode if the safety check fails.
 *
 * Returns: (nullable) (transfer full): @uri suitable for display, or %NULL in
 *    case of error.
 **/
gchar* webkit_uri_for_display(const gchar* uri)
{
    g_return_val_if_fail(uri, nullptr);

    auto coreURI = WebCore::URL { WebCore::URL { }, String::fromUTF8(uri) };
    if (!coreURI.isValid())
        return nullptr;

    // Remove password and percent-decode host name.
    coreURI.setPass(emptyString());
    auto soupURI = coreURI.createSoupURI();
    if (!soupURI.get()->host)
        return nullptr;

    GUniquePtr<gchar> percentDecodedHostChars(soup_uri_decode(soupURI.get()->host));
    auto percentDecodedHost = String::fromUTF8(percentDecodedHostChars.get());

    // Handle Unicode characters in the host name.
    loadIDNScriptWhiteList();
    bool error = false;
    auto convertedHostName = WebCore::URLHelpers::decodePunycode(percentDecodedHost, IDNScriptWhiteList, error);
    if (error)
        return nullptr;

    if (convertedHostName.isNull())
        convertedHostName = percentDecodedHost;

    g_free(soupURI.get()->host);
    soupURI.get()->host = g_strdup(convertedHostName.utf8().data());

    // Now, decode any percent-encoded characters in the URI.
    GUniquePtr<char> percentEncodedURI(soup_uri_to_string(soupURI.get(), FALSE));
    char* decodedURI = g_uri_unescape_string(percentEncodedURI.get(), "/");
    if (!decodedURI)
        return nullptr;

    return decodedURI;
}

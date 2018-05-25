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

#include "TestMain.h"

static void testURIForDisplay(Test*, gconstpointer)
{
    struct {
        const char* input;
        const char* output;
    } const items[] = {
        // Arabic Kasra letter.
        { "https://xn--apple-gkh.com/", "https://xn--apple-gkh.com/" },
        { "http://xn--google-yri.com/", "http://xn--google-yri.com/" },
        // Cyrillic and Serbian.
        { "http://спецодежда.онла\u0439н/", "http://спецодежда.онлайн/" },
        { "http://спецодежда.онла\u0438\u0306н/", "http://спецодежда.онлайн/" },
        { "http://православнајапородица.срб/", "http://православнајапородица.срб/" },
        { "http://www.свеовде.од.срб/", "http://www.свеовде.од.срб/" },
        // Cyrillic country code top-level domain for the Russian Federation.
        { "http://президент.рф/", "http://президент.рф/" },
        { "http://президент.рф./", "http://президент.рф./" },
        { "http://www.президент.рф/", "http://www.президент.рф/" },
        { "http://почта.президент.рф/", "http://почта.президент.рф/" },
        { "http://0ж9.рф/", "http://0ж9.рф/" },
        { "http://туда-сюда.рф/", "http://туда-сюда.рф/" },
        { "http://прeзидент.рф/", "http://xn--e-htbdgf6aiiy.xn--p1ai/" }, // Spoof: Roman 'e'.
        { "http://caxap.рф/", "http://caxap.xn--p1ai/" }, // Spoof: all characters in 'caxap' are Roman.
        // .com top-level domain doesn't allow non-Latin scripts.
        { "http://αβγχψω.com/", "http://xn--mxacd4ffg.com/" },
        { "http://абгдеж.com/", "http://xn--80acgefg.com/" },
        { "http://おかがキギク.com/", "http://おかがキギク.com/" },
        // Percent-decoding.
        { "http://www.%7Bexample%7D.com/", "http://www.{example}.com/" },
        { "http://example.com/a%2Fb", nullptr }, // '/' in path needs to remain encoded.
    };

    for (auto& item : items) {
        GUniquePtr<char> displayURI(webkit_uri_for_display(item.input));
        g_assert_cmpstr(displayURI.get(), ==, item.output);
    }
}

void beforeAll()
{
    Test::add("WebKitURIUtilities", "uri-for-display", testURIForDisplay);
}

void afterAll()
{
}

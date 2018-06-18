/*
 * Copyright (C) 2018 Metrological Group B.V.
 * Copyright (C) 2018 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "URLHelpers.h"

#include "URLParser.h"
#include <unicode/uidna.h>
#include <wtf/Optional.h>

namespace WebCore {

static bool isArmenianLookalikeCharacter(UChar32 codePoint)
{
    return codePoint == 0x0548 || codePoint == 0x054D || codePoint == 0x0578 || codePoint == 0x057D;
}

static bool isArmenianScriptCharacter(UChar32 codePoint)
{
    UErrorCode error = U_ZERO_ERROR;
    UScriptCode script = uscript_getScript(codePoint, &error);
    if (error != U_ZERO_ERROR) {
        LOG_ERROR("got ICU error while trying to look at scripts: %d", error);
        return false;
    }

    return script == USCRIPT_ARMENIAN;
}

template<typename CharacterType> inline bool isASCIIDigitOrValidHostCharacter(CharacterType charCode)
{
    if (!isASCIIDigitOrPunctuation(charCode))
        return false;

    // Things the URL Parser rejects:
    switch (charCode) {
    case '#':
    case '%':
    case '/':
    case ':':
    case '?':
    case '@':
    case '[':
    case '\\':
    case ']':
        return false;
    default:
        return true;
    }
}

bool URLHelpers::isLookalikeCharacter(std::optional<UChar32> previousCodePoint, UChar32 charCode)
{
    // This function treats the following as unsafe, lookalike characters:
    // any non-printable character, any character considered as whitespace,
    // any ignorable character, and emoji characters related to locks.
    
    // We also considered the characters in Mozilla's blacklist <http://kb.mozillazine.org/Network.IDN.blacklist_chars>.

    // Some of the characters here will never appear once ICU has encoded.
    // For example, ICU transforms most spaces into an ASCII space and most
    // slashes into an ASCII solidus. But one of the two callers uses this
    // on characters that have not been processed by ICU, so they are needed here.
    
    if (!u_isprint(charCode) || u_isUWhiteSpace(charCode) || u_hasBinaryProperty(charCode, UCHAR_DEFAULT_IGNORABLE_CODE_POINT))
        return true;
    
    switch (charCode) {
        case 0x00BC: /* VULGAR FRACTION ONE QUARTER */
        case 0x00BD: /* VULGAR FRACTION ONE HALF */
        case 0x00BE: /* VULGAR FRACTION THREE QUARTERS */
        case 0x00ED: /* LATIN SMALL LETTER I WITH ACUTE */
        case 0x01C3: /* LATIN LETTER RETROFLEX CLICK */
        case 0x0251: /* LATIN SMALL LETTER ALPHA */
        case 0x0261: /* LATIN SMALL LETTER SCRIPT G */
        case 0x02D0: /* MODIFIER LETTER TRIANGULAR COLON */
        case 0x0335: /* COMBINING SHORT STROKE OVERLAY */
        case 0x0337: /* COMBINING SHORT SOLIDUS OVERLAY */
        case 0x0338: /* COMBINING LONG SOLIDUS OVERLAY */
        case 0x0589: /* ARMENIAN FULL STOP */
        case 0x05B4: /* HEBREW POINT HIRIQ */
        case 0x05BC: /* HEBREW POINT DAGESH OR MAPIQ */
        case 0x05C3: /* HEBREW PUNCTUATION SOF PASUQ */
        case 0x05F4: /* HEBREW PUNCTUATION GERSHAYIM */
        case 0x0609: /* ARABIC-INDIC PER MILLE SIGN */
        case 0x060A: /* ARABIC-INDIC PER TEN THOUSAND SIGN */
        case 0x0650: /* ARABIC KASRA */
        case 0x0660: /* ARABIC INDIC DIGIT ZERO */
        case 0x066A: /* ARABIC PERCENT SIGN */
        case 0x06D4: /* ARABIC FULL STOP */
        case 0x06F0: /* EXTENDED ARABIC INDIC DIGIT ZERO */
        case 0x0701: /* SYRIAC SUPRALINEAR FULL STOP */
        case 0x0702: /* SYRIAC SUBLINEAR FULL STOP */
        case 0x0703: /* SYRIAC SUPRALINEAR COLON */
        case 0x0704: /* SYRIAC SUBLINEAR COLON */
        case 0x1735: /* PHILIPPINE SINGLE PUNCTUATION */
        case 0x1D04: /* LATIN LETTER SMALL CAPITAL C */
        case 0x1D0F: /* LATIN LETTER SMALL CAPITAL O */
        case 0x1D1C: /* LATIN LETTER SMALL CAPITAL U */
        case 0x1D20: /* LATIN LETTER SMALL CAPITAL V */
        case 0x1D21: /* LATIN LETTER SMALL CAPITAL W */
        case 0x1D22: /* LATIN LETTER SMALL CAPITAL Z */
        case 0x1ECD: /* LATIN SMALL LETTER O WITH DOT BELOW */
        case 0x2010: /* HYPHEN */
        case 0x2011: /* NON-BREAKING HYPHEN */
        case 0x2024: /* ONE DOT LEADER */
        case 0x2027: /* HYPHENATION POINT */
        case 0x2039: /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
        case 0x203A: /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
        case 0x2041: /* CARET INSERTION POINT */
        case 0x2044: /* FRACTION SLASH */
        case 0x2052: /* COMMERCIAL MINUS SIGN */
        case 0x2153: /* VULGAR FRACTION ONE THIRD */
        case 0x2154: /* VULGAR FRACTION TWO THIRDS */
        case 0x2155: /* VULGAR FRACTION ONE FIFTH */
        case 0x2156: /* VULGAR FRACTION TWO FIFTHS */
        case 0x2157: /* VULGAR FRACTION THREE FIFTHS */
        case 0x2158: /* VULGAR FRACTION FOUR FIFTHS */
        case 0x2159: /* VULGAR FRACTION ONE SIXTH */
        case 0x215A: /* VULGAR FRACTION FIVE SIXTHS */
        case 0x215B: /* VULGAR FRACTION ONE EIGHT */
        case 0x215C: /* VULGAR FRACTION THREE EIGHTHS */
        case 0x215D: /* VULGAR FRACTION FIVE EIGHTHS */
        case 0x215E: /* VULGAR FRACTION SEVEN EIGHTHS */
        case 0x215F: /* FRACTION NUMERATOR ONE */
        case 0x2212: /* MINUS SIGN */
        case 0x2215: /* DIVISION SLASH */
        case 0x2216: /* SET MINUS */
        case 0x2236: /* RATIO */
        case 0x233F: /* APL FUNCTIONAL SYMBOL SLASH BAR */
        case 0x23AE: /* INTEGRAL EXTENSION */
        case 0x244A: /* OCR DOUBLE BACKSLASH */
        case 0x2571: /* DisplayType::Box DRAWINGS LIGHT DIAGONAL UPPER RIGHT TO LOWER LEFT */
        case 0x2572: /* DisplayType::Box DRAWINGS LIGHT DIAGONAL UPPER LEFT TO LOWER RIGHT */
        case 0x29F6: /* SOLIDUS WITH OVERBAR */
        case 0x29F8: /* BIG SOLIDUS */
        case 0x2AFB: /* TRIPLE SOLIDUS BINARY RELATION */
        case 0x2AFD: /* DOUBLE SOLIDUS OPERATOR */
        case 0x2FF0: /* IDEOGRAPHIC DESCRIPTION CHARACTER LEFT TO RIGHT */
        case 0x2FF1: /* IDEOGRAPHIC DESCRIPTION CHARACTER ABOVE TO BELOW */
        case 0x2FF2: /* IDEOGRAPHIC DESCRIPTION CHARACTER LEFT TO MIDDLE AND RIGHT */
        case 0x2FF3: /* IDEOGRAPHIC DESCRIPTION CHARACTER ABOVE TO MIDDLE AND BELOW */
        case 0x2FF4: /* IDEOGRAPHIC DESCRIPTION CHARACTER FULL SURROUND */
        case 0x2FF5: /* IDEOGRAPHIC DESCRIPTION CHARACTER SURROUND FROM ABOVE */
        case 0x2FF6: /* IDEOGRAPHIC DESCRIPTION CHARACTER SURROUND FROM BELOW */
        case 0x2FF7: /* IDEOGRAPHIC DESCRIPTION CHARACTER SURROUND FROM LEFT */
        case 0x2FF8: /* IDEOGRAPHIC DESCRIPTION CHARACTER SURROUND FROM UPPER LEFT */
        case 0x2FF9: /* IDEOGRAPHIC DESCRIPTION CHARACTER SURROUND FROM UPPER RIGHT */
        case 0x2FFA: /* IDEOGRAPHIC DESCRIPTION CHARACTER SURROUND FROM LOWER LEFT */
        case 0x2FFB: /* IDEOGRAPHIC DESCRIPTION CHARACTER OVERLAID */
        case 0x3002: /* IDEOGRAPHIC FULL STOP */
        case 0x3008: /* LEFT ANGLE BRACKET */
        case 0x3014: /* LEFT TORTOISE SHELL BRACKET */
        case 0x3015: /* RIGHT TORTOISE SHELL BRACKET */
        case 0x3033: /* VERTICAL KANA REPEAT MARK UPPER HALF */
        case 0x3035: /* VERTICAL KANA REPEAT MARK LOWER HALF */
        case 0x321D: /* PARENTHESIZED KOREAN CHARACTER OJEON */
        case 0x321E: /* PARENTHESIZED KOREAN CHARACTER O HU */
        case 0x33AE: /* SQUARE RAD OVER S */
        case 0x33AF: /* SQUARE RAD OVER S SQUARED */
        case 0x33C6: /* SQUARE C OVER KG */
        case 0x33DF: /* SQUARE A OVER M */
        case 0x05B9: /* HEBREW POINT HOLAM */
        case 0x05BA: /* HEBREW POINT HOLAM HASER FOR VAV */
        case 0x05C1: /* HEBREW POINT SHIN DOT */
        case 0x05C2: /* HEBREW POINT SIN DOT */
        case 0x05C4: /* HEBREW MARK UPPER DOT */
        case 0xA731: /* LATIN LETTER SMALL CAPITAL S */
        case 0xA771: /* LATIN SMALL LETTER DUM */
        case 0xA789: /* MODIFIER LETTER COLON */
        case 0xFE14: /* PRESENTATION FORM FOR VERTICAL SEMICOLON */
        case 0xFE15: /* PRESENTATION FORM FOR VERTICAL EXCLAMATION MARK */
        case 0xFE3F: /* PRESENTATION FORM FOR VERTICAL LEFT ANGLE BRACKET */
        case 0xFE5D: /* SMALL LEFT TORTOISE SHELL BRACKET */
        case 0xFE5E: /* SMALL RIGHT TORTOISE SHELL BRACKET */
        case 0xFF0E: /* FULLWIDTH FULL STOP */
        case 0xFF0F: /* FULL WIDTH SOLIDUS */
        case 0xFF61: /* HALFWIDTH IDEOGRAPHIC FULL STOP */
        case 0xFFFC: /* OBJECT REPLACEMENT CHARACTER */
        case 0xFFFD: /* REPLACEMENT CHARACTER */
        case 0x1F50F: /* LOCK WITH INK PEN */
        case 0x1F510: /* CLOSED LOCK WITH KEY */
        case 0x1F511: /* KEY */
        case 0x1F512: /* LOCK */
        case 0x1F513: /* OPEN LOCK */
            return true;
        case 0x0307: /* COMBINING DOT ABOVE */
            return previousCodePoint == 0x0237 /* LATIN SMALL LETTER DOTLESS J */
                || previousCodePoint == 0x0131 /* LATIN SMALL LETTER DOTLESS I */
                || previousCodePoint == 0x05D5; /* HEBREW LETTER VAV */
        case 0x0548: /* ARMENIAN CAPITAL LETTER VO */
        case 0x054D: /* ARMENIAN CAPITAL LETTER SEH */
        case 0x0578: /* ARMENIAN SMALL LETTER VO */
        case 0x057D: /* ARMENIAN SMALL LETTER SEH */
            return previousCodePoint
                && !isASCIIDigitOrValidHostCharacter(previousCodePoint.value())
                && !isArmenianScriptCharacter(previousCodePoint.value());
        case '.':
            return false;
        default:
            return previousCodePoint
                && isArmenianLookalikeCharacter(previousCodePoint.value())
                && !(isArmenianScriptCharacter(charCode) || isASCIIDigitOrValidHostCharacter(charCode));
    }
}

static bool allCharactersInIDNScriptWhiteList(const UChar *buffer, int32_t length, const uint32_t (&IDNScriptWhiteList)[(USCRIPT_CODE_LIMIT + 31) / 32])
{
    int32_t i = 0;
    std::optional<UChar32> previousCodePoint;
    while (i < length) {
        UChar32 c;
        U16_NEXT(buffer, i, length, c)
        UErrorCode error = U_ZERO_ERROR;
        UScriptCode script = uscript_getScript(c, &error);
        if (error != U_ZERO_ERROR) {
            LOG_ERROR("got ICU error while trying to look at scripts: %d", error);
            return false;
        }
        if (script < 0) {
            LOG_ERROR("got negative number for script code from ICU: %d", script);
            return false;
        }
        if (script >= USCRIPT_CODE_LIMIT)
            return false;

        size_t index = script / 32;
        uint32_t mask = 1 << (script % 32);
        if (!(IDNScriptWhiteList[index] & mask))
            return false;
        
        if (URLHelpers::isLookalikeCharacter(previousCodePoint, c))
            return false;
        previousCodePoint = c;
    }
    return true;
}

static bool isSecondLevelDomainNameAllowedByTLDRules(const UChar* buffer, int32_t length, const WTF::Function<bool(UChar)>& characterIsAllowed)
{
    ASSERT(length > 0);

    for (int32_t i = length - 1; i >= 0; --i) {
        UChar ch = buffer[i];
        
        if (characterIsAllowed(ch))
            continue;
        
        // Only check the second level domain. Lower level registrars may have different rules.
        if (ch == '.')
            break;
        
        return false;
    }
    return true;
}

#define CHECK_RULES_IF_SUFFIX_MATCHES(suffix, function) \
    { \
        static const int32_t suffixLength = sizeof(suffix) / sizeof(suffix[0]); \
        if (length > suffixLength && 0 == memcmp(buffer + length - suffixLength, suffix, sizeof(suffix))) \
            return isSecondLevelDomainNameAllowedByTLDRules(buffer, length - suffixLength, function); \
    }

static bool isRussianDomainNameCharacter(UChar ch)
{
    // Only modern Russian letters, digits and dashes are allowed.
    return (ch >= 0x0430 && ch <= 0x044f) || ch == 0x0451 || isASCIIDigit(ch) || ch == '-';
}

static bool allCharactersAllowedByTLDRules(const UChar* buffer, int32_t length)
{
    // Skip trailing dot for root domain.
    if (buffer[length - 1] == '.')
        length--;

    // http://cctld.ru/files/pdf/docs/rules_ru-rf.pdf
    static const UChar cyrillicRF[] = {
        '.',
        0x0440, // CYRILLIC SMALL LETTER ER
        0x0444  // CYRILLIC SMALL LETTER EF
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicRF, isRussianDomainNameCharacter);

    // http://rusnames.ru/rules.pl
    static const UChar cyrillicRUS[] = {
        '.',
        0x0440, // CYRILLIC SMALL LETTER ER
        0x0443, // CYRILLIC SMALL LETTER U
        0x0441  // CYRILLIC SMALL LETTER ES
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicRUS, isRussianDomainNameCharacter);

    // http://ru.faitid.org/projects/moscow/documents/moskva/idn
    static const UChar cyrillicMOSKVA[] = {
        '.',
        0x043C, // CYRILLIC SMALL LETTER EM
        0x043E, // CYRILLIC SMALL LETTER O
        0x0441, // CYRILLIC SMALL LETTER ES
        0x043A, // CYRILLIC SMALL LETTER KA
        0x0432, // CYRILLIC SMALL LETTER VE
        0x0430  // CYRILLIC SMALL LETTER A
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicMOSKVA, isRussianDomainNameCharacter);

    // http://www.dotdeti.ru/foruser/docs/regrules.php
    static const UChar cyrillicDETI[] = {
        '.',
        0x0434, // CYRILLIC SMALL LETTER DE
        0x0435, // CYRILLIC SMALL LETTER IE
        0x0442, // CYRILLIC SMALL LETTER TE
        0x0438  // CYRILLIC SMALL LETTER I
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicDETI, isRussianDomainNameCharacter);

    // http://corenic.org - rules not published. The word is Russian, so only allowing Russian at this time,
    // although we may need to revise the checks if this ends up being used with other languages spoken in Russia.
    static const UChar cyrillicONLAYN[] = {
        '.',
        0x043E, // CYRILLIC SMALL LETTER O
        0x043D, // CYRILLIC SMALL LETTER EN
        0x043B, // CYRILLIC SMALL LETTER EL
        0x0430, // CYRILLIC SMALL LETTER A
        0x0439, // CYRILLIC SMALL LETTER SHORT I
        0x043D  // CYRILLIC SMALL LETTER EN
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicONLAYN, isRussianDomainNameCharacter);

    // http://corenic.org - same as above.
    static const UChar cyrillicSAYT[] = {
        '.',
        0x0441, // CYRILLIC SMALL LETTER ES
        0x0430, // CYRILLIC SMALL LETTER A
        0x0439, // CYRILLIC SMALL LETTER SHORT I
        0x0442  // CYRILLIC SMALL LETTER TE
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicSAYT, isRussianDomainNameCharacter);

    // http://pir.org/products/opr-domain/ - rules not published. According to the registry site,
    // the intended audience is "Russian and other Slavic-speaking markets".
    // Chrome appears to only allow Russian, so sticking with that for now.
    static const UChar cyrillicORG[] = {
        '.',
        0x043E, // CYRILLIC SMALL LETTER O
        0x0440, // CYRILLIC SMALL LETTER ER
        0x0433  // CYRILLIC SMALL LETTER GHE
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicORG, isRussianDomainNameCharacter);

    // http://cctld.by/rules.html
    static const UChar cyrillicBEL[] = {
        '.',
        0x0431, // CYRILLIC SMALL LETTER BE
        0x0435, // CYRILLIC SMALL LETTER IE
        0x043B  // CYRILLIC SMALL LETTER EL
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicBEL, [](UChar ch) {
        // Russian and Byelorussian letters, digits and dashes are allowed.
        return (ch >= 0x0430 && ch <= 0x044f) || ch == 0x0451 || ch == 0x0456 || ch == 0x045E || ch == 0x2019 || isASCIIDigit(ch) || ch == '-';
    });

    // http://www.nic.kz/docs/poryadok_vnedreniya_kaz_ru.pdf
    static const UChar cyrillicKAZ[] = {
        '.',
        0x049B, // CYRILLIC SMALL LETTER KA WITH DESCENDER
        0x0430, // CYRILLIC SMALL LETTER A
        0x0437  // CYRILLIC SMALL LETTER ZE
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicKAZ, [](UChar ch) {
        // Kazakh letters, digits and dashes are allowed.
        return (ch >= 0x0430 && ch <= 0x044f) || ch == 0x0451 || ch == 0x04D9 || ch == 0x0493 || ch == 0x049B || ch == 0x04A3 || ch == 0x04E9 || ch == 0x04B1 || ch == 0x04AF || ch == 0x04BB || ch == 0x0456 || isASCIIDigit(ch) || ch == '-';
    });

    // http://uanic.net/docs/documents-ukr/Rules%20of%20UKR_v4.0.pdf
    static const UChar cyrillicUKR[] = {
        '.',
        0x0443, // CYRILLIC SMALL LETTER U
        0x043A, // CYRILLIC SMALL LETTER KA
        0x0440  // CYRILLIC SMALL LETTER ER
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicUKR, [](UChar ch) {
        // Russian and Ukrainian letters, digits and dashes are allowed.
        return (ch >= 0x0430 && ch <= 0x044f) || ch == 0x0451 || ch == 0x0491 || ch == 0x0404 || ch == 0x0456 || ch == 0x0457 || isASCIIDigit(ch) || ch == '-';
    });

    // http://www.rnids.rs/data/DOKUMENTI/idn-srb-policy-termsofuse-v1.4-eng.pdf
    static const UChar cyrillicSRB[] = {
        '.',
        0x0441, // CYRILLIC SMALL LETTER ES
        0x0440, // CYRILLIC SMALL LETTER ER
        0x0431  // CYRILLIC SMALL LETTER BE
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicSRB, [](UChar ch) {
        // Serbian letters, digits and dashes are allowed.
        return (ch >= 0x0430 && ch <= 0x0438) || (ch >= 0x043A && ch <= 0x0448) || ch == 0x0452 || ch == 0x0458 || ch == 0x0459 || ch == 0x045A || ch == 0x045B || ch == 0x045F || isASCIIDigit(ch) || ch == '-';
    });

    // http://marnet.mk/doc/pravilnik-mk-mkd.pdf
    static const UChar cyrillicMKD[] = {
        '.',
        0x043C, // CYRILLIC SMALL LETTER EM
        0x043A, // CYRILLIC SMALL LETTER KA
        0x0434  // CYRILLIC SMALL LETTER DE
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicMKD, [](UChar ch) {
        // Macedonian letters, digits and dashes are allowed.
        return (ch >= 0x0430 && ch <= 0x0438) || (ch >= 0x043A && ch <= 0x0448) || ch == 0x0453 || ch == 0x0455 || ch == 0x0458 || ch == 0x0459 || ch == 0x045A || ch == 0x045C || ch == 0x045F || isASCIIDigit(ch) || ch == '-';
    });

    // https://www.mon.mn/cs/
    static const UChar cyrillicMON[] = {
        '.',
        0x043C, // CYRILLIC SMALL LETTER EM
        0x043E, // CYRILLIC SMALL LETTER O
        0x043D  // CYRILLIC SMALL LETTER EN
    };
    CHECK_RULES_IF_SUFFIX_MATCHES(cyrillicMON, [](UChar ch) {
        // Mongolian letters, digits and dashes are allowed.
        return (ch >= 0x0430 && ch <= 0x044f) || ch == 0x0451 || ch == 0x04E9 || ch == 0x04AF || isASCIIDigit(ch) || ch == '-';
    });

    // Not a known top level domain with special rules.
    return false;
}

String URLHelpers::decodePunycode(const String& hostName, bool encode, const uint32_t (&IDNScriptWhiteList)[(USCRIPT_CODE_LIMIT + 31) / 32], bool* error)
{
    if (hostName.isNull() || hostName.isEmpty())
        return String();

    // Needs to be big enough to hold an IDN-encoded name.
    // For host names bigger than this, we won't do IDN encoding, which is almost certainly OK.
    static const int32_t kHostNameBufferLength = 2048;
    int32_t length = static_cast<int32_t>(hostName.length());
    if (length > kHostNameBufferLength)
        return String();

    String bufferFor16BitData;
    const UChar* inputBuffer;
    if (LIKELY(hostName.is8Bit())) {
        bufferFor16BitData = String::make16BitFrom8BitSource(hostName.characters8(), hostName.length());
        inputBuffer = bufferFor16BitData.characters16();
    } else {
        inputBuffer = hostName.characters16();
    }
    UChar outputBuffer[kHostNameBufferLength];
    UErrorCode uerror = U_ZERO_ERROR;
    UIDNAInfo uinfo = UIDNA_INFO_INITIALIZER;
    int32_t numCharactersConverted = (encode ? uidna_nameToASCII : uidna_nameToUnicode)(&URLParser::internationalDomainNameTranscoder(), inputBuffer, length, outputBuffer, kHostNameBufferLength, &uinfo, &uerror);
    if (U_FAILURE(uerror) || uinfo.errors) {
        *error = true;
        return String();
    }
    
    if (numCharactersConverted == length && !memcmp(inputBuffer, outputBuffer, length * sizeof(UChar)))
        return String();
    
    // Decoding needs additional checks.
    if (!encode && !allCharactersInIDNScriptWhiteList(outputBuffer, numCharactersConverted, IDNScriptWhiteList) && !allCharactersAllowedByTLDRules(outputBuffer, numCharactersConverted))
        return String();

    return String(outputBuffer, numCharactersConverted);
}

} // namespace WebCore

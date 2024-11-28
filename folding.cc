/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "folding.hh"
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

namespace Folding {

#include "inc_case_folding.hh"
#include "inc_diacritic_folding.hh"

/// Tests if the given char is one of the Unicode combining marks. Some are
/// caught by the diacritics folding table, but they are only handled there
/// when they come with their main characters, not by themselves. The rest
/// are caught here.
bool isCombiningMark( wchar ch )
{
  return (
           ( ch >= 0x300 && ch <= 0x36F ) ||
           ( ch >= 0x1DC0 && ch <= 0x1DFF ) ||
           ( ch >= 0x20D0 && ch <= 0x20FF ) ||
           ( ch >= 0xFE20 && ch <= 0xFE2F )
         );
}

wstring apply( wstring const & in, bool preserveWildcards )
{
  // First, strip diacritics and apply ws/punctuation removal

  wstring withoutDiacritics;

  withoutDiacritics.reserve( in.size() );

  wchar const * nextChar = in.data();

  size_t consumed;

  for( size_t left = in.size(); left; )
  {
    wchar ch = foldDiacritic( nextChar, left, consumed );

    if ( !isCombiningMark( ch ) && !isWhitespace( ch )
         && ( !isPunct( ch )
              || ( preserveWildcards &&
                   ( ch == '\\' || ch == '?' || ch == '*' || ch == '[' || ch == ']' ) )
            )
    )
      withoutDiacritics.push_back( ch );

    nextChar += consumed;
    left -= consumed;
  }

  // Now, fold the case

  wstring caseFolded;

  caseFolded.reserve( withoutDiacritics.size() * foldCaseMaxOut );

  nextChar = withoutDiacritics.data();

  wchar buf[ foldCaseMaxOut ];

  for( size_t left = withoutDiacritics.size(); left--; )
    caseFolded.append( buf, foldCase(  *nextChar++, buf ) );

  return caseFolded;
}

wstring applySimpleCaseOnly( wstring const & in )
{
  wchar const * nextChar = in.data();

  wstring out;

  out.reserve( in.size() );

  for( size_t left = in.size(); left--; )
    out.push_back( foldCaseSimple( *nextChar++ ) );

  return out;
}

wstring applyFullCaseOnly( wstring const & in )
{
  wstring caseFolded;

  caseFolded.reserve( in.size() * foldCaseMaxOut );

  wchar const * nextChar = in.data();

  wchar buf[ foldCaseMaxOut ];

  for( size_t left = in.size(); left--; )
    caseFolded.append( buf, foldCase(  *nextChar++, buf ) );

  return caseFolded;
}

wstring applyDiacriticsOnly( wstring const & in )
{
  wstring withoutDiacritics;

  withoutDiacritics.reserve( in.size() );

  wchar const * nextChar = in.data();

  size_t consumed;

  for( size_t left = in.size(); left; )
  {
    wchar ch = foldDiacritic( nextChar, left, consumed );

    if ( !isCombiningMark( ch ) )
      withoutDiacritics.push_back( ch );

    nextChar += consumed;
    left -= consumed;
  }

  return withoutDiacritics;
}

wstring applyPunctOnly( wstring const & in )
{
  wchar const * nextChar = in.data();

  wstring out;

  out.reserve( in.size() );

  for( size_t left = in.size(); left--; ++nextChar )
    if ( !isPunct( *nextChar ) )
      out.push_back( *nextChar );

  return out;
}

wstring applyWhitespaceOnly( wstring const & in )
{
  wchar const * nextChar = in.data();

  wstring out;

  out.reserve( in.size() );

  for( size_t left = in.size(); left--; ++nextChar )
    if ( !isWhitespace( *nextChar ) )
      out.push_back( *nextChar );

  return out;
}

wstring applyWhitespaceAndPunctOnly( wstring const & in )
{
  wchar const * nextChar = in.data();

  wstring out;

  out.reserve( in.size() );

  for( size_t left = in.size(); left--; ++nextChar )
    if ( !isWhitespace( *nextChar ) && !isPunct( *nextChar ) )
      out.push_back( *nextChar );

  return out;
}

bool isWhitespace( wchar ch )
{
  switch( ch )
  {
    case '\n':
    case '\r':
    case '\t':

    case 0x2028: // Zl, LINE SEPARATOR

    case 0x2029: // Zp, PARAGRAPH SEPARATOR

    case 0x0020: // Zs, SPACE
    case 0x00A0: // Zs, NO-BREAK SPACE
    case 0x1680: // Zs, OGHAM SPACE MARK
    case 0x180E: // Zs, MONGOLIAN VOWEL SEPARATOR
    case 0x2000: // Zs, EN QUAD
    case 0x2001: // Zs, EM QUAD
    case 0x2002: // Zs, EN SPACE
    case 0x2003: // Zs, EM SPACE
    case 0x2004: // Zs, THREE-PER-EM SPACE
    case 0x2005: // Zs, FOUR-PER-EM SPACE
    case 0x2006: // Zs, SIX-PER-EM SPACE
    case 0x2007: // Zs, FIGURE SPACE
    case 0x2008: // Zs, PUNCTUATION SPACE
    case 0x2009: // Zs, THIN SPACE
    case 0x200A: // Zs, HAIR SPACE
    case 0x202F: // Zs, NARROW NO-BREAK SPACE
    case 0x205F: // Zs, MEDIUM MATHEMATICAL SPACE
    case 0x3000: // Zs, IDEOGRAPHIC SPACE
      return true;

    default:
      return false;
  }
}

bool isPunct( wchar ch )
{
  switch( ch )
  {
    // Pc

    case 0x005F: // LOW LINE
    case 0x203F: // UNDERTIE
    case 0x2040: // CHARACTER TIE
    case 0x2054: // INVERTED UNDERTIE
    case 0x30FB: // KATAKANA MIDDLE DOT
    case 0xFE33: // PRESENTATION FORM FOR VERTICAL LOW LINE
    case 0xFE34: // PRESENTATION FORM FOR VERTICAL WAVY LOW LINE
    case 0xFE4D: // DASHED LOW LINE
    case 0xFE4E: // CENTRELINE LOW LINE
    case 0xFE4F: // WAVY LOW LINE
    case 0xFF3F: // FULLWIDTH LOW LINE
    case 0xFF65: // HALFWIDTH KATAKANA MIDDLE DOT

    // Pd
    case 0x002D: // HYPHEN-MINUS
    case 0x058A: // ARMENIAN HYPHEN
    case 0x1806: // MONGOLIAN TODO SOFT HYPHEN
    case 0x2010: // HYPHEN
    case 0x2011: // NON-BREAKING HYPHEN
    case 0x2012: // FIGURE DASH
    case 0x2013: // EN DASH
    case 0x2014: // EM DASH
    case 0x2015: // HORIZONTAL BAR
    case 0x301C: // WAVE DASH
    case 0x3030: // WAVY DASH
    case 0x30A0: // KATAKANA-HIRAGANA DOUBLE HYPHEN
    case 0xFE31: // PRESENTATION FORM FOR VERTICAL EM DASH
    case 0xFE32: // PRESENTATION FORM FOR VERTICAL EN DASH
    case 0xFE58: // SMALL EM DASH
    case 0xFE63: // SMALL HYPHEN-MINUS
    case 0xFF0D: // FULLWIDTH HYPHEN-MINUS

    // Ps
    case 0x0028: // LEFT PARENTHESIS
    case 0x005B: // LEFT SQUARE BRACKET
    case 0x007B: // LEFT CURLY BRACKET
    case 0x0F3A: // TIBETAN MARK GUG RTAGS GYON
    case 0x0F3C: // TIBETAN MARK ANG KHANG GYON
    case 0x169B: // OGHAM FEATHER MARK
    case 0x201A: // SINGLE LOW-9 QUOTATION MARK
    case 0x201E: // DOUBLE LOW-9 QUOTATION MARK
    case 0x2045: // LEFT SQUARE BRACKET WITH QUILL
    case 0x207D: // SUPERSCRIPT LEFT PARENTHESIS
    case 0x208D: // SUBSCRIPT LEFT PARENTHESIS
    case 0x2329: // LEFT-POINTING ANGLE BRACKET
    case 0x2768: // MEDIUM LEFT PARENTHESIS ORNAMENT
    case 0x276A: // MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT
    case 0x276C: // MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT
    case 0x276E: // HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT
    case 0x2770: // HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT
    case 0x2772: // LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT
    case 0x2774: // MEDIUM LEFT CURLY BRACKET ORNAMENT
    case 0x27C5: // LEFT S-SHAPED BAG DELIMITER
    case 0x27E6: // MATHEMATICAL LEFT WHITE SQUARE BRACKET
    case 0x27E8: // MATHEMATICAL LEFT ANGLE BRACKET
    case 0x27EA: // MATHEMATICAL LEFT DOUBLE ANGLE BRACKET
    case 0x27EC: // MATHEMATICAL LEFT WHITE TORTOISE SHELL BRACKET
    case 0x27EE: // MATHEMATICAL LEFT FLATTENED PARENTHESIS
    case 0x2983: // LEFT WHITE CURLY BRACKET
    case 0x2985: // LEFT WHITE PARENTHESIS
    case 0x2987: // Z NOTATION LEFT IMAGE BRACKET
    case 0x2989: // Z NOTATION LEFT BINDING BRACKET
    case 0x298B: // LEFT SQUARE BRACKET WITH UNDERBAR
    case 0x298D: // LEFT SQUARE BRACKET WITH TICK IN TOP CORNER
    case 0x298F: // LEFT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
    case 0x2991: // LEFT ANGLE BRACKET WITH DOT
    case 0x2993: // LEFT ARC LESS-THAN BRACKET
    case 0x2995: // DOUBLE LEFT ARC GREATER-THAN BRACKET
    case 0x2997: // LEFT BLACK TORTOISE SHELL BRACKET
    case 0x29D8: // LEFT WIGGLY FENCE
    case 0x29DA: // LEFT DOUBLE WIGGLY FENCE
    case 0x29FC: // LEFT-POINTING CURVED ANGLE BRACKET
    case 0x2E22: // TOP LEFT HALF BRACKET
    case 0x2E24: // BOTTOM LEFT HALF BRACKET
    case 0x2E26: // LEFT SIDEWAYS U BRACKET
    case 0x2E28: // LEFT DOUBLE PARENTHESIS
    case 0x3008: // LEFT ANGLE BRACKET
    case 0x300A: // LEFT DOUBLE ANGLE BRACKET
    case 0x300C: // LEFT CORNER BRACKET
    case 0x300E: // LEFT WHITE CORNER BRACKET
    case 0x3010: // LEFT BLACK LENTICULAR BRACKET
    case 0x3014: // LEFT TORTOISE SHELL BRACKET
    case 0x3016: // LEFT WHITE LENTICULAR BRACKET
    case 0x3018: // LEFT WHITE TORTOISE SHELL BRACKET
    case 0x301A: // LEFT WHITE SQUARE BRACKET
    case 0x301D: // REVERSED DOUBLE PRIME QUOTATION MARK
    case 0xFD3E: // ORNATE LEFT PARENTHESIS
    case 0xFE17: // PRESENTATION FORM FOR VERTICAL LEFT WHITE LENTICULAR BRACKET
    case 0xFE35: // PRESENTATION FORM FOR VERTICAL LEFT PARENTHESIS
    case 0xFE37: // PRESENTATION FORM FOR VERTICAL LEFT CURLY BRACKET
    case 0xFE39: // PRESENTATION FORM FOR VERTICAL LEFT TORTOISE SHELL BRACKET
    case 0xFE3B: // PRESENTATION FORM FOR VERTICAL LEFT BLACK LENTICULAR BRACKET
    case 0xFE3D: // PRESENTATION FORM FOR VERTICAL LEFT DOUBLE ANGLE BRACKET
    case 0xFE3F: // PRESENTATION FORM FOR VERTICAL LEFT ANGLE BRACKET
    case 0xFE41: // PRESENTATION FORM FOR VERTICAL LEFT CORNER BRACKET
    case 0xFE43: // PRESENTATION FORM FOR VERTICAL LEFT WHITE CORNER BRACKET
    case 0xFE47: // PRESENTATION FORM FOR VERTICAL LEFT SQUARE BRACKET
    case 0xFE59: // SMALL LEFT PARENTHESIS
    case 0xFE5B: // SMALL LEFT CURLY BRACKET
    case 0xFE5D: // SMALL LEFT TORTOISE SHELL BRACKET
    case 0xFF08: // FULLWIDTH LEFT PARENTHESIS
    case 0xFF3B: // FULLWIDTH LEFT SQUARE BRACKET
    case 0xFF5B: // FULLWIDTH LEFT CURLY BRACKET
    case 0xFF5F: // FULLWIDTH LEFT WHITE PARENTHESIS
    case 0xFF62: // HALFWIDTH LEFT CORNER BRACKET

    // Pe
    case 0x0029: // RIGHT PARENTHESIS
    case 0x005D: // RIGHT SQUARE BRACKET
    case 0x007D: // RIGHT CURLY BRACKET
    case 0x0F3B: // TIBETAN MARK GUG RTAGS GYAS
    case 0x0F3D: // TIBETAN MARK ANG KHANG GYAS
    case 0x169C: // OGHAM REVERSED FEATHER MARK
    case 0x2046: // RIGHT SQUARE BRACKET WITH QUILL
    case 0x207E: // SUPERSCRIPT RIGHT PARENTHESIS
    case 0x208E: // SUBSCRIPT RIGHT PARENTHESIS
    case 0x232A: // RIGHT-POINTING ANGLE BRACKET
    case 0x23B5: // BOTTOM SQUARE BRACKET
    case 0x2769: // MEDIUM RIGHT PARENTHESIS ORNAMENT
    case 0x276B: // MEDIUM FLATTENED RIGHT PARENTHESIS ORNAMENT
    case 0x276D: // MEDIUM RIGHT-POINTING ANGLE BRACKET ORNAMENT
    case 0x276F: // HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT
    case 0x2771: // HEAVY RIGHT-POINTING ANGLE BRACKET ORNAMENT
    case 0x2773: // LIGHT RIGHT TORTOISE SHELL BRACKET ORNAMENT
    case 0x2775: // MEDIUM RIGHT CURLY BRACKET ORNAMENT
    case 0x27E7: // MATHEMATICAL RIGHT WHITE SQUARE BRACKET
    case 0x27E9: // MATHEMATICAL RIGHT ANGLE BRACKET
    case 0x27EB: // MATHEMATICAL RIGHT DOUBLE ANGLE BRACKET
    case 0x2984: // RIGHT WHITE CURLY BRACKET
    case 0x2986: // RIGHT WHITE PARENTHESIS
    case 0x2988: // Z NOTATION RIGHT IMAGE BRACKET
    case 0x298A: // Z NOTATION RIGHT BINDING BRACKET
    case 0x298C: // RIGHT SQUARE BRACKET WITH UNDERBAR
    case 0x298E: // RIGHT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
    case 0x2990: // RIGHT SQUARE BRACKET WITH TICK IN TOP CORNER
    case 0x2992: // RIGHT ANGLE BRACKET WITH DOT
    case 0x2994: // RIGHT ARC GREATER-THAN BRACKET
    case 0x2996: // DOUBLE RIGHT ARC LESS-THAN BRACKET
    case 0x2998: // RIGHT BLACK TORTOISE SHELL BRACKET
    case 0x29D9: // RIGHT WIGGLY FENCE
    case 0x29DB: // RIGHT DOUBLE WIGGLY FENCE
    case 0x29FD: // RIGHT-POINTING CURVED ANGLE BRACKET
    case 0x3009: // RIGHT ANGLE BRACKET
    case 0x300B: // RIGHT DOUBLE ANGLE BRACKET
    case 0x300D: // RIGHT CORNER BRACKET
    case 0x300F: // RIGHT WHITE CORNER BRACKET
    case 0x3011: // RIGHT BLACK LENTICULAR BRACKET
    case 0x3015: // RIGHT TORTOISE SHELL BRACKET
    case 0x3017: // RIGHT WHITE LENTICULAR BRACKET
    case 0x3019: // RIGHT WHITE TORTOISE SHELL BRACKET
    case 0x301B: // RIGHT WHITE SQUARE BRACKET
    case 0x301E: // DOUBLE PRIME QUOTATION MARK
    case 0x301F: // LOW DOUBLE PRIME QUOTATION MARK
    case 0xFD3F: // ORNATE RIGHT PARENTHESIS
    case 0xFE36: // PRESENTATION FORM FOR VERTICAL RIGHT PARENTHESIS
    case 0xFE38: // PRESENTATION FORM FOR VERTICAL RIGHT CURLY BRACKET
    case 0xFE3A: // PRESENTATION FORM FOR VERTICAL RIGHT TORTOISE SHELL BRACKET
    case 0xFE3C: // PRESENTATION FORM FOR VERTICAL RIGHT BLACK LENTICULAR BRACKET
    case 0xFE3E: // PRESENTATION FORM FOR VERTICAL RIGHT DOUBLE ANGLE BRACKET
    case 0xFE40: // PRESENTATION FORM FOR VERTICAL RIGHT ANGLE BRACKET
    case 0xFE42: // PRESENTATION FORM FOR VERTICAL RIGHT CORNER BRACKET
    case 0xFE44: // PRESENTATION FORM FOR VERTICAL RIGHT WHITE CORNER BRACKET
    case 0xFE48: // PRESENTATION FORM FOR VERTICAL RIGHT SQUARE BRACKET
    case 0xFE5A: // SMALL RIGHT PARENTHESIS
    case 0xFE5C: // SMALL RIGHT CURLY BRACKET
    case 0xFE5E: // SMALL RIGHT TORTOISE SHELL BRACKET
    case 0xFF09: // FULLWIDTH RIGHT PARENTHESIS
    case 0xFF3D: // FULLWIDTH RIGHT SQUARE BRACKET
    case 0xFF5D: // FULLWIDTH RIGHT CURLY BRACKET
    case 0xFF60: // FULLWIDTH RIGHT WHITE PARENTHESIS
    case 0xFF63: // HALFWIDTH RIGHT CORNER BRACKET

    // Pf
    case 0x00BB: // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    case 0x2019: // RIGHT SINGLE QUOTATION MARK
    case 0x201D: // RIGHT DOUBLE QUOTATION MARK
    case 0x203A: // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK


    // Pi
    case 0x00AB: // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    case 0x2018: // LEFT SINGLE QUOTATION MARK
    case 0x201C: // LEFT DOUBLE QUOTATION MARK
    case 0x2039: // SINGLE LEFT-POINTING ANGLE QUOTATION MARK

    // Po
    case 0x0021: // EXCLAMATION MARK
    case 0x0022: // QUOTATION MARK
    case 0x0023: // NUMBER SIGN
    case 0x0025: // PERCENT SIGN
    case 0x0026: // AMPERSAND
    case 0x0027: // APOSTROPHE
    case 0x002A: // ASTERISK
    case 0x002C: // COMMA
    case 0x002E: // FULL STOP
    case 0x002F: // SOLIDUS
    case 0x003A: // COLON
    case 0x003B: // SEMICOLON
    case 0x003F: // QUESTION MARK
    case 0x0040: // COMMERCIAL AT
    case 0x005C: // REVERSE SOLIDUS
    case 0x00A1: // INVERTED EXCLAMATION MARK
    case 0x00B7: // MIDDLE DOT
    case 0x00BF: // INVERTED QUESTION MARK
    case 0x037E: // GREEK QUESTION MARK
    case 0x0387: // GREEK ANO TELEIA
    case 0x055A: // ARMENIAN APOSTROPHE
    case 0x055B: // ARMENIAN EMPHASIS MARK
    case 0x055C: // ARMENIAN EXCLAMATION MARK
    case 0x055D: // ARMENIAN COMMA
    case 0x055E: // ARMENIAN QUESTION MARK
    case 0x055F: // ARMENIAN ABBREVIATION MARK
    case 0x0589: // ARMENIAN FULL STOP
    case 0x05BE: // HEBREW PUNCTUATION MAQAF
    case 0x05C0: // HEBREW PUNCTUATION PASEQ
    case 0x05C3: // HEBREW PUNCTUATION SOF PASUQ
    case 0x05F3: // HEBREW PUNCTUATION GERESH
    case 0x05F4: // HEBREW PUNCTUATION GERSHAYIM
    case 0x060C: // ARABIC COMMA
    case 0x060D: // ARABIC DATE SEPARATOR
    case 0x061B: // ARABIC SEMICOLON
    case 0x061F: // ARABIC QUESTION MARK
    case 0x066A: // ARABIC PERCENT SIGN
    case 0x066B: // ARABIC DECIMAL SEPARATOR
    case 0x066C: // ARABIC THOUSANDS SEPARATOR
    case 0x066D: // ARABIC FIVE POINTED STAR
    case 0x06D4: // ARABIC FULL STOP
    case 0x0700: // SYRIAC END OF PARAGRAPH
    case 0x0701: // SYRIAC SUPRALINEAR FULL STOP
    case 0x0702: // SYRIAC SUBLINEAR FULL STOP
    case 0x0703: // SYRIAC SUPRALINEAR COLON
    case 0x0704: // SYRIAC SUBLINEAR COLON
    case 0x0705: // SYRIAC HORIZONTAL COLON
    case 0x0706: // SYRIAC COLON SKEWED LEFT
    case 0x0707: // SYRIAC COLON SKEWED RIGHT
    case 0x0708: // SYRIAC SUPRALINEAR COLON SKEWED LEFT
    case 0x0709: // SYRIAC SUBLINEAR COLON SKEWED RIGHT
    case 0x070A: // SYRIAC CONTRACTION
    case 0x070B: // SYRIAC HARKLEAN OBELUS
    case 0x070C: // SYRIAC HARKLEAN METOBELUS
    case 0x070D: // SYRIAC HARKLEAN ASTERISCUS
    case 0x0964: // DEVANAGARI DANDA
    case 0x0965: // DEVANAGARI DOUBLE DANDA
    case 0x0970: // DEVANAGARI ABBREVIATION SIGN
    case 0x0DF4: // SINHALA PUNCTUATION KUNDDALIYA
    case 0x0E4F: // THAI CHARACTER FONGMAN
    case 0x0E5A: // THAI CHARACTER ANGKHANKHU
    case 0x0E5B: // THAI CHARACTER KHOMUT
    case 0x0F04: // TIBETAN MARK INITIAL YIG MGO MDUN MA
    case 0x0F05: // TIBETAN MARK CLOSING YIG MGO SGAB MA
    case 0x0F06: // TIBETAN MARK CARET YIG MGO PHUR SHAD MA
    case 0x0F07: // TIBETAN MARK YIG MGO TSHEG SHAD MA
    case 0x0F08: // TIBETAN MARK SBRUL SHAD
    case 0x0F09: // TIBETAN MARK BSKUR YIG MGO
    case 0x0F0A: // TIBETAN MARK BKA- SHOG YIG MGO
    case 0x0F0B: // TIBETAN MARK INTERSYLLABIC TSHEG
    case 0x0F0C: // TIBETAN MARK DELIMITER TSHEG BSTAR
    case 0x0F0D: // TIBETAN MARK SHAD
    case 0x0F0E: // TIBETAN MARK NYIS SHAD
    case 0x0F0F: // TIBETAN MARK TSHEG SHAD
    case 0x0F10: // TIBETAN MARK NYIS TSHEG SHAD
    case 0x0F11: // TIBETAN MARK RIN CHEN SPUNGS SHAD
    case 0x0F12: // TIBETAN MARK RGYA GRAM SHAD
    case 0x0F85: // TIBETAN MARK PALUTA
    case 0x104A: // MYANMAR SIGN LITTLE SECTION
    case 0x104B: // MYANMAR SIGN SECTION
    case 0x104C: // MYANMAR SYMBOL LOCATIVE
    case 0x104D: // MYANMAR SYMBOL COMPLETED
    case 0x104E: // MYANMAR SYMBOL AFOREMENTIONED
    case 0x104F: // MYANMAR SYMBOL GENITIVE
    case 0x10FB: // GEORGIAN PARAGRAPH SEPARATOR
    case 0x1361: // ETHIOPIC WORDSPACE
    case 0x1362: // ETHIOPIC FULL STOP
    case 0x1363: // ETHIOPIC COMMA
    case 0x1364: // ETHIOPIC SEMICOLON
    case 0x1365: // ETHIOPIC COLON
    case 0x1366: // ETHIOPIC PREFACE COLON
    case 0x1367: // ETHIOPIC QUESTION MARK
    case 0x1368: // ETHIOPIC PARAGRAPH SEPARATOR
    case 0x166D: // CANADIAN SYLLABICS CHI SIGN
    case 0x166E: // CANADIAN SYLLABICS FULL STOP
    case 0x16EB: // RUNIC SINGLE PUNCTUATION
    case 0x16EC: // RUNIC MULTIPLE PUNCTUATION
    case 0x16ED: // RUNIC CROSS PUNCTUATION
    case 0x1735: // PHILIPPINE SINGLE PUNCTUATION
    case 0x1736: // PHILIPPINE DOUBLE PUNCTUATION
    case 0x17D4: // KHMER SIGN KHAN
    case 0x17D5: // KHMER SIGN BARIYOOSAN
    case 0x17D6: // KHMER SIGN CAMNUC PII KUUH
    case 0x17D8: // KHMER SIGN BEYYAL
    case 0x17D9: // KHMER SIGN PHNAEK MUAN
    case 0x17DA: // KHMER SIGN KOOMUUT
    case 0x1800: // MONGOLIAN BIRGA
    case 0x1801: // MONGOLIAN ELLIPSIS
    case 0x1802: // MONGOLIAN COMMA
    case 0x1803: // MONGOLIAN FULL STOP
    case 0x1804: // MONGOLIAN COLON
    case 0x1805: // MONGOLIAN FOUR DOTS
    case 0x1807: // MONGOLIAN SIBE SYLLABLE BOUNDARY MARKER
    case 0x1808: // MONGOLIAN MANCHU COMMA
    case 0x1809: // MONGOLIAN MANCHU FULL STOP
    case 0x180A: // MONGOLIAN NIRUGU
    case 0x1944: // LIMBU EXCLAMATION MARK
    case 0x1945: // LIMBU QUESTION MARK
    case 0x2016: // DOUBLE VERTICAL LINE
    case 0x2017: // DOUBLE LOW LINE
    case 0x2020: // DAGGER
    case 0x2021: // DOUBLE DAGGER
    case 0x2022: // BULLET
    case 0x2023: // TRIANGULAR BULLET
    case 0x2024: // ONE DOT LEADER
    case 0x2025: // TWO DOT LEADER
    case 0x2026: // HORIZONTAL ELLIPSIS
    case 0x2027: // HYPHENATION POINT
    case 0x2030: // PER MILLE SIGN
    case 0x2031: // PER TEN THOUSAND SIGN
    case 0x2032: // PRIME
    case 0x2033: // DOUBLE PRIME
    case 0x2034: // TRIPLE PRIME
    case 0x2035: // REVERSED PRIME
    case 0x2036: // REVERSED DOUBLE PRIME
    case 0x2037: // REVERSED TRIPLE PRIME
    case 0x2038: // CARET
    case 0x203B: // REFERENCE MARK
    case 0x203C: // DOUBLE EXCLAMATION MARK
    case 0x203D: // INTERROBANG
    case 0x203E: // OVERLINE
    case 0x2041: // CARET INSERTION POINT
    case 0x2042: // ASTERISM
    case 0x2043: // HYPHEN BULLET
    case 0x2047: // DOUBLE QUESTION MARK
    case 0x2048: // QUESTION EXCLAMATION MARK
    case 0x2049: // EXCLAMATION QUESTION MARK
    case 0x204A: // TIRONIAN SIGN ET
    case 0x204B: // REVERSED PILCROW SIGN
    case 0x204C: // BLACK LEFTWARDS BULLET
    case 0x204D: // BLACK RIGHTWARDS BULLET
    case 0x204E: // LOW ASTERISK
    case 0x204F: // REVERSED SEMICOLON
    case 0x2050: // CLOSE UP
    case 0x2051: // TWO ASTERISKS ALIGNED VERTICALLY
    case 0x2053: // SWUNG DASH
    case 0x2057: // QUADRUPLE PRIME
    case 0x23B6: // BOTTOM SQUARE BRACKET OVER TOP SQUARE BRACKET
    case 0x3001: // IDEOGRAPHIC COMMA
    case 0x3002: // IDEOGRAPHIC FULL STOP
    case 0x3003: // DITTO MARK
    case 0x303D: // PART ALTERNATION MARK
    case 0xFE30: // PRESENTATION FORM FOR VERTICAL TWO DOT LEADER
    case 0xFE45: // SESAME DOT
    case 0xFE46: // WHITE SESAME DOT
    case 0xFE49: // DASHED OVERLINE
    case 0xFE4A: // CENTRELINE OVERLINE
    case 0xFE4B: // WAVY OVERLINE
    case 0xFE4C: // DOUBLE WAVY OVERLINE
    case 0xFE50: // SMALL COMMA
    case 0xFE51: // SMALL IDEOGRAPHIC COMMA
    case 0xFE52: // SMALL FULL STOP
    case 0xFE54: // SMALL SEMICOLON
    case 0xFE55: // SMALL COLON
    case 0xFE56: // SMALL QUESTION MARK
    case 0xFE57: // SMALL EXCLAMATION MARK
    case 0xFE5F: // SMALL NUMBER SIGN
    case 0xFE60: // SMALL AMPERSAND
    case 0xFE61: // SMALL ASTERISK
    case 0xFE68: // SMALL REVERSE SOLIDUS
    case 0xFE6A: // SMALL PERCENT SIGN
    case 0xFE6B: // SMALL COMMERCIAL AT
    case 0xFF01: // FULLWIDTH EXCLAMATION MARK
    case 0xFF02: // FULLWIDTH QUOTATION MARK
    case 0xFF03: // FULLWIDTH NUMBER SIGN
    case 0xFF05: // FULLWIDTH PERCENT SIGN
    case 0xFF06: // FULLWIDTH AMPERSAND
    case 0xFF07: // FULLWIDTH APOSTROPHE
    case 0xFF0A: // FULLWIDTH ASTERISK
    case 0xFF0C: // FULLWIDTH COMMA
    case 0xFF0E: // FULLWIDTH FULL STOP
    case 0xFF0F: // FULLWIDTH SOLIDUS
    case 0xFF1A: // FULLWIDTH COLON
    case 0xFF1B: // FULLWIDTH SEMICOLON
    case 0xFF1F: // FULLWIDTH QUESTION MARK
    case 0xFF20: // FULLWIDTH COMMERCIAL AT
    case 0xFF3C: // FULLWIDTH REVERSE SOLIDUS
    case 0xFF61: // HALFWIDTH IDEOGRAPHIC FULL STOP
    case 0xFF64: // HALFWIDTH IDEOGRAPHIC COMMA
      return true;
    default:
      return false;
  }
}

wstring trimWhitespaceOrPunct( wstring const & in )
{
  wchar const * wordBegin = in.c_str();
  wstring::size_type wordSize = in.size();

  // Skip any leading whitespace
  while( *wordBegin && ( Folding::isWhitespace( *wordBegin ) || Folding::isPunct( *wordBegin ) ) )
  {
    ++wordBegin;
    --wordSize;
  }

  // Skip any trailing whitespace
  while( wordSize && ( Folding::isWhitespace( wordBegin[ wordSize - 1 ] ) ||
                       Folding::isPunct( wordBegin[ wordSize - 1 ] ) ) )
    --wordSize;

  return wstring( wordBegin, wordSize );
}

wstring trimWhitespace( wstring const & in )
{
  wchar const * wordBegin = in.c_str();
  wstring::size_type wordSize = in.size();

  // Skip any leading whitespace
  while( *wordBegin && Folding::isWhitespace( *wordBegin ) )
  {
    ++wordBegin;
    --wordSize;
  }

  // Skip any trailing whitespace
  while( wordSize && Folding::isWhitespace( wordBegin[ wordSize - 1 ] ) )
    --wordSize;

  return wstring( wordBegin, wordSize );
}

void normalizeWhitespace( wstring & str )
{
  for( size_t x = str.size(); x-- > 1; ) // >1 -- Don't test the first char
  {
    if ( isWhitespace( str[ x ] ) )
    {
      size_t y;
      for( y = x; y && ( isWhitespace( str[ y - 1 ] ) ) ; --y );

      if ( y != x )
      {
        // Remove extra spaces

        str.erase( y, x - y );

        x = y;

        str[ x ] = ' ';
      }
    }
  }
}

QString escapeWildcardSymbols( const QString & str )
{
  QString escaped( str );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  escaped.replace( QRegularExpression( "([\\[\\]\\?\\*])" ), "\\\\1" );
#else
  escaped.replace( QRegExp( "([\\[\\]\\?\\*])", Qt::CaseInsensitive ), "\\\\1" );
#endif
  return escaped;
}

QString unescapeWildcardSymbols( const QString & str )
{
  QString unescaped( str );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  unescaped.replace( QRegularExpression( "\\\\([\\[\\]\\?\\*])" ), "\\1" );
#else
  unescaped.replace( QRegExp( "\\\\([\\[\\]\\?\\*])", Qt::CaseInsensitive ), "\\1" );
#endif
  return unescaped;
}

void prepareToEmbedRTL( QString & str )
{
  if( str.isRightToLeft() )
  {
    str.insert( 0, (ushort)0x202E ); // RLE, Right-to-Left Embedding
    str.append( (ushort)0x202C ); // PDF, POP DIRECTIONAL FORMATTING
  }
}

wchar foldedDiacritic( wchar const * in, size_t size, size_t & consumed )
{
  return foldDiacritic( in, size, consumed );
}


}

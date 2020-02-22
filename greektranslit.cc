/* This file is (c) 2010 Jennie Petoumenou <epetoumenou@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "greektranslit.hh"
#include "transliteration.hh"
#if defined(Gen_Transliteration) && !defined (_MSC_VER)
#include "utf8.hh"
#endif
#include <QCoreApplication>

namespace GreekTranslit {

class GreekTable: public Transliteration::Table
{
public:
#if defined(Gen_Transliteration) && !defined (_MSC_VER)
    void ins( char const * from, char const * to )
    {
        write_Table_UCS_FILE(GreekTable, from,to)
                Transliteration::Table::ins(from, to);
    }
#endif
    GreekTable();
};

GreekTable::GreekTable()
{
#if defined(Gen_Transliteration) && !defined (_MSC_VER)
    // Utf8

    //I. LATIN -> MODERN & CLASSICAL GREEK (COMMON CHARACTERS)
    ins("a",      "α");
    ins("b",      "β");
    ins("v",      "β");
    ins("g",      "γ");
    ins("d",      "δ");
    ins("e",      "ε");
    ins("z",      "ζ");
    ins("h",      "η");
    ins("q",      "θ");
    ins("8",      "θ");
    ins("i",      "ι");
    ins("k",      "κ");
    ins("l",      "λ");
    ins("m",      "μ");
    ins("n",      "ν");
    ins("c",      "ξ");
    ins("3",      "ξ");
    ins("ks",     "ξ");
    ins("o",      "ο");
    ins("p",      "π");
    ins("r",      "ρ");
    ins("s",      "σ");
    ins("s1",     "σ");
    ins("j",      "ς");
    ins("s2",     "ς");
    ins("s\\n",   "ς");
    ins("t",      "τ");
    ins("u",      "υ");
    ins("f",      "φ");
    ins("x",      "χ");
    ins("y",      "ψ");
    ins("ps",     "ψ");
    ins("w",      "ω");

    ins("A",      "Α");
    ins("B",      "Β");
    ins("V",      "Β");
    ins("G",      "Γ");
    ins("D",      "Δ");
    ins("E",      "Ε");
    ins("Z",      "Ζ");
    ins("H",      "Η");
    ins("Q",      "Θ");
    ins("I",      "Ι");
    ins("K",      "Κ");
    ins("L",      "Λ");
    ins("M",      "Μ");
    ins("N",      "Ν");
    ins("C",      "Ξ");
    ins("KS",     "Ξ");
    ins("Ks",     "Ξ");
    ins("O",      "Ο");
    ins("P",      "Π");
    ins("R",      "Τ");
    ins("S",      "Σ");
    ins("S1",     "Σ");
    ins("J",      "Σ");
    ins("S2",     "Σ");
    ins("S\\n",   "Σ");
    ins("T",      "Τ");
    ins("U",      "Υ");
    ins("F",      "Φ");
    ins("X",      "Χ");
    ins("Y",      "Ψ");
    ins("PS",     "Ψ");
    ins("Ps",     "Ψ");
    ins("W",      "Ω");

    //II. LATIN -> MODERN GREEK (DIACRITICS)
    
    ins("'a",      "ά");
    ins("'e",      "έ");
    ins("'h",      "ή");
    ins("'i",      "ί");
    ins("\"i",     "ϊ");
    ins("\"'i",    "ΐ");
    ins("'\"i",    "ΐ");
    ins("'o",      "ό");
    ins("'u",      "ύ");
    ins("\"u",     "ϋ");
    ins("\"'u",    "ΰ");
    ins("'\"u",    "ΰ");
    ins("'w",      "ώ");

    ins("'A",      "Ά");
    ins("'E",      "Έ");
    ins("'H",      "Ή");
    ins("'I",      "Ί");
    ins("\"I",     "Ϊ");
    ins("'O",      "Ό");
    ins("'U",      "Ύ");
    ins("\"U",     "Ϋ");
    ins("'W",      "Ώ");

    //IV. LATIN -> CLASSICAL GREEK (DIACRITICS - BETA CODE)
    // Adapted from beta2unicode.py by James Tauber <http://jtauber.com/>

    //ORDER: Breathing - Accent

    //uppercase (asterisk & capitals)
    
    //oceia - bareia
    ins("*)A",     "Ἀ");
    ins("*(A",     "Ἁ");
    ins("*\\A",     "Ὰ");
    ins("*/A",      "Ά");
    ins("*)\\A",   "Ἂ");
    ins("*(\\A",   "Ἃ");
    ins("*)/A",    "Ἄ");
    ins("*(/A",    "Ἅ");

    ins("*)E",     "Ἐ");
    ins("*(E",     "Ἑ");
    ins("*\\E",     "Ὲ");
    ins("*/E",      "Έ");
    ins("*)\\E",   "Ἒ");
    ins("*(\\E",   "Ἓ");
    ins("*)/E",    "Ἔ");
    ins("*(/E",    "Ἕ");

    ins("*)H",     "Ἠ");
    ins("*(H",     "Ἡ");
    ins("*\\H",     "Ὴ");
    ins("*/H",      "Ή");
    ins("*)\\H",   "Ἢ");
    ins("*(\\H",   "Ἣ");
    ins("*)/H",    "Ἤ");
    ins("*(/H",    "Ἥ");

    ins("*)I",     "Ἰ");
    ins("*(I",     "Ἱ");
    ins("*\\I",     "Ὶ");
    ins("*/I",      "Ί");
    ins("*)\\I",   "Ἲ");
    ins("*(\\I",   "Ἳ");
    ins("*)/I",    "Ἴ");
    ins("*(/I",    "Ἵ");

    ins("*)O",     "Ὀ");
    ins("*(O",     "Ὁ");
    ins("*\\O",     "Ὸ");
    ins("*/O",      "Ό");
    ins("*)\\O",   "Ὂ");
    ins("*(\\O",   "Ὃ");
    ins("*)/O",    "Ὄ");
    ins("*(/O",    "Ὅ");

    ins("*(R",    "Ῥ");

    ins("*(U",     "Ὑ");
    ins("*\\U",     "Ὺ");
    ins("*/U",      "Ύ");
    ins("*(\\U",   "Ὓ");
    ins("*(/U",    "Ὕ");

    ins("*)W",     "Ὠ");
    ins("*(W",     "Ὡ");
    ins("*\\W",     "Ὼ");
    ins("*/W",      "Ώ");
    ins("*)\\W",   "Ὢ");
    ins("*(\\W",   "Ὣ");
    ins("*)/W",    "Ὤ");
    ins("*(/W",    "Ὥ");

    //perispwmenh
    ins("*)=A",    "Ἆ");
    ins("*(=A",    "Ἇ");
    
    ins("*)=H",    "Ἦ");
    ins("*(=H",    "Ἧ");
    
    ins("*)=I",    "Ἶ");
    ins("*(=I",    "Ἷ");
    
    ins("*(=U",    "Ὗ");
    
    ins("*)=W",    "Ὦ");
    ins("*(=W",    "Ὧ");

    //upogegrammenh
    ins("*A|",      "ᾼ");
    ins("*)A|",    "ᾈ");
    ins("*(A|",    "ᾉ");
    ins("*)/A|",   "ᾌ");
    ins("*(/A|",   "ᾍ");
    ins("*)=A|",   "ᾎ");
    ins("*(=A|",   "ᾏ");

    ins("*H|",      "ῌ");
    ins("*)H|",    "ᾘ");
    ins("*(H|",    "ᾙ");
    ins("*)/H|",   "ᾜ");
    ins("*(/H|",   "ᾝ");
    ins("*)=H|",   "ᾞ");
    ins("*(=H|",   "ᾟ");

    ins("*W|",      "ῼ");
    ins("*)W|",    "ᾨ");
    ins("*(W|",    "ᾩ");
    ins("*)/W|",   "ᾬ");
    ins("*(/W|",   "ᾭ");
    ins("*)=W|",   "ᾮ");
    ins("*(=W|",   "ᾯ");

    ins("*|A",      "ᾼ");
    ins("*)|A",    "ᾈ");
    ins("*(|A",    "ᾉ");
    ins("*)/|A",   "ᾌ");
    ins("*(/|A",   "ᾍ");
    ins("*)=|A",   "ᾎ");
    ins("*(=|A",   "ᾏ");

    ins("*|H",      "ῌ");
    ins("*)|H",    "ᾘ");
    ins("*(|H",    "ᾙ");
    ins("*)/|H",   "ᾜ");
    ins("*(/|H",   "ᾝ");
    ins("*)=|H",   "ᾞ");
    ins("*(=|H",   "ᾟ");

    ins("*|W",      "ῼ");
    ins("*)|W",    "ᾨ");
    ins("*(|W",    "ᾩ");
    ins("*)/|W",   "ᾬ");
    ins("*(/|W",   "ᾭ");
    ins("*)=|W",   "ᾮ");
    ins("*(=|W",   "ᾯ");

    //diairesis
    ins("*+I",      "Ϊ");
    ins("*+U",      "Ϋ");
    
    //uppercase (asterisk & small letters)
    
    //oceia - bareia
    ins("*)a",     "Ἀ");
    ins("*(a",     "Ἁ");
    ins("*\\a",     "Ὰ");
    ins("*/a",      "Ά");
    ins("*)\\a",   "Ἂ");
    ins("*(\\a",   "Ἃ");
    ins("*)/a",    "Ἄ");
    ins("*(/a",    "Ἅ");

    ins("*)e",     "Ἐ");
    ins("*(e",     "Ἑ");
    ins("*\\e",     "Ὲ");
    ins("*/e",      "Έ");
    ins("*)\\e",   "Ἒ");
    ins("*(\\e",   "Ἓ");
    ins("*)/e",    "Ἔ");
    ins("*(/e",    "Ἕ");

    ins("*)h",     "Ἠ");
    ins("*(h",     "Ἡ");
    ins("*\\h",     "Ὴ");
    ins("*/h",      "Ή");
    ins("*)\\h",   "Ἢ");
    ins("*(\\h",   "Ἣ");
    ins("*)/h",    "Ἤ");
    ins("*(/h",    "Ἥ");

    ins("*)i",     "Ἰ");
    ins("*(i",     "Ἱ");
    ins("*\\i",     "Ὶ");
    ins("*/i",      "Ί");
    ins("*)\\i",   "Ἲ");
    ins("*(\\i",   "Ἳ");
    ins("*)/i",    "Ἴ");
    ins("*(/i",    "Ἵ");

    ins("*)o",     "Ὀ");
    ins("*(o",     "Ὁ");
    ins("*\\o",     "Ὸ");
    ins("*/o",      "Ό");
    ins("*)\\o",   "Ὂ");
    ins("*(\\o",   "Ὃ");
    ins("*)/o",    "Ὄ");
    ins("*(/o",    "Ὅ");

    ins("*(r",     "Ῥ");

    ins("*(u",     "Ὑ");
    ins("*\\u",     "Ὺ");
    ins("*/u",      "Ύ");
    ins("*(\\u",   "Ὓ");
    ins("*(/u",    "Ὕ");

    ins("*)w",     "Ὠ");
    ins("*(w",     "Ὡ");
    ins("*\\w",     "Ὼ");
    ins("*/w",      "Ώ");
    ins("*)\\w",   "Ὢ");
    ins("*(\\w",   "Ὣ");
    ins("*)/w",    "Ὤ");
    ins("*(/w",    "Ὥ");

    //perispwmenh
    ins("*)=a",    "Ἆ");
    ins("*(=a",    "Ἇ");
    
    ins("*)=h",    "Ἦ");
    ins("*(=h",    "Ἧ");
    
    ins("*)=i",    "Ἶ");
    ins("*(=i",    "Ἷ");
    
    ins("*(=u",    "Ὗ");
    
    ins("*)=w",    "Ὦ");
    ins("*(=w",    "Ὧ");

    //upogegrammenh
    ins("*a|",      "ᾼ");
    ins("*)a|",    "ᾈ");
    ins("*(a|",    "ᾉ");
    ins("*)/a|",   "ᾌ");
    ins("*(/a|",   "ᾍ");
    ins("*)=a|",   "ᾎ");
    ins("*(=a|",   "ᾏ");

    ins("*h|",      "ῌ");
    ins("*)h|",    "ᾘ");
    ins("*(h|",    "ᾙ");
    ins("*)/h|",   "ᾜ");
    ins("*(/h|",   "ᾝ");
    ins("*)=h|",   "ᾞ");
    ins("*(=h|",   "ᾟ");

    ins("*w|",      "ῼ");
    ins("*)w|",    "ᾨ");
    ins("*(w|",    "ᾩ");
    ins("*)/w|",   "ᾬ");
    ins("*(/w|",   "ᾭ");
    ins("*)=w|",   "ᾮ");
    ins("*(=w|",   "ᾯ");

    ins("*|a",      "ᾼ");
    ins("*)|a",    "ᾈ");
    ins("*(|a",    "ᾉ");
    ins("*)/|a",   "ᾌ");
    ins("*(/|a",   "ᾍ");
    ins("*)=|a",   "ᾎ");
    ins("*(=|a",   "ᾏ");

    ins("*|h",      "ῌ");
    ins("*)|h",    "ᾘ");
    ins("*(|h",    "ᾙ");
    ins("*)/|h",   "ᾜ");
    ins("*(/|h",   "ᾝ");
    ins("*)=|h",   "ᾞ");
    ins("*(=|h",   "ᾟ");

    ins("*|w",      "ῼ");
    ins("*)|w",    "ᾨ");
    ins("*(|w",    "ᾩ");
    ins("*)/|w",   "ᾬ");
    ins("*(/|w",   "ᾭ");
    ins("*)=|w",   "ᾮ");
    ins("*(=|w",   "ᾯ");

    //diairesis
    ins("*+i",      "Ϊ");
    ins("*+u",      "Ϋ");
    
    //uppercase (capitals)
    
    //oceia - bareia
    ins("A)",     "Ἀ");
    ins("A(",     "Ἁ");
    ins("A\\",     "Ὰ");
    ins("A/",      "Ά");
    ins("A)\\",   "Ἂ");
    ins("A(\\",   "Ἃ");
    ins("A)/",    "Ἄ");
    ins("A(/",    "Ἅ");

    ins("E)",     "Ἐ");
    ins("E(",     "Ἑ");
    ins("E\\",     "Ὲ");
    ins("E/",      "Έ");
    ins("E)\\",   "Ἒ");
    ins("E(\\",   "Ἓ");
    ins("E)/",    "Ἔ");
    ins("E(/",    "Ἕ");

    ins("H)",     "Ἠ");
    ins("H(",     "Ἡ");
    ins("H\\",     "Ὴ");
    ins("H/",      "Ή");
    ins("H)\\",   "Ἢ");
    ins("H(\\",   "Ἣ");
    ins("H)/",    "Ἤ");
    ins("H(/",    "Ἥ");

    ins("I)",     "Ἰ");
    ins("I(",     "Ἱ");
    ins("I\\",     "Ὶ");
    ins("I/",      "Ί");
    ins("I)\\",   "Ἲ");
    ins("I(\\",   "Ἳ");
    ins("I)/",    "Ἴ");
    ins("I(/",    "Ἵ");

    ins("O)",     "Ὀ");
    ins("O(",     "Ὁ");
    ins("O\\",     "Ὸ");
    ins("O/",      "Ό");
    ins("O)\\",   "Ὂ");
    ins("O(\\",   "Ὃ");
    ins("O)/",    "Ὄ");
    ins("O(/",    "Ὅ");

    ins("R(",     "Ῥ");

    ins("U)",     "ὐ");
    ins("U(",     "Ὑ");
    ins("U\\",     "Ὺ");
    ins("U/",      "Ύ");
    ins("U)\\",   "ὒ");
    ins("U(\\",   "Ὓ");
    ins("U)/",    "ὔ");
    ins("U(/",    "Ὕ");

    ins("W)",     "Ὠ");
    ins("W(",     "Ὡ");
    ins("W\\",     "Ὼ");
    ins("W/",      "Ώ");
    ins("W)\\",   "Ὢ");
    ins("W(\\",   "Ὣ");
    ins("W)/",    "Ὤ");
    ins("W(/",    "Ὥ");

    //perispwmenh
    ins("A=",      "ᾶ");
    ins("A)=",    "Ἆ");
    ins("A(=",    "Ἇ");
    
    ins("H=",      "ῆ");
    ins("H)=",    "Ἦ");
    ins("H(=",    "Ἧ");
    
    ins("I=",      "ῖ");
    ins("I)=",    "Ἶ");
    ins("I(=",    "Ἷ");
    
    ins("U=",      "ῦ");
    ins("U)=",    "ὖ");
    ins("U(=",    "Ὗ");
    
    ins("W=",      "ῶ");
    ins("W)=",    "Ὦ");
    ins("W(=",    "Ὧ");

    //upogegrammenh
    ins("A|",      "ᾼ");
    ins("A)|",    "ᾈ");
    ins("A(|",    "ᾉ");
    ins("A/|",     "ᾴ");
    ins("A)/|",   "ᾌ");
    ins("A(/|",   "ᾍ");
    ins("A=|",     "ᾷ");
    ins("A)=|",   "ᾎ");
    ins("A(=|",   "ᾏ");

    ins("H|",      "ῌ");
    ins("H)|",    "ᾘ");
    ins("H(|",    "ᾙ");
    ins("H/|",     "ῄ");
    ins("H)/|",   "ᾜ");
    ins("H(/|",   "ᾝ");
    ins("H=|",     "ῇ");
    ins("H)=|",   "ᾞ");
    ins("H(=|",   "ᾟ");

    ins("W|",      "ῼ");
    ins("W)|",    "ᾨ");
    ins("W(|",    "ᾩ");
    ins("W/|",     "ῴ");
    ins("W)/|",   "ᾬ");
    ins("W(/|",   "ᾭ");
    ins("W=|",     "ῷ");
    ins("W)=|",   "ᾮ");
    ins("W(=|",   "ᾯ");

    //diairesis
    ins("I+",      "Ϊ");
    ins("I+\\",    "ῒ");
    ins("I+/",     "ΐ");
    ins("U+",      "Ϋ");
    ins("U+\\",    "ῢ");
    ins("U+/",     "ΰ");

    //lowercase (small letters)
    
    //oceia - bareia
    ins("a)",     "ἀ");
    ins("a(",     "ἁ");
    ins("a\\",     "ὰ");
    ins("a/",      "ά");
    ins("a)\\",   "ἂ");
    ins("a(\\",   "ἃ");
    ins("a)/",    "ἄ");
    ins("a(/",    "ἅ");

    ins("e)",     "ἐ");
    ins("e(",     "ἑ");
    ins("e\\",     "ὲ");
    ins("e/",      "έ");
    ins("e)\\",   "ἒ");
    ins("e(\\",   "ἓ");
    ins("e)/",    "ἔ");
    ins("e(/",    "ἕ");

    ins("h)",     "ἠ");
    ins("h(",     "ἡ");
    ins("h\\",     "ὴ");
    ins("h/",      "ή");
    ins("h)\\",   "ἢ");
    ins("h(\\",   "ἣ");
    ins("h)/",    "ἤ");
    ins("h(/",    "ἥ");

    ins("i)",     "ἰ");
    ins("i(",     "ἱ");
    ins("i\\",     "ὶ");
    ins("i/",      "ί");
    ins("i)\\",   "ἲ");
    ins("i(\\",   "ἳ");
    ins("i)/",    "ἴ");
    ins("i(/",    "ἵ");

    ins("o)",     "ὀ");
    ins("o(",     "ὁ");
    ins("o\\",     "ὸ");
    ins("o/",      "ό");
    ins("o)\\",   "ὂ");
    ins("o(\\",   "ὃ");
    ins("o)/",    "ὄ");
    ins("o(/",    "ὅ");

    ins("r(",      "ῤ");

    ins("u)",     "ὐ");
    ins("u(",     "ὑ");
    ins("u\\",     "ὺ");
    ins("u/",      "ύ");
    ins("u)\\",   "ὒ");
    ins("u(\\",   "ὓ");
    ins("u)/",    "ὔ");
    ins("u(/",    "ὕ");

    ins("w)",     "ὠ");
    ins("w(",     "ὡ");
    ins("w\\",     "ὼ");
    ins("w/",      "ώ");
    ins("w)\\",   "ὢ");
    ins("w(\\",   "ὣ");
    ins("w)/",    "ὤ");
    ins("w(/",    "ὥ");

    //perispwmenh
    ins("a=",      "ᾶ");
    ins("a)=",    "ἆ");
    ins("a(=",    "ἇ");
    
    ins("h=",      "ῆ");
    ins("h)=",    "ἦ");
    ins("h(=",    "ἧ");
    
    ins("i=",      "ῖ");
    ins("i)=",    "ἶ");
    ins("i(=",    "ἷ");
    
    ins("u=",      "ῦ");
    ins("u)=",    "ὖ");
    ins("u(=",    "ὗ");
    
    ins("w=",      "ῶ");
    ins("w)=",    "ὦ");
    ins("w(=",    "ὧ");

    //upogegrammenh
    ins("a|",      "ᾳ");
    ins("a)|",    "ᾀ");
    ins("a(|",    "ᾁ");
    ins("a/|",     "ᾴ");
    ins("a)/|",   "ᾄ");
    ins("a(/|",   "ᾅ");
    ins("a=|",     "ᾷ");
    ins("a)=|",   "ᾆ");
    ins("a(=|",   "ᾇ");

    ins("h|",      "ῃ");
    ins("h)|",    "ᾐ");
    ins("h(|",    "ᾑ");
    ins("h/|",     "ῄ");
    ins("h)/|",   "ᾔ");
    ins("h(/|",   "ᾕ");
    ins("h=|",     "ῇ");
    ins("h)=|",   "ᾖ");
    ins("h(=|",   "ᾗ");

    ins("w|",      "ῳ");
    ins("w)|",    "ᾠ");
    ins("w(|",    "ᾡ");
    ins("w/|",     "ῴ");
    ins("w)/|",   "ᾤ");
    ins("w(/|",   "ᾥ");
    ins("w=|",     "ῷ");
    ins("w)=|",   "ᾦ");
    ins("w(=|",   "ᾧ");

    //diairesis
    ins("i+",      "ϊ");
    ins("i+\\",    "ῒ");
    ins("i+/",     "ΐ");
    ins("u+",      "ϋ");
    ins("u+\\",    "ῢ");
    ins("u+/",     "ΰ");
    

    //ORDER: ACCENT - BREATHING (UNOFFICIAL)

    //uppercase (capitals)
    
    //oceia - bareia
    ins("A\\)",   "Ἂ");
    ins("A\\(",   "Ἃ");
    ins("A/)",    "Ἄ");
    ins("A/(",    "Ἅ");

    ins("E\\)",   "Ἒ");
    ins("E\\(",   "Ἓ");
    ins("E/)",    "Ἔ");
    ins("E/(",    "Ἕ");

    ins("H\\)",   "Ἢ");
    ins("H\\(",   "Ἣ");
    ins("H/)",    "Ἤ");
    ins("H/(",    "Ἥ");

    ins("I\\)",   "Ἲ");
    ins("I\\(",   "Ἳ");
    ins("I/)",    "Ἴ");
    ins("I/(",    "Ἵ");

    ins("O\\)",   "Ὂ");
    ins("O\\(",   "Ὃ");
    ins("O/)",    "Ὄ");
    ins("O/(",    "Ὅ");

    ins("U\\(",   "Ὓ");
    ins("U/(",    "Ὕ");

    ins("W\\)",   "Ὢ");
    ins("W\\(",   "Ὣ");
    ins("W/)",    "Ὤ");
    ins("W/(",    "Ὥ");

    //perispwmenh
    ins("A=)",    "Ἆ");
    ins("A=(",    "Ἇ");
    
    ins("H=)",    "Ἦ");
    ins("H=(",    "Ἧ");
    
    ins("I=)",    "Ἶ");
    ins("I=(",    "Ἷ");
    
    ins("U=(",    "Ὗ");
    
    ins("W=)",    "Ὦ");
    ins("W=(",    "Ὧ");

    //upogegrammenh
    ins("A/)|",   "ᾌ");
    ins("A/(|",   "ᾍ");
    ins("A=)|",   "ᾎ");
    ins("A=(|",   "ᾏ");

    ins("H/)|",   "ᾜ");
    ins("H/(|",   "ᾝ");
    ins("H=)|",   "ᾞ");
    ins("H=(|",   "ᾟ");

    ins("W/)|",   "ᾬ");
    ins("W/(|",   "ᾭ");
    ins("W=)|",   "ᾮ");
    ins("W=(|",   "ᾯ");

    //lowercase (small letters)
    
    //oceia - bareia
    ins("a\\)",   "ἂ");
    ins("a\\(",   "ἃ");
    ins("a/)",    "ἄ");
    ins("a/(",    "ἅ");


    ins("e\\)",   "ἒ");
    ins("e\\(",   "ἓ");
    ins("e/)",    "ἔ");
    ins("e/(",    "ἕ");

    ins("h\\)",   "ἢ");
    ins("h\\(",   "ἣ");
    ins("h/)",    "ἤ");
    ins("h/(",    "ἥ");

    ins("i\\)",   "ἲ");
    ins("i\\(",   "ἳ");
    ins("i/)",    "ἴ");
    ins("i/(",    "ἵ");

    ins("o\\)",   "ὂ");
    ins("o\\(",   "ὃ");
    ins("o/)",    "ὄ");
    ins("o/(",    "ὅ");

    ins("u\\)",   "ὒ");
    ins("u\\(",   "ὓ");
    ins("u/)",    "ὔ");
    ins("u/(",    "ὕ");

    ins("w\\)",   "ὢ");
    ins("w\\(",   "ὣ");
    ins("w/)",    "ὤ");
    ins("w/(",    "ὥ");

    //perispwmenh
    ins("a=)",    "ἆ");
    ins("a=(",    "ἇ");
    
    ins("h=)",    "ἦ");
    ins("h=(",    "ἧ");
    
    ins("i=)",    "ἶ");
    ins("i=(",    "ἷ");
    
    ins("u=)",    "ὖ");
    ins("u=(",    "ὗ");
    
    ins("w=)",    "ὦ");
    ins("w=(",    "ὧ");

    //upogegrammenh
    ins("a/)|",   "ᾄ");
    ins("a/(|",   "ᾅ");
    ins("a=)|",   "ᾆ");
    ins("a=(|",   "ᾇ");

    ins("h/)|",   "ᾔ");
    ins("h/(|",   "ᾕ");
    ins("h=)|",   "ᾖ");
    ins("h=(|",   "ᾗ");

    ins("w/)|",   "ᾤ");
    ins("w/(|",   "ᾥ");
    ins("w=)|",   "ᾦ");
    ins("w=(|",   "ᾧ");

    //diairesis
    ins("i\\+",    "ῒ");
    ins("i/+",     "ΐ");
    ins("u\\+",    "ῢ");
    ins("u/+",     "ΰ");

    //IΙI. MODERN GREEK <-> CLASSICAL GREEK
    //convert tonos to oceia

    ins("ά",      "ά");
    ins("έ",      "έ");
    ins("ή",      "ή");
    ins("ί",      "ί");
    ins("ΐ",      "ΐ");
    ins("ό",      "ό");
    ins("ύ",      "ύ");
    ins("ΰ",      "ΰ");
    ins("ώ",      "ώ");

    ins("Ά",      "Ά");
    ins("Έ",      "Έ");
    ins("Ή",      "Ή");
    ins("Ί",      "Ί");
    ins("Ό",      "Ό");
    ins("Ύ",      "Ύ");
    ins("Ώ",      "Ώ");

    //convert oceia to tonos

    ins("ά",      "ά");
    ins("έ",      "έ");
    ins("ή",      "ή");
    ins("ί",      "ί");
    ins("ΐ",      "ΐ");
    ins("ό",      "ό");
    ins("ύ",      "ύ");
    ins("ΰ",      "ΰ");
    ins("ώ",      "ώ");

    ins("Ά",      "Ά");
    ins("Έ",      "Έ");
    ins("Ή",      "Ή");
    ins("Ί",      "Ί");
    ins("Ό",      "Ό");
    ins("Ύ",      "Ύ");
    ins("Ώ",      "Ώ");
#else
    ins("\x61", "\xce\xb1");
    ins("\x62", "\xce\xb2");
    ins("\x76", "\xce\xb2");
    ins("\x67", "\xce\xb3");
    ins("\x64", "\xce\xb4");
    ins("\x65", "\xce\xb5");
    ins("\x7a", "\xce\xb6");
    ins("\x68", "\xce\xb7");
    ins("\x71", "\xce\xb8");
    ins("\x38", "\xce\xb8");
    ins("\x69", "\xce\xb9");
    ins("\x6b", "\xce\xba");
    ins("\x6c", "\xce\xbb");
    ins("\x6d", "\xce\xbc");
    ins("\x6e", "\xce\xbd");
    ins("\x63", "\xce\xbe");
    ins("\x33", "\xce\xbe");
    ins("\x6b\x73", "\xce\xbe");
    ins("\x6f", "\xce\xbf");
    ins("\x70", "\xcf\x80");
    ins("\x72", "\xcf\x81");
    ins("\x73", "\xcf\x83");
    ins("\x73\x31", "\xcf\x83");
    ins("\x6a", "\xcf\x82");
    ins("\x73\x32", "\xcf\x82");
    ins("\x73\x5c\x6e", "\xcf\x82");
    ins("\x74", "\xcf\x84");
    ins("\x75", "\xcf\x85");
    ins("\x66", "\xcf\x86");
    ins("\x78", "\xcf\x87");
    ins("\x79", "\xcf\x88");
    ins("\x70\x73", "\xcf\x88");
    ins("\x77", "\xcf\x89");
    ins("\x41", "\xce\x91");
    ins("\x42", "\xce\x92");
    ins("\x56", "\xce\x92");
    ins("\x47", "\xce\x93");
    ins("\x44", "\xce\x94");
    ins("\x45", "\xce\x95");
    ins("\x5a", "\xce\x96");
    ins("\x48", "\xce\x97");
    ins("\x51", "\xce\x98");
    ins("\x49", "\xce\x99");
    ins("\x4b", "\xce\x9a");
    ins("\x4c", "\xce\x9b");
    ins("\x4d", "\xce\x9c");
    ins("\x4e", "\xce\x9d");
    ins("\x43", "\xce\x9e");
    ins("\x4b\x53", "\xce\x9e");
    ins("\x4b\x73", "\xce\x9e");
    ins("\x4f", "\xce\x9f");
    ins("\x50", "\xce\xa0");
    ins("\x52", "\xce\xa4");
    ins("\x53", "\xce\xa3");
    ins("\x53\x31", "\xce\xa3");
    ins("\x4a", "\xce\xa3");
    ins("\x53\x32", "\xce\xa3");
    ins("\x53\x5c\x6e", "\xce\xa3");
    ins("\x54", "\xce\xa4");
    ins("\x55", "\xce\xa5");
    ins("\x46", "\xce\xa6");
    ins("\x58", "\xce\xa7");
    ins("\x59", "\xce\xa8");
    ins("\x50\x53", "\xce\xa8");
    ins("\x50\x73", "\xce\xa8");
    ins("\x57", "\xce\xa9");
    ins("\x27\x61", "\xce\xac");
    ins("\x27\x65", "\xce\xad");
    ins("\x27\x68", "\xce\xae");
    ins("\x27\x69", "\xce\xaf");
    ins("\x22\x69", "\xcf\x8a");
    ins("\x22\x27\x69", "\xce\x90");
    ins("\x27\x22\x69", "\xce\x90");
    ins("\x27\x6f", "\xcf\x8c");
    ins("\x27\x75", "\xcf\x8d");
    ins("\x22\x75", "\xcf\x8b");
    ins("\x22\x27\x75", "\xce\xb0");
    ins("\x27\x22\x75", "\xce\xb0");
    ins("\x27\x77", "\xcf\x8e");
    ins("\x27\x41", "\xce\x86");
    ins("\x27\x45", "\xce\x88");
    ins("\x27\x48", "\xce\x89");
    ins("\x27\x49", "\xce\x8a");
    ins("\x22\x49", "\xce\xaa");
    ins("\x27\x4f", "\xce\x8c");
    ins("\x27\x55", "\xce\x8e");
    ins("\x22\x55", "\xce\xab");
    ins("\x27\x57", "\xce\x8f");
    ins("\x2a\x29\x41", "\xe1\xbc\x88");
    ins("\x2a\x28\x41", "\xe1\xbc\x89");
    ins("\x2a\x5c\x41", "\xe1\xbe\xba");
    ins("\x2a\x2f\x41", "\xe1\xbe\xbb");
    ins("\x2a\x29\x5c\x41", "\xe1\xbc\x8a");
    ins("\x2a\x28\x5c\x41", "\xe1\xbc\x8b");
    ins("\x2a\x29\x2f\x41", "\xe1\xbc\x8c");
    ins("\x2a\x28\x2f\x41", "\xe1\xbc\x8d");
    ins("\x2a\x29\x45", "\xe1\xbc\x98");
    ins("\x2a\x28\x45", "\xe1\xbc\x99");
    ins("\x2a\x5c\x45", "\xe1\xbf\x88");
    ins("\x2a\x2f\x45", "\xe1\xbf\x89");
    ins("\x2a\x29\x5c\x45", "\xe1\xbc\x9a");
    ins("\x2a\x28\x5c\x45", "\xe1\xbc\x9b");
    ins("\x2a\x29\x2f\x45", "\xe1\xbc\x9c");
    ins("\x2a\x28\x2f\x45", "\xe1\xbc\x9d");
    ins("\x2a\x29\x48", "\xe1\xbc\xa8");
    ins("\x2a\x28\x48", "\xe1\xbc\xa9");
    ins("\x2a\x5c\x48", "\xe1\xbf\x8a");
    ins("\x2a\x2f\x48", "\xe1\xbf\x8b");
    ins("\x2a\x29\x5c\x48", "\xe1\xbc\xaa");
    ins("\x2a\x28\x5c\x48", "\xe1\xbc\xab");
    ins("\x2a\x29\x2f\x48", "\xe1\xbc\xac");
    ins("\x2a\x28\x2f\x48", "\xe1\xbc\xad");
    ins("\x2a\x29\x49", "\xe1\xbc\xb8");
    ins("\x2a\x28\x49", "\xe1\xbc\xb9");
    ins("\x2a\x5c\x49", "\xe1\xbf\x9a");
    ins("\x2a\x2f\x49", "\xe1\xbf\x9b");
    ins("\x2a\x29\x5c\x49", "\xe1\xbc\xba");
    ins("\x2a\x28\x5c\x49", "\xe1\xbc\xbb");
    ins("\x2a\x29\x2f\x49", "\xe1\xbc\xbc");
    ins("\x2a\x28\x2f\x49", "\xe1\xbc\xbd");
    ins("\x2a\x29\x4f", "\xe1\xbd\x88");
    ins("\x2a\x28\x4f", "\xe1\xbd\x89");
    ins("\x2a\x5c\x4f", "\xe1\xbf\xb8");
    ins("\x2a\x2f\x4f", "\xe1\xbf\xb9");
    ins("\x2a\x29\x5c\x4f", "\xe1\xbd\x8a");
    ins("\x2a\x28\x5c\x4f", "\xe1\xbd\x8b");
    ins("\x2a\x29\x2f\x4f", "\xe1\xbd\x8c");
    ins("\x2a\x28\x2f\x4f", "\xe1\xbd\x8d");
    ins("\x2a\x28\x52", "\xe1\xbf\xac");
    ins("\x2a\x28\x55", "\xe1\xbd\x99");
    ins("\x2a\x5c\x55", "\xe1\xbf\xaa");
    ins("\x2a\x2f\x55", "\xe1\xbf\xab");
    ins("\x2a\x28\x5c\x55", "\xe1\xbd\x9b");
    ins("\x2a\x28\x2f\x55", "\xe1\xbd\x9d");
    ins("\x2a\x29\x57", "\xe1\xbd\xa8");
    ins("\x2a\x28\x57", "\xe1\xbd\xa9");
    ins("\x2a\x5c\x57", "\xe1\xbf\xba");
    ins("\x2a\x2f\x57", "\xe1\xbf\xbb");
    ins("\x2a\x29\x5c\x57", "\xe1\xbd\xaa");
    ins("\x2a\x28\x5c\x57", "\xe1\xbd\xab");
    ins("\x2a\x29\x2f\x57", "\xe1\xbd\xac");
    ins("\x2a\x28\x2f\x57", "\xe1\xbd\xad");
    ins("\x2a\x29\x3d\x41", "\xe1\xbc\x8e");
    ins("\x2a\x28\x3d\x41", "\xe1\xbc\x8f");
    ins("\x2a\x29\x3d\x48", "\xe1\xbc\xae");
    ins("\x2a\x28\x3d\x48", "\xe1\xbc\xaf");
    ins("\x2a\x29\x3d\x49", "\xe1\xbc\xbe");
    ins("\x2a\x28\x3d\x49", "\xe1\xbc\xbf");
    ins("\x2a\x28\x3d\x55", "\xe1\xbd\x9f");
    ins("\x2a\x29\x3d\x57", "\xe1\xbd\xae");
    ins("\x2a\x28\x3d\x57", "\xe1\xbd\xaf");
    ins("\x2a\x41\x7c", "\xe1\xbe\xbc");
    ins("\x2a\x29\x41\x7c", "\xe1\xbe\x88");
    ins("\x2a\x28\x41\x7c", "\xe1\xbe\x89");
    ins("\x2a\x29\x2f\x41\x7c", "\xe1\xbe\x8c");
    ins("\x2a\x28\x2f\x41\x7c", "\xe1\xbe\x8d");
    ins("\x2a\x29\x3d\x41\x7c", "\xe1\xbe\x8e");
    ins("\x2a\x28\x3d\x41\x7c", "\xe1\xbe\x8f");
    ins("\x2a\x48\x7c", "\xe1\xbf\x8c");
    ins("\x2a\x29\x48\x7c", "\xe1\xbe\x98");
    ins("\x2a\x28\x48\x7c", "\xe1\xbe\x99");
    ins("\x2a\x29\x2f\x48\x7c", "\xe1\xbe\x9c");
    ins("\x2a\x28\x2f\x48\x7c", "\xe1\xbe\x9d");
    ins("\x2a\x29\x3d\x48\x7c", "\xe1\xbe\x9e");
    ins("\x2a\x28\x3d\x48\x7c", "\xe1\xbe\x9f");
    ins("\x2a\x57\x7c", "\xe1\xbf\xbc");
    ins("\x2a\x29\x57\x7c", "\xe1\xbe\xa8");
    ins("\x2a\x28\x57\x7c", "\xe1\xbe\xa9");
    ins("\x2a\x29\x2f\x57\x7c", "\xe1\xbe\xac");
    ins("\x2a\x28\x2f\x57\x7c", "\xe1\xbe\xad");
    ins("\x2a\x29\x3d\x57\x7c", "\xe1\xbe\xae");
    ins("\x2a\x28\x3d\x57\x7c", "\xe1\xbe\xaf");
    ins("\x2a\x7c\x41", "\xe1\xbe\xbc");
    ins("\x2a\x29\x7c\x41", "\xe1\xbe\x88");
    ins("\x2a\x28\x7c\x41", "\xe1\xbe\x89");
    ins("\x2a\x29\x2f\x7c\x41", "\xe1\xbe\x8c");
    ins("\x2a\x28\x2f\x7c\x41", "\xe1\xbe\x8d");
    ins("\x2a\x29\x3d\x7c\x41", "\xe1\xbe\x8e");
    ins("\x2a\x28\x3d\x7c\x41", "\xe1\xbe\x8f");
    ins("\x2a\x7c\x48", "\xe1\xbf\x8c");
    ins("\x2a\x29\x7c\x48", "\xe1\xbe\x98");
    ins("\x2a\x28\x7c\x48", "\xe1\xbe\x99");
    ins("\x2a\x29\x2f\x7c\x48", "\xe1\xbe\x9c");
    ins("\x2a\x28\x2f\x7c\x48", "\xe1\xbe\x9d");
    ins("\x2a\x29\x3d\x7c\x48", "\xe1\xbe\x9e");
    ins("\x2a\x28\x3d\x7c\x48", "\xe1\xbe\x9f");
    ins("\x2a\x7c\x57", "\xe1\xbf\xbc");
    ins("\x2a\x29\x7c\x57", "\xe1\xbe\xa8");
    ins("\x2a\x28\x7c\x57", "\xe1\xbe\xa9");
    ins("\x2a\x29\x2f\x7c\x57", "\xe1\xbe\xac");
    ins("\x2a\x28\x2f\x7c\x57", "\xe1\xbe\xad");
    ins("\x2a\x29\x3d\x7c\x57", "\xe1\xbe\xae");
    ins("\x2a\x28\x3d\x7c\x57", "\xe1\xbe\xaf");
    ins("\x2a\x2b\x49", "\xce\xaa");
    ins("\x2a\x2b\x55", "\xce\xab");
    ins("\x2a\x29\x61", "\xe1\xbc\x88");
    ins("\x2a\x28\x61", "\xe1\xbc\x89");
    ins("\x2a\x5c\x61", "\xe1\xbe\xba");
    ins("\x2a\x2f\x61", "\xe1\xbe\xbb");
    ins("\x2a\x29\x5c\x61", "\xe1\xbc\x8a");
    ins("\x2a\x28\x5c\x61", "\xe1\xbc\x8b");
    ins("\x2a\x29\x2f\x61", "\xe1\xbc\x8c");
    ins("\x2a\x28\x2f\x61", "\xe1\xbc\x8d");
    ins("\x2a\x29\x65", "\xe1\xbc\x98");
    ins("\x2a\x28\x65", "\xe1\xbc\x99");
    ins("\x2a\x5c\x65", "\xe1\xbf\x88");
    ins("\x2a\x2f\x65", "\xe1\xbf\x89");
    ins("\x2a\x29\x5c\x65", "\xe1\xbc\x9a");
    ins("\x2a\x28\x5c\x65", "\xe1\xbc\x9b");
    ins("\x2a\x29\x2f\x65", "\xe1\xbc\x9c");
    ins("\x2a\x28\x2f\x65", "\xe1\xbc\x9d");
    ins("\x2a\x29\x68", "\xe1\xbc\xa8");
    ins("\x2a\x28\x68", "\xe1\xbc\xa9");
    ins("\x2a\x5c\x68", "\xe1\xbf\x8a");
    ins("\x2a\x2f\x68", "\xe1\xbf\x8b");
    ins("\x2a\x29\x5c\x68", "\xe1\xbc\xaa");
    ins("\x2a\x28\x5c\x68", "\xe1\xbc\xab");
    ins("\x2a\x29\x2f\x68", "\xe1\xbc\xac");
    ins("\x2a\x28\x2f\x68", "\xe1\xbc\xad");
    ins("\x2a\x29\x69", "\xe1\xbc\xb8");
    ins("\x2a\x28\x69", "\xe1\xbc\xb9");
    ins("\x2a\x5c\x69", "\xe1\xbf\x9a");
    ins("\x2a\x2f\x69", "\xe1\xbf\x9b");
    ins("\x2a\x29\x5c\x69", "\xe1\xbc\xba");
    ins("\x2a\x28\x5c\x69", "\xe1\xbc\xbb");
    ins("\x2a\x29\x2f\x69", "\xe1\xbc\xbc");
    ins("\x2a\x28\x2f\x69", "\xe1\xbc\xbd");
    ins("\x2a\x29\x6f", "\xe1\xbd\x88");
    ins("\x2a\x28\x6f", "\xe1\xbd\x89");
    ins("\x2a\x5c\x6f", "\xe1\xbf\xb8");
    ins("\x2a\x2f\x6f", "\xe1\xbf\xb9");
    ins("\x2a\x29\x5c\x6f", "\xe1\xbd\x8a");
    ins("\x2a\x28\x5c\x6f", "\xe1\xbd\x8b");
    ins("\x2a\x29\x2f\x6f", "\xe1\xbd\x8c");
    ins("\x2a\x28\x2f\x6f", "\xe1\xbd\x8d");
    ins("\x2a\x28\x72", "\xe1\xbf\xac");
    ins("\x2a\x28\x75", "\xe1\xbd\x99");
    ins("\x2a\x5c\x75", "\xe1\xbf\xaa");
    ins("\x2a\x2f\x75", "\xe1\xbf\xab");
    ins("\x2a\x28\x5c\x75", "\xe1\xbd\x9b");
    ins("\x2a\x28\x2f\x75", "\xe1\xbd\x9d");
    ins("\x2a\x29\x77", "\xe1\xbd\xa8");
    ins("\x2a\x28\x77", "\xe1\xbd\xa9");
    ins("\x2a\x5c\x77", "\xe1\xbf\xba");
    ins("\x2a\x2f\x77", "\xe1\xbf\xbb");
    ins("\x2a\x29\x5c\x77", "\xe1\xbd\xaa");
    ins("\x2a\x28\x5c\x77", "\xe1\xbd\xab");
    ins("\x2a\x29\x2f\x77", "\xe1\xbd\xac");
    ins("\x2a\x28\x2f\x77", "\xe1\xbd\xad");
    ins("\x2a\x29\x3d\x61", "\xe1\xbc\x8e");
    ins("\x2a\x28\x3d\x61", "\xe1\xbc\x8f");
    ins("\x2a\x29\x3d\x68", "\xe1\xbc\xae");
    ins("\x2a\x28\x3d\x68", "\xe1\xbc\xaf");
    ins("\x2a\x29\x3d\x69", "\xe1\xbc\xbe");
    ins("\x2a\x28\x3d\x69", "\xe1\xbc\xbf");
    ins("\x2a\x28\x3d\x75", "\xe1\xbd\x9f");
    ins("\x2a\x29\x3d\x77", "\xe1\xbd\xae");
    ins("\x2a\x28\x3d\x77", "\xe1\xbd\xaf");
    ins("\x2a\x61\x7c", "\xe1\xbe\xbc");
    ins("\x2a\x29\x61\x7c", "\xe1\xbe\x88");
    ins("\x2a\x28\x61\x7c", "\xe1\xbe\x89");
    ins("\x2a\x29\x2f\x61\x7c", "\xe1\xbe\x8c");
    ins("\x2a\x28\x2f\x61\x7c", "\xe1\xbe\x8d");
    ins("\x2a\x29\x3d\x61\x7c", "\xe1\xbe\x8e");
    ins("\x2a\x28\x3d\x61\x7c", "\xe1\xbe\x8f");
    ins("\x2a\x68\x7c", "\xe1\xbf\x8c");
    ins("\x2a\x29\x68\x7c", "\xe1\xbe\x98");
    ins("\x2a\x28\x68\x7c", "\xe1\xbe\x99");
    ins("\x2a\x29\x2f\x68\x7c", "\xe1\xbe\x9c");
    ins("\x2a\x28\x2f\x68\x7c", "\xe1\xbe\x9d");
    ins("\x2a\x29\x3d\x68\x7c", "\xe1\xbe\x9e");
    ins("\x2a\x28\x3d\x68\x7c", "\xe1\xbe\x9f");
    ins("\x2a\x77\x7c", "\xe1\xbf\xbc");
    ins("\x2a\x29\x77\x7c", "\xe1\xbe\xa8");
    ins("\x2a\x28\x77\x7c", "\xe1\xbe\xa9");
    ins("\x2a\x29\x2f\x77\x7c", "\xe1\xbe\xac");
    ins("\x2a\x28\x2f\x77\x7c", "\xe1\xbe\xad");
    ins("\x2a\x29\x3d\x77\x7c", "\xe1\xbe\xae");
    ins("\x2a\x28\x3d\x77\x7c", "\xe1\xbe\xaf");
    ins("\x2a\x7c\x61", "\xe1\xbe\xbc");
    ins("\x2a\x29\x7c\x61", "\xe1\xbe\x88");
    ins("\x2a\x28\x7c\x61", "\xe1\xbe\x89");
    ins("\x2a\x29\x2f\x7c\x61", "\xe1\xbe\x8c");
    ins("\x2a\x28\x2f\x7c\x61", "\xe1\xbe\x8d");
    ins("\x2a\x29\x3d\x7c\x61", "\xe1\xbe\x8e");
    ins("\x2a\x28\x3d\x7c\x61", "\xe1\xbe\x8f");
    ins("\x2a\x7c\x68", "\xe1\xbf\x8c");
    ins("\x2a\x29\x7c\x68", "\xe1\xbe\x98");
    ins("\x2a\x28\x7c\x68", "\xe1\xbe\x99");
    ins("\x2a\x29\x2f\x7c\x68", "\xe1\xbe\x9c");
    ins("\x2a\x28\x2f\x7c\x68", "\xe1\xbe\x9d");
    ins("\x2a\x29\x3d\x7c\x68", "\xe1\xbe\x9e");
    ins("\x2a\x28\x3d\x7c\x68", "\xe1\xbe\x9f");
    ins("\x2a\x7c\x77", "\xe1\xbf\xbc");
    ins("\x2a\x29\x7c\x77", "\xe1\xbe\xa8");
    ins("\x2a\x28\x7c\x77", "\xe1\xbe\xa9");
    ins("\x2a\x29\x2f\x7c\x77", "\xe1\xbe\xac");
    ins("\x2a\x28\x2f\x7c\x77", "\xe1\xbe\xad");
    ins("\x2a\x29\x3d\x7c\x77", "\xe1\xbe\xae");
    ins("\x2a\x28\x3d\x7c\x77", "\xe1\xbe\xaf");
    ins("\x2a\x2b\x69", "\xce\xaa");
    ins("\x2a\x2b\x75", "\xce\xab");
    ins("\x41\x29", "\xe1\xbc\x88");
    ins("\x41\x28", "\xe1\xbc\x89");
    ins("\x41\x5c", "\xe1\xbe\xba");
    ins("\x41\x2f", "\xe1\xbe\xbb");
    ins("\x41\x29\x5c", "\xe1\xbc\x8a");
    ins("\x41\x28\x5c", "\xe1\xbc\x8b");
    ins("\x41\x29\x2f", "\xe1\xbc\x8c");
    ins("\x41\x28\x2f", "\xe1\xbc\x8d");
    ins("\x45\x29", "\xe1\xbc\x98");
    ins("\x45\x28", "\xe1\xbc\x99");
    ins("\x45\x5c", "\xe1\xbf\x88");
    ins("\x45\x2f", "\xe1\xbf\x89");
    ins("\x45\x29\x5c", "\xe1\xbc\x9a");
    ins("\x45\x28\x5c", "\xe1\xbc\x9b");
    ins("\x45\x29\x2f", "\xe1\xbc\x9c");
    ins("\x45\x28\x2f", "\xe1\xbc\x9d");
    ins("\x48\x29", "\xe1\xbc\xa8");
    ins("\x48\x28", "\xe1\xbc\xa9");
    ins("\x48\x5c", "\xe1\xbf\x8a");
    ins("\x48\x2f", "\xe1\xbf\x8b");
    ins("\x48\x29\x5c", "\xe1\xbc\xaa");
    ins("\x48\x28\x5c", "\xe1\xbc\xab");
    ins("\x48\x29\x2f", "\xe1\xbc\xac");
    ins("\x48\x28\x2f", "\xe1\xbc\xad");
    ins("\x49\x29", "\xe1\xbc\xb8");
    ins("\x49\x28", "\xe1\xbc\xb9");
    ins("\x49\x5c", "\xe1\xbf\x9a");
    ins("\x49\x2f", "\xe1\xbf\x9b");
    ins("\x49\x29\x5c", "\xe1\xbc\xba");
    ins("\x49\x28\x5c", "\xe1\xbc\xbb");
    ins("\x49\x29\x2f", "\xe1\xbc\xbc");
    ins("\x49\x28\x2f", "\xe1\xbc\xbd");
    ins("\x4f\x29", "\xe1\xbd\x88");
    ins("\x4f\x28", "\xe1\xbd\x89");
    ins("\x4f\x5c", "\xe1\xbf\xb8");
    ins("\x4f\x2f", "\xe1\xbf\xb9");
    ins("\x4f\x29\x5c", "\xe1\xbd\x8a");
    ins("\x4f\x28\x5c", "\xe1\xbd\x8b");
    ins("\x4f\x29\x2f", "\xe1\xbd\x8c");
    ins("\x4f\x28\x2f", "\xe1\xbd\x8d");
    ins("\x52\x28", "\xe1\xbf\xac");
    ins("\x55\x29", "\xe1\xbd\x90");
    ins("\x55\x28", "\xe1\xbd\x99");
    ins("\x55\x5c", "\xe1\xbf\xaa");
    ins("\x55\x2f", "\xe1\xbf\xab");
    ins("\x55\x29\x5c", "\xe1\xbd\x92");
    ins("\x55\x28\x5c", "\xe1\xbd\x9b");
    ins("\x55\x29\x2f", "\xe1\xbd\x94");
    ins("\x55\x28\x2f", "\xe1\xbd\x9d");
    ins("\x57\x29", "\xe1\xbd\xa8");
    ins("\x57\x28", "\xe1\xbd\xa9");
    ins("\x57\x5c", "\xe1\xbf\xba");
    ins("\x57\x2f", "\xe1\xbf\xbb");
    ins("\x57\x29\x5c", "\xe1\xbd\xaa");
    ins("\x57\x28\x5c", "\xe1\xbd\xab");
    ins("\x57\x29\x2f", "\xe1\xbd\xac");
    ins("\x57\x28\x2f", "\xe1\xbd\xad");
    ins("\x41\x3d", "\xe1\xbe\xb6");
    ins("\x41\x29\x3d", "\xe1\xbc\x8e");
    ins("\x41\x28\x3d", "\xe1\xbc\x8f");
    ins("\x48\x3d", "\xe1\xbf\x86");
    ins("\x48\x29\x3d", "\xe1\xbc\xae");
    ins("\x48\x28\x3d", "\xe1\xbc\xaf");
    ins("\x49\x3d", "\xe1\xbf\x96");
    ins("\x49\x29\x3d", "\xe1\xbc\xbe");
    ins("\x49\x28\x3d", "\xe1\xbc\xbf");
    ins("\x55\x3d", "\xe1\xbf\xa6");
    ins("\x55\x29\x3d", "\xe1\xbd\x96");
    ins("\x55\x28\x3d", "\xe1\xbd\x9f");
    ins("\x57\x3d", "\xe1\xbf\xb6");
    ins("\x57\x29\x3d", "\xe1\xbd\xae");
    ins("\x57\x28\x3d", "\xe1\xbd\xaf");
    ins("\x41\x7c", "\xe1\xbe\xbc");
    ins("\x41\x29\x7c", "\xe1\xbe\x88");
    ins("\x41\x28\x7c", "\xe1\xbe\x89");
    ins("\x41\x2f\x7c", "\xe1\xbe\xb4");
    ins("\x41\x29\x2f\x7c", "\xe1\xbe\x8c");
    ins("\x41\x28\x2f\x7c", "\xe1\xbe\x8d");
    ins("\x41\x3d\x7c", "\xe1\xbe\xb7");
    ins("\x41\x29\x3d\x7c", "\xe1\xbe\x8e");
    ins("\x41\x28\x3d\x7c", "\xe1\xbe\x8f");
    ins("\x48\x7c", "\xe1\xbf\x8c");
    ins("\x48\x29\x7c", "\xe1\xbe\x98");
    ins("\x48\x28\x7c", "\xe1\xbe\x99");
    ins("\x48\x2f\x7c", "\xe1\xbf\x84");
    ins("\x48\x29\x2f\x7c", "\xe1\xbe\x9c");
    ins("\x48\x28\x2f\x7c", "\xe1\xbe\x9d");
    ins("\x48\x3d\x7c", "\xe1\xbf\x87");
    ins("\x48\x29\x3d\x7c", "\xe1\xbe\x9e");
    ins("\x48\x28\x3d\x7c", "\xe1\xbe\x9f");
    ins("\x57\x7c", "\xe1\xbf\xbc");
    ins("\x57\x29\x7c", "\xe1\xbe\xa8");
    ins("\x57\x28\x7c", "\xe1\xbe\xa9");
    ins("\x57\x2f\x7c", "\xe1\xbf\xb4");
    ins("\x57\x29\x2f\x7c", "\xe1\xbe\xac");
    ins("\x57\x28\x2f\x7c", "\xe1\xbe\xad");
    ins("\x57\x3d\x7c", "\xe1\xbf\xb7");
    ins("\x57\x29\x3d\x7c", "\xe1\xbe\xae");
    ins("\x57\x28\x3d\x7c", "\xe1\xbe\xaf");
    ins("\x49\x2b", "\xce\xaa");
    ins("\x49\x2b\x5c", "\xe1\xbf\x92");
    ins("\x49\x2b\x2f", "\xe1\xbf\x93");
    ins("\x55\x2b", "\xce\xab");
    ins("\x55\x2b\x5c", "\xe1\xbf\xa2");
    ins("\x55\x2b\x2f", "\xe1\xbf\xa3");
    ins("\x61\x29", "\xe1\xbc\x80");
    ins("\x61\x28", "\xe1\xbc\x81");
    ins("\x61\x5c", "\xe1\xbd\xb0");
    ins("\x61\x2f", "\xe1\xbd\xb1");
    ins("\x61\x29\x5c", "\xe1\xbc\x82");
    ins("\x61\x28\x5c", "\xe1\xbc\x83");
    ins("\x61\x29\x2f", "\xe1\xbc\x84");
    ins("\x61\x28\x2f", "\xe1\xbc\x85");
    ins("\x65\x29", "\xe1\xbc\x90");
    ins("\x65\x28", "\xe1\xbc\x91");
    ins("\x65\x5c", "\xe1\xbd\xb2");
    ins("\x65\x2f", "\xe1\xbd\xb3");
    ins("\x65\x29\x5c", "\xe1\xbc\x92");
    ins("\x65\x28\x5c", "\xe1\xbc\x93");
    ins("\x65\x29\x2f", "\xe1\xbc\x94");
    ins("\x65\x28\x2f", "\xe1\xbc\x95");
    ins("\x68\x29", "\xe1\xbc\xa0");
    ins("\x68\x28", "\xe1\xbc\xa1");
    ins("\x68\x5c", "\xe1\xbd\xb4");
    ins("\x68\x2f", "\xe1\xbd\xb5");
    ins("\x68\x29\x5c", "\xe1\xbc\xa2");
    ins("\x68\x28\x5c", "\xe1\xbc\xa3");
    ins("\x68\x29\x2f", "\xe1\xbc\xa4");
    ins("\x68\x28\x2f", "\xe1\xbc\xa5");
    ins("\x69\x29", "\xe1\xbc\xb0");
    ins("\x69\x28", "\xe1\xbc\xb1");
    ins("\x69\x5c", "\xe1\xbd\xb6");
    ins("\x69\x2f", "\xe1\xbd\xb7");
    ins("\x69\x29\x5c", "\xe1\xbc\xb2");
    ins("\x69\x28\x5c", "\xe1\xbc\xb3");
    ins("\x69\x29\x2f", "\xe1\xbc\xb4");
    ins("\x69\x28\x2f", "\xe1\xbc\xb5");
    ins("\x6f\x29", "\xe1\xbd\x80");
    ins("\x6f\x28", "\xe1\xbd\x81");
    ins("\x6f\x5c", "\xe1\xbd\xb8");
    ins("\x6f\x2f", "\xe1\xbd\xb9");
    ins("\x6f\x29\x5c", "\xe1\xbd\x82");
    ins("\x6f\x28\x5c", "\xe1\xbd\x83");
    ins("\x6f\x29\x2f", "\xe1\xbd\x84");
    ins("\x6f\x28\x2f", "\xe1\xbd\x85");
    ins("\x72\x28", "\xe1\xbf\xa4");
    ins("\x75\x29", "\xe1\xbd\x90");
    ins("\x75\x28", "\xe1\xbd\x91");
    ins("\x75\x5c", "\xe1\xbd\xba");
    ins("\x75\x2f", "\xe1\xbd\xbb");
    ins("\x75\x29\x5c", "\xe1\xbd\x92");
    ins("\x75\x28\x5c", "\xe1\xbd\x93");
    ins("\x75\x29\x2f", "\xe1\xbd\x94");
    ins("\x75\x28\x2f", "\xe1\xbd\x95");
    ins("\x77\x29", "\xe1\xbd\xa0");
    ins("\x77\x28", "\xe1\xbd\xa1");
    ins("\x77\x5c", "\xe1\xbd\xbc");
    ins("\x77\x2f", "\xe1\xbd\xbd");
    ins("\x77\x29\x5c", "\xe1\xbd\xa2");
    ins("\x77\x28\x5c", "\xe1\xbd\xa3");
    ins("\x77\x29\x2f", "\xe1\xbd\xa4");
    ins("\x77\x28\x2f", "\xe1\xbd\xa5");
    ins("\x61\x3d", "\xe1\xbe\xb6");
    ins("\x61\x29\x3d", "\xe1\xbc\x86");
    ins("\x61\x28\x3d", "\xe1\xbc\x87");
    ins("\x68\x3d", "\xe1\xbf\x86");
    ins("\x68\x29\x3d", "\xe1\xbc\xa6");
    ins("\x68\x28\x3d", "\xe1\xbc\xa7");
    ins("\x69\x3d", "\xe1\xbf\x96");
    ins("\x69\x29\x3d", "\xe1\xbc\xb6");
    ins("\x69\x28\x3d", "\xe1\xbc\xb7");
    ins("\x75\x3d", "\xe1\xbf\xa6");
    ins("\x75\x29\x3d", "\xe1\xbd\x96");
    ins("\x75\x28\x3d", "\xe1\xbd\x97");
    ins("\x77\x3d", "\xe1\xbf\xb6");
    ins("\x77\x29\x3d", "\xe1\xbd\xa6");
    ins("\x77\x28\x3d", "\xe1\xbd\xa7");
    ins("\x61\x7c", "\xe1\xbe\xb3");
    ins("\x61\x29\x7c", "\xe1\xbe\x80");
    ins("\x61\x28\x7c", "\xe1\xbe\x81");
    ins("\x61\x2f\x7c", "\xe1\xbe\xb4");
    ins("\x61\x29\x2f\x7c", "\xe1\xbe\x84");
    ins("\x61\x28\x2f\x7c", "\xe1\xbe\x85");
    ins("\x61\x3d\x7c", "\xe1\xbe\xb7");
    ins("\x61\x29\x3d\x7c", "\xe1\xbe\x86");
    ins("\x61\x28\x3d\x7c", "\xe1\xbe\x87");
    ins("\x68\x7c", "\xe1\xbf\x83");
    ins("\x68\x29\x7c", "\xe1\xbe\x90");
    ins("\x68\x28\x7c", "\xe1\xbe\x91");
    ins("\x68\x2f\x7c", "\xe1\xbf\x84");
    ins("\x68\x29\x2f\x7c", "\xe1\xbe\x94");
    ins("\x68\x28\x2f\x7c", "\xe1\xbe\x95");
    ins("\x68\x3d\x7c", "\xe1\xbf\x87");
    ins("\x68\x29\x3d\x7c", "\xe1\xbe\x96");
    ins("\x68\x28\x3d\x7c", "\xe1\xbe\x97");
    ins("\x77\x7c", "\xe1\xbf\xb3");
    ins("\x77\x29\x7c", "\xe1\xbe\xa0");
    ins("\x77\x28\x7c", "\xe1\xbe\xa1");
    ins("\x77\x2f\x7c", "\xe1\xbf\xb4");
    ins("\x77\x29\x2f\x7c", "\xe1\xbe\xa4");
    ins("\x77\x28\x2f\x7c", "\xe1\xbe\xa5");
    ins("\x77\x3d\x7c", "\xe1\xbf\xb7");
    ins("\x77\x29\x3d\x7c", "\xe1\xbe\xa6");
    ins("\x77\x28\x3d\x7c", "\xe1\xbe\xa7");
    ins("\x69\x2b", "\xcf\x8a");
    ins("\x69\x2b\x5c", "\xe1\xbf\x92");
    ins("\x69\x2b\x2f", "\xe1\xbf\x93");
    ins("\x75\x2b", "\xcf\x8b");
    ins("\x75\x2b\x5c", "\xe1\xbf\xa2");
    ins("\x75\x2b\x2f", "\xe1\xbf\xa3");
    ins("\x41\x5c\x29", "\xe1\xbc\x8a");
    ins("\x41\x5c\x28", "\xe1\xbc\x8b");
    ins("\x41\x2f\x29", "\xe1\xbc\x8c");
    ins("\x41\x2f\x28", "\xe1\xbc\x8d");
    ins("\x45\x5c\x29", "\xe1\xbc\x9a");
    ins("\x45\x5c\x28", "\xe1\xbc\x9b");
    ins("\x45\x2f\x29", "\xe1\xbc\x9c");
    ins("\x45\x2f\x28", "\xe1\xbc\x9d");
    ins("\x48\x5c\x29", "\xe1\xbc\xaa");
    ins("\x48\x5c\x28", "\xe1\xbc\xab");
    ins("\x48\x2f\x29", "\xe1\xbc\xac");
    ins("\x48\x2f\x28", "\xe1\xbc\xad");
    ins("\x49\x5c\x29", "\xe1\xbc\xba");
    ins("\x49\x5c\x28", "\xe1\xbc\xbb");
    ins("\x49\x2f\x29", "\xe1\xbc\xbc");
    ins("\x49\x2f\x28", "\xe1\xbc\xbd");
    ins("\x4f\x5c\x29", "\xe1\xbd\x8a");
    ins("\x4f\x5c\x28", "\xe1\xbd\x8b");
    ins("\x4f\x2f\x29", "\xe1\xbd\x8c");
    ins("\x4f\x2f\x28", "\xe1\xbd\x8d");
    ins("\x55\x5c\x28", "\xe1\xbd\x9b");
    ins("\x55\x2f\x28", "\xe1\xbd\x9d");
    ins("\x57\x5c\x29", "\xe1\xbd\xaa");
    ins("\x57\x5c\x28", "\xe1\xbd\xab");
    ins("\x57\x2f\x29", "\xe1\xbd\xac");
    ins("\x57\x2f\x28", "\xe1\xbd\xad");
    ins("\x41\x3d\x29", "\xe1\xbc\x8e");
    ins("\x41\x3d\x28", "\xe1\xbc\x8f");
    ins("\x48\x3d\x29", "\xe1\xbc\xae");
    ins("\x48\x3d\x28", "\xe1\xbc\xaf");
    ins("\x49\x3d\x29", "\xe1\xbc\xbe");
    ins("\x49\x3d\x28", "\xe1\xbc\xbf");
    ins("\x55\x3d\x28", "\xe1\xbd\x9f");
    ins("\x57\x3d\x29", "\xe1\xbd\xae");
    ins("\x57\x3d\x28", "\xe1\xbd\xaf");
    ins("\x41\x2f\x29\x7c", "\xe1\xbe\x8c");
    ins("\x41\x2f\x28\x7c", "\xe1\xbe\x8d");
    ins("\x41\x3d\x29\x7c", "\xe1\xbe\x8e");
    ins("\x41\x3d\x28\x7c", "\xe1\xbe\x8f");
    ins("\x48\x2f\x29\x7c", "\xe1\xbe\x9c");
    ins("\x48\x2f\x28\x7c", "\xe1\xbe\x9d");
    ins("\x48\x3d\x29\x7c", "\xe1\xbe\x9e");
    ins("\x48\x3d\x28\x7c", "\xe1\xbe\x9f");
    ins("\x57\x2f\x29\x7c", "\xe1\xbe\xac");
    ins("\x57\x2f\x28\x7c", "\xe1\xbe\xad");
    ins("\x57\x3d\x29\x7c", "\xe1\xbe\xae");
    ins("\x57\x3d\x28\x7c", "\xe1\xbe\xaf");
    ins("\x61\x5c\x29", "\xe1\xbc\x82");
    ins("\x61\x5c\x28", "\xe1\xbc\x83");
    ins("\x61\x2f\x29", "\xe1\xbc\x84");
    ins("\x61\x2f\x28", "\xe1\xbc\x85");
    ins("\x65\x5c\x29", "\xe1\xbc\x92");
    ins("\x65\x5c\x28", "\xe1\xbc\x93");
    ins("\x65\x2f\x29", "\xe1\xbc\x94");
    ins("\x65\x2f\x28", "\xe1\xbc\x95");
    ins("\x68\x5c\x29", "\xe1\xbc\xa2");
    ins("\x68\x5c\x28", "\xe1\xbc\xa3");
    ins("\x68\x2f\x29", "\xe1\xbc\xa4");
    ins("\x68\x2f\x28", "\xe1\xbc\xa5");
    ins("\x69\x5c\x29", "\xe1\xbc\xb2");
    ins("\x69\x5c\x28", "\xe1\xbc\xb3");
    ins("\x69\x2f\x29", "\xe1\xbc\xb4");
    ins("\x69\x2f\x28", "\xe1\xbc\xb5");
    ins("\x6f\x5c\x29", "\xe1\xbd\x82");
    ins("\x6f\x5c\x28", "\xe1\xbd\x83");
    ins("\x6f\x2f\x29", "\xe1\xbd\x84");
    ins("\x6f\x2f\x28", "\xe1\xbd\x85");
    ins("\x75\x5c\x29", "\xe1\xbd\x92");
    ins("\x75\x5c\x28", "\xe1\xbd\x93");
    ins("\x75\x2f\x29", "\xe1\xbd\x94");
    ins("\x75\x2f\x28", "\xe1\xbd\x95");
    ins("\x77\x5c\x29", "\xe1\xbd\xa2");
    ins("\x77\x5c\x28", "\xe1\xbd\xa3");
    ins("\x77\x2f\x29", "\xe1\xbd\xa4");
    ins("\x77\x2f\x28", "\xe1\xbd\xa5");
    ins("\x61\x3d\x29", "\xe1\xbc\x86");
    ins("\x61\x3d\x28", "\xe1\xbc\x87");
    ins("\x68\x3d\x29", "\xe1\xbc\xa6");
    ins("\x68\x3d\x28", "\xe1\xbc\xa7");
    ins("\x69\x3d\x29", "\xe1\xbc\xb6");
    ins("\x69\x3d\x28", "\xe1\xbc\xb7");
    ins("\x75\x3d\x29", "\xe1\xbd\x96");
    ins("\x75\x3d\x28", "\xe1\xbd\x97");
    ins("\x77\x3d\x29", "\xe1\xbd\xa6");
    ins("\x77\x3d\x28", "\xe1\xbd\xa7");
    ins("\x61\x2f\x29\x7c", "\xe1\xbe\x84");
    ins("\x61\x2f\x28\x7c", "\xe1\xbe\x85");
    ins("\x61\x3d\x29\x7c", "\xe1\xbe\x86");
    ins("\x61\x3d\x28\x7c", "\xe1\xbe\x87");
    ins("\x68\x2f\x29\x7c", "\xe1\xbe\x94");
    ins("\x68\x2f\x28\x7c", "\xe1\xbe\x95");
    ins("\x68\x3d\x29\x7c", "\xe1\xbe\x96");
    ins("\x68\x3d\x28\x7c", "\xe1\xbe\x97");
    ins("\x77\x2f\x29\x7c", "\xe1\xbe\xa4");
    ins("\x77\x2f\x28\x7c", "\xe1\xbe\xa5");
    ins("\x77\x3d\x29\x7c", "\xe1\xbe\xa6");
    ins("\x77\x3d\x28\x7c", "\xe1\xbe\xa7");
    ins("\x69\x5c\x2b", "\xe1\xbf\x92");
    ins("\x69\x2f\x2b", "\xe1\xbf\x93");
    ins("\x75\x5c\x2b", "\xe1\xbf\xa2");
    ins("\x75\x2f\x2b", "\xe1\xbf\xa3");
    ins("\xce\xac", "\xe1\xbd\xb1");
    ins("\xce\xad", "\xe1\xbd\xb3");
    ins("\xce\xae", "\xe1\xbd\xb5");
    ins("\xce\xaf", "\xe1\xbd\xb7");
    ins("\xce\x90", "\xe1\xbf\x93");
    ins("\xcf\x8c", "\xe1\xbd\xb9");
    ins("\xcf\x8d", "\xe1\xbd\xbb");
    ins("\xce\xb0", "\xe1\xbf\xa3");
    ins("\xcf\x8e", "\xe1\xbd\xbd");
    ins("\xce\x86", "\xe1\xbe\xbb");
    ins("\xce\x88", "\xe1\xbf\x89");
    ins("\xce\x89", "\xe1\xbf\x8b");
    ins("\xce\x8a", "\xe1\xbf\x9b");
    ins("\xce\x8c", "\xe1\xbf\xb9");
    ins("\xce\x8e", "\xe1\xbf\xab");
    ins("\xce\x8f", "\xe1\xbf\xbb");
    ins("\xe1\xbd\xb1", "\xce\xac");
    ins("\xe1\xbd\xb3", "\xce\xad");
    ins("\xe1\xbd\xb5", "\xce\xae");
    ins("\xe1\xbd\xb7", "\xce\xaf");
    ins("\xe1\xbf\x93", "\xce\x90");
    ins("\xe1\xbd\xb9", "\xcf\x8c");
    ins("\xe1\xbd\xbb", "\xcf\x8d");
    ins("\xe1\xbf\xa3", "\xce\xb0");
    ins("\xe1\xbd\xbd", "\xcf\x8e");
    ins("\xe1\xbe\xbb", "\xce\x86");
    ins("\xe1\xbf\x89", "\xce\x88");
    ins("\xe1\xbf\x8b", "\xce\x89");
    ins("\xe1\xbf\x9b", "\xce\x8a");
    ins("\xe1\xbf\xb9", "\xce\x8c");
    ins("\xe1\xbf\xab", "\xce\x8e");
    ins("\xe1\xbf\xbb", "\xce\x8f");
#endif
}

sptr< Dictionary::Class > makeDictionary() THROW_SPEC( std::exception )
{
    static const GreekTable t;

    return sptr< Dictionary::Class >(new Transliteration::TransliterationDictionary( "baa9e37a1aa69cdb5daca14a48ffe5ae",
                                                                                     QCoreApplication::translate( "GreekTranslit", "Greek Transliteration" ).toUtf8().data(),
                                                                                     QIcon( ":/flags/gr.png" ), t ) );
}

}

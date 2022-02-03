/* This file is (c) 2010 Jennie Petoumenou <epetoumenou@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "greektranslit.hh"
#include "transliteration.hh"
#include <QCoreApplication>

namespace GreekTranslit {

class GreekTable: public Transliteration::Table
{
public:

  GreekTable();
};

GreekTable::GreekTable()
{
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


}

sptr< Dictionary::Class > makeDictionary() 
{
  static GreekTable t;

  return new Transliteration::TransliterationDictionary( "baa9e37a1aa69cdb5daca14a48ffe5ae",
                      QCoreApplication::translate( "GreekTranslit", "Greek Transliteration" ).toUtf8().data(),
                      QIcon( ":/flags/gr.png" ), t );
}

}

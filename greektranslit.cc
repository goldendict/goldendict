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

    //ANCIENT GREEK DIACRITICS (BETA CODE)
    // Adapted from beta2unicode.py by James Tauber <http://jtauber.com/>

    //uppercase (asterisk & capitals)
	//oceia - bareia
    ins("*)A",     "Ἀ");
    ins("*(A",     "Ἁ");
    ins("*\\A",     "Ὰ");
    ins("*/A",      "Ά");
    ins("*)\\A",   "Ἂ");
    ins("*(\\A",   "Ἃ");
    ins("*)/A",    "Ἄ");
    ins("*(/A",    "Ἅ");

    ins("*)E",     "Ἐ");
    ins("*(E",     "Ἑ");
    ins("*\\E",     "Ὲ");
    ins("*/E",      "Έ");
    ins("*)\\E",   "Ἒ");
    ins("*(\\E",   "Ἓ");
    ins("*)/E",    "Ἔ");
    ins("*(/E",    "Ἕ");

    ins("*)H",     "Ἠ");
    ins("*(H",     "Ἡ");
    ins("*\\H",     "Ὴ");
    ins("*/H",      "Ή");
    ins("*)\\H",   "Ἢ");
    ins("*(\\H",   "Ἣ");
    ins("*)/H",    "Ἤ");
    ins("*(/H",    "Ἥ");

    ins("*)I",     "Ἰ");
    ins("*(I",     "Ἱ");
    ins("*\\I",     "Ὶ");
    ins("*/I",      "Ί");
    ins("*)\\I",   "Ἲ");
    ins("*(\\I",   "Ἳ");
    ins("*)/I",    "Ἴ");
    ins("*(/I",    "Ἵ");

    ins("*)O",     "Ὀ");
    ins("*(O",     "Ὁ");
    ins("*\\O",     "Ὸ");
    ins("*/O",      "Ό");
    ins("*)\\O",   "Ὂ");
    ins("*(\\O",   "Ὃ");
    ins("*)/O",    "Ὄ");
    ins("*(/O",    "Ὅ");

    ins("*(R",    "Ῥ");	

    ins("*(U",     "Ὑ");
    ins("*\\U",     "Ὺ");
    ins("*/U",      "Ύ");
    ins("*(\\U",   "Ὓ");
    ins("*(/U",    "Ὕ");

    ins("*)W",     "Ὠ");
    ins("*(W",     "Ὡ");
    ins("*\\W",     "Ὼ");
    ins("*/W",      "Ώ");
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

	//dialutika
    ins("*+I",      "Ϊ");
    ins("*+U",      "Ϋ");
    
    //uppercase (asterisk & small letters)
	//oceia - bareia
    ins("*)a",     "Ἀ");
    ins("*(a",     "Ἁ");
    ins("*\\a",     "Ὰ");
    ins("*/a",      "Ά");
    ins("*)\\a",   "Ἂ");
    ins("*(\\a",   "Ἃ");
    ins("*)/a",    "Ἄ");
    ins("*(/a",    "Ἅ");

    ins("*)e",     "Ἐ");
    ins("*(e",     "Ἑ");
    ins("*\\e",     "Ὲ");
    ins("*/e",      "Έ");
    ins("*)\\e",   "Ἒ");
    ins("*(\\e",   "Ἓ");
    ins("*)/e",    "Ἔ");
    ins("*(/e",    "Ἕ");

    ins("*)h",     "Ἠ");
    ins("*(h",     "Ἡ");
    ins("*\\h",     "Ὴ");
    ins("*/h",      "Ή");
    ins("*)\\h",   "Ἢ");
    ins("*(\\h",   "Ἣ");
    ins("*)/h",    "Ἤ");
    ins("*(/h",    "Ἥ");

    ins("*)i",     "Ἰ");
    ins("*(i",     "Ἱ");
    ins("*\\i",     "Ὶ");
    ins("*/i",      "Ί");
    ins("*)\\i",   "Ἲ");
    ins("*(\\i",   "Ἳ");
    ins("*)/i",    "Ἴ");
    ins("*(/i",    "Ἵ");

    ins("*)o",     "Ὀ");
    ins("*(o",     "Ὁ");
    ins("*\\o",     "Ὸ");
    ins("*/o",      "Ό");
    ins("*)\\o",   "Ὂ");
    ins("*(\\o",   "Ὃ");
    ins("*)/o",    "Ὄ");
    ins("*(/o",    "Ὅ");

    ins("*(r",     "Ῥ");	

    ins("*(u",     "Ὑ");
    ins("*\\u",     "Ὺ");
    ins("*/u",      "Ύ");
    ins("*(\\u",   "Ὓ");
    ins("*(/u",    "Ὕ");

    ins("*)w",     "Ὠ");
    ins("*(w",     "Ὡ");
    ins("*\\w",     "Ὼ");
    ins("*/w",      "Ώ");
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

	//dialutika
    ins("*+i",      "Ϊ");
    ins("*+u",      "Ϋ");
    
    //lowercase (capitals)
	//oceia - bareia
    ins("A)",     "ἀ");
    ins("A(",     "ἁ");
    ins("A\\",     "ὰ");
    ins("A/",      "ά");
    ins("A)\\",   "ἂ");
    ins("A(\\",   "ἃ");
    ins("A)/",    "ἄ");
    ins("A(/",    "ἅ");

    ins("E)",     "ἐ");
    ins("E(",     "ἑ");
    ins("E\\",     "ὲ");
    ins("E/",      "έ");
    ins("E)\\",   "ἒ");
    ins("E(\\",   "ἓ");
    ins("E)/",    "ἔ");
    ins("E(/",    "ἕ");

    ins("H)",     "ἠ");
    ins("H(",     "ἡ");
    ins("H\\",     "ὴ");
    ins("H/",      "ή");
    ins("H)\\",   "ἢ");
    ins("H(\\",   "ἣ");
    ins("H)/",    "ἤ");
    ins("H(/",    "ἥ");

    ins("I)",     "ἰ");
    ins("I(",     "ἱ");
    ins("I\\",     "ὶ");
    ins("I/",      "ί");
    ins("I)\\",   "ἲ");
    ins("I(\\",   "ἳ");
    ins("I)/",    "ἴ");
    ins("I(/",    "ἵ");

    ins("O)",     "ὀ");
    ins("O(",     "ὁ");
    ins("O\\",     "ὸ");
    ins("O/",      "ό");
    ins("O)\\",   "ὂ");
    ins("O(\\",   "ὃ");
    ins("O)/",    "ὄ");
    ins("O(/",    "ὅ");

    ins("R(",     "ῤ");

    ins("U)",     "ὐ");
    ins("U(",     "ὑ");
    ins("U\\",     "ὺ");
    ins("U/",      "ύ");
    ins("U)\\",   "ὒ");
    ins("U(\\",   "ὓ");
    ins("U)/",    "ὔ");
    ins("U(/",    "ὕ");

    ins("W)",     "ὠ");
    ins("W(",     "ὡ");
    ins("W\\",     "ὼ");
    ins("W/",      "ώ");
    ins("W)\\",   "ὢ");
    ins("W(\\",   "ὣ");
    ins("W)/",    "ὤ");
    ins("W(/",    "ὥ");

	//perispwmenh
    ins("A=",      "ᾶ");
    ins("A)=",    "ἆ");
    ins("A(=",    "ἇ");
    
    ins("H=",      "ῆ");
    ins("H)=",    "ἦ");
    ins("H(=",    "ἧ");
    
    ins("I=",      "ῖ");
    ins("I)=",    "ἶ");
    ins("I(=",    "ἷ");
    
    ins("U=",      "ῦ");
    ins("U)=",    "ὖ");
    ins("U(=",    "ὗ");
    
    ins("W=",      "ῶ");
    ins("W)=",    "ὦ");
    ins("W(=",    "ὧ");

	//upogegrammenh
    ins("A|",      "ᾳ");
    ins("A)|",    "ᾀ");
    ins("A(|",    "ᾁ");
    ins("A/|",     "ᾴ");
    ins("A)/|",   "ᾄ");
    ins("A(/|",   "ᾅ");
    ins("A=|",     "ᾷ");
    ins("A)=|",   "ᾆ");
    ins("A(=|",   "ᾇ");

    ins("H|",      "ῃ");
    ins("H)|",    "ᾐ");
    ins("H(|",    "ᾑ");
    ins("H/|",     "ῄ");
    ins("H)/|",   "ᾔ");
    ins("H(/|",   "ᾕ");
    ins("H=|",     "ῇ");
    ins("H)=|",   "ᾖ");
    ins("H(=|",   "ᾗ");

    ins("W|",      "ῳ"); 
    ins("W)|",    "ᾠ");
    ins("W(|",    "ᾡ");
    ins("W/|",     "ῴ");
    ins("W)/|",   "ᾤ");
    ins("W(/|",   "ᾥ");
    ins("W=|",     "ῷ");
    ins("W)=|",   "ᾦ");
    ins("W(=|",   "ᾧ");

	//dialutika
    ins("I+",      "ϊ");
    ins("I\\+",    "ῒ");
    ins("I+\\",    "ῒ");
    ins("I/+",     "ΐ");
    ins("I+/",     "ΐ");
    ins("U+",      "ϋ");
    ins("U\\+",    "ῢ");
    ins("U+\\",    "ῢ");
    ins("U/+",     "ΰ");
    ins("U+/",     "ΰ");    

    //lowercase (small letters)
	//oceia - bareia
    ins("a)",     "ἀ");
    ins("a(",     "ἁ");
    ins("a\\",     "ὰ");
    ins("a/",      "ά");
    ins("a)\\",   "ἂ");
    ins("a(\\",   "ἃ");
    ins("a)/",    "ἄ");
    ins("a(/",    "ἅ");

    ins("e)",     "ἐ");
    ins("e(",     "ἑ");
    ins("e\\",     "ὲ");
    ins("e/",      "έ");
    ins("e)\\",   "ἒ");
    ins("e(\\",   "ἓ");
    ins("e)/",    "ἔ");
    ins("e(/",    "ἕ");

    ins("h)",     "ἠ");
    ins("h(",     "ἡ");
    ins("h\\",     "ὴ");
    ins("h/",      "ή");
    ins("h)\\",   "ἢ");
    ins("h(\\",   "ἣ");
    ins("h)/",    "ἤ");
    ins("h(/",    "ἥ");

    ins("i)",     "ἰ");
    ins("i(",     "ἱ");
    ins("i\\",     "ὶ");
    ins("i/",      "ί");
    ins("i)\\",   "ἲ");
    ins("i(\\",   "ἳ");
    ins("i)/",    "ἴ");
    ins("i(/",    "ἵ");

    ins("o)",     "ὀ");
    ins("o(",     "ὁ");
    ins("o\\",     "ὸ");
    ins("o/",      "ό");
    ins("o)\\",   "ὂ");
    ins("o(\\",   "ὃ");
    ins("o)/",    "ὄ");
    ins("o(/",    "ὅ");

    ins("r(",      "ῤ");

    ins("u)",     "ὐ");
    ins("u(",     "ὑ");
    ins("u\\",     "ὺ");
    ins("u/",      "ύ");
    ins("u)\\",   "ὒ");
    ins("u(\\",   "ὓ");
    ins("u)/",    "ὔ");
    ins("u(/",    "ὕ");

    ins("w)",     "ὠ");
    ins("w(",     "ὡ");
    ins("w\\",     "ὼ");
    ins("w/",      "ώ");
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

	//dialutika
    ins("i+",      "ϊ");
    ins("i\\+",    "ῒ");
    ins("i+\\",    "ῒ");
    ins("i/+",     "ΐ");
    ins("i+/",     "ΐ");
    ins("u+",      "ϋ");
    ins("u\\+",    "ῢ");
    ins("u+\\",    "ῢ");
    ins("u/+",     "ΰ");
    ins("u+/",     "ΰ");
    
	//MODERN GREEK DIACRITICS
    
    ins("'a",      "ά");
    ins("'e",      "έ");
    ins("'h",      "ή");
    ins("'i",      "ί");
    ins("\"i",     "ϊ");
    ins("\"'i",    "ΐ");
    ins("':i",     "ΐ");
    ins("'o",      "ό");
    ins("'u",      "ύ");
    ins("\"u",     "ϋ");
    ins("\"'u",    "ΰ");
    ins("':u",     "ΰ");
    ins("'w",      "ώ");

    ins("'A",      "ά");
    ins("'E",      "έ");
    ins("'H",      "ή");
    ins("'I",      "ί");
    ins("\"I",     "ϊ");
    ins("'O",      "ό");
    ins("'U",      "ύ");
    ins("\"U",     "ϋ");
    ins("'W",      "ώ");

    //BASE CHARACTERS
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

    ins("A",      "α");
    ins("B",      "β");
    ins("V",      "β");
    ins("G",      "γ");
    ins("D",      "δ");
    ins("E",      "ε");
    ins("Z",      "ζ");
    ins("H",      "η");
    ins("Q",      "θ");
    ins("I",      "ι");
    ins("K",      "κ");
    ins("L",      "λ");
    ins("M",      "μ");
    ins("N",      "ν");
    ins("C",      "ξ");
    ins("KS",     "ξ");
    ins("Ks",     "ξ");
    ins("O",      "ο");
    ins("P",      "π");
    ins("R",      "ρ");
    ins("S",      "σ");
    ins("S1",     "σ");
    ins("J",      "ς");
    ins("S2",     "ς");
    ins("S\\n",   "ς");
    ins("T",      "τ");
    ins("U",      "υ");
    ins("F",      "φ");
    ins("X",      "χ");
    ins("Y",      "ψ");
    ins("PS",     "ψ");
    ins("Ps",     "ψ");
    ins("W",      "ω");
}

sptr< Dictionary::Class > makeDictionary() throw( std::exception )
{
  static GreekTable t;

  return new Transliteration::TransliterationDictionary( "baa9e37a1aa69cdb5daca14a48ffe5ae",
                      QCoreApplication::translate( "GreekTranslit", "Greek Transliteration" ).toUtf8().data(),
                      QIcon( ":/flags/gr.png" ), t );
}

}

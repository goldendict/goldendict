#include "romaji.hh"
#if defined(Gen_Transliteration) && !defined (_MSC_VER)
#include "utf8.hh"
#endif
#include <QCoreApplication>
namespace Romaji {

class HepburnHiragana: public Transliteration::Table
{
public:
#if defined(Gen_Transliteration) && !defined (_MSC_VER)
    void ins( char const * from, char const * to )
    {
        write_Table_UCS_FILE(HepburnHiragana, from,to)
                Transliteration::Table::ins(from, to);
    }
#endif
    HepburnHiragana();
};

HepburnHiragana::HepburnHiragana()
{
#if defined(Gen_Transliteration) && !defined (_MSC_VER)
    // Raw UTF8 -- handle with care. We'd better remap those to \xAB hex encoding
    ins( "a", "あ" ); ins( "i", "い" ); ins( "u", "う" ); ins( "e", "え" ); ins( "o", "お" );

    ins( "ka", "か" ); ins( "ki", "き" ); ins( "ku", "く" ); ins( "ke", "け" ); ins( "ko", "こ" ); ins( "kya", "きゃ" ); ins( "kyu", "きゅ" ); ins( "kyo", "きょ" );
    ins( "sa", "さ" ); ins( "shi", "し" ); ins( "su", "す" ); ins( "se", "せ" ); ins( "so", "そ" ); ins( "sha", "しゃ" ); ins( "shu", "しゅ" ); ins( "sho", "しょ" );
    ins( "ta", "た" ); ins( "chi", "ち" ); ins( "tsu", "つ" ); ins( "te", "て" ); ins( "to", "と" ); ins( "cha", "ちゃ" ); ins( "chu", "ちゅ" ); ins( "cho", "ちょ" );
    ins( "na", "な" ); ins( "ni", "に" ); ins( "nu", "ぬ" ); ins( "ne", "ね" ); ins( "no", "の" ); ins( "nya", "にゃ" ); ins( "nyu", "にゅ" ); ins( "nyo", "にょ" );
    ins( "ha", "は" ); ins( "hi", "ひ" ); ins( "fu", "ふ" ); ins( "he", "へ" ); ins( "ho", "ほ" ); ins( "hya", "ひゃ" ); ins( "hyu", "ひゅ" ); ins( "hyo", "ひょ" );
    ins( "ma", "ま" ); ins( "mi", "み" ); ins( "mu", "む" ); ins( "me", "め" ); ins( "mo", "も" ); ins( "mya", "みゃ" ); ins( "myu", "みゅ" ); ins( "myo", "みょ" );
    ins( "ya", "や" ); ins( "yu", "ゆ" ); ins( "yo", "よ" );
    ins( "ra", "ら" ); ins( "ri", "り" ); ins( "ru", "る" ); ins( "re", "れ" ); ins( "ro", "ろ" ); ins( "rya", "りゃ" ); ins( "ryu", "りゅ" ); ins( "ryo", "りょ" );
    ins( "wa", "わ" ); /*ゐ wi† 		ゑ we† */ ins( "wo", "を" );
    ins( "n", "ん" );
    ins( "ga", "が" ); ins( "gi", "ぎ" ); ins( "gu", "ぐ" ); ins( "ge", "げ" ); ins( "go", "ご" ); ins( "gya", "ぎゃ" ); ins( "gyu", "ぎゅ" ); ins( "gyo", "ぎょ" );
    ins( "za", "ざ" ); ins( "ji", "じ" ); ins( "zu", "ず" ); ins( "ze", "ぜ" ); ins( "zo", "ぞ" ); ins( "ja", "じゃ" ); ins( "ju", "じゅ" ); ins( "jo", "じょ" );
    ins( "da", "だ" ); ins( "(ji)", "ぢ" ); ins( "(zu)", "づ" ); ins( "de", "で" ); ins( "do", "ど" ); ins( "(ja)", "ぢゃ" ); ins( "(ju)", "ぢゅ" ); ins( "(jo)", "ぢょ" );
    ins( "ba", "ば" ); ins( "bi", "び" ); ins( "bu", "ぶ" ); ins( "be", "べ" ); ins( "bo", "ぼ" ); ins( "bya", "びゃ" ); ins( "byu", "びゅ" ); ins( "byo", "びょ" );
    ins( "pa", "ぱ" ); ins( "pi", "ぴ" ); ins( "pu", "ぷ" ); ins( "pe", "ぺ" ); ins( "po", "ぽ" ); ins( "pya", "ぴゃ" ); ins( "pyu", "ぴゅ" ); ins( "pyo", "ぴょ" );

    // Double consonants

    ins( "kka", "っか" ); ins( "kki", "っき" ); ins( "kku", "っく" ); ins( "kke", "っけ" ); ins( "kko", "っこ" ); ins( "kkya", "っきゃ" ); ins( "kkyu", "っきゅ" ); ins( "kkyo", "っきょ" );
    ins( "ssa", "っさ" ); ins( "sshi", "っし" ); ins( "ssu", "っす" ); ins( "sse", "っせ" ); ins( "sso", "っそ" ); ins( "ssha", "っしゃ" ); ins( "sshu", "っしゅ" ); ins( "ssho", "っしょ" );
    ins( "tta", "った" ); ins( "tchi", "っち" ); ins( "ttsu", "っつ" ); ins( "tte", "って" ); ins( "tto", "っと" ); ins( "tcha", "っちゃ" ); ins( "tchu", "っちゅ" ); ins( "tcho", "っちょ" );
    ins( "ppa", "っぱ" ); ins( "ppi", "っぴ" ); ins( "ppu", "っぷ" ); ins( "ppe", "っぺ" ); ins( "ppo", "っぽ" ); ins( "ppya", "っぴゃ" ); ins( "ppyu", "っぴゅ" ); ins( "ppyo", "っぴょ" );
#else
    ins("\x61", "\xe3\x81\x82");
    ins("\x69", "\xe3\x81\x84");
    ins("\x75", "\xe3\x81\x86");
    ins("\x65", "\xe3\x81\x88");
    ins("\x6f", "\xe3\x81\x8a");
    ins("\x6b\x61", "\xe3\x81\x8b");
    ins("\x6b\x69", "\xe3\x81\x8d");
    ins("\x6b\x75", "\xe3\x81\x8f");
    ins("\x6b\x65", "\xe3\x81\x91");
    ins("\x6b\x6f", "\xe3\x81\x93");
    ins("\x6b\x79\x61", "\xe3\x81\x8d\xe3\x82\x83");
    ins("\x6b\x79\x75", "\xe3\x81\x8d\xe3\x82\x85");
    ins("\x6b\x79\x6f", "\xe3\x81\x8d\xe3\x82\x87");
    ins("\x73\x61", "\xe3\x81\x95");
    ins("\x73\x68\x69", "\xe3\x81\x97");
    ins("\x73\x75", "\xe3\x81\x99");
    ins("\x73\x65", "\xe3\x81\x9b");
    ins("\x73\x6f", "\xe3\x81\x9d");
    ins("\x73\x68\x61", "\xe3\x81\x97\xe3\x82\x83");
    ins("\x73\x68\x75", "\xe3\x81\x97\xe3\x82\x85");
    ins("\x73\x68\x6f", "\xe3\x81\x97\xe3\x82\x87");
    ins("\x74\x61", "\xe3\x81\x9f");
    ins("\x63\x68\x69", "\xe3\x81\xa1");
    ins("\x74\x73\x75", "\xe3\x81\xa4");
    ins("\x74\x65", "\xe3\x81\xa6");
    ins("\x74\x6f", "\xe3\x81\xa8");
    ins("\x63\x68\x61", "\xe3\x81\xa1\xe3\x82\x83");
    ins("\x63\x68\x75", "\xe3\x81\xa1\xe3\x82\x85");
    ins("\x63\x68\x6f", "\xe3\x81\xa1\xe3\x82\x87");
    ins("\x6e\x61", "\xe3\x81\xaa");
    ins("\x6e\x69", "\xe3\x81\xab");
    ins("\x6e\x75", "\xe3\x81\xac");
    ins("\x6e\x65", "\xe3\x81\xad");
    ins("\x6e\x6f", "\xe3\x81\xae");
    ins("\x6e\x79\x61", "\xe3\x81\xab\xe3\x82\x83");
    ins("\x6e\x79\x75", "\xe3\x81\xab\xe3\x82\x85");
    ins("\x6e\x79\x6f", "\xe3\x81\xab\xe3\x82\x87");
    ins("\x68\x61", "\xe3\x81\xaf");
    ins("\x68\x69", "\xe3\x81\xb2");
    ins("\x66\x75", "\xe3\x81\xb5");
    ins("\x68\x65", "\xe3\x81\xb8");
    ins("\x68\x6f", "\xe3\x81\xbb");
    ins("\x68\x79\x61", "\xe3\x81\xb2\xe3\x82\x83");
    ins("\x68\x79\x75", "\xe3\x81\xb2\xe3\x82\x85");
    ins("\x68\x79\x6f", "\xe3\x81\xb2\xe3\x82\x87");
    ins("\x6d\x61", "\xe3\x81\xbe");
    ins("\x6d\x69", "\xe3\x81\xbf");
    ins("\x6d\x75", "\xe3\x82\x80");
    ins("\x6d\x65", "\xe3\x82\x81");
    ins("\x6d\x6f", "\xe3\x82\x82");
    ins("\x6d\x79\x61", "\xe3\x81\xbf\xe3\x82\x83");
    ins("\x6d\x79\x75", "\xe3\x81\xbf\xe3\x82\x85");
    ins("\x6d\x79\x6f", "\xe3\x81\xbf\xe3\x82\x87");
    ins("\x79\x61", "\xe3\x82\x84");
    ins("\x79\x75", "\xe3\x82\x86");
    ins("\x79\x6f", "\xe3\x82\x88");
    ins("\x72\x61", "\xe3\x82\x89");
    ins("\x72\x69", "\xe3\x82\x8a");
    ins("\x72\x75", "\xe3\x82\x8b");
    ins("\x72\x65", "\xe3\x82\x8c");
    ins("\x72\x6f", "\xe3\x82\x8d");
    ins("\x72\x79\x61", "\xe3\x82\x8a\xe3\x82\x83");
    ins("\x72\x79\x75", "\xe3\x82\x8a\xe3\x82\x85");
    ins("\x72\x79\x6f", "\xe3\x82\x8a\xe3\x82\x87");
    ins("\x77\x61", "\xe3\x82\x8f");
    ins("\x77\x6f", "\xe3\x82\x92");
    ins("\x6e", "\xe3\x82\x93");
    ins("\x67\x61", "\xe3\x81\x8c");
    ins("\x67\x69", "\xe3\x81\x8e");
    ins("\x67\x75", "\xe3\x81\x90");
    ins("\x67\x65", "\xe3\x81\x92");
    ins("\x67\x6f", "\xe3\x81\x94");
    ins("\x67\x79\x61", "\xe3\x81\x8e\xe3\x82\x83");
    ins("\x67\x79\x75", "\xe3\x81\x8e\xe3\x82\x85");
    ins("\x67\x79\x6f", "\xe3\x81\x8e\xe3\x82\x87");
    ins("\x7a\x61", "\xe3\x81\x96");
    ins("\x6a\x69", "\xe3\x81\x98");
    ins("\x7a\x75", "\xe3\x81\x9a");
    ins("\x7a\x65", "\xe3\x81\x9c");
    ins("\x7a\x6f", "\xe3\x81\x9e");
    ins("\x6a\x61", "\xe3\x81\x98\xe3\x82\x83");
    ins("\x6a\x75", "\xe3\x81\x98\xe3\x82\x85");
    ins("\x6a\x6f", "\xe3\x81\x98\xe3\x82\x87");
    ins("\x64\x61", "\xe3\x81\xa0");
    ins("\x28\x6a\x69\x29", "\xe3\x81\xa2");
    ins("\x28\x7a\x75\x29", "\xe3\x81\xa5");
    ins("\x64\x65", "\xe3\x81\xa7");
    ins("\x64\x6f", "\xe3\x81\xa9");
    ins("\x28\x6a\x61\x29", "\xe3\x81\xa2\xe3\x82\x83");
    ins("\x28\x6a\x75\x29", "\xe3\x81\xa2\xe3\x82\x85");
    ins("\x28\x6a\x6f\x29", "\xe3\x81\xa2\xe3\x82\x87");
    ins("\x62\x61", "\xe3\x81\xb0");
    ins("\x62\x69", "\xe3\x81\xb3");
    ins("\x62\x75", "\xe3\x81\xb6");
    ins("\x62\x65", "\xe3\x81\xb9");
    ins("\x62\x6f", "\xe3\x81\xbc");
    ins("\x62\x79\x61", "\xe3\x81\xb3\xe3\x82\x83");
    ins("\x62\x79\x75", "\xe3\x81\xb3\xe3\x82\x85");
    ins("\x62\x79\x6f", "\xe3\x81\xb3\xe3\x82\x87");
    ins("\x70\x61", "\xe3\x81\xb1");
    ins("\x70\x69", "\xe3\x81\xb4");
    ins("\x70\x75", "\xe3\x81\xb7");
    ins("\x70\x65", "\xe3\x81\xba");
    ins("\x70\x6f", "\xe3\x81\xbd");
    ins("\x70\x79\x61", "\xe3\x81\xb4\xe3\x82\x83");
    ins("\x70\x79\x75", "\xe3\x81\xb4\xe3\x82\x85");
    ins("\x70\x79\x6f", "\xe3\x81\xb4\xe3\x82\x87");
    ins("\x6b\x6b\x61", "\xe3\x81\xa3\xe3\x81\x8b");
    ins("\x6b\x6b\x69", "\xe3\x81\xa3\xe3\x81\x8d");
    ins("\x6b\x6b\x75", "\xe3\x81\xa3\xe3\x81\x8f");
    ins("\x6b\x6b\x65", "\xe3\x81\xa3\xe3\x81\x91");
    ins("\x6b\x6b\x6f", "\xe3\x81\xa3\xe3\x81\x93");
    ins("\x6b\x6b\x79\x61", "\xe3\x81\xa3\xe3\x81\x8d\xe3\x82\x83");
    ins("\x6b\x6b\x79\x75", "\xe3\x81\xa3\xe3\x81\x8d\xe3\x82\x85");
    ins("\x6b\x6b\x79\x6f", "\xe3\x81\xa3\xe3\x81\x8d\xe3\x82\x87");
    ins("\x73\x73\x61", "\xe3\x81\xa3\xe3\x81\x95");
    ins("\x73\x73\x68\x69", "\xe3\x81\xa3\xe3\x81\x97");
    ins("\x73\x73\x75", "\xe3\x81\xa3\xe3\x81\x99");
    ins("\x73\x73\x65", "\xe3\x81\xa3\xe3\x81\x9b");
    ins("\x73\x73\x6f", "\xe3\x81\xa3\xe3\x81\x9d");
    ins("\x73\x73\x68\x61", "\xe3\x81\xa3\xe3\x81\x97\xe3\x82\x83");
    ins("\x73\x73\x68\x75", "\xe3\x81\xa3\xe3\x81\x97\xe3\x82\x85");
    ins("\x73\x73\x68\x6f", "\xe3\x81\xa3\xe3\x81\x97\xe3\x82\x87");
    ins("\x74\x74\x61", "\xe3\x81\xa3\xe3\x81\x9f");
    ins("\x74\x63\x68\x69", "\xe3\x81\xa3\xe3\x81\xa1");
    ins("\x74\x74\x73\x75", "\xe3\x81\xa3\xe3\x81\xa4");
    ins("\x74\x74\x65", "\xe3\x81\xa3\xe3\x81\xa6");
    ins("\x74\x74\x6f", "\xe3\x81\xa3\xe3\x81\xa8");
    ins("\x74\x63\x68\x61", "\xe3\x81\xa3\xe3\x81\xa1\xe3\x82\x83");
    ins("\x74\x63\x68\x75", "\xe3\x81\xa3\xe3\x81\xa1\xe3\x82\x85");
    ins("\x74\x63\x68\x6f", "\xe3\x81\xa3\xe3\x81\xa1\xe3\x82\x87");
    ins("\x70\x70\x61", "\xe3\x81\xa3\xe3\x81\xb1");
    ins("\x70\x70\x69", "\xe3\x81\xa3\xe3\x81\xb4");
    ins("\x70\x70\x75", "\xe3\x81\xa3\xe3\x81\xb7");
    ins("\x70\x70\x65", "\xe3\x81\xa3\xe3\x81\xba");
    ins("\x70\x70\x6f", "\xe3\x81\xa3\xe3\x81\xbd");
    ins("\x70\x70\x79\x61", "\xe3\x81\xa3\xe3\x81\xb4\xe3\x82\x83");
    ins("\x70\x70\x79\x75", "\xe3\x81\xa3\xe3\x81\xb4\xe3\x82\x85");
    ins("\x70\x70\x79\x6f", "\xe3\x81\xa3\xe3\x81\xb4\xe3\x82\x87");
#endif
}

class HepburnKatakana: public Transliteration::Table
{
public:
#if defined(Gen_Transliteration) && !defined (_MSC_VER)
    void ins( char const * from, char const * to )
    {
        write_Table_UCS_FILE(HepburnKatakana, from,to)
                Transliteration::Table::ins(from, to);
    }
#endif
    HepburnKatakana();
};

HepburnKatakana::HepburnKatakana()
{
#if defined(Gen_Transliteration) && !defined (_MSC_VER)
    // Raw UTF8 -- handle with care. We'd better remap those to \xAB hex encoding
    ins( "a", "ア" ); ins( "i", "イ" ); ins( "u", "ウ" ); ins( "e", "エ" ); ins( "o", "オ" );

    ins( "ka", "カ" ); ins( "ki", "キ" ); ins( "ku", "ク" ); ins( "ke", "ケ" ); ins( "ko", "コ" ); ins( "kya", "キャ" ); ins( "kyu", "キュ" ); ins( "kyo", "キョ" );
    ins( "sa", "サ" ); ins( "shi", "シ" ); ins( "su", "ス" ); ins( "se", "セ" ); ins( "so", "ソ" ); ins( "sha", "シャ" ); ins( "shu", "シュ" ); ins( "sho", "ショ" );
    ins( "ta", "タ" ); ins( "chi", "チ" ); ins( "tsu", "ツ" ); ins( "te", "テ" ); ins( "to", "ト" ); ins( "cha", "チャ" ); ins( "chu", "チュ" ); ins( "cho", "チョ" );
    ins( "na", "ナ" ); ins( "ni", "ニ" ); ins( "nu", "ヌ" ); ins( "ne", "ネ" ); ins( "no", "ノ" ); ins( "nya", "ニャ" ); ins( "nyu", "ニュ" ); ins( "nyo", "ニョ" );
    ins( "ha", "ハ" ); ins( "hi", "ヒ" ); ins( "fu", "フ" ); ins( "he", "ヘ" ); ins( "ho", "ホ" ); ins( "hya", "ヒャ" ); ins( "hyu", "ヒュ" ); ins( "hyo", "ヒョ" );
    ins( "ma", "マ" ); ins( "mi", "ミ" ); ins( "mu", "ム" ); ins( "me", "メ" ); ins( "mo", "モ" ); ins( "mya", "ミャ" ); ins( "myu", "ミュ" ); ins( "myo", "ミョ" );
    ins( "ya", "ヤ" ); ins( "yu", "ユ" ); ins( "yo", "ヨ" );
    ins( "ra", "ラ" ); ins( "ri", "リ" ); ins( "ru", "ル" ); ins( "re", "レ" ); ins( "ro", "ロ" ); ins( "rya", "リャ" ); ins( "ryu", "リュ" ); ins( "ryo", "リョ" );
    ins( "wa", "ワ" ); /*ヰ wi† 		ヱ we† 	ヲ wo‡ 	*/
    ins( "n", "ン" );
    ins( "ga", "ガ" ); ins( "gi", "ギ" ); ins( "gu", "グ" ); ins( "ge", "ゲ" ); ins( "go", "ゴ" ); ins( "gya", "ギャ" ); ins( "gyu", "ギュ" ); ins( "gyo", "ギョ" );
    ins( "za", "ザ" ); ins( "ji", "ジ" ); ins( "zu", "ズ" ); ins( "ze", "ゼ" ); ins( "zo", "ゾ" ); ins( "ja", "ジャ" ); ins( "ju", "ジュ" ); ins( "jo", "ジョ" );
    ins( "da", "ダ" ); ins( "(ji)", "ヂ" ); ins( "(zu)", "ヅ" ); ins( "de", "デ" ); ins( "do", "ド" ); ins( "(ja)", "ヂャ" ); ins( "(ju)", "ヂュ" ); ins( "(jo)", "ヂョ" );
    ins( "ba", "バ" ); ins( "bi", "ビ" ); ins( "bu", "ブ" ); ins( "be", "ベ" ); ins( "bo", "ボ" ); ins( "bya", "ビャ" ); ins( "byu", "ビュ" ); ins( "byo", "ビョ" );
    ins( "pa", "パ" ); ins( "pi", "ピ" ); ins( "pu", "プ" ); ins( "pe", "ペ" ); ins( "po", "ポ" ); ins( "pya", "ピャ" ); ins( "pyu", "ピュ" ); ins( "pyo", "ピョ" );
    ins( "ye", "イェ" );
    ins( "wi", "ウィ" ); ins( "we", "ウェ" ); ins( "wo", "ウォ" );
    ins( "va", "ヷ" ); /*ヸ vi† 		ヹ ve†*/ ins( "vo", "ヺ" );
    ins( "va", "ヴァ" ); ins( "vi", "ヴィ" ); ins( "vu", "ヴ" ); ins( "ve", "ヴェ" ); ins( "vo", "ヴォ" );
    ins( "she", "シェ" );
    ins( "je", "ジェ" );
    ins( "che", "チェ" );
    ins( "ti", "ティ" ); ins( "tu", "トゥ" );
    ins( "tyu", "テュ" );
    ins( "di", "ディ" ); ins( "du", "ドゥ" );
    ins( "dyu", "デュ" );
    ins( "tsa", "ツァ" ); ins( "tse", "ツェ" ); ins( "tso", "ツォ" );
    ins( "fa", "ファ" ); ins( "fi", "フィ" ); ins( "fe", "フェ" ); ins( "fo", "フォ" );
    ins( "fyu", "フュ" );
    // Long vowel mark
    ins( "-", "ー" );
    // Double consonants

    ins( "kka", "ッカ" ); ins( "kki", "ッキ" ); ins( "kku", "ック" ); ins( "kke", "ッケ" ); ins( "kko", "ッコ" ); ins( "kkya", "ッキャ" ); ins( "kkyu", "ッキュ" ); ins( "kkyo", "ッキョ" );
    ins( "ssa", "ッサ" ); ins( "sshi", "ッシ" ); ins( "ssu", "ッス" ); ins( "sse", "ッセ" ); ins( "sso", "ッソ" ); ins( "ssha", "ッシャ" ); ins( "sshu", "ッシュ" ); ins( "ssho", "ッショ" );
    ins( "tta", "ッタ" ); ins( "tchi", "ッチ" ); ins( "ttsu", "ッツ" ); ins( "tte", "ッテ" ); ins( "tto", "ット" ); ins( "tcha", "ッチャ" ); ins( "tchu", "ッチュ" ); ins( "tcho", "ッチョ" );
    ins( "ppa", "ッパ" ); ins( "ppi", "ッピ" ); ins( "ppu", "ップ" ); ins( "ppe", "ッペ" ); ins( "ppo", "ッポ" ); ins( "ppya", "ッピャ" ); ins( "ppyu", "ッピュ" ); ins( "ppyo", "ッピョ" );
#else
    ins("\x61", "\xe3\x82\xa2");
    ins("\x69", "\xe3\x82\xa4");
    ins("\x75", "\xe3\x82\xa6");
    ins("\x65", "\xe3\x82\xa8");
    ins("\x6f", "\xe3\x82\xaa");
    ins("\x6b\x61", "\xe3\x82\xab");
    ins("\x6b\x69", "\xe3\x82\xad");
    ins("\x6b\x75", "\xe3\x82\xaf");
    ins("\x6b\x65", "\xe3\x82\xb1");
    ins("\x6b\x6f", "\xe3\x82\xb3");
    ins("\x6b\x79\x61", "\xe3\x82\xad\xe3\x83\xa3");
    ins("\x6b\x79\x75", "\xe3\x82\xad\xe3\x83\xa5");
    ins("\x6b\x79\x6f", "\xe3\x82\xad\xe3\x83\xa7");
    ins("\x73\x61", "\xe3\x82\xb5");
    ins("\x73\x68\x69", "\xe3\x82\xb7");
    ins("\x73\x75", "\xe3\x82\xb9");
    ins("\x73\x65", "\xe3\x82\xbb");
    ins("\x73\x6f", "\xe3\x82\xbd");
    ins("\x73\x68\x61", "\xe3\x82\xb7\xe3\x83\xa3");
    ins("\x73\x68\x75", "\xe3\x82\xb7\xe3\x83\xa5");
    ins("\x73\x68\x6f", "\xe3\x82\xb7\xe3\x83\xa7");
    ins("\x74\x61", "\xe3\x82\xbf");
    ins("\x63\x68\x69", "\xe3\x83\x81");
    ins("\x74\x73\x75", "\xe3\x83\x84");
    ins("\x74\x65", "\xe3\x83\x86");
    ins("\x74\x6f", "\xe3\x83\x88");
    ins("\x63\x68\x61", "\xe3\x83\x81\xe3\x83\xa3");
    ins("\x63\x68\x75", "\xe3\x83\x81\xe3\x83\xa5");
    ins("\x63\x68\x6f", "\xe3\x83\x81\xe3\x83\xa7");
    ins("\x6e\x61", "\xe3\x83\x8a");
    ins("\x6e\x69", "\xe3\x83\x8b");
    ins("\x6e\x75", "\xe3\x83\x8c");
    ins("\x6e\x65", "\xe3\x83\x8d");
    ins("\x6e\x6f", "\xe3\x83\x8e");
    ins("\x6e\x79\x61", "\xe3\x83\x8b\xe3\x83\xa3");
    ins("\x6e\x79\x75", "\xe3\x83\x8b\xe3\x83\xa5");
    ins("\x6e\x79\x6f", "\xe3\x83\x8b\xe3\x83\xa7");
    ins("\x68\x61", "\xe3\x83\x8f");
    ins("\x68\x69", "\xe3\x83\x92");
    ins("\x66\x75", "\xe3\x83\x95");
    ins("\x68\x65", "\xe3\x83\x98");
    ins("\x68\x6f", "\xe3\x83\x9b");
    ins("\x68\x79\x61", "\xe3\x83\x92\xe3\x83\xa3");
    ins("\x68\x79\x75", "\xe3\x83\x92\xe3\x83\xa5");
    ins("\x68\x79\x6f", "\xe3\x83\x92\xe3\x83\xa7");
    ins("\x6d\x61", "\xe3\x83\x9e");
    ins("\x6d\x69", "\xe3\x83\x9f");
    ins("\x6d\x75", "\xe3\x83\xa0");
    ins("\x6d\x65", "\xe3\x83\xa1");
    ins("\x6d\x6f", "\xe3\x83\xa2");
    ins("\x6d\x79\x61", "\xe3\x83\x9f\xe3\x83\xa3");
    ins("\x6d\x79\x75", "\xe3\x83\x9f\xe3\x83\xa5");
    ins("\x6d\x79\x6f", "\xe3\x83\x9f\xe3\x83\xa7");
    ins("\x79\x61", "\xe3\x83\xa4");
    ins("\x79\x75", "\xe3\x83\xa6");
    ins("\x79\x6f", "\xe3\x83\xa8");
    ins("\x72\x61", "\xe3\x83\xa9");
    ins("\x72\x69", "\xe3\x83\xaa");
    ins("\x72\x75", "\xe3\x83\xab");
    ins("\x72\x65", "\xe3\x83\xac");
    ins("\x72\x6f", "\xe3\x83\xad");
    ins("\x72\x79\x61", "\xe3\x83\xaa\xe3\x83\xa3");
    ins("\x72\x79\x75", "\xe3\x83\xaa\xe3\x83\xa5");
    ins("\x72\x79\x6f", "\xe3\x83\xaa\xe3\x83\xa7");
    ins("\x77\x61", "\xe3\x83\xaf");
    ins("\x6e", "\xe3\x83\xb3");
    ins("\x67\x61", "\xe3\x82\xac");
    ins("\x67\x69", "\xe3\x82\xae");
    ins("\x67\x75", "\xe3\x82\xb0");
    ins("\x67\x65", "\xe3\x82\xb2");
    ins("\x67\x6f", "\xe3\x82\xb4");
    ins("\x67\x79\x61", "\xe3\x82\xae\xe3\x83\xa3");
    ins("\x67\x79\x75", "\xe3\x82\xae\xe3\x83\xa5");
    ins("\x67\x79\x6f", "\xe3\x82\xae\xe3\x83\xa7");
    ins("\x7a\x61", "\xe3\x82\xb6");
    ins("\x6a\x69", "\xe3\x82\xb8");
    ins("\x7a\x75", "\xe3\x82\xba");
    ins("\x7a\x65", "\xe3\x82\xbc");
    ins("\x7a\x6f", "\xe3\x82\xbe");
    ins("\x6a\x61", "\xe3\x82\xb8\xe3\x83\xa3");
    ins("\x6a\x75", "\xe3\x82\xb8\xe3\x83\xa5");
    ins("\x6a\x6f", "\xe3\x82\xb8\xe3\x83\xa7");
    ins("\x64\x61", "\xe3\x83\x80");
    ins("\x28\x6a\x69\x29", "\xe3\x83\x82");
    ins("\x28\x7a\x75\x29", "\xe3\x83\x85");
    ins("\x64\x65", "\xe3\x83\x87");
    ins("\x64\x6f", "\xe3\x83\x89");
    ins("\x28\x6a\x61\x29", "\xe3\x83\x82\xe3\x83\xa3");
    ins("\x28\x6a\x75\x29", "\xe3\x83\x82\xe3\x83\xa5");
    ins("\x28\x6a\x6f\x29", "\xe3\x83\x82\xe3\x83\xa7");
    ins("\x62\x61", "\xe3\x83\x90");
    ins("\x62\x69", "\xe3\x83\x93");
    ins("\x62\x75", "\xe3\x83\x96");
    ins("\x62\x65", "\xe3\x83\x99");
    ins("\x62\x6f", "\xe3\x83\x9c");
    ins("\x62\x79\x61", "\xe3\x83\x93\xe3\x83\xa3");
    ins("\x62\x79\x75", "\xe3\x83\x93\xe3\x83\xa5");
    ins("\x62\x79\x6f", "\xe3\x83\x93\xe3\x83\xa7");
    ins("\x70\x61", "\xe3\x83\x91");
    ins("\x70\x69", "\xe3\x83\x94");
    ins("\x70\x75", "\xe3\x83\x97");
    ins("\x70\x65", "\xe3\x83\x9a");
    ins("\x70\x6f", "\xe3\x83\x9d");
    ins("\x70\x79\x61", "\xe3\x83\x94\xe3\x83\xa3");
    ins("\x70\x79\x75", "\xe3\x83\x94\xe3\x83\xa5");
    ins("\x70\x79\x6f", "\xe3\x83\x94\xe3\x83\xa7");
    ins("\x79\x65", "\xe3\x82\xa4\xe3\x82\xa7");
    ins("\x77\x69", "\xe3\x82\xa6\xe3\x82\xa3");
    ins("\x77\x65", "\xe3\x82\xa6\xe3\x82\xa7");
    ins("\x77\x6f", "\xe3\x82\xa6\xe3\x82\xa9");
    ins("\x76\x61", "\xe3\x83\xb7");
    ins("\x76\x6f", "\xe3\x83\xba");
    ins("\x76\x61", "\xe3\x83\xb4\xe3\x82\xa1");
    ins("\x76\x69", "\xe3\x83\xb4\xe3\x82\xa3");
    ins("\x76\x75", "\xe3\x83\xb4");
    ins("\x76\x65", "\xe3\x83\xb4\xe3\x82\xa7");
    ins("\x76\x6f", "\xe3\x83\xb4\xe3\x82\xa9");
    ins("\x73\x68\x65", "\xe3\x82\xb7\xe3\x82\xa7");
    ins("\x6a\x65", "\xe3\x82\xb8\xe3\x82\xa7");
    ins("\x63\x68\x65", "\xe3\x83\x81\xe3\x82\xa7");
    ins("\x74\x69", "\xe3\x83\x86\xe3\x82\xa3");
    ins("\x74\x75", "\xe3\x83\x88\xe3\x82\xa5");
    ins("\x74\x79\x75", "\xe3\x83\x86\xe3\x83\xa5");
    ins("\x64\x69", "\xe3\x83\x87\xe3\x82\xa3");
    ins("\x64\x75", "\xe3\x83\x89\xe3\x82\xa5");
    ins("\x64\x79\x75", "\xe3\x83\x87\xe3\x83\xa5");
    ins("\x74\x73\x61", "\xe3\x83\x84\xe3\x82\xa1");
    ins("\x74\x73\x65", "\xe3\x83\x84\xe3\x82\xa7");
    ins("\x74\x73\x6f", "\xe3\x83\x84\xe3\x82\xa9");
    ins("\x66\x61", "\xe3\x83\x95\xe3\x82\xa1");
    ins("\x66\x69", "\xe3\x83\x95\xe3\x82\xa3");
    ins("\x66\x65", "\xe3\x83\x95\xe3\x82\xa7");
    ins("\x66\x6f", "\xe3\x83\x95\xe3\x82\xa9");
    ins("\x66\x79\x75", "\xe3\x83\x95\xe3\x83\xa5");
    ins("\x2d", "\xe3\x83\xbc");
    ins("\x6b\x6b\x61", "\xe3\x83\x83\xe3\x82\xab");
    ins("\x6b\x6b\x69", "\xe3\x83\x83\xe3\x82\xad");
    ins("\x6b\x6b\x75", "\xe3\x83\x83\xe3\x82\xaf");
    ins("\x6b\x6b\x65", "\xe3\x83\x83\xe3\x82\xb1");
    ins("\x6b\x6b\x6f", "\xe3\x83\x83\xe3\x82\xb3");
    ins("\x6b\x6b\x79\x61", "\xe3\x83\x83\xe3\x82\xad\xe3\x83\xa3");
    ins("\x6b\x6b\x79\x75", "\xe3\x83\x83\xe3\x82\xad\xe3\x83\xa5");
    ins("\x6b\x6b\x79\x6f", "\xe3\x83\x83\xe3\x82\xad\xe3\x83\xa7");
    ins("\x73\x73\x61", "\xe3\x83\x83\xe3\x82\xb5");
    ins("\x73\x73\x68\x69", "\xe3\x83\x83\xe3\x82\xb7");
    ins("\x73\x73\x75", "\xe3\x83\x83\xe3\x82\xb9");
    ins("\x73\x73\x65", "\xe3\x83\x83\xe3\x82\xbb");
    ins("\x73\x73\x6f", "\xe3\x83\x83\xe3\x82\xbd");
    ins("\x73\x73\x68\x61", "\xe3\x83\x83\xe3\x82\xb7\xe3\x83\xa3");
    ins("\x73\x73\x68\x75", "\xe3\x83\x83\xe3\x82\xb7\xe3\x83\xa5");
    ins("\x73\x73\x68\x6f", "\xe3\x83\x83\xe3\x82\xb7\xe3\x83\xa7");
    ins("\x74\x74\x61", "\xe3\x83\x83\xe3\x82\xbf");
    ins("\x74\x63\x68\x69", "\xe3\x83\x83\xe3\x83\x81");
    ins("\x74\x74\x73\x75", "\xe3\x83\x83\xe3\x83\x84");
    ins("\x74\x74\x65", "\xe3\x83\x83\xe3\x83\x86");
    ins("\x74\x74\x6f", "\xe3\x83\x83\xe3\x83\x88");
    ins("\x74\x63\x68\x61", "\xe3\x83\x83\xe3\x83\x81\xe3\x83\xa3");
    ins("\x74\x63\x68\x75", "\xe3\x83\x83\xe3\x83\x81\xe3\x83\xa5");
    ins("\x74\x63\x68\x6f", "\xe3\x83\x83\xe3\x83\x81\xe3\x83\xa7");
    ins("\x70\x70\x61", "\xe3\x83\x83\xe3\x83\x91");
    ins("\x70\x70\x69", "\xe3\x83\x83\xe3\x83\x94");
    ins("\x70\x70\x75", "\xe3\x83\x83\xe3\x83\x97");
    ins("\x70\x70\x65", "\xe3\x83\x83\xe3\x83\x9a");
    ins("\x70\x70\x6f", "\xe3\x83\x83\xe3\x83\x9d");
    ins("\x70\x70\x79\x61", "\xe3\x83\x83\xe3\x83\x94\xe3\x83\xa3");
    ins("\x70\x70\x79\x75", "\xe3\x83\x83\xe3\x83\x94\xe3\x83\xa5");
    ins("\x70\x70\x79\x6f", "\xe3\x83\x83\xe3\x83\x94\xe3\x83\xa7");
#endif
}

vector< sptr< Dictionary::Class > > makeDictionaries( Config::Romaji const & r )
THROW_SPEC( std::exception )
{
    vector< sptr< Dictionary::Class > > result;

    if ( r.enable )
    {
        if ( r.enableHepburn )
        {
            if ( r.enableHiragana )
            {
                static const HepburnHiragana t;

                result.push_back( sptr< Dictionary::Class >(new Transliteration::TransliterationDictionary( "94eae5a5aaf5b0a900490f4d6b36aac0",
                                                                                                            QCoreApplication::translate( "Romaji", "Hepburn Romaji for Hiragana" ).toUtf8().data(),
                                                                                                            QIcon( ":/flags/jp.png" ), t, false ) ) );
            }

            if ( r.enableKatakana )
            {
                static const HepburnKatakana t;

                result.push_back( sptr< Dictionary::Class >(new Transliteration::TransliterationDictionary( "3252a35767d3f6e85e3e39069800dd2f",
                                                                                                            QCoreApplication::translate( "Romaji", "Hepburn Romaji for Katakana" ).toUtf8().data(),
                                                                                                            QIcon( ":/flags/jp.png" ), t, false ) ) );
            }
        }
    }

    return result;
}

}

#include "romaji.hh"
#include <QCoreApplication>

namespace Romaji {

class HepburnHiragana: public Transliteration::Table
{
public:

  HepburnHiragana();
};

HepburnHiragana::HepburnHiragana()
{
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

}

class HepburnKatakana: public Transliteration::Table
{
public:

  HepburnKatakana();
};

HepburnKatakana::HepburnKatakana()
{
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
}

vector< sptr< Dictionary::Class > > makeDictionaries( Config::Romaji const & r )
  throw( std::exception )
{
  vector< sptr< Dictionary::Class > > result;
  
  if ( r.enable )
  {
    if ( r.enableHepburn )
    {
      if ( r.enableHiragana )
      {
        static HepburnHiragana t;

        result.push_back( new Transliteration::TransliterationDictionary( "94eae5a5aaf5b0a900490f4d6b36aac0",
                            QCoreApplication::translate( "Romaji", "Hepburn Romaji for Hiragana" ).toUtf8().data(),
                            QIcon( ":/flags/jp.png" ), t, false ) );
      }
      
      if ( r.enableKatakana )
      {
        static HepburnKatakana t;

        result.push_back( new Transliteration::TransliterationDictionary( "3252a35767d3f6e85e3e39069800dd2f",
                            QCoreApplication::translate( "Romaji", "Hepburn Romaji for Katakana" ).toUtf8().data(),
                            QIcon( ":/flags/jp.png" ), t, false ) );
      }
    }
  }

  return result;
}

}

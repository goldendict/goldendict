/* This file is (c) 2015 Zhe Wang <0x1997@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "chinese.hh"
#include <QCoreApplication>
#include <opencc/Export.hpp>
#include <opencc/SimpleConverter.hpp>
#include "folding.hh"
#include "transliteration.hh"
#include "utf8.hh"

namespace Chinese {

class CharacterConversionDictionary: public Transliteration::BaseTransliterationDictionary
{
  opencc::SimpleConverter converter;

public:

  CharacterConversionDictionary( std::string const & id, std::string const & name,
                                 QIcon icon, std::string const & openccConfig);

  std::vector< std::wstring > getAlternateWritings( std::wstring const & )
    throw();
};

CharacterConversionDictionary::CharacterConversionDictionary( std::string const & id,
                                                              std::string const & name_,
                                                              QIcon icon_,
                                                              std::string const & openccConfig):
  Transliteration::BaseTransliterationDictionary( id, name_, icon_, false ),
  converter(openccConfig)
{
}

std::vector< std::wstring > CharacterConversionDictionary::getAlternateWritings( std::wstring const & str )
  throw()
{
  std::vector< std::wstring > results;

  std::wstring folded = Folding::applySimpleCaseOnly( str );
  std::wstring result = Utf8::decode( converter.Convert( Utf8::encode( folded ) ) );

  if ( result != folded )
    results.push_back( result );

  return results;
}

std::vector< sptr< Dictionary::Class > > makeDictionaries( Config::Chinese const & cfg )
  throw( std::exception )
{
  std::vector< sptr< Dictionary::Class > > result;

  if ( cfg.enable )
  {
    if ( cfg.enableSCToTWConversion )
    {
      result.push_back( new CharacterConversionDictionary( "bf1c33a59cbacea8f39b5b5475787cfd",
                                                           QCoreApplication::translate( "ChineseConversion", "Simplified to traditional Chinese (Taiwan variant) conversion" ).toUtf8().data(),
                                                           QIcon( ":/flags/tw.png" ), "s2tw.json" ) );
    }

    if ( cfg.enableSCToHKConversion )
    {
      result.push_back( new CharacterConversionDictionary( "9e0681fb9e1c0b6c90e6fb46111d96b5",
                                                           QCoreApplication::translate( "ChineseConversion", "Simplified to traditional Chinese (Hong Kong variant) conversion" ).toUtf8().data(),
                                                           QIcon( ":/flags/hk.png" ), "s2hk.json" ) );
    }

    if ( cfg.enableTCToSCConversion )
    {
      result.push_back( new CharacterConversionDictionary( "0db536ce0bdc52ea30d11a82c5db4a27",
                                                           QCoreApplication::translate( "ChineseConversion", "Traditional to simplified Chinese conversion" ).toUtf8().data(),
                                                           QIcon( ":/flags/cn.png" ), "t2s.json" ) );
    }
  }

  return result;
}

}

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
    if ( cfg.enableSimpToTradConversion )
    {
      result.push_back( new CharacterConversionDictionary( "abbd22460acb11992bb089b2ccda7a0c",
                                                           QCoreApplication::translate( "Chinese", "Simplified to traditional Chinese conversion" ).toUtf8().data(),
                                                           QIcon( ":/flags/cn.png" ), "s2t.json" ) );
    }

    if ( cfg.enableTradToSimpConversion )
    {
      result.push_back( new CharacterConversionDictionary( "43d783892e6cd3fa973e4232287cce72",
                                                           QCoreApplication::translate( "Chinese", "Traditional to simplified Chinese conversion" ).toUtf8().data(),
                                                           QIcon( ":/flags/cn.png" ), "t2s.json" ) );
    }
  }

  return result;
}

}

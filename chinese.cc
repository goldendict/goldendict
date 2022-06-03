/* This file is (c) 2015 Zhe Wang <0x1997@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "chinese.hh"
#include <stdexcept>
#include <QCoreApplication>
// #ifdef Q_OS_MAC
#include <opencc/opencc.h>
// #endif
#include <opencc/Export.hpp>
// #include <opencc/SimpleConverter.hpp>
#include "folding.hh"
#include "gddebug.hh"
#include "transliteration.hh"
#include "utf8.hh"

namespace Chinese {

class CharacterConversionDictionary: public Transliteration::BaseTransliterationDictionary
{
// #ifdef Q_OS_MAC
  opencc_t converter;
// #else
//   opencc::SimpleConverter* converter;
// #endif

public:

  CharacterConversionDictionary( std::string const & id, std::string const & name,
                                 QIcon icon, QString const & openccConfig);
  ~CharacterConversionDictionary();

  std::vector< gd::wstring > getAlternateWritings( gd::wstring const & )
    noexcept;
};

CharacterConversionDictionary::CharacterConversionDictionary( std::string const & id,
                                                              std::string const & name_,
                                                              QIcon icon_,
                                                              QString const & openccConfig):
  Transliteration::BaseTransliterationDictionary( id, name_, icon_, false ),
  converter( NULL )
{
  try {
// #ifdef Q_OS_MAC
    converter = opencc_open( openccConfig.toLocal8Bit().constData() );
    if( converter == reinterpret_cast< opencc_t >( -1 ) )
    {
      gdWarning( "CharacterConversionDictionary: failed to initialize OpenCC from config %s: %s\n",
                 openccConfig.toLocal8Bit().constData(), opencc_error() );
    }
// #else
//     converter = new opencc::SimpleConverter( openccConfig.toLocal8Bit().constData() );
// #endif
  } catch ( std::runtime_error& e ) {
    gdWarning( "CharacterConversionDictionary: failed to initialize OpenCC from config %s: %s\n",
               openccConfig.toLocal8Bit().constData(), e.what() );
  }
}

CharacterConversionDictionary::~CharacterConversionDictionary()
{
// #ifdef Q_OS_MAC
  if ( converter != NULL && converter != reinterpret_cast< opencc_t >( -1 ) )
    opencc_close( converter );
// #else
//   if ( converter != NULL )
//     delete converter;
// #endif
}

std::vector< gd::wstring > CharacterConversionDictionary::getAlternateWritings( gd::wstring const & str )
  noexcept
{
  std::vector< gd::wstring > results;

  if ( converter != NULL ) {
    gd::wstring folded = Folding::applySimpleCaseOnly( str );
    std::string input = Utf8::encode( folded );
    std::string output;
    gd::wstring result;

    try {
// #ifdef Q_OS_MAC
      if ( converter != NULL && converter != reinterpret_cast< opencc_t >( -1 ) )
      {
        char * tmp = opencc_convert_utf8( converter, input.c_str(), input.length() );
        if( tmp )
        {
          output = std::string( tmp );
          opencc_convert_utf8_free( tmp );
        }
        else
          gdWarning( "OpenCC: conversion failed %s\n", opencc_error() );
      }
// #else
//       output = converter->Convert( input );
// #endif
      result = Utf8::decode( output );
    } catch ( std::exception& ex ) {
      gdWarning( "OpenCC: conversion failed %s\n", ex.what() );
    }

    if ( !result.empty() && result != folded )
      results.push_back( result );
  }

  return results;
}

std::vector< sptr< Dictionary::Class > > makeDictionaries( Config::Chinese const & cfg )
  
{
  std::vector< sptr< Dictionary::Class > > result;

#ifdef Q_OS_LINUX
  QString configDir = "";
#else
  QString configDir = Config::getOpenCCDir();
  if( !configDir.isEmpty() )
    configDir += "/";
#endif

  if ( cfg.enable )
  {
    if ( cfg.enableSCToTWConversion )
    {
      result.push_back( new CharacterConversionDictionary( "bf1c33a59cbacea8f39b5b5475787cfd",
                                                           QCoreApplication::translate( "ChineseConversion", "Simplified to traditional Chinese (Taiwan variant) conversion" ).toUtf8().data(),
                                                           QIcon( ":/flags/tw.png" ), configDir + "s2tw.json" ) );
    }

    if ( cfg.enableSCToHKConversion )
    {
      result.push_back( new CharacterConversionDictionary( "9e0681fb9e1c0b6c90e6fb46111d96b5",
                                                           QCoreApplication::translate( "ChineseConversion", "Simplified to traditional Chinese (Hong Kong variant) conversion" ).toUtf8().data(),
                                                           QIcon( ":/flags/hk.png" ), configDir + "s2hk.json" ) );
    }

    if ( cfg.enableTCToSCConversion )
    {
      result.push_back( new CharacterConversionDictionary( "0db536ce0bdc52ea30d11a82c5db4a27",
                                                           QCoreApplication::translate( "ChineseConversion", "Traditional to simplified Chinese conversion" ).toUtf8().data(),
                                                           QIcon( ":/flags/cn.png" ), configDir + "t2s.json" ) );
    }
  }

  return result;
}

}

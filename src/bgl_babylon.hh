/***************************************************************************
 *   Copyright (C) 2007 by Raul Fernandes and Karl Grill                   *
 *   rgbr@yahoo.com.br                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef BABYLON_H
#define BABYLON_H

#include <stdlib.h>
#include <zlib.h>

#include <string>
#include <vector>
#include <qglobal.h>
#if defined( _MSC_VER ) && _MSC_VER < 1800 // VS2012 and older
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif

//const std::string bgl_language[] = {
#ifndef blgCode2Int
#define blgCode2Int( index, code0, code1 ) (((uint32_t)index) << 16 ) + (((uint32_t)code1) << 8 ) + (uint32_t)code0
#endif
const quint32 bgl_language[] = {
    blgCode2Int( 0, 'e', 'n' ),// "English",
    blgCode2Int( 0, 'f', 'r' ),//"French",
    blgCode2Int( 0, 'i', 't' ),//"Italian",
    blgCode2Int( 0, 'e', 's' ),//"Spanish",
    blgCode2Int( 0, 'n', 'l' ),//"Dutch",
    blgCode2Int( 0, 'p', 't' ),//"Portuguese",
    blgCode2Int( 0, 'd', 'e' ),//"German",
    blgCode2Int( 0, 'r', 'u' ),//"Russian",
    blgCode2Int( 0, 'j', 'a' ),//"Japanese",
    blgCode2Int( 1, 'z', 'h' ),//"\x01",//"Traditional Chinese",
    blgCode2Int( 2, 'z', 'h' ),//"\x02",//"Simplified Chinese",
    blgCode2Int( 0, 'e', 'l' ),//"Greek",
    blgCode2Int( 0, 'k', 'o' ),//"Korean",
    blgCode2Int( 0, 't', 'r' ),//"Turkish",
    blgCode2Int( 0, 'h', 'e' ),//"Hebrew",
    blgCode2Int( 0, 'a', 'r' ),//"Arabic",
    blgCode2Int( 0, 't', 'h' ),//"Thai",
    blgCode2Int( 3, 0, 0 ),//"\x03",//"Other",
    blgCode2Int( 4, 'z', 'h' ),//"\x04",//"Other Simplified Chinese dialects",
    blgCode2Int( 5, 'z', 'h' ),//"\x05",//Other Traditional Chinese dialects",
    blgCode2Int( 6, 0, 0 ),//"\x06",//Other Eastern-European languages",
    blgCode2Int( 7, 0, 0 ),//"\x07",//Other Western-European languages",
    blgCode2Int( 8, 'r', 'u' ),//"\x08",//Other Russian languages",
    blgCode2Int( 9, 'j', 'a' ),//"\x09",//Other Japanese languages",
    blgCode2Int( 10, 0, 0 ),//"\x0A",//"Other Baltic languages",
    blgCode2Int( 11, 'e', 'l' ),//"\x0B",//Other Greek languages",
    blgCode2Int( 12, 'k', 'o' ),//"\x0C",//"Other Korean dialects",
    blgCode2Int( 13, 't', 'r' ),//"\x0D",//Other Turkish dialects",
    blgCode2Int( 14, 't', 'h' ),//"\x0E",//"Other Thai dialects",
    blgCode2Int( 0, 'p', 'l' ),//"Polish",
    blgCode2Int( 0, 'h', 'u' ),//"Hungarian",
    blgCode2Int( 0, 'c', 's' ),//"Czech",
    blgCode2Int( 0, 'l', 't' ),//"Lithuanian",
    blgCode2Int( 0, 'l', 'v' ),//"Latvian",
    blgCode2Int( 0, 'c', 'a' ),//"Catalan",
    blgCode2Int( 0, 'h', 'r' ),//"Croatian",
    blgCode2Int( 0, 's', 'r' ),//"Serbian",
    blgCode2Int( 0, 's', 'k' ),//"Slovak",
    blgCode2Int( 0, 's', 'q' ),//"Albanian",
    blgCode2Int( 0, 'u', 'r' ),//"Urdu",
    blgCode2Int( 0, 's', 'l' ),//"Slovenian",
    blgCode2Int( 0, 'e', 't' ),//"Estonian",
    blgCode2Int( 0, 'b', 'g' ),//"Bulgarian",
    blgCode2Int( 0, 'd', 'a' ),//"Danish",
    blgCode2Int( 0, 'f', 'i' ),//"Finnish",
    blgCode2Int( 0, 'i', 's' ),//"Icelandic",
    blgCode2Int( 0, 'n', 'o' ),//"Norwegian",
    blgCode2Int( 0, 'r', 'o' ),//"Romanian",
    blgCode2Int( 0, 's', 'v' ),//"Swedish",
    blgCode2Int( 0, 'u', 'k' ),//"Ukrainian",
    blgCode2Int( 0, 'b', 'e' ),//"Belarusian",
    blgCode2Int( 0, 'f', 'a' ),//"Farsi"=Persian,
    blgCode2Int( 0, 'e', 'u' ),//"Basque",
    blgCode2Int( 0, 'm', 'k' ),//"Macedonian",
    blgCode2Int( 0, 'a', 'f' ),//"Afrikaans",
    blgCode2Int( 0, 'f', 'o' ),//"Faeroese"=Faroese,
    blgCode2Int( 0, 'l', 'a' ),//"Latin",
    blgCode2Int( 0, 'e', 'o' ),//"Esperanto",
    blgCode2Int( 15, 0, 0 ),//"Tamazight",
    blgCode2Int( 0, 'h', 'y' )//"Armenian"
};


const std::string bgl_charsetname[] = {
	"Default" ,
	"Latin",
	"Eastern European",
	"Cyrillic",
	"Japanese",
	"Traditional Chinese",
	"Simplified Chinese",
	"Baltic",
	"Greek",
	"Korean",
	"Turkish",
	"Hebrew",
	"Arabic",
	"Thai" };

const std::string bgl_charset[] = {
	"WINDOWS-1252", /*Default*/
	"WINDOWS-1252", /*Latin*/
	"WINDOWS-1250", /*Eastern European*/
	"WINDOWS-1251", /*Cyriilic*/
	"CP932", /*Japanese*/
	"BIG5", /*Traditional Chinese*/
	"GB18030", /*Simplified Chinese*/
	"CP1257", /*Baltic*/
	"CP1253", /*Greek*/
	"EUC-KR",  /*Korean*/
	"ISO-8859-9", /*Turkish*/
	"WINDOWS-1255", /*Hebrew*/
	"CP1256", /*Arabic*/
	"CP874"  /*Thai*/ };

const std::string partOfSpeech[] = {
  "n.",
  "adj.",
  "v.",
  "adv.",
  "interj.",
  "pron.",
  "prep.",
  "conj.",
  "suff.",
  "pref.",
  "art." };

typedef struct {
	unsigned type;
	unsigned length;
	char * data;
} bgl_block;

typedef struct {
        std::string headword;
        std::string definition;
        std::string displayedHeadword;
        std::vector<std::string> alternates;
} bgl_entry;

class Babylon
{
public:
    Babylon( std::string );
    ~Babylon();

    // Subclass this to store resources
    class ResourceHandler
    {
    public:

      virtual void handleBabylonResource( std::string const & filename,
                                          char const * data, size_t size )=0;

      virtual ~ResourceHandler()
      {}
    };

    /// Sets a prefix string to append to each resource reference in hyperlinks.
    void setResourcePrefix( std::string const & prefix )
    { m_resourcePrefix = prefix; }

    bool open();
    void close();
    bool readBlock( bgl_block& );
    bool read(std::string &source_charset, std::string &target_charset);
    bgl_entry readEntry( ResourceHandler * = 0 );

    inline std::string title() const { return m_title; }
    inline std::string author() const { return m_author; }
    inline std::string email() const { return m_email; }
    inline std::string description() const { return m_description; }
    inline std::string copyright() const { return m_copyright; }
    inline quint32 sourceLang() const { return m_sourceLang; }//std::string sourceLang() const { return m_sourceLang; }
    inline quint32 targetLang() const { return m_targetLang; }//inline std::string targetLang() const { return m_targetLang; }
    inline unsigned int numEntries() const { return m_numEntries; }
    inline std::string charset() const { return m_defaultCharset; }

    inline std::string filename() const { return m_filename; }

    std::vector< char > const & getIcon() const
    { return icon; }

    enum
    {
      ParserVersion = 17
    };

private:
    unsigned int bgl_readnum( int );
    void convertToUtf8( std::string &, unsigned int = 0 );

    std::string m_filename;
    gzFile file;

    std::string m_title;
    std::string m_author;
    std::string m_email;
    std::string m_description;
    std::string m_copyright;
    quint32 m_sourceLang; //std::string m_sourceLang;
    quint32 m_targetLang;//std::string m_targetLang;
    unsigned int m_numEntries;
    std::string m_defaultCharset;
    std::string m_sourceCharset;
    std::string m_targetCharset;
    std::vector< char > icon;

    std::string m_resourcePrefix;

    enum CHARSET { BGL_DEFAULT_CHARSET, BGL_SOURCE_CHARSET, BGL_TARGET_CHARSET };
};

#endif // BABYLON_H

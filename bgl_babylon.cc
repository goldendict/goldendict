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

/* Various improvements were made by Konstantin Isakov for the GoldenDict
 * program. */

#include "bgl_babylon.hh"
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iconv.h>
#include <QTextDocument>
#include "dprintf.hh"
#include "ufile.hh"
#include "iconv.hh"
#include "htmlescape.hh"
#include <QString>
#include <QDebug>

#ifdef _WIN32
#include <io.h>
#define DUP _dup
#else
#define DUP dup
#endif

using std::string;

Babylon::Babylon( std::string filename ) :
m_filename( filename )
{
  file = NULL;
}


Babylon::~Babylon()
{
  close();
}


bool Babylon::open()
{
  FILE *f;
  unsigned char buf[6];
  int i;

  f = gd_fopen( m_filename.c_str(), "rb" );
  if( f == NULL )
    return false;

  i = fread( buf, 1, 6, f );

  /* First four bytes: BGL signature 0x12340001 or 0x12340002 (big-endian) */
  if( i < 6 || memcmp( buf, "\x12\x34\x00", 3 ) || buf[3] == 0 || buf[3] > 2 )
  {
    fclose( f );
    return false;
  }

  /* Calculate position of gz header */

  i = buf[4] << 8 | buf[5];

  if( i < 6 )
  {
    fclose( f );
    return false;
  }

  if( fseek( f, i, SEEK_SET ) ) /* can't seek - emulate */
      for(int j=0;j < i - 6;j++) fgetc( f );

  if( ferror( f ) || feof( f ) )
  {
    fclose( f );
    return false;
  }

  /* we need to flush the file because otherwise some nfs mounts don't seem
   * to properly update the file position for the following reopen */

  fflush( f );

#ifdef Q_OS_MACX
  /* Under Mac OS X the above technique don't set reopen position properly */
  int fn = DUP( fileno( f ) );
  lseek( fn, i, SEEK_SET );
  file = gzdopen( fn, "r" );
#else
  file = gzdopen( DUP( fileno( f ) ), "r" );
#endif

  fclose( f );

  if( file == NULL )
    return false;

  return true;
}


void Babylon::close()
{
  if ( file )
  {
    gzclose( file );
    file = 0;
  }
}


bool Babylon::readBlock( bgl_block &block )
{
  if( gzeof( file ) || file == NULL )
    return false;

  block.length = bgl_readnum( 1 );
  block.type = block.length & 0xf;
  if( block.type == 4 ) return false; // end of file marker
  block.length >>= 4;
  block.length = block.length < 4 ? bgl_readnum( block.length + 1 ) : block.length - 4 ;
  if( block.length )
  {
    block.data = (char *)malloc( block.length );
    unsigned res = gzread( file, block.data, block.length );
    if( block.length != res )
    {
      free( block.data );
      block.length = 0;
      gzclearerr( file );
      return false;
    }
  }

  return true;
}


unsigned int Babylon::bgl_readnum( int bytes )
{
  unsigned char buf[4];
  unsigned val = 0;

  if ( bytes < 1 || bytes > 4 ) return (0);

  int res = gzread( file, buf, bytes );

  if( res != bytes )
  {
    gzclearerr( file );
    return 4;  // Read error - return end of file marker
  }

  for(int i=0;i<bytes;i++) val= (val << 8) | buf[i];
  return val;
}


bool Babylon::read(std::string &source_charset, std::string &target_charset)
{
  if( file == NULL ) return false;

  bgl_block block;
  unsigned int pos;
  unsigned int type;
  std::string headword;
  std::string definition;

  bool isUtf8File = false;

  m_sourceCharset = source_charset;
  m_targetCharset = target_charset;
  m_numEntries = 0;
  while( readBlock( block ) )
  {
    headword.clear();
    definition.clear();
    switch( block.type )
    {
      case 0:
        switch( block.data[0] )
        {
          case 8:
            type = (unsigned int)block.data[2];
            if( type == 67 ) type = 1;
            if( type > 64 ) type -= 65;

            if ( type >= 14 )
              type = 0;

            m_defaultCharset = bgl_charset[type];
            break;
          default:
            break;
        }
        break;
      case 1:
      case 7:
      case 10:
      case 11:
        // Only count entries
        m_numEntries++;
        break;
      case 3:
        pos = 2;
        switch( block.data[1] )
        {
          case 1:
            headword.reserve( block.length - 2 );
            for(unsigned int a=0;a<block.length-2;a++) headword += block.data[pos++];
            m_title = headword;
            break;
          case 2:
            headword.reserve( block.length - 2 );
            for(unsigned int a=0;a<block.length-2;a++) headword += block.data[pos++];
            m_author = headword;
            break;
          case 3:
            headword.reserve( block.length - 2 );
            for(unsigned int a=0;a<block.length-2;a++) headword += block.data[pos++];
            m_email = headword;
            break;
          case 4:
            headword.reserve( block.length - 2 );
            for(unsigned int a=0;a<block.length-2;a++) headword += block.data[pos++];
            m_copyright = headword;
            break;
          case 7:
            m_sourceLang = bgl_language[(unsigned char)(block.data[5])];
            //m_sourceLang = headword;
            break;
          case 8:
            m_targetLang = bgl_language[(unsigned char)(block.data[5])];
            //m_targetLang = headword;
            break;
          case 9:
            headword.reserve( block.length - 2 );
            for(unsigned int a=0;a<block.length-2;a++) {
              if (block.data[pos] == '\r') {
              } else if (block.data[pos] == '\n') {
                headword += "<br>";
              } else {
                headword += block.data[pos];
              }
              pos++;
            }
            m_description = headword;
            break;
          case 11:
            icon.resize( block.length - 2 );
            memcpy( &icon.front(), &(block.data[ 2 ]), icon.size() );
          break;
          case 17:
            if ( block.length >= 5 && ( (unsigned char) block.data[ 4 ] & 0x80 ) != 0 )
              isUtf8File = true;
          break;
          case 26:
            type = (unsigned int)block.data[2];
            if( type == 67 ) type = 1;
            if( type > 64 ) type -= 65;
            if ( type >= 14 )
              type = 0;
            if (m_sourceCharset.empty())
              m_sourceCharset = bgl_charset[type];
            break;
          case 27:
            type = (unsigned int)block.data[2];
            if( type == 67 ) type = 1;
            if( type > 64 ) type -= 65;
            if ( type >= 14 )
              type = 0;
            if (m_targetCharset.empty())
              m_targetCharset = bgl_charset[type];
            break;
          default:
            break;
        }
        break;
      default:
        ;
    }
    if( block.length ) free( block.data );
  }
  gzseek( file, 0, SEEK_SET );

  if ( isUtf8File )
  {
    //FDPRINTF( stderr, "%s: utf8 file.\n", m_title.c_str() );
    m_defaultCharset = "UTF-8";
    m_sourceCharset = "UTF-8";
    m_targetCharset = "UTF-8";
  }

  convertToUtf8( m_title, TARGET_CHARSET );
  convertToUtf8( m_author, TARGET_CHARSET );
  convertToUtf8( m_email, TARGET_CHARSET );
  convertToUtf8( m_copyright, TARGET_CHARSET );
  convertToUtf8( m_description, TARGET_CHARSET );
  DPRINTF("Default charset: %s\nSource Charset: %s\nTargetCharset: %s\n", m_defaultCharset.c_str(), m_sourceCharset.c_str(), m_targetCharset.c_str());
  return true;
}


bgl_entry Babylon::readEntry( ResourceHandler * resourceHandler )
{
  bgl_entry entry;

  if( file == NULL )
  {
    entry.headword = "";
    return entry;
  }

  bgl_block block;
  unsigned int len, pos;
  unsigned int alts_num;
  std::string headword, displayedHeadword;
  std::string definition;
  std::string temp;
  std::vector<std::string> alternates;
  std::string alternate;
  std::string root;
  bool defBodyEnded = false;
  std::string transcription;

  while( readBlock( block ) )
  {
    switch( block.type )
    {
      case 2:
      {
        pos = 0;
        len = (unsigned char)block.data[pos++];
        if( pos + len > block.length )
          break;
        std::string filename( block.data+pos, len );
        //if (filename != "8EAF66FD.bmp" && filename != "C2EEF3F6.html") {
            pos += len;
        if ( resourceHandler )
          resourceHandler->handleBabylonResource( filename,
                                                  block.data + pos,
                                                  block.length - pos );
    #if 0
            FILE *ifile = gd_fopen(filename.c_str(), "w");
            fwrite(block.data + pos, 1, block.length -pos, ifile);
            fclose(ifile);
    #endif
        break;
      }
      case 1:
      case 7:
      case 10:
      case 11:
        alternate.clear();
        headword.clear();
        displayedHeadword.clear();
        root.clear();
        definition.clear();
        temp.clear();
        pos = 0;

        // Headword
        if( block.type == 11 )
        {
          pos = 1;
          len = 0;
          if( pos + 4 > block.length )
            break;
          for( int i = 0; i < 4; i++ )
          {
            len = len << 8;
            len |= (unsigned char)block.data[ pos++ ];
          }
        }
        else
        {
          len = (unsigned char)block.data[pos++];
        }

        if( pos + len > block.length )
          break;

        headword.reserve( len );
        for(unsigned int a=0;a<len;a++)
          headword += block.data[pos++];

        convertToUtf8( headword, SOURCE_CHARSET );

        // Try to repair malformed headwords
        if( headword.find( "&#" ) != string::npos )
          headword = Html::unescapeUtf8( headword );

        if( block.type == 11 )
        {
          // Alternate forms
          if( pos + 4 >= block.length )
            break;

          alts_num = 0;
          for( int i = 0; i < 4; i++ )
          {
            alts_num = alts_num << 8;
            alts_num |= (unsigned char)block.data[ pos++ ];
          }

          for( unsigned j = 0; j < alts_num; j++ )
          {
            len = 0;
            if( pos + 4 > block.length )
              break;
            for( int i = 0; i < 4; i++ )
            {
              len = len << 8;
              len |= (unsigned char)block.data[ pos++ ];
            }
            if( pos + len >= block.length )
              break;
            alternate.reserve( len );
            for(unsigned int a=0;a<len;a++) alternate += block.data[pos++];
            convertToUtf8( alternate, SOURCE_CHARSET );

            // Try to repair malformed forms
            if( alternate.find( "&#" ) != string::npos )
              alternate = Html::unescapeUtf8( alternate );

            alternates.push_back( alternate );
            alternate.clear();
          }
        }

        // Definition

        if( block.type == 11 )
        {
          len = 0;
          if( pos + 4 > block.length )
            break;
          for( int i = 0; i < 4; i++ )
          {
            len = len << 8;
            len |= (unsigned char)block.data[ pos++ ];
          }
        }
        else
        {
          len = (unsigned char)block.data[pos++] << 8;
          len |= (unsigned char)block.data[pos++];
        }

        if( pos + len > block.length )
          break;

        definition.reserve( len );

        for(unsigned int a=0;a<len;a++)
        {
          if( (unsigned char)block.data[pos] == 0x0a )
          {
            definition += "<br>";
            pos++;
          }
          else if ( (unsigned char)block.data[pos] == 6 )
          {
            // Something
            pos += 2;
            ++a;
            definition += " ";
          }
          else if ( (unsigned char)block.data[pos] >= 0x40 &&
                    len - a >= 2 &&
                    (unsigned char)block.data[pos + 1 ] == 0x18 )
          {
            // Hidden displayed headword (a displayed headword which
            // contains some garbage and shouldn't probably be visible).
            unsigned length = (unsigned char)block.data[ pos ] - 0x3F;

            if ( length > len - a - 2 )
            {
              FDPRINTF( stderr, "Hidden displayed headword is too large %s\n", headword.c_str() );
              pos += len - a;
              break;
            }

            pos += length + 2;
            a += length + 1;
          }
          else if ( (unsigned char)block.data[pos] == 0x18 )
          {
            // Displayed headword
              unsigned length = (unsigned char)block.data[ pos + 1 ];

            if ( length > len - a - 2 )
            {
              FDPRINTF( stderr, "Displayed headword's length is too large for headword %s\n", headword.c_str() );
              pos += len - a;
              break;
            }

            displayedHeadword = std::string( block.data + pos + 2, length );
            pos += length + 2;
            a += length + 1;
          }
          else
          if ( block.data[ pos ] == 0x28 && defBodyEnded &&
               len - a >= 3 )
          {
            // 2-byte sized displayed headword
            unsigned length = (unsigned char)block.data[ pos + 1 ];
            length <<= 8;
            length += (unsigned char)block.data[ pos + 2 ];

            if ( length > len - a - 3 )
            {
              FDPRINTF( stderr, "2-byte sized displayed headword for %s is too large\n", headword.c_str() );
              pos += len - a;
              break;
            }

            displayedHeadword = std::string( block.data + pos + 3, length );

            pos += length + 3;
            a += length + 2;
          }
          else if ( (unsigned char)block.data[pos] == 0x50 && len - a - 1 >= 2 &&
                    (unsigned char)block.data[pos + 1 ] == 0x1B )
          {
            // 1-byte-sized transcription
            unsigned length = (unsigned char)block.data[pos + 2 ];

            if ( length > len - a - 3 )
            {
              FDPRINTF( stderr, "1-byte-sized transcription's length is too large for headword %s\n", headword.c_str() );
              pos += len - a;
              break;
            }

            if( m_targetCharset.compare( "UTF-8" ) != 0 )
            {
              try
              {
                transcription = Iconv::toUtf8( "CP1252", block.data + pos + 3, length );
              }
              catch( Iconv::Ex & e )
              {
                qWarning() << "Bgl: charset convertion error, no trancription processing's done: " << e.what();
                transcription = std::string( block.data + pos + 3, length );
              }
            }
            else
              transcription = std::string( block.data + pos + 3, length );

            pos += length + 3;
            a += length + 2;
          }
          else if ( (unsigned char)block.data[pos] == 0x60 && len - a - 1 >= 3 &&
                    (unsigned char)block.data[pos + 1 ] == 0x1B )
          {
            // 2-byte-sized transcription
            unsigned length = (unsigned char)block.data[pos + 2 ];
            length <<= 8;
            length += (unsigned char)block.data[pos + 3 ];

            if ( length > len - a - 4)
            {
              FDPRINTF( stderr, "2-byte-sized transcription's length is too large for headword %s\n", headword.c_str() );
              pos += len - a;
              break;
            }

            if( m_targetCharset.compare( "UTF-8" ) != 0 )
            {
              try
              {
                transcription = Iconv::toUtf8( "CP1252", block.data + pos + 4, length );
              }
              catch( Iconv::Ex & e )
              {
                qWarning() << "Bgl: charset convertion error, no trancription processing's done: " << e.what();
                transcription = std::string( block.data + pos + 4, length );
              }
            }
            else
              transcription = std::string( block.data + pos + 4, length );

            pos += length + 4;
            a += length + 3;
          }
          else if ( (unsigned char)block.data[pos] >= 0x40 &&
                    len - a >= 2 &&
                    (unsigned char)block.data[pos + 1 ] == 0x1B )
          {
            // Hidden transcription (a transcription which is usually the same
            // as the headword and shouldn't probably be visible).
            unsigned length = (unsigned char)block.data[ pos ] - 0x3F;

            if ( length > len - a - 2 )
            {
              FDPRINTF( stderr, "Hidden transcription is too large %s\n", headword.c_str() );
              pos += len - a;
              break;
            }

            pos += length + 2;
            a += length + 1;
          }
          else if ( (unsigned char)block.data[pos] == 0x1E )
          {
            // Resource reference begin marker
            definition += m_resourcePrefix;
            ++pos;
          }
          else if ( (unsigned char)block.data[pos] == 0x1F )
          {
            // Resource reference end marker
            ++pos;
          }
          else if( (unsigned char)block.data[pos] < 0x20 )
          {
            if( a <= len - 3 && block.data[pos] == 0x14 && block.data[pos+1] == 0x02 ) {
              int index = (unsigned char)block.data[pos+2] - 0x30;
              if (index >= 0 && index <= 10) {
                definition = "<span class=\"bglpos\">" + partOfSpeech[index] + "</span> " + definition;
              }
              pos += 3;
              a += 2;
              //pos += len - a;
              //break;
            }
            else
            if (block.data[pos] == 0x14) {
              defBodyEnded = true; // Presumably
              pos++;
            } else if ((unsigned char)block.data[pos] == 0x1A){
                unsigned length = (unsigned char)block.data[ pos + 1 ];
                if (length <= 10){// 0x1A identifies two different data types.
                                  // data about the Hebrew root should be shorter then
                                  // 10 bytes, and in the other data type the byte
                          // after 0x1A is > 10 (at least it is in Bybylon's
                          // Hebrew dictionaries).   
                    root = std::string( block.data + pos + 2, length );
                    std::reverse(root.begin(),root.end());
                    definition += " (" + root + ")";
                    pos += length + 2;
                    a += length + 1;
               }
                else
                    pos++;
            } else {
                definition += block.data[pos++];
            }
          }else definition += block.data[pos++];
        }
        convertToUtf8( definition, TARGET_CHARSET );
        if( !transcription.empty() )
          definition = std::string( "<span class=\"bgltrn\">" ) +  transcription + "</span>" + definition;

        if ( displayedHeadword.size() )
          convertToUtf8( displayedHeadword, TARGET_CHARSET );

        // Alternate forms
        while( pos < block.length )
        {
          len = (unsigned char)block.data[pos++];
          if( pos + len > block.length ) break;
          alternate.reserve( len );
          for(unsigned int a=0;a<len;a++) alternate += block.data[pos++];
          convertToUtf8( alternate, SOURCE_CHARSET );

          // Try to repair malformed forms
          if( alternate.find( "&#" ) != string::npos )
            alternate = Html::unescapeUtf8( alternate );

          alternates.push_back( alternate );
          alternate.clear();
        }

        // Try adding displayed headword to the list of alts
        if ( headword != displayedHeadword )
        {
          // Only add displayed headword if the normal one has two or more digits.
          // This would indicate some irregularity in it (like e.g. if it serves
          // as some kind of an identifier instead of being an actual headword)
          int totalDigits = 0;

          for( char const * p = headword.c_str(); *p; ++p )
            if ( *p >= '0' && *p <= '9' )
            {
              if ( ++totalDigits > 1 )
                break;
            }

          if ( totalDigits > 1 )
          {
            // Ok, let's add it.

            // Does it contain HTML? If it does, we need to strip it
            if ( displayedHeadword.find( '<' ) != string::npos ||
                 displayedHeadword.find( '&' ) != string::npos )
            {
              string result = Html::unescapeUtf8( displayedHeadword );

              if ( result != headword )
                alternates.push_back( result );
            }
            else
              alternates.push_back(displayedHeadword);
          }
        }

        entry.headword = headword;

        entry.displayedHeadword = displayedHeadword;
        entry.definition = definition;

        entry.alternates = alternates;

        if( block.length ) free( block.data );

        // Some dictionaries can in fact have an empty headword, so we
        // make it non-empty here to differentiate between the end of entries.
        if ( entry.headword.empty() )
          entry.headword += ' ';

        return entry;

        break;
      default:
        ;
    }
    if( block.length ) free( block.data );
  }
  entry.headword = "";
  return entry;
}



void Babylon::convertToUtf8( std::string &s, unsigned int type )
{
  if( s.size() < 1 ) return;
  if( type > 2 ) return;

  if( s.compare( 0, 13, "<charset c=U>") == 0 )
      return;

  std::string charset;
  switch( type )
  {
    case DEFAULT_CHARSET:
      if(!m_defaultCharset.empty()) charset = m_defaultCharset;
      else charset = m_sourceCharset;
      break;
    case SOURCE_CHARSET:
      if(!m_sourceCharset.empty()) charset = m_sourceCharset;
      else charset = m_defaultCharset;
      break;
    case TARGET_CHARSET:
      if(!m_targetCharset.empty()) charset = m_targetCharset;
      else charset = m_defaultCharset;
      break;
    default:
      ;
  }

  if( charset == "UTF-8" )
    return;

  iconv_t cd = iconv_open( "UTF-8", charset.c_str() );
  if( cd == (iconv_t)(-1) )
  {
    qFatal( "Error openning iconv library" );
    exit(1);
  }

  char *outbuf, *defbuf;
  size_t inbufbytes, outbufbytes;

  inbufbytes = s.size();
  outbufbytes = s.size() * 6;

  char *inbuf;
  inbuf = (char *)s.data();
  outbuf = (char*)malloc( outbufbytes + 1 );
  memset( outbuf, '\0', outbufbytes + 1 );
  defbuf = outbuf;
  while (inbufbytes) {
    if (iconv(cd, &inbuf, &inbufbytes, &outbuf, &outbufbytes) == (size_t)-1) {
      qWarning() << "\"" << inbuf << "\" - error in iconv conversion";
      break;
//      inbuf++;
//      inbufbytes--;
    }
  }
  
  // Flush the state. This fixes CP1255 problems.
  iconv( cd, 0, 0, &outbuf, &outbufbytes );
  
  if( inbufbytes == 0 )
    s = std::string( defbuf );

  free( defbuf );
  iconv_close( cd );
}

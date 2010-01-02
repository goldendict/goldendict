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
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<iconv.h>
#include <QTextDocument>

#ifdef _WIN32
#include <io.h>
#define DUP _dup
#else
#define DUP dup
#endif

using std::string;

Babylon::Babylon( std::string filename )
{
  m_filename = filename;
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

  f = fopen( m_filename.c_str(), "rb" );
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

  file = gzdopen( DUP( fileno( f ) ), "r" );

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
    gzread( file, block.data, block.length );
  }

  return true;
}


unsigned int Babylon::bgl_readnum( int bytes )
{
  unsigned char buf[4];
  unsigned val = 0;

  if ( bytes < 1 || bytes > 4 ) return (0);

  gzread( file, buf, bytes );
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
      case 10:
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
            headword = bgl_language[(unsigned char)(block.data[5])];
            m_sourceLang = headword;
            break;
          case 8:
            headword = bgl_language[(unsigned char)(block.data[5])];
            m_targetLang = headword;
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
            if ( block.length >= 5 && (unsigned char) block.data[ 4 ] == 0x80 )
              isUtf8File = true;
          break;
          case 26:
            type = (unsigned int)block.data[2];
            if( type > 64 ) type -= 65;
            if ( type >= 14 )
              type = 0;
            if (m_sourceCharset.empty())
              m_sourceCharset = bgl_charset[type];
            break;
          case 27:
            type = (unsigned int)block.data[2];
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
    //fprintf( stderr, "%s: utf8 file.\n", m_title.c_str() );
    m_defaultCharset = "UTF-8";
    m_sourceCharset = "UTF-8";
    m_targetCharset = "UTF-8";
  }

  convertToUtf8( m_title, TARGET_CHARSET );
  convertToUtf8( m_author, TARGET_CHARSET );
  convertToUtf8( m_email, TARGET_CHARSET );
  convertToUtf8( m_copyright, TARGET_CHARSET );
  convertToUtf8( m_description, TARGET_CHARSET );
  printf("Default charset: %s\nSource Charset: %s\nTargetCharset: %s\n", m_defaultCharset.c_str(), m_sourceCharset.c_str(), m_targetCharset.c_str());
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
  std::string headword, displayedHeadword;
  std::string definition;
  std::string temp;
  std::vector<std::string> alternates;
  std::string alternate;
  std::string root;

  while( readBlock( block ) )
  {
    switch( block.type )
    {
      case 2:
      {
        pos = 0;
        len = (unsigned char)block.data[pos++];
        std::string filename( block.data+pos, len );
        //if (filename != "8EAF66FD.bmp" && filename != "C2EEF3F6.html") {
            pos += len;
        if ( resourceHandler )
          resourceHandler->handleBabylonResource( filename,
                                                  block.data + pos,
                                                  block.length - pos );
    #if 0
            FILE *ifile = fopen(filename.c_str(), "w");
            fwrite(block.data + pos, 1, block.length -pos, ifile);
            fclose(ifile);
    #endif
        break;
      }
      case 1:
      case 10:
        alternate.clear();
        headword.clear();
        displayedHeadword.clear();
        root.clear();
        definition.clear();
        temp.clear();
        pos = 0;

        // Headword
        len = 0;
        len = (unsigned char)block.data[pos++];

        headword.reserve( len );
        for(unsigned int a=0;a<len;a++)
          headword += block.data[pos++];

        convertToUtf8( headword, SOURCE_CHARSET );

        // Definition
        len = 0;
        len = (unsigned char)block.data[pos++] << 8;
        len |= (unsigned char)block.data[pos++];
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
              fprintf( stderr, "Hidden displayed headword is too large %s\n", headword.c_str() );
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
              fprintf( stderr, "Displayed headword's length is too large for headword %s\n", headword.c_str() );
              pos += len - a;
              break;
            }

            displayedHeadword = std::string( block.data + pos + 2, length );
            pos += length + 2;
            a += length + 1;
          }
          else if ( (unsigned char)block.data[pos] == 0x50 && len - a - 1 >= 2 &&
                    (unsigned char)block.data[pos + 1 ] == 0x1B )
          {
            // 1-byte-sized transcription
            unsigned length = (unsigned char)block.data[pos + 2 ];

            if ( length > len - a - 3 )
            {
              fprintf( stderr, "1-byte-sized transcription's length is too large for headword %s\n", headword.c_str() );
              pos += len - a;
              break;
            }

            std::string transcription( block.data + pos + 3, length );

            definition = std::string( "<span class=\"bgltrn\">" ) +  transcription + "</span>" + definition;

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
              fprintf( stderr, "2-byte-sized transcription's length is too large for headword %s\n", headword.c_str() );
              pos += len - a;
              break;
            }

            std::string transcription( block.data + pos + 4, length );

            definition = std::string( "<span class=\"bgltrn\">" ) +  transcription + "</span>" + definition;

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
              fprintf( stderr, "Hidden transcription is too large %s\n", headword.c_str() );
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
            } else if (block.data[pos] == 0x14) {
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

        if ( displayedHeadword.size() )
          convertToUtf8( displayedHeadword, SOURCE_CHARSET );

        // Alternate forms
        while( pos != block.length )
        {
          len = (unsigned char)block.data[pos++];
          alternate.reserve( len );
          for(unsigned int a=0;a<len;a++) alternate += block.data[pos++];
          convertToUtf8( alternate, SOURCE_CHARSET );
          alternates.push_back( alternate );
          alternate.clear();
        }

        // Try adding displayed headword to the list of alts
        if ( headword != displayedHeadword )
        {
          // Does it contain HTML? If it does, we need to strip it
          if ( displayedHeadword.find( '<' ) != string::npos ||
               displayedHeadword.find( '&' ) != string::npos )
          {
            QTextDocument d;
            d.setHtml( QString::fromUtf8( displayedHeadword.data(), displayedHeadword.size() ) );

            string result = d.toPlainText().toUtf8().data();

            if ( result != headword )
            alternates.push_back( result );
          }
          else
            alternates.push_back(displayedHeadword);
        }

        entry.headword = headword;

        entry.displayedHeadword = displayedHeadword;
        entry.definition = definition;

        entry.alternates = alternates;

        if( block.length ) free( block.data );

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

  iconv_t cd = iconv_open( "UTF-8", charset.c_str() );
  if( cd == (iconv_t)(-1) )
  {
    printf( "Error openning iconv library\n" );
    exit(1);
  }

  char *outbuf, *defbuf;
  size_t inbufbytes, outbufbytes;

  inbufbytes = s.size();
  outbufbytes = s.size() * 6;
#ifdef _WIN32
  const char *inbuf;
  inbuf = s.data();
#else
  char *inbuf;
  inbuf = (char *)s.data();
#endif
  outbuf = (char*)malloc( outbufbytes + 1 );
  memset( outbuf, '\0', outbufbytes + 1 );
  defbuf = outbuf;
  while (inbufbytes) {
    if (iconv(cd, &inbuf, &inbufbytes, &outbuf, &outbufbytes) == (size_t)-1) {
      printf( "\n%s\n", inbuf );
      printf( "Error in iconv conversion\n" );
      inbuf++;
      inbufbytes--;
    }
  }
  
  // Flush the state. This fixes CP1255 problems.
  iconv( cd, 0, 0, &outbuf, &outbufbytes );
  
  s = std::string( defbuf );

  free( defbuf );
  iconv_close( cd );
}

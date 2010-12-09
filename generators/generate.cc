/* This file is (c) 2008-2009 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

// This program generates the required .cc files to perform Unicode foldings,
// parsing the Unicode standards' text files.
// Normally, we issue rebuilds manually in case either the generator or
// the data files were updated to avoid the hurdles an end user might get
// into while compiling the program.

#include <stdio.h>
#include <map>
#include <string>

using std::map;
using std::wstring;

struct Node
{
  map< wchar_t, Node > nodes;
  wchar_t tail;

  Node(): tail( 0 )
  {}
};

void indent( FILE * outf, size_t steps )
{
  for( unsigned x = steps; x--; )
    fprintf( outf, "  " );
}

void handleForest( FILE * outf, map< wchar_t, Node > const & forest, size_t
                   prev, size_t steps )
{
  indent( outf, steps );
  fprintf( outf, "if ( size > %u )\n", prev );

  indent( outf, steps );
  fprintf( outf, "{\n" );

  indent( outf, steps + 1 );
  fprintf( outf, "switch( in[ %u ] )\n", prev );

  indent( outf, steps + 1 );
  fprintf( outf, "{\n" );

  for( map< wchar_t, Node >::const_iterator i = forest.begin();
       i != forest.end(); ++i )
  {
    indent( outf, steps + 2 );
    fprintf( outf, "case 0x%x:\n", i->first );

    if ( i->second.nodes.size() )
      handleForest( outf, i->second.nodes, prev + 1, steps + 3 );

    indent( outf, steps + 3 );

    if ( i->second.tail )
      fprintf( outf, "consumed = %u; return 0x%x;\n", prev + 1, i->second.tail );
    else
      fprintf( outf, "consumed = 1; return *in;\n" );
  }

  indent( outf, steps + 1 );
  fprintf( outf, "}\n" );

  indent( outf, steps );
  fprintf( outf, "}\n" );
}

int main()
{
  // Case folding
  {
    FILE * inf = fopen( "CaseFolding.txt", "r" );
  
    if ( !inf )
    {
      fprintf( stderr, "Failed to open CaseFolding.txt\n" );
  
      return 1;
    }
  
    char buf[ 4096 ];

    map< wchar_t, wstring > foldTable;
    map< wchar_t, wchar_t > simpleFoldTable;

    while( fgets( buf, sizeof( buf ), inf ) )
    {
      if ( *buf == '#' )
        continue; // A comment


      unsigned long in, out[ 4 ];
      char type;

      unsigned totalOut;

      if ( sscanf( buf, "%lx; %c; %lx %lx %lx %lx;", &in, &type,
                         out, out + 1, out + 2, out + 3 ) == 6 )
      {
        fprintf( stderr, "Four output chars ecountered in CaseFolding.txt, which we expected"
                         "the file didn't have, make changes into the program.\n" );

        return 1;
      }

      if ( sscanf( buf, "%lx; %c; %lx %lx %lx;", &in, &type,
                         out, out + 1, out + 2 ) == 5 )
        totalOut = 3;
      else
      if ( sscanf( buf, "%lx; %c; %lx %lx;", &in, &type,
                         out, out + 1 ) == 4 )
        totalOut = 2;
      else
      if ( sscanf( buf, "%lx; %c; %lx;", &in, &type,
                         out ) == 3 )
        totalOut = 1;
      else
      {
        fprintf( stderr, "Erroneous input in CaseFolding.txt: %s\n", buf );

        return 1;
      }

      switch( type )
      {
        case 'C':
          if ( totalOut != 1 )
          {
            fprintf( stderr, "C-record has more than one output char in CaseFolding.txt: %s\n", buf );

            return 1;
          }
          simpleFoldTable[ in ] = out[ 0 ];
          // fall-through
        case 'F':
        {
          wstring result( totalOut, 0 );

          for( unsigned x = 0; x < totalOut; ++x )
          {
            result[ x ] = out[ x ];
          }
          foldTable[ in ] = result;
        }
        break;
        case 'S':
        {
          if ( totalOut != 1 )
          {
            fprintf( stderr, "S-record has more than one output char in CaseFolding.txt: %s\n", buf );

            return 1;
          }
          simpleFoldTable[ in ] = out[ 0 ];
        }
        break;
        case 'T':
          // Ignore special foldings
        break;

        default:

          fprintf( stderr, "Unknown folding type encountered: %c\n", type );
          return 1;
      }
    }

    fclose( inf );

    // Create an outfile

    FILE * outf = fopen( "../inc_case_folding.hh", "w" );

    if ( !outf )
    {
      fprintf( stderr, "Failed to create outfile\n" );

      return 1;
    }

    fprintf( outf, "// This file was generated automatically. Do not edit directly.\n\n" );

    fprintf( outf, "enum { foldCaseMaxOut = 3 };\n\n" );
    fprintf( outf, "size_t foldCase( wchar in, wchar * out )\n{\n  switch( in )\n  {\n" );

    for( map< wchar_t, wstring >::const_iterator i = foldTable.begin();
         i != foldTable.end(); ++i )
    {
      if ( i->second.size() == 1 )
        fprintf( outf, "    case 0x%x: *out = 0x%x; return 1;\n", i->first, i->second[ 0 ] );
      else
      {
        fprintf( outf, "    case 0x%x: ", i->first );

        for( unsigned x = 0; x < i->second.size(); ++x )
          fprintf( outf, "out[ %u ] = 0x%x; ", x, i->second[ x ] );

        fprintf( outf, "return %u;\n", i->second.size() );
      }
    }

    fprintf( outf, "    default: *out = in; return 1;\n" );
    fprintf( outf, "  }\n}\n\n" );

    fprintf( outf, "wchar foldCaseSimple( wchar in )\n{\n  switch( in )\n  {\n" );

    for( map< wchar_t, wchar_t >::const_iterator i = simpleFoldTable.begin();
         i != simpleFoldTable.end(); ++i )
        fprintf( outf, "    case 0x%x: return 0x%x;\n", i->first, i->second );

    fprintf( outf, "    default: return in;\n" );
    fprintf( outf, "  }\n}\n" );

    fclose( outf );
  }

  // Diacritic folding
  {
    FILE * inf = fopen( "DiacriticFolding.txt", "r" );

    if ( !inf )
    {
      fprintf( stderr, "Failed to open DiacriticFolding.txt\n" );

      return 1;
    }

    char buf[ 4096 ];

    map< wchar_t, Node > forest;

    while( fgets( buf, sizeof( buf ), inf ) )
    {
      if ( *buf == '#' || *buf == '\n' )
        continue; // A comment or a whiteline

      unsigned long in[ 4 ], out;

      unsigned totalIn;

      if ( sscanf( buf, "%lx %lx %lx %lx; %lx", in, in + 1, in + 2, in + 3, &out ) == 5 )
      {
        fprintf( stderr, "Four input chars ecountered in DiacriticFolding.txt, which we expected"
                         "the file didn't have, make changes into the program.\n" );

        return 1;
      }
      else
      if ( sscanf( buf, "%lx %lx %lx; %lx", in, in + 1, in + 2, &out ) == 4 )
        totalIn = 3;
      else
      if ( sscanf( buf, "%lx %lx; %lx", in, in + 1, &out ) == 3 )
        totalIn = 2;
      else
      if ( sscanf( buf, "%lx; %lx", in, &out ) == 2 )
        totalIn = 1;
      else
      {
        fprintf( stderr, "Erroneous input in DiacriticFolding.txt: %s\n", buf );

        return 1;
      }

      map< wchar_t, Node > * cur = &forest;

      for( unsigned x = 0; x < totalIn - 1; ++x )
      {
        //printf( "%x ", in[ x ] );

        cur = &( (*cur)[ in[ x ] ].nodes );
      }

      //printf( "%x\n", in[ totalIn - 1 ] );

      if ( (*cur)[ in[ totalIn - 1 ] ].tail )
      {
        fprintf( stderr, "Ambiguity in DiacriticFolding.txt\n" );
        return 1;
      }

      (*cur)[ in[ totalIn - 1 ] ].tail = out;
    }
    fclose( inf );

     // Create an outfile

    FILE * outf = fopen( "../inc_diacritic_folding.hh", "w" );

    if ( !outf )
    {
      fprintf( stderr, "Failed to create outfile\n" );

      return 1;
    }

    fprintf( outf, "// This file was generated automatically. Do not edit directly.\n\n" );

    fprintf( outf, "enum { foldDiacriticMaxIn = 3 };\n\n" );
    fprintf( outf, "wchar foldDiacritic( wchar const * in, size_t size, size_t & consumed )\n{\n" );

    handleForest( outf, forest, 0, 1 );

    fprintf( outf, "  if ( size )\n"
                   "  {\n"
                   "    consumed = 1; return *in;\n"
                   "  }\n"
                   "  consumed = 0; return 0;\n"
                   "}\n" );

    fclose( outf );
 }

  return 0;
}

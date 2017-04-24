/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __INDEXEDZIP_HH_INCLUDED__
#define __INDEXEDZIP_HH_INCLUDED__

#include "btreeidx.hh"
#include <QFile>
#include "zipfile.hh"

/// Allows using a btree index to read zip files. Basically built on top of
/// the base dictionary infrastructure adapted for zips.
class IndexedZip: public BtreeIndexing::BtreeIndex
{
  ZipFile::SplitZipFile zip;
  bool zipIsOpen;

public:

  IndexedZip(): zipIsOpen( false )
  {}

  /// Opens the index. The values are those previously returned by buildIndex().
  using BtreeIndexing::BtreeIndex::openIndex;

  /// Opens the zip file itself. Returns true if succeeded, false otherwise.
  bool openZipFile( QString const & );

  /// Returns true if the zip is open, false otherwise.
  bool isOpen() const
  { return zipIsOpen; }

  /// Checks whether the given file exists in the zip file or not.
  /// Note that this function is thread-safe, since it does not access zip file.
  bool hasFile( gd::wstring const & name );

  /// Attempts loading the given file into the given vector. Returns true on
  /// success, false otherwise.
  bool loadFile( gd::wstring const & name, std::vector< char > & );
  bool loadFile( uint32_t offset, std::vector< char > & );

  /// Index compressed files in zip file
  bool indexFile( BtreeIndexing::IndexedWords &zipFileNames, quint32 * filesCount = 0 );
};

#endif

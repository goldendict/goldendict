/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SOURCES_HH_INCLUDED__
#define __SOURCES_HH_INCLUDED__

#include "ui_sources.h"
#include "config.hh"
#include <QAbstractItemModel>

/// A model to be projected into the mediawikis view, according to Qt's MVC model
class MediaWikisModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  MediaWikisModel( QWidget * parent, Config::MediaWikis const & );

  void removeWiki( int index );
  void addNewWiki();

  /// Returns the wikis the model currently has listed
  Config::MediaWikis const & getCurrentWikis() const
  { return mediawikis; }

  QModelIndex index( int row, int column, QModelIndex const & parent ) const;
  QModelIndex parent( QModelIndex const & parent ) const;
  Qt::ItemFlags flags( QModelIndex const & index ) const;
  int rowCount( QModelIndex const & parent ) const;
  int columnCount( QModelIndex const & parent ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool setData( QModelIndex const & index, const QVariant & value, int role );

private:

  Config::MediaWikis mediawikis;
};

/// A model to be projected into the paths view, according to Qt's MVC model
class PathsModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  PathsModel( QWidget * parent, Config::Paths const & );

  void removePath( int index );
  void addNewPath( QString const & );

  /// Returns the paths the model currently has listed
  Config::Paths const & getCurrentPaths() const
  { return paths; }

  QModelIndex index( int row, int column, QModelIndex const & parent ) const;
  QModelIndex parent( QModelIndex const & parent ) const;
  Qt::ItemFlags flags( QModelIndex const & index ) const;
  int rowCount( QModelIndex const & parent ) const;
  int columnCount( QModelIndex const & parent ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool setData( QModelIndex const & index, const QVariant & value, int role );

private:

  Config::Paths paths;
};

/// A model to be projected into the soundDirs view, according to Qt's MVC model
class SoundDirsModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  SoundDirsModel( QWidget * parent, Config::SoundDirs const & );

  void removeSoundDir( int index );
  void addNewSoundDir( QString const & path, QString const & name );

  /// Returns the soundDirs the model currently has listed
  Config::SoundDirs const & getCurrentSoundDirs() const
  { return soundDirs; }

  QModelIndex index( int row, int column, QModelIndex const & parent ) const;
  QModelIndex parent( QModelIndex const & parent ) const;
  Qt::ItemFlags flags( QModelIndex const & index ) const;
  int rowCount( QModelIndex const & parent ) const;
  int columnCount( QModelIndex const & parent ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool setData( QModelIndex const & index, const QVariant & value, int role );

private:

  Config::SoundDirs soundDirs;
};

class Sources: public QDialog
{
  Q_OBJECT

public:
  Sources( QWidget * parent, Config::Paths const &,
           Config::SoundDirs const &, Config::MediaWikis const & );

  Config::Paths const & getPaths() const
  { return pathsModel.getCurrentPaths(); }
  
  Config::SoundDirs const & getSoundDirs() const
  { return soundDirsModel.getCurrentSoundDirs(); }

  Config::MediaWikis const & getMediaWikis() const
  { return mediawikisModel.getCurrentWikis(); }

private:
  Ui::Sources ui;
  MediaWikisModel mediawikisModel;
  PathsModel pathsModel;
  SoundDirsModel soundDirsModel;

  void fitPathsColumns();
  void fitSoundDirsColumns();

private slots:

  void on_addPath_clicked();
  void on_removePath_clicked();
  
  void on_addSoundDir_clicked();
  void on_removeSoundDir_clicked();

  void on_addMediaWiki_clicked();
  void on_removeMediaWiki_clicked();
};

#endif

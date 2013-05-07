/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SOURCES_HH_INCLUDED__
#define __SOURCES_HH_INCLUDED__

#include "ui_sources.h"
#include "config.hh"
#include "hunspell.hh"
#include <QAbstractItemModel>
#include <QComboBox>
#include <QItemDelegate>
#include <QItemEditorFactory>

#if defined( Q_OS_WIN32 ) || defined( Q_OS_MACX )
#include "texttospeechsource.hh"
#endif

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

/// A model to be projected into the webSites view, according to Qt's MVC model
class WebSitesModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  WebSitesModel( QWidget * parent, Config::WebSites const & );

  void removeSite( int index );
  void addNewSite();

  /// Returns the sites the model currently has listed
  Config::WebSites const & getCurrentWebSites() const
  { return webSites; }

  QModelIndex index( int row, int column, QModelIndex const & parent ) const;
  QModelIndex parent( QModelIndex const & parent ) const;
  Qt::ItemFlags flags( QModelIndex const & index ) const;
  int rowCount( QModelIndex const & parent ) const;
  int columnCount( QModelIndex const & parent ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool setData( QModelIndex const & index, const QVariant & value, int role );

private:

  Config::WebSites webSites;
};

/// A model to be projected into the programs view, according to Qt's MVC model
class ProgramsModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  ProgramsModel( QWidget * parent, Config::Programs const & );

  void removeProgram( int index );
  void addNewProgram();

  /// Returns the sites the model currently has listed
  Config::Programs const & getCurrentPrograms() const
  { return programs; }

  QModelIndex index( int row, int column, QModelIndex const & parent ) const;
  QModelIndex parent( QModelIndex const & parent ) const;
  Qt::ItemFlags flags( QModelIndex const & index ) const;
  int rowCount( QModelIndex const & parent ) const;
  int columnCount( QModelIndex const & parent ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool setData( QModelIndex const & index, const QVariant & value, int role );

private:

  Config::Programs programs;
};

class ProgramTypeEditor: public QComboBox
{
Q_OBJECT
Q_PROPERTY(int type READ getType WRITE setType USER true)

public:
  ProgramTypeEditor( QWidget * widget = 0 );

  // Returns localized name for the given program type
  static QString getNameForType( int );

public:
  int getType() const;
  void setType( int );
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

/// A model to be projected into the hunspell dictionaries view, according to Qt's MVC model
class HunspellDictsModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  HunspellDictsModel( QWidget * parent, Config::Hunspell const & );

  void changePath( QString const & newPath );

  /// Returns the dictionaries currently enabled
  Config::Hunspell::Dictionaries const & getEnabledDictionaries() const
  { return enabledDictionaries; }

  QModelIndex index( int row, int column, QModelIndex const & parent ) const;
  QModelIndex parent( QModelIndex const & parent ) const;
  Qt::ItemFlags flags( QModelIndex const & index ) const;
  int rowCount( QModelIndex const & parent ) const;
  int columnCount( QModelIndex const & parent ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool setData( QModelIndex const & index, const QVariant & value, int role );

private:

  Config::Hunspell::Dictionaries enabledDictionaries;
  std::vector< HunspellMorpho::DataFiles > dataFiles;
};


class Sources: public QWidget
{
  Q_OBJECT

public:
  Sources( QWidget * parent, Config::Class const &);

  Config::Paths const & getPaths() const
  { return pathsModel.getCurrentPaths(); }

  Config::SoundDirs const & getSoundDirs() const
  { return soundDirsModel.getCurrentSoundDirs(); }

  Config::MediaWikis const & getMediaWikis() const
  { return mediawikisModel.getCurrentWikis(); }

  Config::WebSites const & getWebSites() const
  { return webSitesModel.getCurrentWebSites(); }

  Config::Programs const & getPrograms() const
  { return programsModel.getCurrentPrograms(); }

  Config::VoiceEngines getVoiceEngines() const;

  Config::Hunspell getHunspell() const;

  Config::Transliteration getTransliteration() const;

  Config::Forvo getForvo() const;

signals:

  /// Emitted when a 'Rescan' button is clicked.
  void rescan();

private:
  Ui::Sources ui;

#if defined( Q_OS_WIN32 ) || defined( Q_OS_MACX )
  TextToSpeechSource *textToSpeechSource;
#endif

  QItemDelegate * itemDelegate;
  QItemEditorFactory * itemEditorFactory;

  MediaWikisModel mediawikisModel;
  WebSitesModel webSitesModel;
  ProgramsModel programsModel;
  PathsModel pathsModel;
  SoundDirsModel soundDirsModel;
  HunspellDictsModel hunspellDictsModel;

  void fitPathsColumns();
  void fitSoundDirsColumns();
  void fitHunspellDictsColumns();

private slots:

  void on_addPath_clicked();
  void on_removePath_clicked();

  void on_addSoundDir_clicked();
  void on_removeSoundDir_clicked();

  void on_changeHunspellPath_clicked();

  void on_addMediaWiki_clicked();
  void on_removeMediaWiki_clicked();

  void on_addWebSite_clicked();
  void on_removeWebSite_clicked();

  void on_addProgram_clicked();
  void on_removeProgram_clicked();

  void on_rescan_clicked();
};

#endif

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


class Sources: public QDialog
{
  Q_OBJECT

public:
  Sources( QWidget * parent, Config::Paths const &, Config::MediaWikis const & );

  Config::Paths const & getPaths() const
  { return paths; }

  Config::MediaWikis const & getMediaWikis() const
  { return mediawikisModel.getCurrentWikis(); }

private:
  Ui::Sources ui;
  MediaWikisModel mediawikisModel;
  Config::Paths paths;

private slots:

  void add();
  void remove();

  void on_addMediaWiki_clicked();
  void on_removeMediaWiki_clicked();
};

#endif

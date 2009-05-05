/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __EDITDICTIONARIES_HH_INCLUDED__
#define __EDITDICTIONARIES_HH_INCLUDED__

#include "dictionary.hh"
#include "config.hh"
#include "ui_editdictionaries.h"
#include "sources.hh"
#include "groups.hh"
#include <QNetworkAccessManager>

class EditDictionaries: public QDialog
{
  Q_OBJECT

public:

  EditDictionaries( QWidget * parent, Config::Class & cfg,
                    std::vector< sptr< Dictionary::Class > > & dictionaries,
                    QNetworkAccessManager & dictNetMgr );

  /// Returns true if any changes to the 'dictionaries' vector passed were done.
  bool areDictionariesChanged() const
  { return dictionariesChanged; }

  /// Returns true if groups were changed.
  bool areGroupsChanged() const
  { return groupsChanged; }

protected:

  virtual void accept();
  
private slots:

  void on_tabs_currentChanged( int index );

  void rescanSources();
  
private:

  bool isSourcesChanged() const;

  void acceptChangedSources();
  
private:
   
  Config::Class & cfg;
  std::vector< sptr< Dictionary::Class > > & dictionaries;
  QNetworkAccessManager & dictNetMgr;
  
  // Backed up to decide later if something was changed or not
  Config::Class origCfg;

  Ui::EditDictionaries ui;
  Sources sources;
  sptr< Groups > groups;

  bool dictionariesChanged;
  bool groupsChanged;
  
  int lastCurrentTab;  
};

#endif

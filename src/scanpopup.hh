/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SCANPOPUP_HH_INCLUDED__
#define __SCANPOPUP_HH_INCLUDED__

#include "article_netmgr.hh"
#include "ui_scanpopup.h"
#include <QDialog>
#include <QClipboard>

class ScanPopup: public QDialog
{
  Q_OBJECT

public:

  ScanPopup( QWidget * parent, ArticleNetworkAccessManager & articleNetMgr );

private:

  ArticleNetworkAccessManager & articleNetMgr;
  Ui::ScanPopup ui;

private slots:

  void clipboardChanged( QClipboard::Mode );
};

#endif

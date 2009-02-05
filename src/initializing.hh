/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __INITIALIZING_HH_INCLUDED__
#define __INITIALIZING_HH_INCLUDED__

#include <QDialog>
#include "ui_initializing.h"

class Initializing: public QDialog
{
  Q_OBJECT

public:

  Initializing( QWidget * parent );

  void indexing( QString const & dictionaryName );

private:

  Ui::Initializing ui;
};

#endif


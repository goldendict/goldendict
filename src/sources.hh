/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SOURCES_HH_INCLUDED__
#define __SOURCES_HH_INCLUDED__

#include "ui_sources.h"
#include "config.hh"

class Sources: public QDialog
{
  Q_OBJECT

public:
  Sources( QWidget * parent, Config::Paths const & );

  Config::Paths const & getPaths() const
  { return paths; }

private:
  Ui::Sources ui;
  Config::Paths paths;

private slots:

  void add();
  void remove();
};

#endif

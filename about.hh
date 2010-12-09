/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef ABOUT_HH
#define ABOUT_HH

#include <QDialog>
#include "ui_about.h"

class About: public QDialog
{
  Q_OBJECT
public:

  About( QWidget * parent = 0 );

private:

  Ui::About ui;
};

#endif // ABOUT_HH

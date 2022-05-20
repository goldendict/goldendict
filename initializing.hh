/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __INITIALIZING_HH_INCLUDED__
#define __INITIALIZING_HH_INCLUDED__

#include <QDialog>
#include "ui_initializing.h"

#if defined( Q_OS_WIN32 )

#include <QtWidgets/QStyleFactory>

class WindowsStyle {
public:
  /// The class is a singleton.
  static WindowsStyle & instance();

  QStyle * getStyle()
  { return style; }

private:
  WindowsStyle();

  QStyle * style;
};

#endif

class Initializing: public QDialog
{
  Q_OBJECT

public:

  Initializing( QWidget * parent, bool showOnStartup );
#if defined( Q_OS_WIN32 )
  ~Initializing();
#endif

public slots:

  void indexing( QString const & dictionaryName );

private:

  virtual void closeEvent( QCloseEvent * );
  virtual void reject();

  Ui::Initializing ui;
};

#endif


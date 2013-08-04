/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef ABOUT_HH
#define ABOUT_HH

#include "ui_about.h"
#include <QDialog>

// Microsoft Visual C++ version
#if defined (_MSC_VER)
   // how many digits does the build number have?
#  if _MSC_FULL_VER / 10000 == _MSC_VER
#    define GD_CXX_MSVC_BUILD (_MSC_FULL_VER % 10000)  // four digits
#  elif _MSC_FULL_VER / 100000 == _MSC_VER
#    define GD_CXX_MSVC_BUILD (_MSC_FULL_VER % 100000) // five digits
#  else
#    define GD_CXX_MSVC_BUILD 0
#  endif
#  define GD_CXX_MSVC_MAJOR (_MSC_VER/100-6)
#  define GD_CXX_MSVC_MINOR (_MSC_VER%100)
#endif

class About: public QDialog
{
  Q_OBJECT
public:

  About( QWidget * parent = 0 );

private:

  Ui::About ui;
};

#endif // ABOUT_HH

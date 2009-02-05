#ifndef __PREFERENCES_HH_INCLUDED__
#define __PREFERENCES_HH_INCLUDED__

#include <QDialog>
#include "config.hh"
#include "ui_preferences.h"

/// Preferences dialog -- allows changing various program options.
class Preferences: public QDialog
{
  Q_OBJECT

public:

  Preferences( QWidget * parent, Config::Preferences const & );

  Config::Preferences getPreferences();

private:

  Ui::Preferences ui;

private slots:

  void enableScanPopupToggled( bool );
  void enableScanPopupModifiersToggled( bool );

  void wholeAltClicked( bool );
  void wholeCtrlClicked( bool );
  void wholeShiftClicked( bool );

  void sideAltClicked( bool );
  void sideCtrlClicked( bool );
  void sideShiftClicked( bool );
};

#endif


#ifndef DICTINFO_HH
#define DICTINFO_HH

#include <QDialog>
#include "ui_dictinfo.h"
#include "dictionary.hh"
#include "config.hh"

class DictInfo: public QDialog
{
  Q_OBJECT
public:

  DictInfo( Config::Class &cfg_, QWidget * parent = 0 );
  void showInfo( sptr< Dictionary::Class > dict );

private:
  Ui::DictInfo ui;
  Config::Class &cfg;
private slots:
  void savePos( int );
};

#endif // DICTINFO_HH

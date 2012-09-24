#ifndef DICTINFO_HH
#define DICTINFO_HH

#include <QDialog>
#include "ui_dictinfo.h"
#include "dictionary.hh"

class DictInfo: public QDialog
{
  Q_OBJECT
public:

  DictInfo( QWidget * parent = 0 );
  void showInfo( sptr< Dictionary::Class > dict );

private:
  Ui::DictInfo ui;
};

#endif // DICTINFO_HH

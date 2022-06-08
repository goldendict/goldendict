#ifndef DICTINFO_HH
#define DICTINFO_HH

#include <QDialog>
#include "ui_dictinfo.h"
#include "dict/dictionary.hh"
#include "config.hh"

class DictInfo: public QDialog
{
  Q_OBJECT
public:

  enum Actions
  {
    REJECTED,
    ACCEPTED,
    OPEN_FOLDER,
    EDIT_DICTIONARY,
    SHOW_HEADWORDS
  };

  DictInfo( Config::Class &cfg_, QWidget * parent = 0 );
  void showInfo( sptr< Dictionary::Class > dict );

private:
  Ui::DictInfo ui;
  Config::Class &cfg;
private slots:
  void savePos( int );
  void on_editDictionary_clicked();
  void on_openFolder_clicked();
  void on_OKButton_clicked();
  void on_headwordsButton_clicked();
};

#endif // DICTINFO_HH

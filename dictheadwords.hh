#ifndef __DICTHEADWORDS_H_INCLUDED__
#define __DICTHEADWORDS_H_INCLUDED__

#include <QDialog>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QSortFilterProxyModel>

#include "config.hh"
#include "ui_dictheadwords.h"
#include "dictionary.hh"
#include "delegate.hh"

class DictHeadwords : public QDialog
{
  Q_OBJECT

public:
  explicit DictHeadwords( QWidget * parent, Config::Class & cfg_,
                        Dictionary::Class * dict_ );
  virtual ~DictHeadwords();

  void setup( Dictionary::Class * dict_ );

protected:
  Config::Class & cfg;
  Dictionary::Class * dict;
  QStringList headers;
  QStringListModel * model;
  QSortFilterProxyModel * proxy;
  WordListItemDelegate * delegate;

  void saveHeadersToFile();

private:
  Ui::DictHeadwords ui;
private slots:
  void savePos();
  void filterChangedInternal();
  void filterChanged();
  void exportButtonClicked();
  void okButtonClicked();
  void itemClicked( const QModelIndex & index );
  void autoApplyStateChanged( int state );
  void showHeadwordsNumber();
  virtual void reject();

signals:
  void headwordSelected( QString const & );
  void closeDialog();
};

#endif // __DICTHEADWORDS_H_INCLUDED__

/* This file is (c) 2013 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef WORDLIST_HH
#define WORDLIST_HH

#include <QListWidget>
#include <QLineEdit>

#include "wordfinder.hh"
#include "delegate.hh"

class WordList : public QListWidget
{
  Q_OBJECT
public:
  explicit WordList(QWidget * parent = 0);
  void attachFinder(WordFinder * finder);
  virtual void setTranslateLine(QLineEdit * line)
  { translateLine = line; }

signals:
  void statusBarMessage(QString const & message, int timeout = 0, QPixmap const & pixmap = QPixmap());
  void contentChanged();

public slots:

private slots:
  void prefixMatchUpdated();
  void prefixMatchFinished();
  void updateMatchResults( bool finished );

private:
  void refreshTranslateLine();

  WordFinder * wordFinder;
  QLineEdit * translateLine;
  WordListItemDelegate listItemDelegate;
};

#endif // WORDLIST_HH

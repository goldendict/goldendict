/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef TRANSLATEBOX_HH
#define TRANSLATEBOX_HH

#include <extlineedit.hh>

#include <QWidget>
#include <QListWidget>
#include <QFocusEvent>

class CompletionList : public QListWidget
{
  Q_OBJECT

public:
  CompletionList(QWidget *parent = 0);
  int preferredHeight() const;

#if defined(Q_OS_WIN)
  void focusOutEvent (QFocusEvent * event)  {
    if (event->reason() == Qt::ActiveWindowFocusReason)
      hide();
  }
#endif

public slots:
  bool acceptCurrentEntry();
};

class TranslateBox : public QWidget
{
  Q_OBJECT

public:
  explicit TranslateBox(QWidget * parent = 0);
  void setPlaceholderText(const QString &text);
  QLineEdit * translateLine();
  QListWidget * wordList();

signals:

public slots:

private slots:
  void showPopup();
  void rightButtonClicked();

private:
  bool eventFilter(QObject *obj, QEvent *event);
  CompletionList * word_list;
  ExtLineEdit * translate_line;
  // QCompleter * completer; // disabled for now
};

#endif // TRANSLATEBOX_HH

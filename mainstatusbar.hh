/* This file is (c) 2011 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef MAINSTATUSBAR_HH
#define MAINSTATUSBAR_HH

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QTimer>

class MainStatusBar : public QFrame
{
  Q_OBJECT

public:
  explicit MainStatusBar(QWidget * parent);
  QString currentMessage() const;

signals:

public slots:
  void showMessage(const QString & text, int timeout = 0);
  void clearMessage();

protected:
  virtual void mousePressEvent(QMouseEvent * event);

private:
  QLabel * label;
  QTimer * timer;
  QString message;
  bool eventFilter(QObject *obj, QEvent * event);
  void refresh(bool forceHide = false);
};

#endif // MAINSTATUSBAR_HH

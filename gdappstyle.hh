/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef GDAPPSTYLE_HH
#define GDAPPSTYLE_HH

#include <QtGlobal>

#if QT_VERSION >= 0x040600

#include <QStyle>
#include <QProxyStyle>
#include <QStyleOption>

class GdAppStyle : public QProxyStyle
{
    Q_OBJECT

public:
  explicit GdAppStyle(QProxyStyle *style = 0);
  virtual int pixelMetric ( PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const;

signals:

public slots:

private:
  /// is this widget a tool button on the dictionary bar?
  bool dictionaryBarButton(const QWidget * widget) const;

};

#endif // QT_VERSION

#endif // GDAPPSTYLE_HH

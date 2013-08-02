/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "gdappstyle.hh"

#if QT_VERSION >= 0x040600

#include "dictionarybar.hh"

#include <QWidget>
#include <QToolButton>
#include <QDebug>

GdAppStyle::GdAppStyle(QProxyStyle * parent) : QProxyStyle(parent) {}

int GdAppStyle::pixelMetric ( PixelMetric metric, const QStyleOption * option, const QWidget * widget) const
{
  int defaultVal = QProxyStyle::pixelMetric(metric, option, widget);

  if ( dictionaryBarButton( widget ) )
  {
    if ( metric == QStyle::PM_ButtonShiftVertical || metric == QStyle::PM_ButtonShiftHorizontal )
    {
      if (option ->state & State_Sunken ) {
        return defaultVal;
      }

      if ( option ->state & State_On ) {
        // No shift for for the checked tool buttons on the dictionary bar,
        // that's why the whole thing with QProxyStyle is neded, to achieve this.
        return 0;
      }
    }
  }

  return defaultVal;
}

bool GdAppStyle::dictionaryBarButton(const QWidget * widget) const {
  if (widget) {
    const QWidget * parent = widget->parentWidget();
    if ( parent &&
         qobject_cast<const DictionaryBar *>( parent ) &&
         qobject_cast<const QToolButton *>( widget ) )
      return true;
  }

  return false;
}

#endif // QT_VERSION

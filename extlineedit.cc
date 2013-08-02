/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "extlineedit.hh"

#include <QPainter>

#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

ExtLineEdit::ExtLineEdit(QWidget *parent) :
    QLineEdit(parent)
{

  for (int i = 0; i < 2; ++i) {
      iconButtons[i] = new IconButton(this);
      iconButtons[i]->installEventFilter(this);
      iconButtons[i]->hide();
      iconButtons[i]->setAutoHide(false);
      iconEnabled[i] = false;
  }

    ensurePolished();
    updateMargins();

    connect(iconButtons[Left], SIGNAL(clicked()), this, SLOT(iconClicked()));
    connect(iconButtons[Right], SIGNAL(clicked()), this, SLOT(iconClicked()));

    connect(this, SIGNAL( textChanged( QString ) ), this, SLOT( updateButtons( QString ) ) );
}

ExtLineEdit::~ExtLineEdit()
{
}

void ExtLineEdit::setButtonVisible(Side side, bool visible)
{
    iconButtons[side]->setVisible(visible);
    iconEnabled[side] = visible;
    updateMargins();
}

bool ExtLineEdit::isButtonVisible(Side side) const
{
    return iconEnabled[side];
}

void ExtLineEdit::setButtonAutoHide(Side side, bool autohide)
{
    iconButtons[side]->setAutoHide(autohide);

    if (autohide)
    {
      iconButtons[side]->setOpacity( text().isEmpty() ? 0.0 : 1.0 );
    }
    else
    {
      iconButtons[side]->setOpacity( 1.0 );
    }
}

void ExtLineEdit::updateButtons(QString text)
{
  if ( oldText.isEmpty() || text.isEmpty() ) {
    for (int i = 0; i < 2; ++i) {
      if ( iconButtons[i]->isAutoHide() )
      {
       iconButtons[i]->animate( !text.isEmpty() );
      }
    }
    oldText = text;
  }
}

void ExtLineEdit::iconClicked()
{
    IconButton * button = qobject_cast<IconButton *>( sender() );
    int index = -1;
    for (int i = 0; i < 2; ++i)
        if (iconButtons[i] == button)
            index = i;

    if (index == -1)
        return;

    if (index == Left)
      emit leftButtonClicked();
    else if (index == Right)
      emit rightButtonClicked();
}

void ExtLineEdit::updateMargins()
{
    bool leftToRight = (layoutDirection() == Qt::LeftToRight);
    Side realLeft = (leftToRight ? Left : Right);
    Side realRight = (leftToRight ? Right : Left);

    int leftMargin = iconButtons[realLeft]->pixmap().width() + 8;
    int rightMargin = iconButtons[realRight]->pixmap().width() + 8;

    setTextMargins(
            (iconEnabled[realLeft] ? leftMargin : 0), 1,
            (iconEnabled[realRight] ? rightMargin : 0), 1);
}

void ExtLineEdit::updateButtonPositions()
{
    QRect contentRect = rect();
    for (int i = 0; i < 2; ++i) {
        Side iconPos = (Side)i;
        if (layoutDirection() == Qt::RightToLeft)
            iconPos = (iconPos == Left ? Right : Left);

        if (iconPos == ExtLineEdit::Right) {
            int right;
            getTextMargins(0, 0, &right, 0);
            const int iconoffset = right + 4;
            iconButtons[i]->setGeometry(contentRect.adjusted(width() - iconoffset, 0, 0, 0));
        } else {
            int left;
            getTextMargins(&left, 0, 0, 0);
            const int iconoffset = left + 4;
            iconButtons[i]->setGeometry(contentRect.adjusted(0, 0, -width() + iconoffset, 0));
        }
    }
}

void ExtLineEdit::resizeEvent(QResizeEvent *)
{
    updateButtonPositions();
}

void ExtLineEdit::setButtonPixmap(Side side, const QPixmap &buttonPixmap)
{
    iconButtons[side]->setPixmap(buttonPixmap);
    updateMargins();
    updateButtonPositions();
    update();
}

QPixmap ExtLineEdit::buttonPixmap(Side side) const
{
    return pixmaps[side];
}

void ExtLineEdit::setButtonToolTip(Side side, const QString &tip)
{
    iconButtons[side]->setToolTip(tip);
}

void ExtLineEdit::setButtonFocusPolicy(Side side, Qt::FocusPolicy policy)
{
    iconButtons[side]->setFocusPolicy(policy);
}

IconButton::IconButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
    setFocusProxy(parent);
}

void IconButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QRect pixmapRect = QRect(0, 0, m_pixmap.width(), m_pixmap.height());
    pixmapRect.moveCenter(rect().center());

    if (m_autohide)
    {
      painter.setOpacity(m_opacity);
    }

    painter.drawPixmap(pixmapRect, m_pixmap);
}

void IconButton::animate(bool visible)
{
#if QT_VERSION >= 0x040600
  QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
  animation->setDuration(250);
  if (visible)
  {
    animation->setEndValue(1.0);
  }
  else
  {
    animation->setEndValue(0.0);
  }
  animation->start(QAbstractAnimation::DeleteWhenStopped);
#else
  setOpacity(visible ? 1.0 : 0.0);
#endif
}

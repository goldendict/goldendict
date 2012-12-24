/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "extlineedit.hh"

#include <QPainter>

ExtLineEdit::ExtLineEdit(QWidget *parent) :
    QLineEdit(parent)
{

  for (int i = 0; i < 2; ++i) {
      iconButtons[i] = new IconButton(parent);
      iconButtons[i]->installEventFilter(this);
      iconButtons[i]->hide();
      iconEnabled[i] = false;
  }

    ensurePolished();
    updateMargins();

    connect(iconButtons[Left], SIGNAL(clicked()), this, SLOT(iconClicked()));
    connect(iconButtons[Right], SIGNAL(clicked()), this, SLOT(iconClicked()));
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

    QMargins margins((iconEnabled[realLeft] ? leftMargin : 0), 0,
                     (iconEnabled[realRight] ? rightMargin : 0), 0);

    setTextMargins(margins);
}

void ExtLineEdit::updateButtonPositions()
{
    QRect contentRect = rect();
    for (int i = 0; i < 2; ++i) {
        Side iconPos = (Side)i;
        if (layoutDirection() == Qt::RightToLeft)
            iconPos = (iconPos == Left ? Right : Left);

        if (iconPos == ExtLineEdit::Right) {
            const int iconoffset = textMargins().right() + 4;
            iconButtons[i]->setGeometry(contentRect.adjusted(width() - iconoffset, 0, 0, 0));
        } else {
            const int iconoffset = textMargins().left() + 4;
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
}

void IconButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QIcon::Mode state = QIcon::Disabled;
    if (isEnabled())
        state = isDown() ? QIcon::Selected : QIcon::Normal;
    QRect pixmapRect = QRect(0, 0, m_pixmap.width(), m_pixmap.height());
    pixmapRect.moveCenter(rect().center());

    painter.drawPixmap(pixmapRect, m_pixmap);
}

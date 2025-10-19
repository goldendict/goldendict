/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef EXTLINEEDIT_H
#define EXTLINEEDIT_H

#include <QLineEdit>
#include <QAbstractButton>

class IconButton: public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)

public:
    explicit IconButton(QWidget * parent = 0);
    void paintEvent(QPaintEvent * event);
    void animate(bool visible);

    void setPixmap(const QPixmap & pixmap) { m_pixmap = pixmap; update(); }
    QPixmap pixmap() const { return m_pixmap; }

    void setAutoHide(bool autohide) { m_autohide = autohide; update(); }
    bool isAutoHide() const { return m_autohide; }

    float opacity() { return m_opacity; }
    void setOpacity(float opacity) { m_opacity = opacity; update(); }

private:
    QPixmap m_pixmap;
    bool m_autohide;
    float m_opacity;
};

class ExtLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_ENUMS(Side)

public:
    static QPixmap scaledForButtonPixmap( QPixmap const & pixmap );

    enum Side { Left = 0, Right = 1 };

    explicit ExtLineEdit(QWidget * parent = 0);
    ~ExtLineEdit();

    void setButtonPixmap(Side side, const QPixmap &pixmap);

    void setButtonVisible(Side side, bool visible);
    bool isButtonVisible(Side side) const;

    void setButtonToolTip(Side side, const QString &);
    void setButtonFocusPolicy(Side side, Qt::FocusPolicy policy);

    void setButtonAutoHide(Side side, bool autohide);

    /// @param pixmap a pixmap that may be intermittently displayed in place of the left button pixmap.
    /// @pre pixmap.size() equals the size of the left button pixmap.
    /// @warning Do not change the size of the left button pixmap after setting an alternative left pixmap.
    void setAlternativeLeftPixmap( QPixmap const & pixmap );
    /// Shows the alternative left pixmap in place of the left button pixmap if @p visible;
    /// shows the left button pixmap in its own place (the default until this function is called) otherwise.
    /// @pre The alternative left pixmap has been set and is not null.
    void setAlternativeLeftPixmapVisible( bool visible );

signals:
    void leftButtonClicked();
    void rightButtonClicked();

private slots:
    void iconClicked();
    void updateButtons(QString text);

protected:
    virtual void resizeEvent( QResizeEvent * e );

private:
    void updateMargins();
    void updateButtonPositions();
    IconButton * iconButtons[2];
    bool iconEnabled[2];
    bool altLeftPixmapVisible;
    QPixmap altLeftPixmap;
    QString oldText;
};

#endif // EXTLINEEDIT_H

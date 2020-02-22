/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef TRANSLATEBOX_HH
#define TRANSLATEBOX_HH

#include "wordlist.hh"
#include "mutex.hh"
#include <QLineEdit>

class TranslateBox;
class ExtLineEdit;

class CompletionList : public WordList
{
    Q_OBJECT

public:
    CompletionList(TranslateBox * parent);
    int preferredHeight() const;
    virtual void setTranslateLine(QLineEdit * line)
    {
        WordList::setTranslateLine( line );
        setFocusProxy( line );
    }

public slots:
    bool acceptCurrentEntry();

private:
    virtual bool eventFilter( QObject *, QEvent * );
    TranslateBox * translateBox;
};

class TranslateBox : public QWidget
{
    Q_OBJECT

public:
    explicit TranslateBox(QWidget * parent = 0);
    void setPlaceholderText(const QString &text);
    QLineEdit * translateLine();
    WordList * wordList();
    void setText(QString text, bool showPopup=true);
    void setSizePolicy(QSizePolicy policy);
    inline void setSizePolicy(QSizePolicy::Policy hor, QSizePolicy::Policy ver)
    { setSizePolicy(QSizePolicy(hor, ver)); }

signals:

public slots:
    void setPopupEnabled(bool enable);

private slots:
    void showPopup();
    void rightButtonClicked();
    void onTextEdit();

private:
    bool eventFilter(QObject *obj, QEvent *event);
    CompletionList * word_list;
    ExtLineEdit * translate_line;
    bool m_popupEnabled;
    Mutex translateBoxMutex;
    // QCompleter * completer; // disabled for now
};

#endif // TRANSLATEBOX_HH

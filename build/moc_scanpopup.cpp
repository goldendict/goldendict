/****************************************************************************
** Meta object code from reading C++ file 'scanpopup.hh'
**
** Created: Wed Nov 19 16:06:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../scanpopup.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scanpopup.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ScanPopup[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      38,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   11,   10,   10, 0x05,
      44,   39,   10,   10, 0x05,
      74,   10,   10,   10, 0x05,
      93,   86,   10,   10, 0x05,
     113,   86,   10,   10, 0x05,
     137,   10,   10,   10, 0x05,
     156,   39,   10,   10, 0x05,
     187,   11,   10,   10, 0x05,
     215,   11,   10,   10, 0x05,
     245,   39,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     272,   10,   10,   10, 0x0a,
     293,   10,   10,   10, 0x0a,
     322,   10,   10,   10, 0x0a,
     351,   10,   10,   10, 0x0a,
     372,   10,   10,   10, 0x08,
     419,  407,   10,   10, 0x08,
     446,   10,   10,   10, 0x08,
     475,   10,   10,   10, 0x08,
     497,   10,   10,   10, 0x08,
     534,  526,   10,   10, 0x08,
     557,  526,   10,   10, 0x08,
     595,  592,   10,   10, 0x08,
     637,   10,   10,   10, 0x08,
     665,   10,   10,   10, 0x08,
     691,   10,   10,   10, 0x08,
     720,   10,   10,   10, 0x08,
     739,   10,   10,   10, 0x08,
     756,   10,   10,   10, 0x08,
     770,   10,   10,   10, 0x08,
     786,   10,   10,   10, 0x08,
     811,   10,   10,   10, 0x08,
     827,   10,   10,   10, 0x08,
     854,   10,   10,   10, 0x08,
     891,  886,   10,   10, 0x08,
     922,   10,   10,   10, 0x08,
     947,   10,   10,   10, 0x08,
     987,   10,   10,   10, 0x08,
    1008,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ScanPopup[] = {
    "ScanPopup\0\0id\0editGroupRequested(uint)\0"
    "word\0sendWordToMainWindow(QString)\0"
    "closeMenu()\0expand\0setExpandMode(bool)\0"
    "setViewExpandMode(bool)\0switchExpandMode()\0"
    "forceAddWordToHistory(QString)\0"
    "showDictionaryInfo(QString)\0"
    "openDictionaryFolder(QString)\0"
    "sendWordToHistory(QString)\0"
    "requestWindowFocus()\0translateWordFromClipboard()\0"
    "translateWordFromSelection()\0"
    "editGroupRequested()\0"
    "clipboardChanged(QClipboard::Mode)\0"
    ",forcePopup\0mouseHovered(QString,bool)\0"
    "currentGroupChanged(QString)\0"
    "prefixMatchFinished()\0"
    "on_pronounceButton_clicked()\0checked\0"
    "pinButtonClicked(bool)\0"
    "on_showDictionaryBar_clicked(bool)\0"
    ",,\0showStatusBarMessage(QString,int,QPixmap)\0"
    "on_sendWordButton_clicked()\0"
    "on_goBackButton_clicked()\0"
    "on_goForwardButton_clicked()\0"
    "hideTimerExpired()\0altModeExpired()\0"
    "altModePoll()\0mouseGrabPoll()\0"
    "pageLoaded(ArticleView*)\0escapePressed()\0"
    "mutedDictionariesChanged()\0"
    "switchExpandOptionalPartsMode()\0text\0"
    "translateInputChanged(QString)\0"
    "translateInputFinished()\0"
    "wordListItemActivated(QListWidgetItem*)\0"
    "focusTranslateLine()\0typingEvent(QString)\0"
};

void ScanPopup::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ScanPopup *_t = static_cast<ScanPopup *>(_o);
        switch (_id) {
        case 0: _t->editGroupRequested((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 1: _t->sendWordToMainWindow((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->closeMenu(); break;
        case 3: _t->setExpandMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->setViewExpandMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->switchExpandMode(); break;
        case 6: _t->forceAddWordToHistory((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->showDictionaryInfo((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->openDictionaryFolder((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->sendWordToHistory((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->requestWindowFocus(); break;
        case 11: _t->translateWordFromClipboard(); break;
        case 12: _t->translateWordFromSelection(); break;
        case 13: _t->editGroupRequested(); break;
        case 14: _t->clipboardChanged((*reinterpret_cast< QClipboard::Mode(*)>(_a[1]))); break;
        case 15: _t->mouseHovered((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 16: _t->currentGroupChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: _t->prefixMatchFinished(); break;
        case 18: _t->on_pronounceButton_clicked(); break;
        case 19: _t->pinButtonClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 20: _t->on_showDictionaryBar_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 21: _t->showStatusBarMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QPixmap(*)>(_a[3]))); break;
        case 22: _t->on_sendWordButton_clicked(); break;
        case 23: _t->on_goBackButton_clicked(); break;
        case 24: _t->on_goForwardButton_clicked(); break;
        case 25: _t->hideTimerExpired(); break;
        case 26: _t->altModeExpired(); break;
        case 27: _t->altModePoll(); break;
        case 28: _t->mouseGrabPoll(); break;
        case 29: _t->pageLoaded((*reinterpret_cast< ArticleView*(*)>(_a[1]))); break;
        case 30: _t->escapePressed(); break;
        case 31: _t->mutedDictionariesChanged(); break;
        case 32: _t->switchExpandOptionalPartsMode(); break;
        case 33: _t->translateInputChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 34: _t->translateInputFinished(); break;
        case 35: _t->wordListItemActivated((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 36: _t->focusTranslateLine(); break;
        case 37: _t->typingEvent((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ScanPopup::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ScanPopup::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_ScanPopup,
      qt_meta_data_ScanPopup, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ScanPopup::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ScanPopup::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ScanPopup::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ScanPopup))
        return static_cast<void*>(const_cast< ScanPopup*>(this));
    if (!strcmp(_clname, "KeyboardState"))
        return static_cast< KeyboardState*>(const_cast< ScanPopup*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int ScanPopup::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 38)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 38;
    }
    return _id;
}

// SIGNAL 0
void ScanPopup::editGroupRequested(unsigned  _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ScanPopup::sendWordToMainWindow(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ScanPopup::closeMenu()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void ScanPopup::setExpandMode(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ScanPopup::setViewExpandMode(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ScanPopup::switchExpandMode()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void ScanPopup::forceAddWordToHistory(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void ScanPopup::showDictionaryInfo(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void ScanPopup::openDictionaryFolder(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void ScanPopup::sendWordToHistory(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}
QT_END_MOC_NAMESPACE

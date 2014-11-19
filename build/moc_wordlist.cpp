/****************************************************************************
** Meta object code from reading C++ file 'wordlist.hh'
**
** Created: Mon Apr 28 13:37:38 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../wordlist.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wordlist.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WordList[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   10,    9,    9, 0x05,
      87,   71,    9,    9, 0x25,
     125,  117,    9,    9, 0x25,
     151,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
     168,    9,    9,    9, 0x08,
     189,    9,    9,    9, 0x08,
     220,  211,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WordList[] = {
    "WordList\0\0message,timeout,pixmap\0"
    "statusBarMessage(QString,int,QPixmap)\0"
    "message,timeout\0statusBarMessage(QString,int)\0"
    "message\0statusBarMessage(QString)\0"
    "contentChanged()\0prefixMatchUpdated()\0"
    "prefixMatchFinished()\0finished\0"
    "updateMatchResults(bool)\0"
};

void WordList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        WordList *_t = static_cast<WordList *>(_o);
        switch (_id) {
        case 0: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QPixmap(*)>(_a[3]))); break;
        case 1: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->contentChanged(); break;
        case 4: _t->prefixMatchUpdated(); break;
        case 5: _t->prefixMatchFinished(); break;
        case 6: _t->updateMatchResults((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData WordList::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject WordList::staticMetaObject = {
    { &QListWidget::staticMetaObject, qt_meta_stringdata_WordList,
      qt_meta_data_WordList, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WordList::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WordList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WordList::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WordList))
        return static_cast<void*>(const_cast< WordList*>(this));
    return QListWidget::qt_metacast(_clname);
}

int WordList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void WordList::statusBarMessage(QString const & _t1, int _t2, QPixmap const & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 3
void WordList::contentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}
QT_END_MOC_NAMESPACE

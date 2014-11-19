/****************************************************************************
** Meta object code from reading C++ file 'wordfinder.hh'
**
** Created: Mon Apr 28 13:36:57 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../wordfinder.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wordfinder.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WordFinder[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,
      22,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      33,   11,   11,   11, 0x08,
      51,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WordFinder[] = {
    "WordFinder\0\0updated()\0finished()\0"
    "requestFinished()\0updateResults()\0"
};

void WordFinder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        WordFinder *_t = static_cast<WordFinder *>(_o);
        switch (_id) {
        case 0: _t->updated(); break;
        case 1: _t->finished(); break;
        case 2: _t->requestFinished(); break;
        case 3: _t->updateResults(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData WordFinder::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject WordFinder::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_WordFinder,
      qt_meta_data_WordFinder, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WordFinder::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WordFinder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WordFinder::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WordFinder))
        return static_cast<void*>(const_cast< WordFinder*>(this));
    return QObject::qt_metacast(_clname);
}

int WordFinder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void WordFinder::updated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void WordFinder::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE

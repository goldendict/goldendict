/****************************************************************************
** Meta object code from reading C++ file 'loaddictionaries.hh'
**
** Created: Mon Apr 28 13:37:11 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../loaddictionaries.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'loaddictionaries.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LoadDictionaries[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   18,   17,   17, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_LoadDictionaries[] = {
    "LoadDictionaries\0\0dictionaryName\0"
    "indexingDictionarySignal(QString)\0"
};

void LoadDictionaries::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LoadDictionaries *_t = static_cast<LoadDictionaries *>(_o);
        switch (_id) {
        case 0: _t->indexingDictionarySignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData LoadDictionaries::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LoadDictionaries::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_LoadDictionaries,
      qt_meta_data_LoadDictionaries, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LoadDictionaries::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LoadDictionaries::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LoadDictionaries::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LoadDictionaries))
        return static_cast<void*>(const_cast< LoadDictionaries*>(this));
    if (!strcmp(_clname, "Dictionary::Initializing"))
        return static_cast< Dictionary::Initializing*>(const_cast< LoadDictionaries*>(this));
    return QThread::qt_metacast(_clname);
}

int LoadDictionaries::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void LoadDictionaries::indexingDictionarySignal(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

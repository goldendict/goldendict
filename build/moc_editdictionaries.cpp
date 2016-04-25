/****************************************************************************
** Meta object code from reading C++ file 'editdictionaries.hh'
**
** Created: Wed Nov 19 16:06:33 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../editdictionaries.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'editdictionaries.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_EditDictionaries[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      25,   18,   17,   17, 0x05,
      53,   18,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      92,   86,   17,   17, 0x08,
     127,  120,   17,   17, 0x08,
     162,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_EditDictionaries[] = {
    "EditDictionaries\0\0dictId\0"
    "showDictionaryInfo(QString)\0"
    "showDictionaryHeadwords(QString)\0index\0"
    "on_tabs_currentChanged(int)\0button\0"
    "buttonBoxClicked(QAbstractButton*)\0"
    "rescanSources()\0"
};

void EditDictionaries::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        EditDictionaries *_t = static_cast<EditDictionaries *>(_o);
        switch (_id) {
        case 0: _t->showDictionaryInfo((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->showDictionaryHeadwords((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->on_tabs_currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->buttonBoxClicked((*reinterpret_cast< QAbstractButton*(*)>(_a[1]))); break;
        case 4: _t->rescanSources(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData EditDictionaries::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject EditDictionaries::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_EditDictionaries,
      qt_meta_data_EditDictionaries, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &EditDictionaries::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *EditDictionaries::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *EditDictionaries::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EditDictionaries))
        return static_cast<void*>(const_cast< EditDictionaries*>(this));
    return QDialog::qt_metacast(_clname);
}

int EditDictionaries::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void EditDictionaries::showDictionaryInfo(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void EditDictionaries::showDictionaryHeadwords(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE

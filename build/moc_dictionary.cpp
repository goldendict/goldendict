/****************************************************************************
** Meta object code from reading C++ file 'dictionary.hh'
**
** Created: Mon Apr 28 13:36:35 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../dictionary.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dictionary.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dictionary__Request[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,
      31,   20,   20,   20, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Dictionary__Request[] = {
    "Dictionary::Request\0\0updated()\0"
    "finished()\0"
};

void Dictionary::Request::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Request *_t = static_cast<Request *>(_o);
        switch (_id) {
        case 0: _t->updated(); break;
        case 1: _t->finished(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Dictionary::Request::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Dictionary::Request::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Dictionary__Request,
      qt_meta_data_Dictionary__Request, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dictionary::Request::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dictionary::Request::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dictionary::Request::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dictionary__Request))
        return static_cast<void*>(const_cast< Request*>(this));
    return QObject::qt_metacast(_clname);
}

int Dictionary::Request::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Dictionary::Request::updated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void Dictionary::Request::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_Dictionary__WordSearchRequest[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_Dictionary__WordSearchRequest[] = {
    "Dictionary::WordSearchRequest\0"
};

void Dictionary::WordSearchRequest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Dictionary::WordSearchRequest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Dictionary::WordSearchRequest::staticMetaObject = {
    { &Request::staticMetaObject, qt_meta_stringdata_Dictionary__WordSearchRequest,
      qt_meta_data_Dictionary__WordSearchRequest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dictionary::WordSearchRequest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dictionary::WordSearchRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dictionary::WordSearchRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dictionary__WordSearchRequest))
        return static_cast<void*>(const_cast< WordSearchRequest*>(this));
    return Request::qt_metacast(_clname);
}

int Dictionary::WordSearchRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Request::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_Dictionary__DataRequest[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_Dictionary__DataRequest[] = {
    "Dictionary::DataRequest\0"
};

void Dictionary::DataRequest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Dictionary::DataRequest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Dictionary::DataRequest::staticMetaObject = {
    { &Request::staticMetaObject, qt_meta_stringdata_Dictionary__DataRequest,
      qt_meta_data_Dictionary__DataRequest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dictionary::DataRequest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dictionary::DataRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dictionary::DataRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dictionary__DataRequest))
        return static_cast<void*>(const_cast< DataRequest*>(this));
    return Request::qt_metacast(_clname);
}

int Dictionary::DataRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Request::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE

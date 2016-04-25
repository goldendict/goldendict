/****************************************************************************
** Meta object code from reading C++ file 'programs.hh'
**
** Created: Wed Nov 19 16:06:51 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../programs.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'programs.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Programs__RunInstance[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,
      65,   22,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      83,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Programs__RunInstance[] = {
    "Programs::RunInstance\0\0output,error\0"
    "finished(QByteArray,QString)\0"
    "processFinished()\0handleProcessFinished()\0"
};

void Programs::RunInstance::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RunInstance *_t = static_cast<RunInstance *>(_o);
        switch (_id) {
        case 0: _t->finished((*reinterpret_cast< QByteArray(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->processFinished(); break;
        case 2: _t->handleProcessFinished(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Programs::RunInstance::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Programs::RunInstance::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Programs__RunInstance,
      qt_meta_data_Programs__RunInstance, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Programs::RunInstance::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Programs::RunInstance::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Programs::RunInstance::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Programs__RunInstance))
        return static_cast<void*>(const_cast< RunInstance*>(this));
    return QObject::qt_metacast(_clname);
}

int Programs::RunInstance::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void Programs::RunInstance::finished(QByteArray _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Programs::RunInstance::processFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_Programs__ProgramDataRequest[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      43,   30,   29,   29, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Programs__ProgramDataRequest[] = {
    "Programs::ProgramDataRequest\0\0"
    "output,error\0instanceFinished(QByteArray,QString)\0"
};

void Programs::ProgramDataRequest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ProgramDataRequest *_t = static_cast<ProgramDataRequest *>(_o);
        switch (_id) {
        case 0: _t->instanceFinished((*reinterpret_cast< QByteArray(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Programs::ProgramDataRequest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Programs::ProgramDataRequest::staticMetaObject = {
    { &Dictionary::DataRequest::staticMetaObject, qt_meta_stringdata_Programs__ProgramDataRequest,
      qt_meta_data_Programs__ProgramDataRequest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Programs::ProgramDataRequest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Programs::ProgramDataRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Programs::ProgramDataRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Programs__ProgramDataRequest))
        return static_cast<void*>(const_cast< ProgramDataRequest*>(this));
    typedef Dictionary::DataRequest QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Programs::ProgramDataRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Dictionary::DataRequest QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_Programs__ProgramWordSearchRequest[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      49,   36,   35,   35, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Programs__ProgramWordSearchRequest[] = {
    "Programs::ProgramWordSearchRequest\0\0"
    "output,error\0instanceFinished(QByteArray,QString)\0"
};

void Programs::ProgramWordSearchRequest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ProgramWordSearchRequest *_t = static_cast<ProgramWordSearchRequest *>(_o);
        switch (_id) {
        case 0: _t->instanceFinished((*reinterpret_cast< QByteArray(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Programs::ProgramWordSearchRequest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Programs::ProgramWordSearchRequest::staticMetaObject = {
    { &Dictionary::WordSearchRequest::staticMetaObject, qt_meta_stringdata_Programs__ProgramWordSearchRequest,
      qt_meta_data_Programs__ProgramWordSearchRequest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Programs::ProgramWordSearchRequest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Programs::ProgramWordSearchRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Programs::ProgramWordSearchRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Programs__ProgramWordSearchRequest))
        return static_cast<void*>(const_cast< ProgramWordSearchRequest*>(this));
    typedef Dictionary::WordSearchRequest QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Programs::ProgramWordSearchRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Dictionary::WordSearchRequest QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'article_netmgr.hh'
**
** Created: Wed Nov 19 16:06:05 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../article_netmgr.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'article_netmgr.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ArticleResourceReply[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x05,
      40,   21,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      57,   21,   21,   21, 0x08,
      70,   21,   21,   21, 0x08,
      84,   21,   21,   21, 0x08,
     100,   21,   21,   21, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ArticleResourceReply[] = {
    "ArticleResourceReply\0\0readyReadSignal()\0"
    "finishedSignal()\0reqUpdated()\0"
    "reqFinished()\0readyReadSlot()\0"
    "finishedSlot()\0"
};

void ArticleResourceReply::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ArticleResourceReply *_t = static_cast<ArticleResourceReply *>(_o);
        switch (_id) {
        case 0: _t->readyReadSignal(); break;
        case 1: _t->finishedSignal(); break;
        case 2: _t->reqUpdated(); break;
        case 3: _t->reqFinished(); break;
        case 4: _t->readyReadSlot(); break;
        case 5: _t->finishedSlot(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ArticleResourceReply::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ArticleResourceReply::staticMetaObject = {
    { &QNetworkReply::staticMetaObject, qt_meta_stringdata_ArticleResourceReply,
      qt_meta_data_ArticleResourceReply, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ArticleResourceReply::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ArticleResourceReply::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ArticleResourceReply::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ArticleResourceReply))
        return static_cast<void*>(const_cast< ArticleResourceReply*>(this));
    return QNetworkReply::qt_metacast(_clname);
}

int ArticleResourceReply::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QNetworkReply::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ArticleResourceReply::readyReadSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ArticleResourceReply::finishedSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_BlockedNetworkReply[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      38,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_BlockedNetworkReply[] = {
    "BlockedNetworkReply\0\0finishedSignal()\0"
    "finishedSlot()\0"
};

void BlockedNetworkReply::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        BlockedNetworkReply *_t = static_cast<BlockedNetworkReply *>(_o);
        switch (_id) {
        case 0: _t->finishedSignal(); break;
        case 1: _t->finishedSlot(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData BlockedNetworkReply::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject BlockedNetworkReply::staticMetaObject = {
    { &QNetworkReply::staticMetaObject, qt_meta_stringdata_BlockedNetworkReply,
      qt_meta_data_BlockedNetworkReply, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BlockedNetworkReply::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BlockedNetworkReply::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BlockedNetworkReply::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BlockedNetworkReply))
        return static_cast<void*>(const_cast< BlockedNetworkReply*>(this));
    return QNetworkReply::qt_metacast(_clname);
}

int BlockedNetworkReply::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QNetworkReply::qt_metacall(_c, _id, _a);
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
void BlockedNetworkReply::finishedSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE

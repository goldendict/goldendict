/****************************************************************************
** Meta object code from reading C++ file 'mediawiki.hh'
**
** Created: Mon Apr 28 13:37:03 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mediawiki.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mediawiki.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MediaWiki__MediaWikiWordSearchRequestSlots[] = {

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
      44,   43,   43,   43, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_MediaWiki__MediaWikiWordSearchRequestSlots[] = {
    "MediaWiki::MediaWikiWordSearchRequestSlots\0"
    "\0downloadFinished()\0"
};

void MediaWiki::MediaWikiWordSearchRequestSlots::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MediaWikiWordSearchRequestSlots *_t = static_cast<MediaWikiWordSearchRequestSlots *>(_o);
        switch (_id) {
        case 0: _t->downloadFinished(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MediaWiki::MediaWikiWordSearchRequestSlots::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MediaWiki::MediaWikiWordSearchRequestSlots::staticMetaObject = {
    { &Dictionary::WordSearchRequest::staticMetaObject, qt_meta_stringdata_MediaWiki__MediaWikiWordSearchRequestSlots,
      qt_meta_data_MediaWiki__MediaWikiWordSearchRequestSlots, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MediaWiki::MediaWikiWordSearchRequestSlots::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MediaWiki::MediaWikiWordSearchRequestSlots::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MediaWiki::MediaWikiWordSearchRequestSlots::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MediaWiki__MediaWikiWordSearchRequestSlots))
        return static_cast<void*>(const_cast< MediaWikiWordSearchRequestSlots*>(this));
    typedef Dictionary::WordSearchRequest QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int MediaWiki::MediaWikiWordSearchRequestSlots::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_MediaWiki__MediaWikiDataRequestSlots[] = {

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
      38,   37,   37,   37, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_MediaWiki__MediaWikiDataRequestSlots[] = {
    "MediaWiki::MediaWikiDataRequestSlots\0"
    "\0requestFinished(QNetworkReply*)\0"
};

void MediaWiki::MediaWikiDataRequestSlots::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MediaWikiDataRequestSlots *_t = static_cast<MediaWikiDataRequestSlots *>(_o);
        switch (_id) {
        case 0: _t->requestFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MediaWiki::MediaWikiDataRequestSlots::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MediaWiki::MediaWikiDataRequestSlots::staticMetaObject = {
    { &Dictionary::DataRequest::staticMetaObject, qt_meta_stringdata_MediaWiki__MediaWikiDataRequestSlots,
      qt_meta_data_MediaWiki__MediaWikiDataRequestSlots, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MediaWiki::MediaWikiDataRequestSlots::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MediaWiki::MediaWikiDataRequestSlots::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MediaWiki::MediaWikiDataRequestSlots::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MediaWiki__MediaWikiDataRequestSlots))
        return static_cast<void*>(const_cast< MediaWikiDataRequestSlots*>(this));
    typedef Dictionary::DataRequest QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int MediaWiki::MediaWikiDataRequestSlots::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE

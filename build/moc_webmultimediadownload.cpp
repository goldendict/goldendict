/****************************************************************************
** Meta object code from reading C++ file 'webmultimediadownload.hh'
**
** Created: Wed Nov 19 16:06:44 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../webmultimediadownload.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'webmultimediadownload.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dictionary__WebMultimediaDownload[] = {

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
      35,   34,   34,   34, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Dictionary__WebMultimediaDownload[] = {
    "Dictionary::WebMultimediaDownload\0\0"
    "replyFinished(QNetworkReply*)\0"
};

void Dictionary::WebMultimediaDownload::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        WebMultimediaDownload *_t = static_cast<WebMultimediaDownload *>(_o);
        switch (_id) {
        case 0: _t->replyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Dictionary::WebMultimediaDownload::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Dictionary::WebMultimediaDownload::staticMetaObject = {
    { &DataRequest::staticMetaObject, qt_meta_stringdata_Dictionary__WebMultimediaDownload,
      qt_meta_data_Dictionary__WebMultimediaDownload, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dictionary::WebMultimediaDownload::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dictionary::WebMultimediaDownload::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dictionary::WebMultimediaDownload::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dictionary__WebMultimediaDownload))
        return static_cast<void*>(const_cast< WebMultimediaDownload*>(this));
    return DataRequest::qt_metacast(_clname);
}

int Dictionary::WebMultimediaDownload::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataRequest::qt_metacall(_c, _id, _a);
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

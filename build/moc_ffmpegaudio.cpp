/****************************************************************************
** Meta object code from reading C++ file 'ffmpegaudio.hh'
**
** Created: Wed Nov 19 16:07:07 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ffmpegaudio.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ffmpegaudio.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ffmpeg__AudioPlayer[] = {

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
      39,   21,   20,   20, 0x05,
      67,   59,   20,   20, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Ffmpeg__AudioPlayer[] = {
    "Ffmpeg::AudioPlayer\0\0waitUntilFinished\0"
    "cancelPlaying(bool)\0message\0error(QString)\0"
};

void Ffmpeg::AudioPlayer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AudioPlayer *_t = static_cast<AudioPlayer *>(_o);
        switch (_id) {
        case 0: _t->cancelPlaying((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Ffmpeg::AudioPlayer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ffmpeg::AudioPlayer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Ffmpeg__AudioPlayer,
      qt_meta_data_Ffmpeg__AudioPlayer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ffmpeg::AudioPlayer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ffmpeg::AudioPlayer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ffmpeg::AudioPlayer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ffmpeg__AudioPlayer))
        return static_cast<void*>(const_cast< AudioPlayer*>(this));
    return QObject::qt_metacast(_clname);
}

int Ffmpeg::AudioPlayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Ffmpeg::AudioPlayer::cancelPlaying(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Ffmpeg::AudioPlayer::error(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_Ffmpeg__DecoderThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      31,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      46,   22,   22,   22, 0x0a,
      70,   52,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ffmpeg__DecoderThread[] = {
    "Ffmpeg::DecoderThread\0\0message\0"
    "error(QString)\0run()\0waitUntilFinished\0"
    "cancel(bool)\0"
};

void Ffmpeg::DecoderThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DecoderThread *_t = static_cast<DecoderThread *>(_o);
        switch (_id) {
        case 0: _t->error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->run(); break;
        case 2: _t->cancel((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Ffmpeg::DecoderThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ffmpeg::DecoderThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_Ffmpeg__DecoderThread,
      qt_meta_data_Ffmpeg__DecoderThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ffmpeg::DecoderThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ffmpeg::DecoderThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ffmpeg::DecoderThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ffmpeg__DecoderThread))
        return static_cast<void*>(const_cast< DecoderThread*>(this));
    return QThread::qt_metacast(_clname);
}

int Ffmpeg::DecoderThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
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
void Ffmpeg::DecoderThread::error(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

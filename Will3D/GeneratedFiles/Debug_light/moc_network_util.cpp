/****************************************************************************
** Meta object code from reading C++ file 'network_util.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../network_util.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'network_util.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_network_util_t {
    QByteArrayData data[17];
    char stringdata0[241];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_network_util_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_network_util_t qt_meta_stringdata_network_util = {
    {
QT_MOC_LITERAL(0, 0, 12), // "network_util"
QT_MOC_LITERAL(1, 13, 14), // "sigSetProgress"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 14), // "onNetworkError"
QT_MOC_LITERAL(4, 44, 27), // "QNetworkReply::NetworkError"
QT_MOC_LITERAL(5, 72, 4), // "code"
QT_MOC_LITERAL(6, 77, 18), // "onDownloadProgress"
QT_MOC_LITERAL(7, 96, 13), // "bytesReceived"
QT_MOC_LITERAL(8, 110, 10), // "bytesTotal"
QT_MOC_LITERAL(9, 121, 11), // "onReadyRead"
QT_MOC_LITERAL(10, 133, 15), // "onReplyFinished"
QT_MOC_LITERAL(11, 149, 21), // "onProcessStateChanged"
QT_MOC_LITERAL(12, 171, 22), // "QProcess::ProcessState"
QT_MOC_LITERAL(13, 194, 8), // "newState"
QT_MOC_LITERAL(14, 203, 17), // "onWatcherFinished"
QT_MOC_LITERAL(15, 221, 13), // "onSetProgress"
QT_MOC_LITERAL(16, 235, 5) // "value"

    },
    "network_util\0sigSetProgress\0\0"
    "onNetworkError\0QNetworkReply::NetworkError\0"
    "code\0onDownloadProgress\0bytesReceived\0"
    "bytesTotal\0onReadyRead\0onReplyFinished\0"
    "onProcessStateChanged\0QProcess::ProcessState\0"
    "newState\0onWatcherFinished\0onSetProgress\0"
    "value"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_network_util[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   57,    2, 0x0a /* Public */,
       6,    2,   60,    2, 0x0a /* Public */,
       9,    0,   65,    2, 0x0a /* Public */,
      10,    0,   66,    2, 0x0a /* Public */,
      11,    1,   67,    2, 0x0a /* Public */,
      14,    0,   70,    2, 0x0a /* Public */,
      15,    1,   71,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::LongLong, QMetaType::LongLong,    7,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   16,

       0        // eod
};

void network_util::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        network_util *_t = static_cast<network_util *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigSetProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onNetworkError((*reinterpret_cast< QNetworkReply::NetworkError(*)>(_a[1]))); break;
        case 2: _t->onDownloadProgress((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 3: _t->onReadyRead(); break;
        case 4: _t->onReplyFinished(); break;
        case 5: _t->onProcessStateChanged((*reinterpret_cast< QProcess::ProcessState(*)>(_a[1]))); break;
        case 6: _t->onWatcherFinished(); break;
        case 7: _t->onSetProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply::NetworkError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (network_util::*_t)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&network_util::sigSetProgress)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject network_util::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_network_util.data,
      qt_meta_data_network_util,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *network_util::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *network_util::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_network_util.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int network_util::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void network_util::sigSetProgress(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

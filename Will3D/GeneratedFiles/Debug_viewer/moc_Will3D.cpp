/****************************************************************************
** Meta object code from reading C++ file 'Will3D.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Will3D.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Will3D.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Will3DEventFilter_t {
    QByteArrayData data[6];
    char stringdata0[104];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Will3DEventFilter_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Will3DEventFilter_t qt_meta_stringdata_Will3DEventFilter = {
    {
QT_MOC_LITERAL(0, 0, 17), // "Will3DEventFilter"
QT_MOC_LITERAL(1, 18, 33), // "sigGraphicsSceneMouseReleaseE..."
QT_MOC_LITERAL(2, 52, 0), // ""
QT_MOC_LITERAL(3, 53, 12), // "QMouseEvent*"
QT_MOC_LITERAL(4, 66, 5), // "event"
QT_MOC_LITERAL(5, 72, 31) // "sigGraphicsSceneMousePressEvent"

    },
    "Will3DEventFilter\0sigGraphicsSceneMouseReleaseEvent\0"
    "\0QMouseEvent*\0event\0sigGraphicsSceneMousePressEvent"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Will3DEventFilter[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       5,    1,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void Will3DEventFilter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Will3DEventFilter *_t = static_cast<Will3DEventFilter *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigGraphicsSceneMouseReleaseEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        case 1: _t->sigGraphicsSceneMousePressEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (Will3DEventFilter::*_t)(QMouseEvent * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Will3DEventFilter::sigGraphicsSceneMouseReleaseEvent)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (Will3DEventFilter::*_t)(QMouseEvent * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Will3DEventFilter::sigGraphicsSceneMousePressEvent)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject Will3DEventFilter::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Will3DEventFilter.data,
      qt_meta_data_Will3DEventFilter,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *Will3DEventFilter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Will3DEventFilter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Will3DEventFilter.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Will3DEventFilter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Will3DEventFilter::sigGraphicsSceneMouseReleaseEvent(QMouseEvent * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Will3DEventFilter::sigGraphicsSceneMousePressEvent(QMouseEvent * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_CWill3D_t {
    QByteArrayData data[45];
    char stringdata0[631];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CWill3D_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CWill3D_t qt_meta_stringdata_CWill3D = {
    {
QT_MOC_LITERAL(0, 0, 7), // "CWill3D"
QT_MOC_LITERAL(1, 8, 12), // "sigSetVolume"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 7), // "TabType"
QT_MOC_LITERAL(4, 30, 10), // "sigGoToMPR"
QT_MOC_LITERAL(5, 41, 18), // "slotShowMaxRestore"
QT_MOC_LITERAL(6, 60, 11), // "slotCapture"
QT_MOC_LITERAL(7, 72, 9), // "slotPrint"
QT_MOC_LITERAL(8, 82, 15), // "slotPreferences"
QT_MOC_LITERAL(9, 98, 9), // "slotAbout"
QT_MOC_LITERAL(10, 108, 11), // "slotSupport"
QT_MOC_LITERAL(11, 120, 11), // "slotOTFSave"
QT_MOC_LITERAL(12, 132, 12), // "slotUpdateTF"
QT_MOC_LITERAL(13, 145, 15), // "changed_min_max"
QT_MOC_LITERAL(14, 161, 16), // "slotUpdateDoneTF"
QT_MOC_LITERAL(15, 178, 19), // "slotShadeOnSwitchTF"
QT_MOC_LITERAL(16, 198, 11), // "is_shade_on"
QT_MOC_LITERAL(17, 210, 13), // "slotOTFAdjust"
QT_MOC_LITERAL(18, 224, 10), // "AdjustOTF&"
QT_MOC_LITERAL(19, 235, 10), // "adjust_otf"
QT_MOC_LITERAL(20, 246, 13), // "slotOTFPreset"
QT_MOC_LITERAL(21, 260, 6), // "preset"
QT_MOC_LITERAL(22, 267, 18), // "slotOTFManualOnOff"
QT_MOC_LITERAL(23, 286, 11), // "slotOTFAuto"
QT_MOC_LITERAL(24, 298, 16), // "slotSaveTfPreset"
QT_MOC_LITERAL(25, 315, 9), // "file_path"
QT_MOC_LITERAL(26, 325, 15), // "slotInitProgram"
QT_MOC_LITERAL(27, 341, 16), // "slotSetDicomInfo"
QT_MOC_LITERAL(28, 358, 24), // "slotGetTabSlotGlobalRect"
QT_MOC_LITERAL(29, 383, 6), // "QRect&"
QT_MOC_LITERAL(30, 390, 11), // "global_rect"
QT_MOC_LITERAL(31, 402, 18), // "slotGetTabSlotRect"
QT_MOC_LITERAL(32, 421, 4), // "rect"
QT_MOC_LITERAL(33, 426, 13), // "slotChangeTab"
QT_MOC_LITERAL(34, 440, 7), // "tabType"
QT_MOC_LITERAL(35, 448, 23), // "slotChangeTabFromTabBar"
QT_MOC_LITERAL(36, 472, 17), // "slotSetMainVolume"
QT_MOC_LITERAL(37, 490, 19), // "slotSetSecondVolume"
QT_MOC_LITERAL(38, 510, 17), // "slotSetPanoVolume"
QT_MOC_LITERAL(39, 528, 19), // "slotClearPanoVolume"
QT_MOC_LITERAL(40, 548, 17), // "slotMoveTFpolygon"
QT_MOC_LITERAL(41, 566, 5), // "value"
QT_MOC_LITERAL(42, 572, 20), // "slotTimeoutAdjustOTF"
QT_MOC_LITERAL(43, 593, 20), // "slotSetSoftTissueMin"
QT_MOC_LITERAL(44, 614, 16) // "slotCDViewerLoad"

    },
    "CWill3D\0sigSetVolume\0\0TabType\0sigGoToMPR\0"
    "slotShowMaxRestore\0slotCapture\0slotPrint\0"
    "slotPreferences\0slotAbout\0slotSupport\0"
    "slotOTFSave\0slotUpdateTF\0changed_min_max\0"
    "slotUpdateDoneTF\0slotShadeOnSwitchTF\0"
    "is_shade_on\0slotOTFAdjust\0AdjustOTF&\0"
    "adjust_otf\0slotOTFPreset\0preset\0"
    "slotOTFManualOnOff\0slotOTFAuto\0"
    "slotSaveTfPreset\0file_path\0slotInitProgram\0"
    "slotSetDicomInfo\0slotGetTabSlotGlobalRect\0"
    "QRect&\0global_rect\0slotGetTabSlotRect\0"
    "rect\0slotChangeTab\0tabType\0"
    "slotChangeTabFromTabBar\0slotSetMainVolume\0"
    "slotSetSecondVolume\0slotSetPanoVolume\0"
    "slotClearPanoVolume\0slotMoveTFpolygon\0"
    "value\0slotTimeoutAdjustOTF\0"
    "slotSetSoftTissueMin\0slotCDViewerLoad"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CWill3D[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      31,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  169,    2, 0x06 /* Public */,
       4,    1,  172,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,  175,    2, 0x08 /* Private */,
       6,    0,  178,    2, 0x08 /* Private */,
       7,    0,  179,    2, 0x08 /* Private */,
       8,    0,  180,    2, 0x08 /* Private */,
       9,    0,  181,    2, 0x08 /* Private */,
      10,    0,  182,    2, 0x08 /* Private */,
      11,    0,  183,    2, 0x08 /* Private */,
      12,    1,  184,    2, 0x08 /* Private */,
      14,    0,  187,    2, 0x08 /* Private */,
      15,    1,  188,    2, 0x08 /* Private */,
      17,    1,  191,    2, 0x08 /* Private */,
      20,    1,  194,    2, 0x08 /* Private */,
      22,    0,  197,    2, 0x08 /* Private */,
      23,    0,  198,    2, 0x08 /* Private */,
      24,    1,  199,    2, 0x08 /* Private */,
      26,    0,  202,    2, 0x08 /* Private */,
      27,    0,  203,    2, 0x08 /* Private */,
      28,    1,  204,    2, 0x08 /* Private */,
      31,    1,  207,    2, 0x08 /* Private */,
      33,    1,  210,    2, 0x08 /* Private */,
      35,    1,  213,    2, 0x08 /* Private */,
      36,    0,  216,    2, 0x08 /* Private */,
      37,    0,  217,    2, 0x08 /* Private */,
      38,    0,  218,    2, 0x08 /* Private */,
      39,    0,  219,    2, 0x08 /* Private */,
      40,    1,  220,    2, 0x08 /* Private */,
      42,    0,  223,    2, 0x08 /* Private */,
      43,    1,  224,    2, 0x08 /* Private */,
      44,    0,  227,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   16,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 29,   30,
    QMetaType::Void, 0x80000000 | 29,   32,
    QMetaType::Void, 0x80000000 | 3,   34,
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   41,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,   41,
    QMetaType::Void,

       0        // eod
};

void CWill3D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CWill3D *_t = static_cast<CWill3D *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigSetVolume((*reinterpret_cast< TabType(*)>(_a[1]))); break;
        case 1: _t->sigGoToMPR((*reinterpret_cast< TabType(*)>(_a[1]))); break;
        case 2: _t->slotShowMaxRestore((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->slotCapture(); break;
        case 4: _t->slotPrint(); break;
        case 5: _t->slotPreferences(); break;
        case 6: _t->slotAbout(); break;
        case 7: _t->slotSupport(); break;
        case 8: _t->slotOTFSave(); break;
        case 9: _t->slotUpdateTF((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->slotUpdateDoneTF(); break;
        case 11: _t->slotShadeOnSwitchTF((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->slotOTFAdjust((*reinterpret_cast< AdjustOTF(*)>(_a[1]))); break;
        case 13: _t->slotOTFPreset((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 14: _t->slotOTFManualOnOff(); break;
        case 15: _t->slotOTFAuto(); break;
        case 16: _t->slotSaveTfPreset((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 17: _t->slotInitProgram(); break;
        case 18: _t->slotSetDicomInfo(); break;
        case 19: _t->slotGetTabSlotGlobalRect((*reinterpret_cast< QRect(*)>(_a[1]))); break;
        case 20: _t->slotGetTabSlotRect((*reinterpret_cast< QRect(*)>(_a[1]))); break;
        case 21: _t->slotChangeTab((*reinterpret_cast< TabType(*)>(_a[1]))); break;
        case 22: _t->slotChangeTabFromTabBar((*reinterpret_cast< TabType(*)>(_a[1]))); break;
        case 23: _t->slotSetMainVolume(); break;
        case 24: _t->slotSetSecondVolume(); break;
        case 25: _t->slotSetPanoVolume(); break;
        case 26: _t->slotClearPanoVolume(); break;
        case 27: _t->slotMoveTFpolygon((*reinterpret_cast< const double(*)>(_a[1]))); break;
        case 28: _t->slotTimeoutAdjustOTF(); break;
        case 29: _t->slotSetSoftTissueMin((*reinterpret_cast< const float(*)>(_a[1]))); break;
        case 30: _t->slotCDViewerLoad(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (CWill3D::*_t)(TabType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWill3D::sigSetVolume)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (CWill3D::*_t)(TabType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWill3D::sigGoToMPR)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject CWill3D::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_CWill3D.data,
      qt_meta_data_CWill3D,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CWill3D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CWill3D::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CWill3D.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int CWill3D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 31;
    }
    return _id;
}

// SIGNAL 0
void CWill3D::sigSetVolume(TabType _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CWill3D::sigGoToMPR(TabType _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

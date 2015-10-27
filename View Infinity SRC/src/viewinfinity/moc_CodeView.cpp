/****************************************************************************
** Meta object code from reading C++ file 'CodeView.h'
**
** Created: Sun 8. Aug 18:07:28 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "CodeView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CodeView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CodeView[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   10,    9,    9, 0x08,
      54,   52,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CodeView[] = {
    "CodeView\0\0newBlockCount\0"
    "updateFeatureAreaWidth(int)\0,\0"
    "updateFeatureArea(QRect,int)\0"
};

const QMetaObject CodeView::staticMetaObject = {
    { &QTextEdit::staticMetaObject, qt_meta_stringdata_CodeView,
      qt_meta_data_CodeView, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CodeView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CodeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CodeView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CodeView))
        return static_cast<void*>(const_cast< CodeView*>(this));
    return QTextEdit::qt_metacast(_clname);
}

int CodeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateFeatureAreaWidth((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: updateFeatureArea((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

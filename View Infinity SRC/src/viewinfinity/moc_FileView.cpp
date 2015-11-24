/****************************************************************************
** Meta object code from reading C++ file 'FileView.h'
**
** Created: Sun 8. Aug 18:07:28 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FileView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FileView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FileView[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x0a,
      20,    9,    9,    9, 0x0a,
      30,    9,    9,    9, 0x0a,
      40,    9,    9,    9, 0x0a,
      50,    9,    9,    9, 0x0a,
      60,    9,    9,    9, 0x0a,
      70,    9,    9,    9, 0x0a,
      80,    9,    9,    9, 0x0a,
      90,    9,    9,    9, 0x0a,
     100,    9,    9,    9, 0x0a,
     110,    9,    9,    9, 0x0a,
     121,    9,    9,    9, 0x0a,
     132,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FileView[] = {
    "FileView\0\0stripe0()\0stripe1()\0stripe2()\0"
    "stripe3()\0stripe4()\0stripe5()\0stripe6()\0"
    "stripe7()\0stripe8()\0stripe9()\0stripe10()\0"
    "zmButton()\0zpButton()\0"
};

const QMetaObject FileView::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_FileView,
      qt_meta_data_FileView, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FileView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FileView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FileView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FileView))
        return static_cast<void*>(const_cast< FileView*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int FileView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: stripe0(); break;
        case 1: stripe1(); break;
        case 2: stripe2(); break;
        case 3: stripe3(); break;
        case 4: stripe4(); break;
        case 5: stripe5(); break;
        case 6: stripe6(); break;
        case 7: stripe7(); break;
        case 8: stripe8(); break;
        case 9: stripe9(); break;
        case 10: stripe10(); break;
        case 11: zmButton(); break;
        case 12: zpButton(); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

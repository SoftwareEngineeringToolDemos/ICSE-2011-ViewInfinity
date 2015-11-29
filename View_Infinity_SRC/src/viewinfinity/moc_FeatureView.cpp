/****************************************************************************
** Meta object code from reading C++ file 'FeatureView.h'
**
** Created: Wed 18. Aug 22:24:14 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FeatureView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FeatureView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FeatureView[] = {

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
      13,   12,   12,   12, 0x0a,
      23,   12,   12,   12, 0x0a,
      33,   12,   12,   12, 0x0a,
      43,   12,   12,   12, 0x0a,
      53,   12,   12,   12, 0x0a,
      63,   12,   12,   12, 0x0a,
      73,   12,   12,   12, 0x0a,
      83,   12,   12,   12, 0x0a,
      93,   12,   12,   12, 0x0a,
     103,   12,   12,   12, 0x0a,
     113,   12,   12,   12, 0x0a,
     124,   12,   12,   12, 0x0a,
     135,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FeatureView[] = {
    "FeatureView\0\0stripe0()\0stripe1()\0"
    "stripe2()\0stripe3()\0stripe4()\0stripe5()\0"
    "stripe6()\0stripe7()\0stripe8()\0stripe9()\0"
    "stripe10()\0zmButton()\0zpButton()\0"
};

const QMetaObject FeatureView::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_FeatureView,
      qt_meta_data_FeatureView, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FeatureView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FeatureView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FeatureView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FeatureView))
        return static_cast<void*>(const_cast< FeatureView*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int FeatureView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

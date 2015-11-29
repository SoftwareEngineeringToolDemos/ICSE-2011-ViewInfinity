/****************************************************************************
** Meta object code from reading C++ file 'SMVMain.h'
**
** Created: Wed 18. Aug 00:48:38 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SMVMain.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SMVMain.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SMVMain[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      48,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,    9,    8,    8, 0x0a,
      46,    9,    8,    8, 0x0a,
      78,    9,    8,    8, 0x0a,
     118,    9,    8,    8, 0x0a,
     157,    8,    8,    8, 0x0a,
     167,    8,    8,    8, 0x0a,
     177,    8,    8,    8, 0x0a,
     187,    8,    8,    8, 0x0a,
     197,    8,    8,    8, 0x0a,
     207,    8,    8,    8, 0x0a,
     217,    8,    8,    8, 0x0a,
     227,    8,    8,    8, 0x0a,
     237,    8,    8,    8, 0x0a,
     247,    8,    8,    8, 0x0a,
     257,    8,    8,    8, 0x0a,
     268,    8,    8,    8, 0x0a,
     279,    8,    8,    8, 0x0a,
     290,    8,    8,    8, 0x0a,
     308,    8,    8,    8, 0x0a,
     327,    8,    8,    8, 0x0a,
     353,    8,    8,    8, 0x0a,
     376,    8,    8,    8, 0x0a,
     399,    8,    8,    8, 0x0a,
     419,    8,    8,    8, 0x0a,
     436,    8,    8,    8, 0x0a,
     453,    8,    8,    8, 0x0a,
     471,    8,    8,    8, 0x0a,
     486,    8,    8,    8, 0x0a,
     501,    8,    8,    8, 0x08,
     517,  512,    8,    8, 0x08,
     545,    8,    8,    8, 0x28,
     565,  512,    8,    8, 0x08,
     601,    8,    8,    8, 0x28,
     629,  512,    8,    8, 0x08,
     655,    8,    8,    8, 0x28,
     673,  512,    8,    8, 0x08,
     697,    8,    8,    8, 0x28,
     713,  512,    8,    8, 0x08,
     745,    8,    8,    8, 0x28,
     769,  512,    8,    8, 0x08,
     800,    8,    8,    8, 0x28,
     823,  512,    8,    8, 0x08,
     853,    8,    8,    8, 0x28,
     875,    8,    8,    8, 0x08,
     886,    8,    8,    8, 0x08,
     894,    8,    8,    8, 0x08,
     921,    8,    8,    8, 0x08,
     946,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SMVMain[] = {
    "SMVMain\0\0idx\0treeViewCollapsSlot(QModelIndex)\0"
    "treeViewExpandSlot(QModelIndex)\0"
    "featureTreeViewCollapsSlot(QModelIndex)\0"
    "featureTreeViewExpandSlot(QModelIndex)\0"
    "stripe0()\0stripe1()\0stripe2()\0stripe3()\0"
    "stripe4()\0stripe5()\0stripe6()\0stripe7()\0"
    "stripe8()\0stripe9()\0stripe10()\0"
    "zmButton()\0zpButton()\0openColorDialog()\0"
    "setFullFileGraph()\0directFadeInFeatureView()\0"
    "directFadeInFileView()\0directFadeInCodeView()\0"
    "fadeInFeatureView()\0fadeInFileView()\0"
    "fadeInCodeView()\0showFeatureView()\0"
    "showFileView()\0showCodeView()\0openFile()\0"
    "view\0setCircularLayout(ViewMode)\0"
    "setCircularLayout()\0"
    "setFastHierarchicalLayout(ViewMode)\0"
    "setFastHierarchicalLayout()\0"
    "setBallonLayout(ViewMode)\0setBallonLayout()\0"
    "setTreeLayout(ViewMode)\0setTreeLayout()\0"
    "setHierarchicalLayout(ViewMode)\0"
    "setHierarchicalLayout()\0"
    "setEnergybasedLayout(ViewMode)\0"
    "setEnergybasedLayout()\0"
    "setOrthogonalLayout(ViewMode)\0"
    "setOrthogonalLayout()\0writeGML()\0"
    "about()\0CodeColorTransparency(int)\0"
    "setFeaturesTransparent()\0setFeaturesOpaque()\0"
};

const QMetaObject SMVMain::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_SMVMain,
      qt_meta_data_SMVMain, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SMVMain::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SMVMain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SMVMain::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SMVMain))
        return static_cast<void*>(const_cast< SMVMain*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int SMVMain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: treeViewCollapsSlot((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 1: treeViewExpandSlot((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 2: featureTreeViewCollapsSlot((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 3: featureTreeViewExpandSlot((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 4: stripe0(); break;
        case 5: stripe1(); break;
        case 6: stripe2(); break;
        case 7: stripe3(); break;
        case 8: stripe4(); break;
        case 9: stripe5(); break;
        case 10: stripe6(); break;
        case 11: stripe7(); break;
        case 12: stripe8(); break;
        case 13: stripe9(); break;
        case 14: stripe10(); break;
        case 15: zmButton(); break;
        case 16: zpButton(); break;
        case 17: openColorDialog(); break;
        case 18: setFullFileGraph(); break;
        case 19: directFadeInFeatureView(); break;
        case 20: directFadeInFileView(); break;
        case 21: directFadeInCodeView(); break;
        case 22: fadeInFeatureView(); break;
        case 23: fadeInFileView(); break;
        case 24: fadeInCodeView(); break;
        case 25: showFeatureView(); break;
        case 26: showFileView(); break;
        case 27: showCodeView(); break;
        case 28: openFile(); break;
        case 29: setCircularLayout((*reinterpret_cast< ViewMode(*)>(_a[1]))); break;
        case 30: setCircularLayout(); break;
        case 31: setFastHierarchicalLayout((*reinterpret_cast< ViewMode(*)>(_a[1]))); break;
        case 32: setFastHierarchicalLayout(); break;
        case 33: setBallonLayout((*reinterpret_cast< ViewMode(*)>(_a[1]))); break;
        case 34: setBallonLayout(); break;
        case 35: setTreeLayout((*reinterpret_cast< ViewMode(*)>(_a[1]))); break;
        case 36: setTreeLayout(); break;
        case 37: setHierarchicalLayout((*reinterpret_cast< ViewMode(*)>(_a[1]))); break;
        case 38: setHierarchicalLayout(); break;
        case 39: setEnergybasedLayout((*reinterpret_cast< ViewMode(*)>(_a[1]))); break;
        case 40: setEnergybasedLayout(); break;
        case 41: setOrthogonalLayout((*reinterpret_cast< ViewMode(*)>(_a[1]))); break;
        case 42: setOrthogonalLayout(); break;
        case 43: writeGML(); break;
        case 44: about(); break;
        case 45: CodeColorTransparency((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 46: setFeaturesTransparent(); break;
        case 47: setFeaturesOpaque(); break;
        default: ;
        }
        _id -= 48;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

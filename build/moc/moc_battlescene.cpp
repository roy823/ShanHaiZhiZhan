/****************************************************************************
** Meta object code from reading C++ file 'battlescene.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/ui/battlescene.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'battlescene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN11BattleSceneE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN11BattleSceneE = QtMocHelpers::stringData(
    "BattleScene",
    "onSkillButtonClicked",
    "",
    "skillIndex",
    "onSwitchButtonClicked",
    "onEscapeButtonClicked",
    "onBattleLogUpdated",
    "message",
    "onTurnStarted",
    "turn",
    "isPlayerTurn",
    "onTurnEnded",
    "onDamageCaused",
    "Creature*",
    "creature",
    "damage",
    "onHealingReceived",
    "amount",
    "onCreatureSwitched",
    "oldCreature",
    "newCreature",
    "isPlayer",
    "onPlayerActionConfirmed",
    "onOpponentActionConfirmed",
    "onFifthSkillButtonClicked",
    "onRestorePPButtonClicked"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN11BattleSceneE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   92,    2, 0x08,    1 /* Private */,
       4,    0,   95,    2, 0x08,    3 /* Private */,
       5,    0,   96,    2, 0x08,    4 /* Private */,
       6,    1,   97,    2, 0x08,    5 /* Private */,
       8,    2,  100,    2, 0x08,    7 /* Private */,
      11,    1,  105,    2, 0x08,   10 /* Private */,
      12,    2,  108,    2, 0x08,   12 /* Private */,
      16,    2,  113,    2, 0x08,   15 /* Private */,
      18,    3,  118,    2, 0x08,   18 /* Private */,
      22,    0,  125,    2, 0x08,   22 /* Private */,
      23,    0,  126,    2, 0x08,   23 /* Private */,
      24,    0,  127,    2, 0x08,   24 /* Private */,
      25,    0,  128,    2, 0x08,   25 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    9,   10,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, 0x80000000 | 13, QMetaType::Int,   14,   15,
    QMetaType::Void, 0x80000000 | 13, QMetaType::Int,   14,   17,
    QMetaType::Void, 0x80000000 | 13, 0x80000000 | 13, QMetaType::Bool,   19,   20,   21,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject BattleScene::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN11BattleSceneE.offsetsAndSizes,
    qt_meta_data_ZN11BattleSceneE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN11BattleSceneE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<BattleScene, std::true_type>,
        // method 'onSkillButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onSwitchButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEscapeButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBattleLogUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onTurnStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onTurnEnded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onDamageCaused'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onHealingReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onCreatureSwitched'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onPlayerActionConfirmed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onOpponentActionConfirmed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFifthSkillButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRestorePPButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void BattleScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BattleScene *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onSkillButtonClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->onSwitchButtonClicked(); break;
        case 2: _t->onEscapeButtonClicked(); break;
        case 3: _t->onBattleLogUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->onTurnStarted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 5: _t->onTurnEnded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->onDamageCaused((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->onHealingReceived((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 8: _t->onCreatureSwitched((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 9: _t->onPlayerActionConfirmed(); break;
        case 10: _t->onOpponentActionConfirmed(); break;
        case 11: _t->onFifthSkillButtonClicked(); break;
        case 12: _t->onRestorePPButtonClicked(); break;
        default: ;
        }
    }
}

const QMetaObject *BattleScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BattleScene::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN11BattleSceneE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int BattleScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP

/****************************************************************************
** Meta object code from reading C++ file 'battlesystem.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/battle/battlesystem.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'battlesystem.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12BattleSystemE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12BattleSystemE = QtMocHelpers::stringData(
    "BattleSystem",
    "battleStarted",
    "",
    "battleEnded",
    "BattleResult",
    "result",
    "turnStarted",
    "turn",
    "isPlayerTurn",
    "turnEnded",
    "skillUsed",
    "Creature*",
    "user",
    "target",
    "Skill*",
    "skill",
    "hit",
    "damage",
    "damageCaused",
    "creature",
    "healingReceived",
    "amount",
    "statusChanged",
    "StatusCondition",
    "oldStatus",
    "newStatus",
    "statStageChanged",
    "StatType",
    "stat",
    "oldStage",
    "newStage",
    "creatureSwitched",
    "oldCreature",
    "newCreature",
    "isPlayer",
    "battleLogUpdated",
    "message",
    "playerActionConfirmed",
    "opponentActionConfirmed"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12BattleSystemE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      13,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   92,    2, 0x06,    1 /* Public */,
       3,    1,   93,    2, 0x06,    2 /* Public */,
       6,    2,   96,    2, 0x06,    4 /* Public */,
       9,    1,  101,    2, 0x06,    7 /* Public */,
      10,    5,  104,    2, 0x06,    9 /* Public */,
      18,    2,  115,    2, 0x06,   15 /* Public */,
      20,    2,  120,    2, 0x06,   18 /* Public */,
      22,    3,  125,    2, 0x06,   21 /* Public */,
      26,    4,  132,    2, 0x06,   25 /* Public */,
      31,    3,  141,    2, 0x06,   30 /* Public */,
      35,    1,  148,    2, 0x06,   34 /* Public */,
      37,    0,  151,    2, 0x06,   36 /* Public */,
      38,    0,  152,    2, 0x06,   37 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    7,    8,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, 0x80000000 | 11, 0x80000000 | 11, 0x80000000 | 14, QMetaType::Bool, QMetaType::Int,   12,   13,   15,   16,   17,
    QMetaType::Void, 0x80000000 | 11, QMetaType::Int,   19,   17,
    QMetaType::Void, 0x80000000 | 11, QMetaType::Int,   19,   21,
    QMetaType::Void, 0x80000000 | 11, 0x80000000 | 23, 0x80000000 | 23,   19,   24,   25,
    QMetaType::Void, 0x80000000 | 11, 0x80000000 | 27, QMetaType::Int, QMetaType::Int,   19,   28,   29,   30,
    QMetaType::Void, 0x80000000 | 11, 0x80000000 | 11, QMetaType::Bool,   32,   33,   34,
    QMetaType::Void, QMetaType::QString,   36,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject BattleSystem::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12BattleSystemE.offsetsAndSizes,
    qt_meta_data_ZN12BattleSystemE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12BattleSystemE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<BattleSystem, std::true_type>,
        // method 'battleStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'battleEnded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<BattleResult, std::false_type>,
        // method 'turnStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'turnEnded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'skillUsed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<Skill *, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'damageCaused'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'healingReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'statusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<StatusCondition, std::false_type>,
        QtPrivate::TypeAndForceComplete<StatusCondition, std::false_type>,
        // method 'statStageChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<StatType, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'creatureSwitched'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<Creature *, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'battleLogUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'playerActionConfirmed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'opponentActionConfirmed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void BattleSystem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BattleSystem *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->battleStarted(); break;
        case 1: _t->battleEnded((*reinterpret_cast< std::add_pointer_t<BattleResult>>(_a[1]))); break;
        case 2: _t->turnStarted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 3: _t->turnEnded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->skillUsed((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<Skill*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[5]))); break;
        case 5: _t->damageCaused((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->healingReceived((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->statusChanged((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<StatusCondition>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<StatusCondition>>(_a[3]))); break;
        case 8: _t->statStageChanged((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<StatType>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4]))); break;
        case 9: _t->creatureSwitched((*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Creature*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 10: _t->battleLogUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->playerActionConfirmed(); break;
        case 12: _t->opponentActionConfirmed(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (BattleSystem::*)();
            if (_q_method_type _q_method = &BattleSystem::battleStarted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(BattleResult );
            if (_q_method_type _q_method = &BattleSystem::battleEnded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(int , bool );
            if (_q_method_type _q_method = &BattleSystem::turnStarted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(int );
            if (_q_method_type _q_method = &BattleSystem::turnEnded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(Creature * , Creature * , Skill * , bool , int );
            if (_q_method_type _q_method = &BattleSystem::skillUsed; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(Creature * , int );
            if (_q_method_type _q_method = &BattleSystem::damageCaused; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(Creature * , int );
            if (_q_method_type _q_method = &BattleSystem::healingReceived; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(Creature * , StatusCondition , StatusCondition );
            if (_q_method_type _q_method = &BattleSystem::statusChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(Creature * , StatType , int , int );
            if (_q_method_type _q_method = &BattleSystem::statStageChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(Creature * , Creature * , bool );
            if (_q_method_type _q_method = &BattleSystem::creatureSwitched; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)(const QString & );
            if (_q_method_type _q_method = &BattleSystem::battleLogUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)();
            if (_q_method_type _q_method = &BattleSystem::playerActionConfirmed; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _q_method_type = void (BattleSystem::*)();
            if (_q_method_type _q_method = &BattleSystem::opponentActionConfirmed; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
    }
}

const QMetaObject *BattleSystem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BattleSystem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12BattleSystemE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BattleSystem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void BattleSystem::battleStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void BattleSystem::battleEnded(BattleResult _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void BattleSystem::turnStarted(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void BattleSystem::turnEnded(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void BattleSystem::skillUsed(Creature * _t1, Creature * _t2, Skill * _t3, bool _t4, int _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void BattleSystem::damageCaused(Creature * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void BattleSystem::healingReceived(Creature * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void BattleSystem::statusChanged(Creature * _t1, StatusCondition _t2, StatusCondition _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void BattleSystem::statStageChanged(Creature * _t1, StatType _t2, int _t3, int _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void BattleSystem::creatureSwitched(Creature * _t1, Creature * _t2, bool _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void BattleSystem::battleLogUpdated(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void BattleSystem::playerActionConfirmed()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void BattleSystem::opponentActionConfirmed()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}
QT_WARNING_POP

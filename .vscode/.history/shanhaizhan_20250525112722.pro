QT += core gui widgets multimedia

CONFIG += c++17

HEADERS += \
    src/ui/scenes/battlescene.h \
    src/ui/scenes/mainwindow.h \
    src/ui/scenes/preparationscene.h \
    src/resource/resourcemanager.h \
    src/core/status/battleeffects.h \
    src/core/status/bleedingstatus.h \
    src/core/status/burnstatus.h \
    src/core/status/confusionstatus.h \
    src/core/status/fatiguestatus.h \
    src/core/status/fearstatus.h \
    src/core/status/frozenstatus.h \
    src/core/status/paralysisstatus.h \
    src/core/status/poisonstatus.h \
    src/core/status/sleepstatus.h \
    src/core/status/status.h \
    src/core/status/statusfactory.h \
    src/core/status/unbreakablestatus.h \
    src/core/spirit/spirit.h \
    src/core/skill/skill.h \
    src/core/effect/damageeffect.h \
    src/core/effect/effect.h \
    src/core/effect/healingeffect.h \
    src/core/effect/immuneeffect.h \
    src/core/effect/priorityeffect.h \
    src/core/effect/statchangeeffect.h \
    src/core/effect/statuseffect.h \
    src/core/effect/traiteffect.h \
    src/core/effect/turnbasedeffect.h \
    src/core/battle/actionresult.h \
    src/core/battle/battle.h \
    src/core/battle/battleaction.h \
    src/core/battle/battleobserver.h \
    src/core/battle/battleturn.h \
    src/core/typechart.h

SOURCES += \
    src/main.cpp \
    src/ui/scenes/battlescene.cpp \
    src/ui/scenes/mainwindow.cpp \
    src/ui/scenes/preparationscene.cpp \
    src/resource/resourcemanager.cpp \
    src/core/status/battleeffects.cpp \
    src/core/status/bleedingstatus.cpp \
    src/core/status/burnstatus.cpp \
    src/core/status/confusionstatus.cpp \
    src/core/status/fatiguestatus.cpp \
    src/core/status/fearstatus.cpp \
    src/core/status/frozenstatus.cpp \
    src/core/status/paralysisstatus.cpp \
    src/core/status/poisonstatus.cpp \
    src/core/status/sleepstatus.cpp \
    src/core/status/status.cpp \
    src/core/status/statusfactory.cpp \
    src/core/status/unbreakablestatus.cpp \
    src/core/spirit/spirit.cpp \
    src/core/skill/skill.cpp \
    src/core/effect/damageeffect.cpp \
    src/core/effect/effect.cpp \
    src/core/effect/healingeffect.cpp \
    src/core/effect/immuneeffect.cpp \
    src/core/effect/priorityeffect.cpp \
    src/core/effect/statchangeeffect.cpp \
    src/core/effect/statuseffect.cpp \
    src/core/effect/traiteffect.cpp \
    src/core/effect/turnbasedeffect.cpp \
    src/core/battle/actionresult.cpp \
    src/core/battle/battle.cpp \
    src/core/battle/battleaction.cpp \
    src/core/battle/battleturn.cpp \
    src/core/typechart.cpp

FORMS += \
    src/ui/scenes/mainwindow.ui \
    src/ui/scenes/preparationscene.ui \
    src/ui/scenes/battlescene.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
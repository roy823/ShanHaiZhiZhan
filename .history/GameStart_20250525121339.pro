QT       += core gui widgets multimedia

TARGET = ShanHaiZhiZhan
TEMPLATE = app

CONFIG += c++17

# 源码文件
SOURCES += \
    src/main.cpp \
    src/core/creature.cpp \
    src/core/ability.cpp \
    src/core/type.cpp \
    src/core/gameengine.cpp \
    src/core/savesystem.cpp \
    src/battle/battlesystem.cpp \
    src/battle/skill.cpp \
    src/battle/effect.cpp \
    src/ui/mainwindow.cpp \
    src/ui/battlescene.cpp \
    src/ui/preparescene.cpp \
    src/ui/loadgamedialog.cpp \
    src/ui/savegamedialog.cpp

# 头文件
HEADERS += \
    src/core/creature.h \
    src/core/ability.h \
    src/core/type.h \
    src/core/gameengine.h \
    src/battle/battlesystem.h \
    src/battle/skill.h \
    src/battle/effect.h \
    src/ui/mainwindow.h \
    src/ui/battlescene.h \
    src/ui/preparescene.h

# UI文件
FORMS += \
    src/ui/mainwindow.ui \
    src/ui/battlescene.ui \
    src/ui/preparescene.ui

# 资源文件
RESOURCES += \
    src/resources/resources.qrc

# 默认规则，使新编译器的警告作为错误
QMAKE_CXXFLAGS += -Wall -Wextra -pedantic

# 指定编译路径
DESTDIR = $$PWD/build/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
UI_DIR = $$PWD/build/ui
RCC_DIR = $$PWD/build/rcc

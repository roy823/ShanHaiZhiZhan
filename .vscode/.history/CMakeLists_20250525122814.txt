cmake_minimum_required(VERSION 3.16)
project(ShanHaiZhiZhan VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加MSVC特定的编译器选项
if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

# 添加资源文件
set(RESOURCE_FILES
    src/resources/resources.qrc
)

# 定义源文件
set(SOURCE_FILES
    src/main.cpp
    
    # 核心组件
    src/core/creature.h
    src/core/creature.cpp
    src/core/ability.h
    src/core/ability.cpp
    src/core/type.h
    src/core/type.cpp
    src/core/gameengine.h
    src/core/gameengine.cpp
    src/core/savesystem.h
    src/core/savesystem.cpp
    
    # 战斗系统
    src/battle/battlesystem.h
    src/battle/battlesystem.cpp
    src/battle/skill.h
    src/battle/skill.cpp
    src/battle/effect.h
    src/battle/effect.cpp
    
    # UI组件
    src/ui/mainwindow.h
    src/ui/mainwindow.cpp
    src/ui/mainwindow.ui
    src/ui/battlescene.h
    src/ui/battlescene.cpp
    src/ui/battlescene.ui
    src/ui/preparescene.h
    src/ui/preparescene.cpp
    src/ui/preparescene.ui
    src/ui/loadgamedialog.h
    src/ui/loadgamedialog.cpp
    src/ui/savegamedialog.h
    src/ui/savegamedialog.cpp
)

# 添加执行文件
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${RESOURCE_FILES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

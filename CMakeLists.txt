cmake_minimum_required(VERSION 3.5)
project(objViewer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理Qt的moc、uic、rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

if(UNIX AND NOT APPLE)  # 检查是否为Linux系统（Unix且不是苹果）
    message(STATUS "Building for Linux system")
    
    find_package(Eigen3 REQUIRED)
    set(MY_TRI ${CMAKE_CURRENT_SOURCE_DIR}/meshutils/build/lib/libmymesh.so)
    
    # 检查 MY_TRI 库是否存在
    if(NOT EXISTS ${MY_TRI})
        message(WARNING "MeshIO library not found at ${MY_TRI}. You may need to build it first.")
    else()
        message(STATUS "Found MY_TRI library: ${MY_TRI}")
    endif()
    
    # 查找Qt5组件
    find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)
    # 添加着色器资源文件
    qt5_add_resources(RESOURCE_FILES shaders.qrc)
    
    find_package(OpenMesh REQUIRED)
    
    # 添加可执行文件
    add_executable(${PROJECT_NAME}
        main.cpp
        glwidget/baseglwidget.h
        glwidget/baseglwidget.cpp
        menu_utils.cpp
        menu_utils.h
        tab_manager.cpp
        tab_manager.h
        tabs/basic_tab.h
        ${MY_TRI}
        ${RESOURCE_FILES}
    )
    
    # 链接库
    target_link_libraries(${PROJECT_NAME}
        Qt5::Widgets
        Qt5::OpenGL
        GL
        Eigen3::Eigen
        OpenMeshCore
        OpenMeshTools
        ${MY_TRI}
    )
    
    # 设置安装路径
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    
elseif(APPLE)
    message(FATAL_ERROR "macOS build not yet implemented. Please add macOS-specific configuration here.")
elseif(WIN32)
    message(FATAL_ERROR "Windows build not yet implemented. Please add Windows-specific configuration here.")
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()
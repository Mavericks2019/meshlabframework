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
    find_package(OpenMesh REQUIRED)
    find_package(Eigen3 REQUIRED)
    find_package(CGAL REQUIRED)
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
    
    
    # 添加可执行文件
    add_executable(${PROJECT_NAME}
        main.cpp
        glwidget/baseglwidget.h
        glwidget/baseglwidget.cpp
        glwidget/cgalglwidget.h
        glwidget/cgalglwidget.cpp
        menu_utils.cpp
        menu_utils.h
        tab_manager.cpp
        tab_manager.h
        tabs/basic_tab.h
        tabs/cgal_tab.h
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
        CGAL::CGAL
        ${MY_TRI}
    )
    
    # 设置安装路径
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    
elseif(APPLE)
    message(FATAL_ERROR "macOS build not yet implemented. Please add macOS-specific configuration here.")
elseif(WIN32)
    # Windows 特定配置
    message(STATUS "Building for Windows system")
    
    # 设置Qt5路径
    set(QT5_ROOT "D:/qt/5.11.1/msvc2017_64")
    set(CMAKE_PREFIX_PATH "${QT5_ROOT};${CMAKE_PREFIX_PATH}")
    
    # 设置Eigen3包含目录
    set(EIGEN3_INCLUDE_DIR "C:/Users/Administrator/vcpkg/installed/x64-windows/include/eigen3")
    
    # 设置CGAL路径（使用vcpkg安装）
    set(CGAL_DIR "C:/Users/Administrator/vcpkg/installed/x64-windows/share/cgal")
    set(CGAL_INCLUDE_DIR "C:/Users/Administrator/vcpkg/installed/x64-windows/include")
    set(CGAL_LIB_DIR "C:/Users/Administrator/vcpkg/installed/x64-windows/lib")
    
    # 设置MY_TRI库路径
    set(MY_TRI_LIB "D:/projects/meshlab_framework/libs/mymesh.lib")
    set(MY_TRI_DLL "D:/projects/meshlab_framework/libs/mymesh.dll")
    
    # 检查MY_TRI库是否存在
    if(NOT EXISTS ${MY_TRI_LIB})
        message(WARNING "MY_TRI library not found at ${MY_TRI_LIB}.")
    else()
        message(STATUS "Found MY_TRI library: ${MY_TRI_LIB}")
    endif()
    
    # 设置OpenMesh路径（假设使用vcpkg安装）
    set(OPENMESH_INCLUDE_DIR "C:/Users/Administrator/vcpkg/installed/x64-windows/include")
    set(OPENMESH_LIB_DIR "C:/Users/Administrator/vcpkg/installed/x64-windows/lib")
    
    # 查找Qt5组件
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL)
    
    # 添加着色器资源文件
    qt5_add_resources(RESOURCE_FILES shaders.qrc)
    
    # 添加可执行文件
    add_executable(${PROJECT_NAME}
        main.cpp
        glwidget/baseglwidget.h
        glwidget/baseglwidget.cpp
        glwidget/cgalglwidget.h
        glwidget/cgalglwidget.cpp
        menu_utils.cpp
        menu_utils.h
        tab_manager.cpp
        tab_manager.h
        tabs/basic_tab.h
        tabs/cgal_tab.h
        ${RESOURCE_FILES}
    )
    
    # 设置包含目录
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${EIGEN3_INCLUDE_DIR}
        ${OPENMESH_INCLUDE_DIR}
        ${CGAL_INCLUDE_DIR}
    )
    
    # 设置编译器定义
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _USE_MATH_DEFINES  # 解决编译器错误
        NOMINMAX           # 防止Windows min/max宏与std::min/max冲突
        UNICODE
        _UNICODE
    )
    
    # 链接库
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::OpenGL
        ${MY_TRI_LIB}
        "${OPENMESH_LIB_DIR}/OpenMeshCore.lib"
        "${OPENMESH_LIB_DIR}/OpenMeshTools.lib"
        "${CGAL_LIB_DIR}/CGAL.lib"
        opengl32.lib
    )
    
    # 设置输出目录
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    
    # 复制必要的DLL到输出目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT5_ROOT}/bin/Qt5Core.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT5_ROOT}/bin/Qt5Widgets.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT5_ROOT}/bin/Qt5Gui.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT5_ROOT}/bin/Qt5OpenGL.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
    )
    
    # 复制MY_TRI DLL
    if(EXISTS ${MY_TRI_DLL})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MY_TRI_DLL}"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        )
    endif()
    
    # 复制OpenMesh DLL
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENMESH_LIB_DIR}/../bin/OpenMeshCore.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENMESH_LIB_DIR}/../bin/OpenMeshTools.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
    )
    
    # 复制CGAL DLL
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CGAL_LIB_DIR}/../bin/CGAL.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
    )
    
    if(MSVC)
        # 禁用某些警告
        add_compile_options(/wd4251)  # 禁用dll接口警告
        add_compile_options(/wd4275)  # 禁用非dll接口警告
        add_compile_options(/wd4996)  # 禁用安全警告
    endif()

else()
    message(FATAL_ERROR "Unsupported operating system")
endif()
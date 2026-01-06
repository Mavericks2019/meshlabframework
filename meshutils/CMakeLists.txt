cmake_minimum_required(VERSION 3.10)
project(mymesh VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 确保 .lib 文件也输出到 lib 目录

if(UNIX AND NOT APPLE) 
    # 寻找依赖包
    find_package(OpenMesh REQUIRED)
    find_package(Eigen3 REQUIRED)

    # 包含目录
    include_directories(${OPENMESH_INCLUDE_DIR} ${EIGEN3_INCLUDE_DIR})

    # 源文件
    set(SOURCES
        my_traits.cpp
    )

    # 头文件
    set(HEADERS
        my_traits.h
    )

    # 创建动态链接库
    add_library(mymesh SHARED ${SOURCES} ${HEADERS})

    # 链接库
    target_link_libraries(mymesh ${OPENMESH_LIBRARIES})

    # 设置属性（可选）
    set_target_properties(mymesh PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER "${HEADERS}"
    )

    # 安装规则（如果需要）
    install(TARGETS mymesh
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
    )

    # 测试可执行文件（可选）
    if(BUILD_TESTING)
        enable_testing()
        add_executable(test_mymesh test.cpp)  # 需要创建test.cpp
        target_link_libraries(test_mymesh mymesh)
    endif()

elseif(APPLE)
    message(FATAL_ERROR "macOS build not yet implemented. Please add macOS-specific configuration here.")
    
elseif(WIN32)
    # Windows 配置
    message(STATUS "Configuring for Windows build")
    
    # OpenMesh 静态库配置
    set(OPENMESH_INCLUDE_DIR "D:/projects/AAAtlas/include")
    set(OPENMESH_LIBRARIES 
        "D:/projects/AAAtlas/lib/OpenMeshCore.lib"
        "D:/projects/AAAtlas/lib/OpenMeshTools.lib"
    )
    
    # Eigen 头文件
    set(EIGEN3_INCLUDE_DIR "D:/projects/AAAtlas/include/Eigen")
    
    # 包含目录
    include_directories(${OPENMESH_INCLUDE_DIR} ${EIGEN3_INCLUDE_DIR})
    
    # 源文件
    set(SOURCES
        my_traits.cpp
    )
    
    # 头文件
    set(HEADERS
        my_traits.h
    )
    
    # 强制所有符号都被导出（这会生成 .lib 文件，即使没有显式导出符号）
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    
    # 创建动态链接库
    add_library(mymesh SHARED ${SOURCES} ${HEADERS})
    
    # 链接OpenMesh静态库
    target_link_libraries(mymesh ${OPENMESH_LIBRARIES})
    
    # 设置Windows动态库属性
    set_target_properties(mymesh PROPERTIES
        VERSION ${PROJECT_VERSION}
        PUBLIC_HEADER "${HEADERS}"
        # 明确设置输出文件的位置和名称
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        # 设置输出文件命名规则
        RUNTIME_OUTPUT_NAME mymesh
        ARCHIVE_OUTPUT_NAME mymesh
        LIBRARY_OUTPUT_NAME mymesh
    )
    
    # Windows 特定编译器选项
    if(MSVC)
        # 添加Windows DLL导出相关定义
        target_compile_definitions(mymesh PRIVATE 
            NOMINMAX  # 禁用min/max宏
            _USE_MATH_DEFINES  # 启用数学常量定义（如M_PI）
        )
    endif()
    
    # 确保在Windows上生成 .lib 和 .dll
    # 添加一个空的源文件来确保有符号被导出
    add_custom_command(TARGET mymesh POST_BUILD
        COMMENT "确保生成了 .lib 和 .dll 文件"
    )
    
    # 安装规则（如果需要）
    install(TARGETS mymesh
        RUNTIME DESTINATION bin      # .dll 文件
        LIBRARY DESTINATION lib      # .dll 文件
        ARCHIVE DESTINATION lib      # .lib 文件（导入库）
        PUBLIC_HEADER DESTINATION include
    )
    
    # 测试可执行文件（可选）
    if(BUILD_TESTING)
        enable_testing()
        add_executable(test_mymesh test.cpp)
        target_link_libraries(test_mymesh mymesh)
        
        # 如果测试程序也需要这些宏定义
        if(MSVC)
            target_compile_definitions(test_mymesh PRIVATE 
                NOMINMAX
                _USE_MATH_DEFINES
            )
        endif()
    endif()
    
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()
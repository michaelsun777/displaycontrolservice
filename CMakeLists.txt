CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

project(DisplayControlServer)



# 添加自定义命令来运行Makefile???
# add_custom_command(
#         TARGET MyTarget
#         PRE_BUILD
#         COMMAND make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/3rd/nvidia-settings/
#         COMMON "make nvidia-settings"
#         VERBATIM    
# )
 


# if (${CMAKE_TOOLCHAIN_FILE} MATCHES ".*vcpkg.cmake.*")
#     message(STATUS "Building with vcpkg toolchain.")
#     set(USING_VCPKG ON)
#     set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/triplets)
# endif()
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")

# SET(CMAKE_BUILD_TYPE "Release")
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) #相当于-fpic


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)





#Check C++11
# include(CheckCXXCompilerFlag)
# if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
#         CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#     check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
#     if(NOT SUPPORTS_CXX11)
#         message(FATAL_ERROR "Compiler doesn't support C++11")
#     endif()
# endif()


#ADD_SUBDIRECTORY(Fast-DDS)




# �˴�������thread��chrono  �������������Ҫ��
find_package(Boost REQUIRED COMPONENTS thread chrono)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
endif()




########## Find FastDDS requirements ############
# find_package(fastcdr REQUIRED)
# find_package(fastrtps REQUIRED)
# if(NOT fastcdr_FOUND)
#     find_package(fastcdr REQUIRED)
# endif()

# if(NOT foonathan_memory_FOUND)
#     find_package(foonathan_memory REQUIRED)
# endif()

# set(fastrtps_FOUND TRUE)
# if(NOT fastrtps_FOUND)
#     find_package(fastrtps REQUIRED)
# endif()
########## Find FastDDS requirements end ############


# find_package( OpenCV 4 REQUIRED )
# INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS}) # opencv


#set(QT_PATH "/opt/Qt5.14.2/5.14.2/gcc_64/")
#set(CMAKE_PREFIX_PATH ${QT_PATH}/lib/cmake)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
## Set the QT_LIBRARIES variable for Qt5, so it can be used below.
# set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Network)
SET(QT_LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES}
        ${Qt5Network_LIBRARIES} ${Qt5Xml_LIBRARIES} ${Qt5Qml_LIBRARIES} ${Qt5WebEngineWidgets_LIBRARIES} )

INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS} ${Qt5Qml_INCLUDE_DIRS} ${Qt5WebEngineWidgets_INCLUDE_DIRS})


# MESSAGE(STATUS "OpenCV library status:")
# MESSAGE(STATUS "    version: ${OpenCV_VERSION}")
# MESSAGE(STATUS "    libraries: ${OpenCV_LIBS}")
# MESSAGE(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


# INCLUDE_DIRECTORIES(Fast-DDS)


# INCLUDE_DIRECTORIES(hidapi)
# INCLUDE_DIRECTORIES(hidapi/hidapi)
# INCLUDE_DIRECTORIES(hidapi/linux)
# include_directories(/usr/local/include/osgEarth)
INCLUDE_DIRECTORIES(3rd/json/include/nlohmann)
INCLUDE_DIRECTORIES(3rd/json/include)
INCLUDE_DIRECTORIES(3rd/spdlog/)
INCLUDE_DIRECTORIES(3rd/spdlog/include)
INCLUDE_DIRECTORIES(3rd/md5/src/)
#INCLUDE_DIRECTORIES(/usr/include/spdlog)
INCLUDE_DIRECTORIES(/usr/include/X11)

# INCLUDE_DIRECTORIES(/usr/include/gtk-2.0/)
# INCLUDE_DIRECTORIES(/usr/include/glib-2.0)
# INCLUDE_DIRECTORIES(/usr/include/glib-2.0/glib)
# INCLUDE_DIRECTORIES(/usr/lib/x86_64-linux-gnu/glib-2.0/include)
# INCLUDE_DIRECTORIES(/usr/lib/x86_64-linux-gnu/gtk-2.0/include/)
# INCLUDE_DIRECTORIES(/usr/include/cairo)
# INCLUDE_DIRECTORIES(/usr/include/pango-1.0)
# INCLUDE_DIRECTORIES(/usr/include/harfbuzz)
# INCLUDE_DIRECTORIES(3rd/cpp-httplib/cpp-httplib-base/)
# INCLUDE_DIRECTORIES(3rd/cpp-subprocess/)
INCLUDE_DIRECTORIES(3rd/nvidia-settings/src/libXNVCtrl)
INCLUDE_DIRECTORIES(3rd/nvidia-settings/src/libXNVCtrlAttributes)
INCLUDE_DIRECTORIES(3rd/httpserver/)
INCLUDE_DIRECTORIES(src/UI)

INCLUDE_DIRECTORIES(src)
# INCLUDE_DIRECTORIES(src/communication)
# INCLUDE_DIRECTORIES(src/DLPTriangle)
# INCLUDE_DIRECTORIES(src/include_bd)
# INCLUDE_DIRECTORIES(src/include_hk)
# INCLUDE_DIRECTORIES(src/other)
# INCLUDE_DIRECTORIES(src/spdlog)
# INCLUDE_DIRECTORIES(src/dlp/)
# INCLUDE_DIRECTORIES(/usr/local/include/hidapi)
# INCLUDE_DIRECTORIES(/usr/local/cuda/include/)

#INCLUDE_DIRECTORIES(nlohmann_json)
#INCLUDE_DIRECTORIES(nlohmann_json/include)


# aux_source_directory(src/communication communication_SRC)
# aux_source_directory(src/PCLAlgorithm PCLAlgorithm_SRC)
# aux_source_directory(hidapi hidapi_SRC)
# aux_source_directory(hidapi/linux hidapi_linux_SRC)


# aux_source_directory(src/include_hk include_hk_SRC)
# aux_source_directory(src/other other_SRC)
# aux_source_directory(src/spdlog spdlog_SRC)

aux_source_directory(3rd/httpserver http_SRC)

##find_path(CPPUNIT_INCLUDE_DIR PATHS "/usr/include/x86_64-linux-gnu" NO_DEFAULT_PATH)       
##find_library(CPPUNIT_LIBRARY PATHS "/usr/lib/x86_64-linux-gnu" NO_DEFAULT_PATH)

#link_directories(/opt/Qt5.14.2/5.14.2/gcc_64/lib/)
link_directories(${QT_LIBRARIES})
link_directories(/usr/lib/x86_64-linux-gnu)
# link_directories(/opt/MVS/lib/64/)
# link_directories(hidapi/linux/.libs)
#link_directories(src/arm_tool/lib)
link_directories(/usr/local/lib/)
link_directories(/home/amplesky/work/displayctrolservice/3rd/nvidia-settings/src/_out/Linux_x86_64/)


aux_source_directory(src/ SRC_SRC)
# aux_source_directory(src/UI SRC_SRC_UI)

add_executable(dpcs
main.cpp
# ${SRC_SRC}
src/build_date_time.cpp
src/cspdlog.cpp
# src/dlgManager.cpp
src/cmyxrandr.cpp
src/exec_cmd.cpp

3rd/nvidia-settings/src/libXNVCtrl/NVCtrl.c
src/UI/dlgurl.cpp
src/UI/dlgurl.h
src/UI/dlgurl.ui
src/UI/mainwindow.cpp
src/UI/mainwindow.h
src/UI/mainwindow.ui
src/cdataProcess.cpp
src/config.cpp
# src/httpManager.cpp
src/nvControlInfo.cpp
######dlg
src/dialogController.cpp
src/requesthandler.cpp
3rd/cpp-subprocess/subprocess.hpp
src/CNvControlEvents.cpp
3rd/md5/src/md5.cpp
src/autoDelete.cpp

${http_SRC}

# ${spdlog_SRC} 
)

#add_subdirectory(3rd/nvidia-settings)
add_subdirectory(3rd/spdlog)

target_link_libraries(dpcs PRIVATE
pthread
#  -lusb -ludev -lhidapi-hidraw
# -lboost_system -lboost_thread -lboost_filesystem -lboost_iostreams -lboost_chrono
# -lMvCameraControl -luuid -lcurl -lstdc++fs
${QT_LIBRARIES}
-lspdlog 
-lX11 -lXrandr -lm -lXext #-lXNVCtrl #  // 
#-LXNVCtrl
#-L/usr/lib/x86_64-linux-gnu/libXNVCtrl.so.0
-ldl


# ${OpenCV_LIBS} 
${Boost_LIBRARIES}

# osgearth lib
# -lOpenThreads
# -losg
# -losgDB
# -losgGA
# -losgUtil
# -losgViewer

####FastDDS lib#####
# fastcdr fastrtps #to_robot_lib to_camera_lib


#######��ʱ�ò���
#Qt5::Gui Qt5::Widgets Qt5::Core Qt5::Network 
# /opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5OpenGL.so.5
# /opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Sql.so.5
# /opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Network.so.5
# /opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Widgets.so.5
# /opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Gui.so.5
# /opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Core.so.5
# /opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5WebEngine.so.5

)


#file(GLOB infer_lib  ${PROJECT_SOURCE_DIR}/src/infer_lib/*)
#file(COPY ${infer_lib} DESTINATION infer_lib)

#file(GLOB include_hk  ${PROJECT_SOURCE_DIR}/src/include_hk/*)
#file(COPY ${include_hk} DESTINATION include_hk)

#file(GLOB lib  ${PROJECT_SOURCE_DIR}/src/lib/MVS/64/*)
#file(COPY ${include_hk} DESTINATION lib/MVS/64/)

#configure_file(src/dlp/PixelConfig.ini PixelConfig.ini COPYONLY)
#configure_file(src/dlp/dlpConfig.ini dlpConfig.ini COPYONLY)

#~/code/fastdds/Fast-DDS-Gen/scripts/fastddsgen -example CMake testSecurity.idl

configure_file(${CMAKE_SOURCE_DIR}/dpcs.service.in ${CMAKE_CURRENT_BINARY_DIR}/dpcs.service @ONLY)

INSTALL(TARGETS dpcs CONFIGURATIONS Debug RUNTIME DESTINATION /usr/src/dpcs/)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/dpcs.service DESTINATION /etc/systemd/system/)

#INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/dpcs DESTINATION /usr/src/dpcs)



#说明要生成的是deb包
SET(CPACK_GENERATOR "DEB")
# 指定deb包的版本号
set(_VERSION_MAJOR 0)
set(_VERSION_MINOR 0)
set(_VERSION_PATCH 1)
set(CPACK_PACKAGE_VERSION_MAJOR "${_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${_VERSION_PATCH}")
#设置架构
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

# 指定程序名，就是程序安装后的名字
set(CPACK_DEBIAN_PACKAGE_NAME "dpcs")
#set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_NAME "dpcs")

#设置依赖
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6)")
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libgcc1 (>= 1:3.0)")
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS qtbase5-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS libboost-all-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS qtcreator)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS qtwebengine5-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS libspdlog-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS libgtk2.0-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS libgtk-3-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS libxv-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS libxrandr-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS libgl1-mesa-dev libgtk-3-dev libxxf86vm-dev libvdpau-dev)
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS cmake)
#SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a, libqt5widgets5, libqt5network5, libqt5gui5")

#设置description
SET(CPACK_PACKAGE_DESCRIPTION "dpcs description")

#设置联系方式
#SET(CPACK_PACKAGE_CONTACT "sunhaitao@amplesky.com")

#设置维护人
#set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Michael Sun")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Michael Sun <sunhaitao@amplesky.com>")

#获取编译时间
string(TIMESTAMP COMPILE_TIME %Y%m%d_%H%M%S)
set(build_time    ${COMPILE_TIME})
#设置打出来的包名
#set(CPACK_PACKAGE_FILE_NAME  xcl-${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${COMPILE_TIME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}) 
set(CPACK_PACKAGE_FILE_NAME  ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}-${CPACK_PACKAGE_VERSION_MINOR}-${CPACK_PACKAGE_VERSION_PATCH}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}) 

#设置安装路径
#set(CPACK_PACKAGING_INSTALL_PREFIX  "/home/xcl/install")

#设置包的描述文件
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/INSTALL.md")



include(CPack)


##############debian/control设置完成了，下面设置控制脚本，动态库，changelog等#################

#lib目录下保存这一些动态库和安装控制脚本（postinst、postrm、preinst、prerm）  
#doc目录下是changelog和copyright等文件

#SET(LIBS_DIR "$ENV{DBXML_DIR}/lib")
#将库文件安装到/usr/lib下
#INSTALL(FILES ${LIBS_DIR}/libdb-4.8.so ${LIBS_DIR}/libdbxml-2.5.so ${LIBS_DIR}/libxerces-c-3.0.so ${LIBS_DIR}/libxqilla.so.5 ${LIBS_DIR}/libxqilla.so.5.0.2  DESTINATION /usr/lib)

#SET(DOCS_DIR ${CMAKE_SOURCE_DIR}/doc)

#将pro-cc.conf文件安装时copy到/etc/init下
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/pro-cc.conf DESTINATION /etc/init)



##############################其他选项#############################
#设置section
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

#设置priority
set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")
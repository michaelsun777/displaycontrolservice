CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

project(DisplayCtrlServer)


# if (${CMAKE_TOOLCHAIN_FILE} MATCHES ".*vcpkg.cmake.*")
#     message(STATUS "Building with vcpkg toolchain.")
#     set(USING_VCPKG ON)
#     set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/triplets)
# endif()


SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) #相当于-fpic


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)

# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_EXTENSIONS OFF)

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


# find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
# find_package(Qt5Core REQUIRED)
# find_package(Qt5Widgets REQUIRED)
# find_package(Qt5Network REQUIRED)
# find_package(Qt5Gui REQUIRED)
# find_package(Qt5OpenGL REQUIRED)
## Set the QT_LIBRARIES variable for Qt5, so it can be used below.
#set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Network)
# SET(QT_LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES}
#         ${Qt5Network_LIBRARIES} ${Qt5Xml_LIBRARIES} ${Qt5Qml_LIBRARIES})

# include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}
#         ${Qt5Widgets_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS}
#         ${Qt5Xml_INCLUDE_DIRS} ${Qt5Qml_INCLUDE_DIRS}
#         )


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
INCLUDE_DIRECTORIES(/usr/include/spdlog)
INCLUDE_DIRECTORIES(/usr/include/X11)

# INCLUDE_DIRECTORIES(/usr/include/gtk-2.0/)
# INCLUDE_DIRECTORIES(/usr/include/glib-2.0)
# INCLUDE_DIRECTORIES(/usr/include/glib-2.0/glib)
# INCLUDE_DIRECTORIES(/usr/lib/x86_64-linux-gnu/glib-2.0/include)
# INCLUDE_DIRECTORIES(/usr/lib/x86_64-linux-gnu/gtk-2.0/include/)
# INCLUDE_DIRECTORIES(/usr/include/cairo)
# INCLUDE_DIRECTORIES(/usr/include/pango-1.0)
# INCLUDE_DIRECTORIES(/usr/include/harfbuzz)
INCLUDE_DIRECTORIES(3rd/cpp-httplib/cpp-httplib-base/)
INCLUDE_DIRECTORIES(3rd/cpp-subprocess/)
INCLUDE_DIRECTORIES(3rd/nvidia-settings/src/libXNVCtrl)
INCLUDE_DIRECTORIES(3rd/nvidia-settings/src/libXNVCtrlAttributes)

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
# aux_source_directory(src/dlp dlp_SRC)

##find_path(CPPUNIT_INCLUDE_DIR PATHS "/usr/include/x86_64-linux-gnu" NO_DEFAULT_PATH)       
##find_library(CPPUNIT_LIBRARY PATHS "/usr/lib/x86_64-linux-gnu" NO_DEFAULT_PATH)

#link_directories(/opt/Qt5.14.2/5.14.2/gcc_64/lib/)
#link_directories(${QT_LIBRARIES})
link_directories(/usr/lib/x86_64-linux-gnu)
# link_directories(/opt/MVS/lib/64/)
# link_directories(hidapi/linux/.libs)
#link_directories(src/arm_tool/lib)
link_directories(/usr/local/lib/)


aux_source_directory(src/ SRC_SRC)

add_executable(dpcs
main.cpp
${SRC_SRC}
3rd/nvidia-settings/src/libXNVCtrl/NVCtrl.c


# src/comm.cpp
#src/dlpManager.cpp
#src/dlpsetdlg.cpp
#src/dlpsetdlg.ui
#src/hkcamera.cpp
#src/hkImage.cpp
#src/imagepixelcalculate.cpp
#src/MsgDataProcess.cpp
#src/MvCamera.cpp
#src/paddlePredictThread.cpp
#src/paddlexmodel.cpp
#./hidapi/linux/hid.c
#Fast-DDS/to_camera.cxx
#Fast-DDS/to_cameraSubscriber.cxx
#Fast-DDS/to_robot.cxx
#Fast-DDS/to_robotPublisher.cxx
#Fast-DDS/to_cameraSubscriber.cxx
#Fast-DDS/to_cameraPubSubTypes.cxx
#Fast-DDS/to_robotPubSubTypes.cxx
# ${communication_SRC}
${spdlog_SRC} 
#${hidapi_SRC} ${hidapi_linux_SRC}
#${include_bd_SRC} ${include_hk_SRC} ${other_SRC} ${dlp_SRC}} 
)


target_link_libraries(dpcs 
pthread
#  -lusb -ludev -lhidapi-hidraw
-lboost_system -lboost_thread -lboost_filesystem -lboost_iostreams -lboost_chrono
# -lMvCameraControl -luuid -lcurl -lstdc++fs
-lspdlog 
-lX11 -lXext -lXrandr -lm  # -lXNVCtrl
-ldl
${QT_LIBRARIES}

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
#/opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5OpenGL.so.5
#/opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Sql.so.5
#/opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Network.so.5
#/opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Widgets.so.5
#/opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Gui.so.5
#/opt/Qt5.14.2/5.14.2/gcc_64/lib/libQt5Core.so.5

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
sudo apt-get install cmake libboost-all-dev qtcreator qtbase5-dev qtwebengine5-dev libspdlog-dev libgtk2.0-dev libgtk-3-dev libxv-dev libxrandr-dev libgl1-mesa-dev libgtk-3-dev libxxf86vm-dev libvdpau-dev libxcb-randr0-dev libsystemd-dev libyaml-cpp-dev libcurl4-openssl-dev -y 

## 编译QCefView前替换指定文件
用 src/UI/cef/CefViewSchemeHandlerFactory.cpp 文件替换子库生成的 CefViewSchemeHandlerFactory.cpp 文件。

* 方式一：

修改QCefView目录下的cmake文件，在末尾加上下面内容，然后再编译子库。

```
set(COPY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../src/UI/cef/CefViewSchemeHandlerFactory.cpp" "${CMAKE_SOURCE_DIR}/../../src/UI/cef/CefViewSchemeHandlerFactory.h")
set(COPY_TARGET_DIR "${CMAKE_BINARY_DIR}/_deps/cefviewcore-src/src/CefView/CefBrowserApp/CefViewSchemeHandler/")

if(EXISTS ${COPY_TARGET_DIR})
    foreach(FILE ${COPY_SOURCE_DIR})
        file(COPY ${FILE} DESTINATION ${COPY_TARGET_DIR})
    endforeach()
endif()
```

* 方式二：

先cmake QCefView子库，在生成的文件中找到 CefViewSchemeHandlerFactory.cpp 文件，用 src/UI/cef/CefViewSchemeHandlerFactory.cpp 将其替换，然后再make编译。

* 验证是否成功

在make编译子库前检查生成的 CefViewSchemeHandlerFactory.cpp 与 src/UI/cef/CefViewSchemeHandlerFactory.cpp 文件内容相同即可。
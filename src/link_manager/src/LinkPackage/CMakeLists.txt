

<<<<<<< HEAD
set(LINK_PACKAGE_RAW_FILES
    bridge.h bridge.cpp
    linkconfiguration.h linkconfiguration.cpp
    linkinterface.h linkinterface.cpp
    mavlinkprotocol.h mavlinkprotocol.cpp
    MAVLinkLib.h
    linkmanager.h linkmanager.cpp
=======
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core SerialPort Network)

add_executable(${CMAKE_PROJECT_NAME}
  main.cpp
  LinkPackage.qrc
  Bridge.h Bridge.cpp
)


target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
    LinkConfiguration.h LinkConfiguration.cpp
    LinkInterface.h LinkInterface.cpp
    MAVLinkProtocol.h
    MAVLinkProtocol.cpp
    MAVLinkLib.h

    LinkManager.h LinkManager.cpp
>>>>>>> 88ad4e6365a16ced63c6d2fad7b8ac30a54e9e6f
    SerialLink.h SerialLink.cc
    UDPLink.h UDPLink.cc
    UdpIODevice.cc UdpIODevice.h
    QGCSerialPortInfo.h QGCSerialPortInfo.cc
    JsonHelper.h JsonHelper.cc
)

# 【核心步驟】自動幫所有檔案加上絕對路徑前綴
set(LINK_PACKAGE_ABS_FILES "")
foreach(src_file ${LINK_PACKAGE_RAW_FILES})
    list(APPEND LINK_PACKAGE_ABS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${src_file}")
endforeach()


target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${LINK_PACKAGE_ABS_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
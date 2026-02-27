

set(LINK_PACKAGE_RAW_FILES
    bridge.h bridge.cpp
    linkconfiguration.h linkconfiguration.cpp
    linkinterface.h linkinterface.cpp
    mavlinkprotocol.h mavlinkprotocol.cpp
    MAVLinkLib.h
    linkmanager.h linkmanager.cpp
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
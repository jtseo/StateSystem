cmake_minimum_required(VERSION 3.10)

# Project Name
project(stConsole)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add preprocessor definitions globally
add_definitions(-DVScriptBuild)
add_definitions(-DUbuntu)
add_definitions(-DDebug)

# Include directories
include_directories(include)

# Source files
file(GLOB SOURCES "*.cpp")

#include_directories(/opt/homebrew/Cellar/libmicrohttpd/1.0.1/include/)
#include_directories(/opt/homebrew/Cellar/mysql-client/9.0.1/include/mysql/)
include_directories(/usr/include/)
include_directories(/usr/include/mysql/)

# Add executable
add_executable(stConsole 
    stConsole/stConsole.cpp
    stConsole/StateManagerPnID.cpp
    stConsole/BaseInterface.cpp
    PtBase/BaseCircleQueue.cpp
    PtBase/BaseDStructure.cpp
    PtBase/BaseEventHandler.cpp
    PtBase/BaseEventManager.cpp
    PtBase/BaseFile.cpp
    PtBase/BaseHttp.cpp
    PtBase/BaseJson.cpp
    PtBase/BaseMemoryPool.cpp
    PtBase/BaseMemoryPool3.cpp
    PtBase/BaseNetConnector.cpp
    PtBase/BaseObject.cpp
    PtBase/BaseResDStructure.cpp
    PtBase/BaseResFile.cpp
    PtBase/BaseResFilterIP.cpp
    PtBase/BaseResGrid.cpp
    PtBase/BaseResManager.cpp
    PtBase/BaseResResource.cpp
    PtBase/BaseSCCurl.cpp
    PtBase/BaseSCMysql.cpp
    PtBase/BaseSCTableGrid.cpp
    PtBase/BaseSCTcp.cpp
    PtBase/BaseSCTcpSession.cpp
    PtBase/BaseSFParser.cpp
    PtBase/BaseSFuncDirectory.cpp
    PtBase/BaseSetString.cpp
    PtBase/BaseSocket.cpp
    PtBase/BaseState.cpp
    PtBase/BaseStateFunc.cpp
    PtBase/BaseStateMain.cpp
    PtBase/BaseStateManager.cpp
    PtBase/BaseStateSpace.cpp
    PtBase/BaseStringTable.cpp
    PtBase/BaseSystem.cpp
    PtBase/BaseTime.cpp
    PtBase/BaseTree.cpp
    PtBase/Defines.cpp
    PtBase/EncodeCodeFile.cpp
    PtBase/EncoderOperator.cpp
    PtBase/Fixed.cpp
    PtBase/KISA_SHA256.cpp
    PtBase/MD52.cpp
    PtBase/Matrix3.cpp
    PtBase/Matrix4.cpp
    PtBase/Quater.cpp
    PtBase/ResTif.cpp
    PtBase/SPLine.cpp
    PtBase/Vector3.cpp
    PtBase/allocator.cpp
    PtBase/base.cpp
    PtBase/base64.cpp
    PtBase/hashstr.cpp
    PtBase/lodepng.cpp
    PtBase/md5.cpp
    PtBase/seedx.cpp
    PtBase/BaseXML.cpp
    PtExtend/BaseSCHttpD.cpp
    PtExtend/BaseSCHttpDSession.cpp
    PtExtend/BaseSCJson.cpp
    PtExtend/BaseStateSystem.cpp
    PtExtend/StateContext.cpp
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

# Link libraries (optional)
target_link_libraries(stConsole mysqlclient ssl crypto microhttpd curl) 

#target_link_libraries(stConsole PRIVATE 
#    /opt/homebrew/Cellar/libmicrohttpd/1.0.1/lib/libmicrohttpd.12.dylib
#    /opt/homebrew/Cellar/mysql-client/9.0.1/lib/libmysqlclient.24.dylib
#    /opt/homebrew/Cellar/curl/8.9.1/lib/libcurl.4.dylib
#    )

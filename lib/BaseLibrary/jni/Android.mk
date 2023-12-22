LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := ../PtBase/base.cpp \
../PtBase/BaseSocket.cpp \
../PtBase/BaseSystem.cpp \
../PtBase/BaseMemoryPool.cpp \
../PtBase/BaseMemoryPool3.cpp \
../PtBase/BaseCircleQueue.cpp \
../PtBase/BaseStringTable.cpp \
../PtBase/BaseWFile.cpp \
../PtBase/hashstr.cpp \
../PtBase/BaseFile.cpp \
../PtBase/BaseResFile.cpp \
../PtBase/md5.cpp \
../PtBase/EncoderOperator.cpp \
../PtBase/seedx.cpp \
../PtBase/MD52.cpp \
../PtBase/EncodeCodeFile.cpp \
../PtBase/BaseStateManager.cpp \
../PtBase/BaseResManager.cpp \
../PtBase/BaseDStructure.cpp \
../PtBase/BaseState.cpp \
../PtBase/BaseObject.cpp \
../PtBase/BaseStateSpace.cpp \
../PtBase/BaseEventManager.cpp \
../PtBase/BaseNetConnector.cpp \
../PtBase/BaseStateMain.cpp \
../PtBase/BaseResResource.cpp \
../PtBase/BaseResFilterIP.cpp \
../PtBase/BaseResDStructure.cpp \
../PtBase/BaseTime.cpp \
../PtBase/BaseTree.cpp \
../PtBase/BaseHttp.cpp \
../PtBase/BaseXML.cpp \
../PtBase/Vector3.cpp \
../PtBase/Matrix4.cpp \
../PtBase/Matrix3.cpp \
../PtBase/BaseJson.cpp \
../PtBase/BaseSetString.cpp \
../PtBase/BaseStateFunc.cpp \
../PtBase/BaseEventHandler.cpp \
../PtBase/BaseSFuncDirectory.cpp \
../ApStateSystem/BaseInterface.cpp \
../ApStateSystem/StateManagerPnID.cpp

LOCAL_MODULE := libApStateSystem
LOCAL_ARM_MODE  := arm
LOCAL_C_INCLUDES := ../PtBase
LOCAL_C_INCLUDES += ../ApStateSystem
LOCAL_LDLIBS    := -llog
LOCAL_CFLAGS := -DANDROID $(LOCAL_CFLAGS)

include $(BUILD_SHARED_LIBRARY)

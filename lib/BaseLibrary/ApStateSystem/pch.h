// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _MSC_VER
// add headers that you want to pre-compile here
#include "framework.h"
#endif

//#ifdef _DEBUG

#include <stdarg.h>

//#endif

#include <stdarg.h>
#include <stdio.h>


#pragma warning(disable:4996)

#ifdef _MSC_VER
#include <malloc.h>
#endif
#include <stdlib.h>

#include <list>
#include <vector>
#include <map>
#include <vector>
#include <string>
#include <deque>
#include <queue>
#include <set>
//#include <codecvt>

#include <assert.h>
#include <algorithm>
#include <unordered_set>
#include <atomic>
#include "../PtBase/base.h"

#include "../PtBase/Vector3.h"
#include "../PtBase/Matrix3.h"
#include "../PtBase/Matrix4.h"

#include "../PtBase/BaseObject.h"

#include "../PtBase/hashstr.h"
#include "../PtBase/BaseDStructure.h"

#include "../PtBase/BaseTree.h"



#endif //PCH_H

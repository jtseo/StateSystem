// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//#define _DEBUG

#ifdef _MSC_VER

    #include <windows.h>

#endif

#include <stdarg.h>

//#endif

#include <stdarg.h>
#include <stdio.h>

#include <wchar.h>

#pragma warning(disable:4996)
#pragma warning(disable:4819)

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
#include <unordered_set>
//#include <codecvt>

#include <assert.h>
#include <algorithm>

#include "base.h"

#include "Vector3.h"
#include "Matrix3.h"
#include "Matrix4.h"

#include "BaseObject.h"

#include "hashstr.h"
#include "BaseDStructure.h"
//#include <Util/DebugConsol.h>
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.


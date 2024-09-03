// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

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
#include <cstring>
#include <deque>
#include <queue>
#include <set>
#include <unordered_set>
//#include <codecvt>

#include <assert.h>
#include <algorithm>

//#include <stdatomic.h>
#include "base.h"

#include "Vector3.h"
#include "Matrix3.h"
#include "Matrix4.h"

#include "BaseObject.h"

#include "hashstr.h"
#include "BaseDStructure.h"
//#include <Util/DebugConsol.h>
// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.


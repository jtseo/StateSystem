// ConsoleUnityTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <list>
#include <queue>
#include <string>
//#include <deque>
#include <unordered_set>

#include <assert.h>
#include <map>

#include "../PtBase/base.h"

#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseSocket.h"

#include "../PtBase/BaseFile.h"
#include "../PtBase/hashstr.h"

#include "../PtBase/BaseDStructure.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseFile.h"

#include "../ApStateSystem/BaseInterface.h"
#include <process.h>

#include <conio.h>

int main(int argc, const char*argv[])
{
	//vscript_init(".", ".", ".", "192.168.105.50", 80);
	vscript_init(".", ".", ".", "localhost", 19106);
	//vscript_init(".", ".", ".", "forrelease", 19106);
	void* pManager = manager_create(".");


	while (1)
	{
		manager_update(pManager);

		Sleep(10);
	}
	//*/

	manager_close(pManager);
    return 1;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

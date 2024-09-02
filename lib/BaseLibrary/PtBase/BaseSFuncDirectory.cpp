#include "stdafx.h"

#include "BaseStateFunc.h"
#include "BaseObject.h"
#include "BaseState.h"
#include "BaseFile.h"

#include "BaseSFuncDirectory.h"

#include "BaseStateMain.h"
#include "BaseStateManager.h"
#include "BaseStateSpace.h"

#include "BaseResFilterIP.h"
#include "BaseResManager.h"
#include "BaseTime.h"
#include "BaseStringTable.h"
#include <iostream>
#include <fstream>

PtObjectCpp(BaseSFuncDirectory);
// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSFuncDirectory::s_func_hash_a;
const char* BaseSFuncDirectory::s_class_name = "BaseSFuncDirectory";

int BaseSFuncDirectory::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0) {
		s_iId = STRTOHASH(s_class_name);
	}

	return s_iId;
}

BaseSFuncDirectory::BaseSFuncDirectory()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFuncDirectory::FunctionProcessor);
}

int BaseSFuncDirectory::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enum_text_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);		
		func_str = _class_name + ".fileCast_nF";	(*_func_hash)[Enum_fileCast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		func_str = _class_name + ".folerCreate_varF";	(*_func_hash)[Enum_folerCreate_varF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		func_str = _class_name + ".folerCurrentPush_varF";	(*_func_hash)[Enum_folerCurrentPush_varF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		func_str = _class_name + ".folerCurrentPop_varF";	(*_func_hash)[Enum_folerCurrentPop_varF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		STDEF_SFREGIST(fileCopy_avarIf);
		STDEF_SFREGIST(fileDelete_varF);
		STDEF_SFREGIST(RunCLI_nF);
		STDEF_SFREGIST(FilenameExtChange_varF);
		STDEF_SFREGIST(DirectoryListGet_varIf);
		STDEF_SFREGIST(FileWritableCheck_nIf);
		STDEF_SFREGIST(FileLoad_varF);
		STDEF_SFREGIST(FileSave_varF);
		STDEF_SFREGIST(ListCasting_varF);
		STDEF_SFREGIST(FileUpdated_varIf);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSFuncDirectory::~BaseSFuncDirectory()
{

}

void BaseSFuncDirectory::init()
{
	BaseStateFunc::init();
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFuncDirectory::FunctionProcessor);
}

void BaseSFuncDirectory::release()
{
}

BaseStateFuncEx* BaseSFuncDirectory::CreatorCallback(const void* _param)
{
	BaseSFuncDirectory* bs_func;
	PT_OAlloc(bs_func, BaseSFuncDirectory);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFuncDirectory::FunctionProcessor);

	return bs_func;
}

int BaseSFuncDirectory::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(fileCast_nF);
		STDEF_SFFUNCALL(folerCreate_varF);
		STDEF_SFFUNCALL(folerCurrentPush_varF);
		STDEF_SFFUNCALL(folerCurrentPop_varF);
		STDEF_SFFUNCALL(fileCopy_avarIf);
		STDEF_SFFUNCALL(fileDelete_varF);
		STDEF_SFFUNCALL(RunCLI_nF);
		STDEF_SFFUNCALL(FilenameExtChange_varF);
		STDEF_SFFUNCALL(DirectoryListGet_varIf);
		STDEF_SFFUNCALL(FileWritableCheck_nIf);
		STDEF_SFFUNCALL(FileLoad_varF);
		STDEF_SFFUNCALL(FileSave_varF);
		STDEF_SFFUNCALL(ListCasting_varF);
		STDEF_SFFUNCALL(FileUpdated_varIf);
		//#SF_FuncCallInsert
		return 0;
	}
	return ret;
}

int BaseSFuncDirectory::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSFuncDirectory* bs_func = (BaseSFuncDirectory*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSFuncDirectory::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);
	return ret;
}

int BaseSFuncDirectory::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	//m_dwg_p = BaseResFileDWG::dwg_get(m_param_base);
	//GroupLevelSet(100); // 100 is BaseSFuncDirectory
    
    const char *path = (const char*)paramFallowGet(0);

    if(!path)
		path = m_manager_p->path_get_save();
    
    STLString filename = path;
	
	if (filename.empty())
		return 0;

	m_pathCurrent = BaseFile::get_path( filename);

	return 1;
}

int BaseSFuncDirectory::fileCast_nF()
{
	int evt_hash = *(const int*)m_param_value;
	const char* select = (const char*)paramFallowGet(0);

    if(!select)
        return 0;
    
	STLString path_str = m_pathCurrent;
	if (select == NULL)
		path_str += "*.*";
	else
		path_str += select;

	STLVString files;
	BaseSystem::GetFileList(path_str.c_str(), &files, NULL);

	for (int i = 0; i < files.size(); i++)
	{
		BaseDStructureValue* evt = EventMake(evt_hash);
		STLString filename;
		filename = m_pathCurrent;
		filename += files[i];
		evt->set_alloc("TempString_strV", (const void*)filename.c_str());
		EventPost(evt);
	}

	EventPost(evt_hash); // end of list
	return 1;
}

int BaseSFuncDirectory::folerCreate_varF()
{
	const char* path = (const char*)paramVariableGet();
	if (!path)
		return 0;

	STLString path_str = m_pathCurrent;
	path_str += path;
	BaseSystem::folder_create(path_str.c_str());
	return 1;
}

int BaseSFuncDirectory::folerCurrentPush_varF()
{
	m_pathStack.push_back(m_pathCurrent);
	const char *path = (const char*)paramVariableGet();
	if (!path)
		return 0;
	m_pathCurrent = path;
	return 1;
}

int BaseSFuncDirectory::folerCurrentPop_varF()
{
	if (m_pathStack.empty())
		return 0;
    m_pathCurrent = m_pathStack.back();
	m_pathStack.pop_back();
    paramVariableSet(m_pathCurrent.c_str());
	return 1;
}
int BaseSFuncDirectory::fileCopy_avarIf()
{
	const char* cpy = (const char*)m_param_value;
	STLVString params;
	if (BaseFile::paser_list_seperate(cpy, &params, ",") < 2)
		return 0;

	int hash[2];
	hash[0] = STRTOHASH(params[0].c_str());
	hash[1] = STRTOHASH(params[1].c_str());

	char buff[255];
	if (!m_state_variable->get(params[0].c_str(), buff))
		return 0;
	params[0] = buff;
	if (!m_state_variable->get(params[1].c_str(), buff))
		return 0;
	params[1] = buff;

	char filename[255], ext[255];
	BaseFile::get_filename(params[0].c_str(), filename, 255);
	STLString target = params[1];
	target += filename;
	BaseFile::get_filext(params[0].c_str(), ext, 255);
	target += ext;
	BaseFile::copy(params[0].c_str(), target.c_str());
	return 1;
}

int BaseSFuncDirectory::fileDelete_varF()
{
	const char* path_str = (const char*)paramVariableGet();
	if (!path_str)
		return 0;

	BaseSystem::file_delete(path_str);
	return 1;
}

int BaseSFuncDirectory::FilenameExtChange_varF()
{
	const char* filename = (const char*)paramVariableGet();
	const char* ext = (const char*)paramFallowGet(0);

	if (!ext || !filename)
		return 0;

	char buff[255];
	strcpy_s(buff, 255, filename);
	char* start = strrchr(buff, '.');
	if (start == NULL)
		return 0;
	start++;
	*start = NULL;
	strcat_s(buff, 255, ext);

	paramVariableSet(buff);
	return 1;
}

int BaseSFuncDirectory::DirectoryListGet_varIf()
{
	const char *path_str = (const char*)paramVariableGet();
	const char *sel_str = (const char*)paramFallowGet(0);
	    
	STLString path = path_str;
#ifdef _WIN32
	if (sel_str == NULL)
		path += "*.*";
	else
		path += sel_str;
#endif
    
	STLVString files;
	BaseSystem::GetFileList(path.c_str(), &files, NULL);
	STLVString dates;
	BaseSystem::GetFileDateList(path_str, files, &dates);

	STLString ret, ret2;
	for(int i=0; i<(int)files.size(); i++)
	{
		ret += files[i];
		if(i != (int)files.size()-1)
			ret += ",";
		ret2 += dates[i];
		if(i != (int)dates.size()-1)
			ret2 += ",";
	}
	
	paramFallowSet(1, ret.c_str());
	paramFallowSet(2, ret2.c_str());
	return 1;
}

int BaseSFuncDirectory::FileWritableCheck_nIf()
{
	const char* filepath = (const char*)paramFallowGet(0);

	FILE* pf = fopen(filepath, "r+");

	if (!pf) {
		return 0;
	}

	fclose(pf);
	return 1;

	std::fstream file(filepath, std::ios::in | std::ios::out | std::ios::app);

	// Check if the file is open
	if (!file.is_open()) {
		std::cerr << "Error: Couldn't open file." << std::endl;
		return 0;
	}

	// Try to acquire a lock
	file.seekg(0, std::ios::end);
	if (file.tellg() != 0) {
		// If the file size is not 0, it's being written by another process
		return 0;
	}

	return 1;
}

int BaseSFuncDirectory::FileLoad_varF()
{
	const char* filepath = (const char*)paramFallowGet(0);

	BaseFile file;
	if (file.OpenFile(filepath, BaseFile::OPEN_READ))
		return 0;

	UINT32 size_n = file.get_size_file();
	char* buff = PT_Alloc(char, (int)size_n + 1);
	*(buff + size_n) = 0;

	file.Read(buff, size_n);
	file.CloseFile();

	const int* hash = (const int*)m_param_value;
	int index = BaseDStructure::get_index(*hash);

	if (BaseDStructure::type_get(index).nType >= TYPE_STRING)
	{
		m_state_variable->set_mass(*hash, buff, size_n);
	}
	else {
		m_state_variable->set_alloc(*hash, buff, size_n);
	}

	PT_Free(buff);
	return 1;
}

int BaseSFuncDirectory::FileSave_varF()
{
	const char* filepath = (const char*)paramFallowGet(0);
	const char* buff = NULL;
	short cnt = 0;
	int cnt2 = 0;

    if(filepath == NULL)
        return 0;
    
	const int* hash = (const int*)m_param_value;
	bool largeable = m_state_variable->get_base()->type_get(m_state_variable->get_base()->get_index(*hash)).nType >= TYPE_STRING;

	BaseFile file;
    
	if (file.OpenFile(filepath, BaseFile::OPEN_WRITE))
		return 0;

	if (largeable)
	{
		if (!m_state_variable->get_mass(*hash, (const void**)&buff, &cnt2))
			return 0;
	}
	else
	{
		if (!m_state_variable->get(*hash, (const void**)&buff, &cnt))
			return 0;
	}

	if (largeable)
		file.Write(buff, cnt2);
	else
		file.Write(buff, (int)cnt);

	file.CloseFile();
	return 1;
}

int BaseSFuncDirectory::RunCLI_nF()
{
	const int* show_pn = (const int*)m_param_value;

	int show_n = 1;
	char opr_str[10], cmd_str[1024] = "\0";
	const char* strValue = (const char*)paramFallowGet(0);
	strcpy_s(opr_str, 10, strValue);
	strValue = (const char*)paramFallowGet(1);
	strcpy_s(cmd_str, 1024, strValue);
	STLString params, space = "";
	
	int cnt = 2;
	do{
		params += space;
		strValue = (const char*)paramFallowGet(cnt);
		if (strValue)
			params += strValue;
		space = " ";
		cnt++;
	} while (strValue);
	
	char param2[1024];
	BaseSystem::tomulti(params.c_str(), param2, 1024);

	BaseSystem::run_shell_command(opr_str, cmd_str, param2, *show_pn != 0);

	return 1;
}

int BaseSFuncDirectory::FileUpdated_varIf()
{
	const char *file = (const char*)paramVariableGet();
	const char *date = (const char*)paramFallowGet(0);
	
    FILE* pf = fopen(file, "r"); // check file is exist.
    if (!pf) {
        return 1;
    }
    fclose(pf);
    
	SPtDateTime dtCur = BaseSystem::file_datetime_get(file);
	SPtDateTime dtServer;
	
	int y, m, d, h, mm, s;
	sscanf(date, "%d/%d/%d %d:%d:%d", &y, &m, &d, &h, &mm, &s);
	dtServer.s.sDate.s.year = y;
	dtServer.s.sDate.s.month = m;
	dtServer.s.sDate.s.day = d;
	dtServer.s.sTime.s.nHour = h;
	dtServer.s.sTime.s.nMinute = mm;
	dtServer.s.sTime.s.nSecond = s;
	
	if(dtCur >= dtServer)
		return 0;
	
	return 1;
}

int BaseSFuncDirectory::ListCasting_varF()
{
	const char *list = (const char*)paramFallowGet(0);
	const char *list2 = (const char*)paramFallowGet(1);
	const int *event = (const int*)m_param_value;
	
	if(!list)
		return 0;
	
	STLVString list_a;
	BaseFile::paser_list_seperate(list, &list_a, ",");
	STLVString list2_a;
	if(list2)
		BaseFile::paser_list_seperate(list2, &list2_a, ",");
	
	int count = list_a.size();
	paramFallowSet(2, &count);
	for(int i=0; i<list_a.size(); i++)
	{
		BaseDStructureValue *evt = EventMake(*event);
		const char *name = list_a[i].c_str();
		printf("file: %s\n", name);

		evt->set_alloc("TempString_strV", (const void*)name);
		if(list2)
		{
			evt->set_alloc("TempString2_strV", (const void*)list2_a[i].c_str());
		}
		EventPost(evt);
	}
	return 1;
}
//#SF_functionInsert

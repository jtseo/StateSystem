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
		STDEF_SFREGIST(DirectoryListGet_varIf);
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
		STDEF_SFFUNCALL(DirectoryListGet_varIf);
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

int BaseSFuncDirectory::DirectoryListGet_varIf()
{
	const char *path_str = (const char*)paramVariableGet();
	const char *sel_str = (const char*)paramFallowGet(0);
	    
	STLString path = path_str;
	if (sel_str == NULL)
		path += "*.*";
	else
		path += sel_str;

	STLVString files;
	BaseSystem::GetFileList(path.c_str(), &files, NULL);

	STLString ret;
	for(int i=0; i<(int)files.size(); i++)
	{
		ret += files[i];
		if(i != (int)files.size()-1)
			ret += ",";
	}
	
	paramFallowSet(1, ret.c_str());
	return 1;
}
//#SF_functionInsert

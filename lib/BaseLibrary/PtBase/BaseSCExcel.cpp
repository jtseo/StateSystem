#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSCExcel.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseSCExcel);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCExcel::s_func_hash_a;
const char* BaseSCExcel::s_class_name = "BaseSCExcel";


void BaseExcelColumn::init()
{
	mergeLeft = false;
	mergeUp = false;
	text = "";
	size_v3.SetInit();
}

int BaseSCExcel::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCExcel::BaseSCExcel()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCExcel::FunctionProcessor);
}

int BaseSCExcel::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(loadCheck_nIf);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCExcel::~BaseSCExcel()
{

}

void BaseSCExcel::init()
{
	BaseStateFunc::init();
}

void BaseSCExcel::release()
{
}

BaseStateFuncEx* BaseSCExcel::CreatorCallback(const void* _param)
{
	BaseSCExcel* bs_func;
	PT_OAlloc(bs_func, BaseSCExcel);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCExcel::FunctionProcessor);

	return bs_func;
}

int BaseSCExcel::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(loadCheck_nIf);
		//#SF_FuncCallInsert
		return 0;
	}
	return ret;
}


int BaseSCExcel::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCExcel* bs_func = (BaseSCExcel*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCExcel::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCExcel::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	const char* filepath = (const char*)paramFallowGet(0);

	if (!filepath)
		return 0;

	BaseFile file;
	file.set_asc_seperator("\t");
	if (file.OpenFile(filepath, BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
		return 0;

	m_excel_aa.clear();

	while (file.read_asc_line())
	{
		STLString text, size_str;
		STLVExcelColumn col_a;
		while (file.read_asc_string(&text))
		{
			BaseExcelColumn	col;
			col.init();
			if (text[0] == '`')
			{
				if (text[1] == 'u')
					col.mergeUp = true;
				else
					col.mergeLeft = true;
			}
			else {
				col.text = text;
			}

			file.read_asc_string(&size_str);

			col.size_v3.SetValue(size_str.c_str());
			col.size_v3.z = 0;

			col_a.push_back(col);
		}
		m_excel_aa.push_back(col_a);
	}

	file.CloseFile();

	return 1;
}

int BaseSCExcel::loadCheck_nIf()
{
	if (m_excel_aa.empty())
		return 0;
	return 1;
}
//#SF_functionInsert

const STLVVExcelColumn& BaseSCExcel::columnGet() const
{
	return m_excel_aa;
}
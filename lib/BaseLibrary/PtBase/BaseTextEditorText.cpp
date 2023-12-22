#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"

#include "../PtBase/BaseFile.h"
#include "BaseTextEditorText.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseFile.h"
#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

#include "BaseTextEditor.h"

PtObjectCpp(BaseTextEditorText);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseTextEditorText::s_func_hash_a;
const char* BaseTextEditorText::s_class_name = "BaseTextEditorText";

int BaseTextEditorText::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseTextEditorText::BaseTextEditorText()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseTextEditorText::FunctionProcessor);
}

int BaseTextEditorText::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		STDEF_SFREGIST(BraceSet_nF);
		STDEF_SFREGIST(LineFind_varF);
		STDEF_SFREGIST(WordFind_varF);
		STDEF_SFREGIST(LinesAppend_varF);
		STDEF_SFREGIST(WordsAppend_varF);
		STDEF_SFREGIST(FileAppend_varF);
		STDEF_SFREGIST(LineDelete_nF);
		STDEF_SFREGIST(LineCommentMake_nF);
		STDEF_SFREGIST(LineCommentRelease_nF);
		STDEF_SFREGIST(WordDelete_nF);
		STDEF_SFREGIST(OpenEditorVS_nF);
		STDEF_SFREGIST(analysisFind_strIf);
		STDEF_SFREGIST(analysisBraceGetInt_strIf);
		STDEF_SFREGIST(exportFormat_strF);
		STDEF_SFREGIST(seperatorPush_strF);
		STDEF_SFREGIST(seperatorPop_nF);
		STDEF_SFREGIST(analysisStringGet_nF);
		STDEF_SFREGIST(analysisNumGet_nF);
		STDEF_SFREGIST(delectorPush_strF);
		STDEF_SFREGIST(delectorPop_nF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseTextEditorText::~BaseTextEditorText()
{

}

void BaseTextEditorText::init()
{
	BaseStateFunc::init();
}

void BaseTextEditorText::release()
{
}

BaseStateFuncEx* BaseTextEditorText::CreatorCallback(const void* _param)
{
	BaseTextEditorText* bs_func;
	PT_OAlloc(bs_func, BaseTextEditorText);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseTextEditorText::FunctionProcessor);

	return bs_func;
}

int BaseTextEditorText::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(BraceSet_nF);
		STDEF_SFFUNCALL(LineFind_varF);
		STDEF_SFFUNCALL(WordFind_varF);
		STDEF_SFFUNCALL(LinesAppend_varF);
		STDEF_SFFUNCALL(WordsAppend_varF);
		STDEF_SFFUNCALL(FileAppend_varF);
		STDEF_SFFUNCALL(LineDelete_nF);
		STDEF_SFFUNCALL(LineCommentMake_nF);
		STDEF_SFFUNCALL(LineCommentRelease_nF);
		STDEF_SFFUNCALL(WordDelete_nF);
		STDEF_SFFUNCALL(OpenEditorVS_nF);
		STDEF_SFFUNCALL(analysisFind_strIf);
		STDEF_SFFUNCALL(analysisBraceGetInt_strIf);
		STDEF_SFFUNCALL(exportFormat_strF);
		STDEF_SFFUNCALL(seperatorPush_strF);
		STDEF_SFFUNCALL(seperatorPop_nF);
		STDEF_SFFUNCALL(analysisStringGet_nF);
		STDEF_SFFUNCALL(analysisNumGet_nF);
		STDEF_SFFUNCALL(delectorPush_strF);
		STDEF_SFFUNCALL(delectorPop_nF);
		//#SF_FuncCallInsert
		return 0;
	}
	return ret;
}

int BaseTextEditorText::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseTextEditorText* bs_func = (BaseTextEditorText*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseTextEditorText::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseTextEditorText::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	//m_dwg_p = BaseResFileDWG::dwg_get(m_param_base);
	//GroupLevelSet(1); // Level limit is 4.
	m_super = (BaseStateFunc*)m_state_p->class_get(BaseTextEditor::GetObjectId());
	if (!m_super)
		return 0;

	const char* line_str = (const char*)paramFallowGet(0);
	if(line_str)
	{
		m_line_str = line_str;
		m_parser.openParser((char*)m_line_str.c_str(), (UINT32)m_line_str.size());
		m_parser.read_asc_line();
	}

	return 1;
}

int BaseTextEditorText::BraceSet_nF()
{
	return 1;
}

int BaseTextEditorText::LineFind_varF()
{
	return 1;
}
int BaseTextEditorText::WordFind_varF()
{
	return 1;
}
int BaseTextEditorText::LinesAppend_varF()
{
	return 1;
}
int BaseTextEditorText::WordsAppend_varF()
{
	return 1;
}
int BaseTextEditorText::FileAppend_varF()
{
	return 1;
}
int BaseTextEditorText::LineDelete_nF()
{
	return 1;
}
int BaseTextEditorText::LineCommentMake_nF()
{
	return 1;
}
int BaseTextEditorText::LineCommentRelease_nF()
{
	return 1;
}
int BaseTextEditorText::WordDelete_nF()
{
	return 1;
}
int BaseTextEditorText::OpenEditorVS_nF()
{
	return 1;
}

int BaseTextEditorText::analysisFind_strIf()
{
	const char* find_str = (const char*)m_param_value;
	do{
		m_analysis_current_str = m_parser.read_asc_str();
		if(m_analysis_current_str == find_str)
			return 1;
	}while(!m_analysis_current_str.empty());
	return 0;
}
int BaseTextEditorText::analysisBraceGetInt_strIf()
{
	return 1;
}
int BaseTextEditorText::exportFormat_strF()
{
	STLString exp_str = paramFormatGet();
	if(exp_str.empty())
		return 0;
	
	((BaseTextEditor*)m_super)->analysisContentsAdd(exp_str);
	return 1;
}
int BaseTextEditorText::analysisStringGet_nF()
{
	const int *step_p = (const int*)m_param_value;
	if(*step_p == 0)
		paramFallowSet(0, m_analysis_current_str.c_str());
	for(int i=0; i<*step_p; i++)
	{
		m_analysis_current_str = m_parser.read_asc_str();
	}
	if(m_analysis_current_str.empty())
		return 0;
	paramFallowSet(0, m_analysis_current_str.c_str());
	return 1;
}
int BaseTextEditorText::analysisNumGet_nF()
{
	const int *step_p = (const int*)m_param_value;
	if(*step_p == 0)
		paramFallowSet(0, m_analysis_current_str.c_str());
	for(int i=0; i<*step_p; i++)
	{
		m_analysis_current_str = m_parser.read_asc_str();
	}
	if(m_analysis_current_str.empty())
		return 0;
	int num = 0;
	sscanf_s(m_analysis_current_str.c_str(), "%d", &num);
	paramFallowSet(0, (const void*)&num);
	return 1;
}
int BaseTextEditorText::seperatorPush_strF()
{
    const char*sep_str = (const char*)m_param_value;

    STLString sep;
    m_seperator_stack_a.push_back(m_seperator_str);
    m_seperator_str = sep_str;
    m_parser.set_asc_seperator(sep_str);
	return 1;
}
int BaseTextEditorText::seperatorPop_nF()
{
    STLString sep;
    if(m_seperator_stack_a.empty())
        return 0;
    m_seperator_str = m_seperator_stack_a.back();
    m_seperator_stack_a.erase(m_seperator_stack_a.begin()+(m_seperator_stack_a.size()-1));
    m_parser.set_asc_seperator(m_seperator_str.c_str());
	return 1;
}

int BaseTextEditorText::delectorPush_strF()
{
    const char*sep_str = (const char*)m_param_value;

    STLString sep;
    m_delector_stack_a.push_back(m_delector_str);
    m_delector_str = sep_str;
    m_parser.set_asc_deletor(sep_str);
	return 1;
}
int BaseTextEditorText::delectorPop_nF()
{
    STLString sep;
    if(m_delector_stack_a.empty())
        return 0;
    m_delector_str = m_delector_stack_a.back();
    m_delector_stack_a.erase(m_delector_stack_a.begin()+(m_delector_stack_a.size()-1));
    m_parser.set_asc_deletor(m_delector_str.c_str());
	return 1;
}
//#SF_functionInsert

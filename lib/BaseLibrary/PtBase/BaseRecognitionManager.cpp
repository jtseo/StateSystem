#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseRecognitionManager.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

#include "BaseRecognitionPrjVector.h"

PtObjectCpp(BaseRecognitionManager);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseRecognitionManager::s_func_hash_a;
const char* BaseRecognitionManager::s_class_name = "BaseRecognitionManager";

int BaseRecognitionManager::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0) {
		s_iId = STRTOHASH(s_class_name);
	}

	return s_iId;
}

BaseRecognitionManager::BaseRecognitionManager()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseRecognitionManager::FunctionProcessor);
}

int BaseRecognitionManager::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(text_info_cast_nF);
		STDEF_SFREGIST(add_nF);
		STDEF_SFREGIST(predict_nF);
		STDEF_SFREGIST(crossCheck_nF);
		STDEF_SFREGIST(load_nF);
		//#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseRecognitionManager::~BaseRecognitionManager()
{

}

void BaseRecognitionManager::init()
{
	BaseStateFunc::init();
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseRecognitionManager::FunctionProcessor);
}

void BaseRecognitionManager::release()
{
}

BaseStateFuncEx* BaseRecognitionManager::CreatorCallback(const void* _param)
{
	BaseRecognitionManager* bs_func;
	PT_OAlloc(bs_func, BaseRecognitionManager);

	bs_func->init();

	return bs_func;
}

int BaseRecognitionManager::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])		return 0;
		//STDEF_SFFUNCALL(typeCheck_nIf);
		STDEF_SFFUNCALL(add_nF);
		STDEF_SFFUNCALL(predict_nF);
		STDEF_SFFUNCALL(crossCheck_nF);
		STDEF_SFFUNCALL(load_nF);
		//#SF_FuncCallInsert

		return 0;
	}
	return ret;
}

int BaseRecognitionManager::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseRecognitionManager* bs_func = (BaseRecognitionManager*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseRecognitionManager::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);
	return ret;
}

int BaseRecognitionManager::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	//GroupIdSet(m_node->id);

	return 1;
}

int BaseRecognitionManager::add_nF()
{
	int margin = *(const int*)m_param_value;
	BaseProjectedVector *prjVector = (BaseProjectedVector*)paramFallowPointGet(0);
	STLString name = prjVector->nameGet();

	for (int i = 0; i < m_prjVecName_a.size(); i++)
	{
		if (m_prjVecName_a[i].name() == name)
		{
			return m_prjVecName_a[i].add(prjVector);
		}
	}

	BasePrjVectorName prjName;
	prjName.groupMargin() = (float)margin;
	prjName.name() = name;
	prjName.add(prjVector);
	m_prjVecName_a.push_back(prjName);

	BaseFile file;
	file.OpenFile("train_data/recongnitionManager.cfg", BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8);

	for (int i = 0; i < m_prjVecName_a.size(); i++)
	{
		file.write_asc_string(m_prjVecName_a[i].name().c_str(), 0);
		file.write_asc_line();
	}

	file.CloseFile();

	return 1;
}

int BaseRecognitionManager::load_nF()
{	
	BaseFile file;
	file.set_asc_seperator(",\t");
	if (file.OpenFile("train_data/recongnitionManager.cfg", BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
		return 0;

	m_prjVecName_a.clear();

	while(file.read_asc_line())
	{
		BasePrjVectorName prjVecName;
		prjVecName.name() = file.read_asc_str();
		prjVecName.layer() = file.read_asc_str();
		STLString name = file.read_asc_str();
		if (!name.empty())
		{
			prjVecName.filename() = prjVecName.name();
			prjVecName.name() = name;
		}
		m_prjVecName_a.push_back(prjVecName);
	}
	file.CloseFile();

	int ret = 1;
	for (int i = 0; i < m_prjVecName_a.size(); i++)
	{
		if (!m_prjVecName_a[i].load(m_prjVecName_a[i].name()))
			ret = 0;
	}
	return ret;
}

int BaseRecognitionManager::predict_nF()
{
	int margin = *(const int*)m_param_value;
	int initValue = margin;
	BaseProjectedVector* prjVector = (BaseProjectedVector*)paramFallowPointGet(0);
	
	STLVInt _index_a, _index2_a;
	STLVString	_name_a;
	STLVString _layer_a;
	STLVfloat	_differ_a;

	//printf("=========================\n");

	do {
		for (int i = 0; i < m_prjVecName_a.size(); i++)
		{	
			STLVInt index_a, index2_a;
			STLVfloat	differ_a;

				if (m_prjVecName_a[i].predic(&index_a, &index2_a, &differ_a, prjVector, (float)margin))
				{
					for (int j = 0; j < index_a.size(); j++)
					{
						_index_a.push_back(index_a[j]);
						_index2_a.push_back(index2_a[j]);
						_differ_a.push_back(differ_a[j]);
						_name_a.push_back(m_prjVecName_a[i].name());
						_layer_a.push_back(m_prjVecName_a[i].layer());
					}
				}
		}
		margin += initValue;
	} while (_index_a.empty() && margin < 60);

	const char* filename_str = (const char*)paramFallowGet(1);
	if (!filename_str)
		filename_str = "Unknow";

	for (int k = 0; k < 10; k++)
	{
		float min = (float)margin;
		int m;

		if (_index_a.size() <= k)
			break;

		for (int i = 0; i < _index_a.size(); i++)
		{
			if (_differ_a[i] < min)
			{
				min = _differ_a[i];
				m = i;
			}
		}

		if (k == 0)
		{
			paramFallowSet(1, _name_a[m].c_str());
			paramFallowSet(2, _layer_a[m].c_str());
		}
		//printf("%s=>%s %d:%d %0.3f\n", filename_str, _name_a[m].c_str(), _index_a[m], _index2_a[m], _differ_a[m]);
		_differ_a[m] = (float)margin;
	}

	return 1;
}

int BaseRecognitionManager::crossCheck_nF()
{
	int margin = *(const int*)m_param_value;
	if (margin < 10)
		margin = 50;
	for (int i = 0; i < m_prjVecName_a.size(); i++)
	{
		BaseProjectedVector prjVec;

		const STLVPrjVectorGroup& group = m_prjVecName_a[i].groupGet();
		STLString name = m_prjVecName_a[i].name();

		for (int k = 0; k < group.size(); k++)
		{
			group[k].averageGet(&prjVec);

			for (int j = i+1; j < m_prjVecName_a.size(); j++)
			{
				STLVInt	_index_a;
				STLVInt	_index2_a;
				STLVfloat	_diff_a;
				STLString name2 = m_prjVecName_a[j].name();

				if (m_prjVecName_a[j].predic(&_index_a, &_index2_a, &_diff_a, &prjVec, (float)margin))
				{
					for (int n = 0; n < _index_a.size(); n++)
					{
						int index1 = prjVec.index();
						//printf("%s:%d vs %s:%d differ:%.3f\n", name.c_str(), index1, name2.c_str(), _index_a[n], _diff_a[n]);
					}
				}
			}
			
		}
	}

	return 1;
}

bool BasePrjVectorName::predic(STLVInt* _index_a, STLVInt* _index2_a, STLVfloat* _diff_a, const BaseProjectedVector* _prjVec_p, float _margin)
{
	float differ = 0;
	STLVInt index2_a;
	STLVfloat	diff_a;

	bool added = false;
	for (int i = 0; i < m_prjVecGroup_a.size(); i++)
	{
		int index = 0;
		//float diff = m_prjVecGroup_a[i].differ_min(_prjVec_p, &index);
		float diff = m_prjVecGroup_a[i].differ(_prjVec_p);
		if(diff < _margin){
			diff = m_prjVecGroup_a[i].differ_min(_prjVec_p, &index);
			if (diff < _margin) {
				_index_a->push_back(i);
				_index2_a->push_back(m_prjVecGroup_a[i].indexGet(index));
				_diff_a->push_back(diff);
				added = true;
			}
		}
	}
	return added;
}

int BasePrjVectorName::load(STLString _name)
{
	BaseFile file;
	STLString filename = "train_data/";
	filename += _name;
	if (!m_filename.empty())
		filename += m_filename;	
	filename += ".cfg";
	if (file.OpenFile(filename.c_str(), BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
		return 0;

	int cnt = 0;
	file.read_asc_line();
	file.read_asc_integer(&cnt);

	file.CloseFile();

	m_prjVecGroup_a.resize(cnt);
	int ret = 1;
	for (int i = 0; i < cnt; i++)
	{
		if (!m_prjVecGroup_a[i].load(_name, i))
			ret = 0;
	}
	return ret;
}

int BasePrjVectorName::add(BaseProjectedVector* _prjVec_p)
{
	int ret = _add(_prjVec_p);

	BaseProjectedVector prjVec;
	BaseProjectedVector* prjVec_p, *prjVec2_p;

	prjVec2_p = _prjVec_p;

	for (int i = 0; i < 3; i++)
	{
		PT_OAllocS(prjVec_p, BaseProjectedVector, prjVec);
		*prjVec_p = *prjVec2_p;
		prjVec_p->rotate(PM_HALF);
//
//#ifdef _DEBUG
//		char buf[255];
//		sprintf_s(buf, 255, "%d.svg", i);
//		prjVec_p->svgSave(buf);
//#endif

		_add(prjVec_p);
		prjVec2_p = prjVec_p;
	}
	return ret;
}

int BasePrjVectorName::_add(BaseProjectedVector* _prjVec_p)
{
	float differ = 0;
	for (int i = 0; i < m_prjVecGroup_a.size(); i++)
	{
		if (m_prjVecGroup_a[i].add(_prjVec_p, i))
			return 1;
	}

	BasePrjVectorGroup group;
	group.margin() = m_groupMargin;
	group.add(_prjVec_p, (int)m_prjVecGroup_a.size());
	m_prjVecGroup_a.push_back(group);

	BaseFile file;
	STLString filename = "train_data/";
	filename += m_name;
	filename += ".cfg";
	file.OpenFile(filename.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8);

	file.write_asc_integer((int)m_prjVecGroup_a.size());
	file.write_asc_line();

	file.CloseFile();
	return 1;
}

BasePrjVectorGroup::BasePrjVectorGroup()
{
	m_groupMargin = 2000.f;
}

int BasePrjVectorGroup::load(STLString _name, int _index)
{
	release();

	char buf[255];
	BaseFile file;
	STLString filename = "train_data/";
	sprintf_s(buf, 255, "%s_%05d.cfg", _name.c_str(), _index);
	filename += buf;
	file.OpenFile(filename.c_str(), BaseFile::OPEN_READ | BaseFile::OPEN_UTF8);

	file.read_asc_line();
	file.read_asc_float(&m_groupMargin);
	int cnt = 0;
	file.read_asc_integer(&cnt);
	
	m_avarge_a.clear();
	while (file.read_asc_line())
	{
		PtVector3 vec(0, 0, 0);
		file.read_asc_float(&vec.x);
		file.read_asc_float(&vec.y);
		m_avarge_a.push_back(vec);
	}
	file.CloseFile();

	int ret = 1;
	for (int i = 0; i < cnt; i++)
	{
		sprintf_s(buf, 255, "train_data/%s_%03d_%03d.prjvec", _name.c_str(), (int)_index, i);
		BaseProjectedVector prjVec;
		BaseProjectedVector *prjVec_p;

		PT_OAllocS(prjVec_p, BaseProjectedVector, prjVec);
		if (prjVec_p->prjvecLoad(buf))
			m_prjVec_ap.push_back(prjVec_p);
		else {
			PT_OFreeS(prjVec_p);
			ret = 0;
		}
	}
	return ret;
}

void BasePrjVectorGroup::release()
{
	m_avarge_a.clear();
	for (int i = 0; m_prjVec_ap.size(); i++)
	{
		PT_OFreeS(m_prjVec_ap[i]);
	}
	m_prjVec_ap.clear();
}

int BasePrjVectorGroup::add(BaseProjectedVector* _prjVec_p, int _index)
{
	float diff = differ(_prjVec_p);

	if (diff > m_groupMargin)
		return 0;

	diff = differ_min(_prjVec_p);
	if (diff >= 0 && diff < 0.1f)
		return 1;
#ifdef _DEBUG
	if (diff > 10000)
		diff = differ_min(_prjVec_p);
#endif

	m_prjVec_ap.push_back(_prjVec_p);

	averageRemake();

	STLString name = m_prjVec_ap[0]->nameGet();
	char buf[255];
	sprintf_s(buf, 255, "train_data/%s_%03d_%03d.prjvec", name.c_str(), (int)_index, (int)m_prjVec_ap.size() - 1);
	_prjVec_p->prjvecSave(buf);
	
	BaseFile file;
	STLString filename = "train_data/";
	sprintf_s(buf, 255, "%s_%05d.cfg", name.c_str(), _index);
	filename += buf;
	file.OpenFile(filename.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8);

	file.write_asc_float(m_groupMargin);
	file.write_asc_integer((int)m_prjVec_ap.size());
	file.write_asc_line();

	for (int i = 0; i < m_avarge_a.size(); i++)
	{
		file.write_asc_float(m_avarge_a[i].x);
		file.write_asc_float(m_avarge_a[i].y);
		file.write_asc_line();
	}

	file.CloseFile();
	return 1;
}

bool BasePrjVectorGroup::predic(STLVInt* _index2_a, STLVfloat* _diff_a, const BaseProjectedVector* _prjVec_p)
{
	float differ = 0;
	STLVInt index2_a;
	STLVfloat	diff_a;

	bool added = false;
	for (int i = 0; i < m_prjVec_ap.size(); i++)
	{
		float diff = m_prjVec_ap[i]->differ(_prjVec_p);

		//if(diff < m_groupMargin)
	}
	return false;
}

float BasePrjVectorGroup::differ_min(const BaseProjectedVector* _prjVec_p, int* _index_p) const
{
	BaseProjectedVector prjVec;
	if (m_prjVec_ap.empty())
		return -1;

	float min = 10000000, diff;
	for (int i = 0; i < m_prjVec_ap.size(); i++)
	{
		diff = m_prjVec_ap[i]->differ(_prjVec_p);
		if (diff < min) {
			min = diff;
			if(_index_p)
				*_index_p = i;
		}
	}
	return min;
}

float BasePrjVectorGroup::differ(const BaseProjectedVector* _prjVec_p) const
{
	BaseProjectedVector prjVec;
	if (m_prjVec_ap.empty())
		return -1;

	prjVec = *m_prjVec_ap[0];
	prjVec.vectorSet(m_avarge_a);
//#ifdef _DEBUG
//	static int count = 7000;
//	int cnt = count;
//	char buff[255];
//
//	sprintf_s(buff, 255, "%d.svg", cnt);
//	STLString filepath = _prjVec_p->nameGet();
//	filepath = filepath.substr(0, filepath.find_last_of('.'));
//	filepath += buff;
//
//	prjVec.svgSave(filepath);
//	count++;
//#endif
	float diff = *_prjVec_p - prjVec;
	if (diff < 0)
		diff *= -1;
	return diff;
}

int BasePrjVectorGroup::averageRemake()
{
	if (m_prjVec_ap.empty())
		return 0;

	m_avarge_a.resize(m_prjVec_ap[0]->vectorGet().size());

	for (int i = 0; i < m_avarge_a.size(); i++)
	{
		m_avarge_a[i].SetInit();
	}

	for (int i = 0; i < m_prjVec_ap.size(); i++)
	{
		const STLVVec3& vec_a = m_prjVec_ap[i]->vectorGet();

		for (int j = 0; j < m_avarge_a.size(); j++)
		{
			m_avarge_a[j] += vec_a[j];
		}
	}

	float cnt = (float)m_prjVec_ap.size();
	for (int j = 0; j < m_avarge_a.size(); j++)
	{
		m_avarge_a[j] /= cnt;
	}

	return 1;
}

bool BasePrjVectorGroup::averageGet(BaseProjectedVector* _prjVec_p) const
{
	if (m_prjVec_ap.empty() || m_avarge_a.empty())
		return false;

	*_prjVec_p = *m_prjVec_ap[0];
	_prjVec_p->vectorSet(m_avarge_a);
	return true;
}

int BasePrjVectorGroup::indexGet(int _seq)
{
	if (m_prjVec_ap.empty()
		|| m_prjVec_ap.size() <= _seq)
		return -1;

	return m_prjVec_ap[_seq]->index();
}
#include "stdafx.h"
#include "BaseXML.h"

PtObjectCpp(BaseXML);

#include "Matrix4.h"

static unsigned s_object_serial = 80000;


bool BaseXMLOption::set(BaseFile* _file)
{
	value = "";
	_file->push_asc_seperator("=:;\"");
	name = _file->read_asc_str(); 
	_file->pop_asc_seperator();
	if (name.empty())
		return false;
	if (_file->get_seperator_last() == ';' ||
		_file->get_seperator_last() == '\"')
		return true;
	_file->push_asc_seperator(";\"");
	value = _file->read_asc_str();
	_file->pop_asc_seperator();
	if (value.empty())
		return false;
	if (_file->get_seperator_last() == ';'
		|| _file->get_seperator_last() == '\"')
		return true;
	return false;
}

STLString BaseXMLOption::get()
{
	STLString format;
	format = name;
	format += "=";
	format += value;
	format += ';';
	return format;
}

STLSInt BaseXMLParam::s_param_multi_s;

void BaseXMLParam::float_set(STLString _name, float _value)
{
	char buf[128];
	sprintf_s(buf, 128, "%.3f", _value);
	set(_name, buf);
}

void BaseXMLParam::color_set(STLString _name, const PtVector3& _color)
{
	char buf[128];
	int r, g, b;
	r = (int)(_color[0] * 255.f);
	g = (int)(_color[1] * 255.f);
	b = (int)(_color[2] * 255.f);
	sprintf_s(buf, 128, "#%02x%02x%02x", r, g, b);
	set(_name, buf);
}

void BaseXMLParam::set(STLString _name, STLString _option)
{
	for (int i = 0; i < option_a.size(); i++)
	{
		if (option_a[i].name == _name)
		{
			option_a[i].value = _option;
			return;
		}
	}
	
	BaseXMLOption option;
	option.name = _name;
	option.value = _option;

	option_a.push_back(option);
}

bool BaseXMLParam::set(BaseFile* _file)
{
	style = EnStyle::Single;
	XMLREAD(name);
	STLSInt::iterator it;
	it = s_param_multi_s.find(STRTOHASH(name.c_str()));
	if (it != s_param_multi_s.end())
		style = EnStyle::Multi;
	if (style == EnStyle::Single)
	{
		XMLREADQ(value);
	}
	else {
		BaseXMLOption option;
		STLString temp = _file->read_asc_str();
		XMLCheck('\"');
		do {
			if (option.set(_file))
			{
				if (option.value.empty())
					value = option.name;
				else
					option_a.push_back(option);
			}
			else
				return true;
		} while (_file->get_seperator_last() != '\"');
	}
	return true;
}

STLString BaseXMLParam::get()
{
	STLString format;
	format = " ";
	format += name;
	format += "=";
	format += "\"";

	if (!value.empty()) {
		format += value;
		if(option_a.size() > 0)
			format += ";";
	}

	for (int i = 0; i < option_a.size(); i++)
	{
		format += option_a[i].get();
	}
	format += "\"";
	return format;
}

bool BaseXMLNode::color_get(const char* _name, PtVector3 *_color)
{
	PtVector3 ret_v3;
	STLString str = get(_name);

	char buf[10];
	buf[2] = 0;
	if (str.empty())
		return false;

	if (str[0] == '#')
	{
		for (int i = 0; i < 3; i++)
		{
			int value;
			int idx = i * 2 + 1;
			buf[0] = str[idx];	buf[1] = str[idx + 1];
			sscanf_s(buf, "%x", &value);
			(*_color)[i] = (float)value / 255.f;
		}
		return true;
	}

	return false;
}

void BaseXMLNode::color_set(const char* _name, const PtVector3& _color)
{
	char buf[128];
	int r, g, b;
	r = (int)(_color[0] * 255.f);
	g = (int)(_color[1] * 255.f);
	b = (int)(_color[2] * 255.f);
	sprintf_s(buf, 128, "#%02x%02x%02x", r, g, b);
	set(_name, buf);
}

bool BaseXMLNode::float_get(const char* _name, float* _value)
{
	STLString str = get(_name);

	if (str.empty())
		return false;

	*_value = BaseFile::get_float(str.c_str());
	return true;
}

void BaseXMLNode::int_set(const char* _name, int _value)
{
	char buf[128];
	sprintf_s(buf, 128, "%d", _value);
	set(_name, buf);
}

void BaseXMLNode::float_set(const char* _name, float _value)
{
	char buf[128];
	sprintf_s(buf, 128, "%f", _value);
	set(_name, buf);
}

bool BaseXMLNode::int_get(const char* _name, int* _value) const
{
	STLString tag = get(_name);
	if (tag.empty())
		return false;

	sscanf_s(tag.c_str(), "%d", _value);
	return true;
}

bool BaseXMLNode::vector_get(const char* _name, PtVector3* _pv3)
{
	STLString tag = get(_name);
	if (tag.empty())
		return false;

	sscanf_s(tag.c_str(), "%f,%f,%f", &_pv3->x, &_pv3->y, &_pv3->z);
	return true;
}

void BaseXMLNode::vector_set(const char* _name, const PtVector3& _v3)
{
	char buf[128];
	sprintf_s(buf, 128, "%.2f,%.2f,%.2f", _v3.x, _v3.y, _v3.z);
	set(_name, buf);
}

float BaseXMLNode::get_float(const char* _name, const char* _option)
{
	STLString tag = get(_name, _option);

	if (tag.empty())
		return 0;

	float ret = BaseFile::get_float(tag.c_str());
	return ret;
}

bool BaseXMLNode::fontsize_get(const char* _name, float* _value)
{
	STLString tag = get(_name);

	if (tag.empty())
		return false;

	sscanf_s(tag.c_str(), "%fpt", _value);
	return true;
}

void BaseXMLNode::fontsize_set(const char* _name, float _value)
{
	char buf[128];
	sprintf_s(buf, 128, "%fpt", _value);
	set(_name, buf);
}

BaseXMLNode* BaseXMLNode::node_get(const char* _name)
{
	if (name == _name)
		return this;

	BaseXMLNode* node;
	for (int i = 0; i < child_pa.size(); i++)
	{
		node = child_pa[i].node_get(_name);
		if (node)
			return node;
	}

	return NULL;
}

bool BaseXMLNode::text_set(const STLString& _text)
{
	bool checked = false;
	if (!text.empty())
	{
		for (int i = 0; i < text.size(); i++)
		{
			if (text[i] != 32)
			{
				checked = true;
				break;
			}
		}
	}
	if (checked)
	{
		text = _text;

		for (int i = 0; i < child_pa.size(); i++)
			child_pa[i].text_set("");
		return true;
	}
	
	bool update = false;
	for (int i = 0; i < child_pa.size(); i++)
	{
		if (!update)
		{
			if (child_pa[i].text_set(_text))
				update = true;
		}
		else
			child_pa[i].text_set("");
	}
	return update;
}

STLString BaseXMLNode::text_get() const
{
	STLString ret_str = text;
	for (int i = 0; i < child_pa.size(); i++)
	{
		ret_str += child_pa[i].text_get();
	}
	return ret_str;
}

STLVVec3 BaseXMLNode::vertex_get(STLString& _str, STLVInt *_seperator)
{
	STLVVec3 ret_a;
	PtVector3 v3;
	STLString str;
	BaseFile paser;
	paser.set_asc_seperator("MLCz \t,");
	paser.set_asc_sign('-');
	paser.set_asc_deletor(" \t");
	paser.OpenFile((void*)_str.c_str(), (UINT32)_str.size());
	if (paser.read_asc_line() && paser.read_asc_string(&str))
	{
		v3.z = 0;
		float value;
		if (_seperator)
			_seperator->push_back(paser.get_seperator_last());

		while (paser.read_asc_float(&value))
		{
			int last = paser.get_seperator_last();
			if (last == 'M')
			{
				if (_seperator)
					_seperator->push_back(last);
				continue;
			}
			v3.x = value;
			paser.read_asc_float(&value);
			last = paser.get_seperator_last();
			v3.y = value;
			ret_a.push_back(v3);
			if (_seperator)
				_seperator->push_back(last);
		}
	}
	return ret_a;
}

bool matrix_get(STLString& _str, PtMatrix4 *_m4)
{
	_m4->MakeIdentity();
	STLString str;
	BaseFile paser;
	paser.set_asc_seperator("() , ");
	paser.set_asc_deletor(" ");
	paser.OpenFile((void*)_str.c_str(), (UINT32)_str.size());
	if (paser.read_asc_line() && paser.read_asc_string(&str))
	{
		if (str == "matrix")
		{
			float value;
			for (int y = 0; y < 2; y++)
				for (int x = 0; x < 2; x++)
				{
					paser.read_asc_float(&value);
					(*_m4)[y][x] = value;
				}
			for (int x = 0; x < 2; x++)
			{
				paser.read_asc_float(&value);
				(*_m4)[3][x] = value;
			}
			return true;
		}
	}
	return false;
}

bool BaseXMLNode::take(void* _data, fnCallBack _func, BaseXMLNode *_node)
{	
	STLVXMLNode::iterator it;
	it = child_pa.begin();
	for (; it != child_pa.end(); it++)
	{
		if (_func(&(*it), _data))
		{
			*_node = *it;
			child_pa.erase(it);
			return true;
		}

		if ((*it).take(_data, _func, _node))
			return true;
	}
	return false;
}

const BaseXMLNode *BaseXMLNode::find(void* _data, fnConstCallBack _func) const
{
	if (_func(this, _data))
		return this;

	STLVXMLNode::const_iterator it;
	it = child_pa.begin();
	for (; it!=child_pa.end(); it++)
	{
		const BaseXMLNode* node = (*it).find(_data, _func);
		if(node)
			return node;
	}
	return NULL;
}

BaseXMLNode& BaseXMLNode::operator=(const BaseXMLNode& _node)
{
	id = _node.id;
	name = _node.name;
	text = _node.text;
	text_tos = _node.text_tos;
	param_a = _node.param_a;
	block_b = _node.block_b; // it has end tag, if not it should be ended by '/'
	child_pa = _node.child_pa;
	m_end = _node.m_end;

	return *this;
}

bool BaseXMLNode::transform(const PtMatrix4& _m4)
{
	STLString vector_str = get("d");
	char buf[128];
	if (!vector_str.empty())
	{
		STLVInt seperator;
		STLVVec3 vec_a = vertex_get(vector_str, &seperator);

		for (int i = 0; i < vec_a.size(); i++)
			vec_a[i] *= _m4;

		vertex_set(vec_a, seperator);
	}

	STLString x_value = get("x");
	STLString y_value = get("y");

	PtVector3 v3(0,0,0);
	if (!x_value.empty() && !y_value.empty())
	{	
		v3.SetValue(BaseFile::get_float(x_value.c_str()), BaseFile::get_float(y_value.c_str()), 0);
		v3 *= _m4;
		sprintf_s(buf, 128, "%f", v3.x);
		set("x", buf);
		sprintf_s(buf, 128, "%f", v3.y);
		set("y", buf);
	}

	float angle = _m4.GetRotate(2) / PM_PI * 180.f;
	if (angle != 0 && name == "text")
	{
		sprintf_s(buf, 128, "rotate(%f %f,%f)", angle, v3.x, v3.y);
		set("transform", buf);
	}

	for (int i = 0; i < child_pa.size(); i++)
	{
		child_pa[i].transform(_m4);
	}

	return true;
}

bool BaseXMLNode::scale(PtVector3 _scale)
{
	STLString vector_str = get("d");
	char buf[128];
	if (!vector_str.empty())
	{
		STLVInt seperator;
		STLVVec3 vec_a = vertex_get(vector_str, &seperator);

		for (int i = 0; i < vec_a.size(); i++)
			vec_a[i] *= _scale;
		
		vertex_set(vec_a, seperator);
	}

	STLString x_value = get("x");
	STLString y_value = get("y");
	if (!x_value.empty() && !y_value.empty())
	{
		PtVector3 v3;
		v3.SetValue(BaseFile::get_float(x_value.c_str()), BaseFile::get_float(y_value.c_str()), 0);
		v3 *= _scale;
		sprintf_s(buf, 128, "%f", v3.x);
		set("x", buf);
		sprintf_s(buf, 128, "%f", v3.y);
		set("y", buf);

		STLString size_str = get("font-size");
		if (!size_str.empty())
		{
			float scale;
			sscanf_s(size_str.c_str(), "%fpt", &scale);
			if (_scale.x < _scale.y)
				scale *= _scale.x;
			else
				scale *= _scale.y;
			sprintf_s(buf, 128, "%fpt", scale);
			set("font-size", buf);
		}

		STLString transform_str;
		transform_str = get("transform");
		if (name == "text" && !transform_str.empty() && transform_str.find("rotate") != STLString::npos)
		{
			float rot;
			sscanf_s(transform_str.c_str(), "rotate(%f", &rot);
			sprintf_s(buf, 128, "rotate(%f %f,%f)", rot, v3.x, v3.y);
			set("transform", buf);
		}
	}

	for (int i = 0; i < child_pa.size(); i++)
	{
		child_pa[i].scale(_scale);
	}

	return true;
}

float BaseXMLNode::lineCoverRate(PtBound& _bnd, float* _total_p)
{
	STLVVec3 vertex_a = vertex_get();

	float ret_f = 0;
	PtVector3 v3, dir;
	int	idx;
	float l;
	for (int i = 0; i < vertex_a.size() - 1; i++)
	{
		dir = vertex_a[i + 1] - vertex_a[0];
		l = dir.Length();
		*_total_p += l;
		if (_bnd.collision(&vertex_a[i], &v3, &idx))
		{
			if (idx == 3)
			{
				ret_f += l;
			}
			else {
				dir = v3 - vertex_a[idx];
				ret_f += dir.Length();
			}
		}
	}
	return ret_f;
}

STLVVec3 BaseXMLNode::vertex_get(bool _include_z) const
{
	//if (!m_vertex_a.empty() && !_include_z)
	//	return m_vertex_a;

	STLString vector_str = get("d");
	if (_include_z)
	{
		STLVInt control_a;
		STLVVec3 vec_a = vertex_get(vector_str, &control_a);
		if (!control_a.empty() && control_a.back() == 'z')
			vec_a.push_back(vec_a.front());
		return vec_a;
	}
	
	return vertex_get(vector_str);
}

bool BaseXMLNode::move(PtVector3 _v3)
{
	STLString vector_str = get("d");
	char buf[128];
	if (!vector_str.empty())
	{
		STLVInt seperator;
		STLVVec3 vec_a = vertex_get(vector_str, &seperator);

		for (int i = 0; i < vec_a.size(); i++)
			vec_a[i] += _v3;

		vertex_set(vec_a, seperator);
	}

	STLString x_value = get("x");
	STLString y_value = get("y");
	if (!x_value.empty() && !y_value.empty())
	{
		PtVector3 v3;
		v3.SetValue(BaseFile::get_float(x_value.c_str()), BaseFile::get_float(y_value.c_str()), 0);
		v3 += _v3;
		sprintf_s(buf, 128, "%f", v3.x);
		set("x", buf);
		sprintf_s(buf, 128, "%f", v3.y);
		set("y", buf);

		STLString transform_str;
		transform_str = get("transform");
		if (name == "text" && !transform_str.empty() && transform_str.find("rotate") != STLString::npos)
		{
			float rot;
			sscanf_s(transform_str.c_str(), "rotate(%f", &rot);
			sprintf_s(buf, 128, "rotate(%f %f,%f)", rot, v3.x, v3.y);
			set("transform", buf);
		}
	}

	for (int i = 0; i < child_pa.size(); i++)
	{
		child_pa[i].move(_v3);
	}

	return true;
}

bool BaseXMLNode::vertexPoly_set(STLVVec3& _vec_a)
{
	if (_vec_a.size() < 2)
		return false;

	bool closed = false;
	if (_vec_a[0] == _vec_a.back())
	{
		if (_vec_a.size() < 3)
			return false;
		closed = true;
	}

	STLVInt sep_a;
	sep_a.push_back('M');
	sep_a.push_back('L');
	int end = (int)_vec_a.size();
	if (closed)
		end--;

	for (int i = 2; i < end; i++)
		sep_a.push_back(' ');

	STLVVec3 vec_a;
	if (closed) {
		sep_a.push_back('z');
		for (int i = 0; i < end; i++)
			vec_a.push_back(_vec_a[i]);
	}else {
		sep_a.push_back(0);
		vec_a = _vec_a;
	}

	return vertex_set(vec_a, sep_a);
}

bool BaseXMLNode::vertexCurve_set(STLVVec3& _vec_a)
{
	if (_vec_a.size() < 2)
		return false;

	STLVInt sep_a;
	sep_a.push_back('M');
	sep_a.push_back('C');
	int end = (int)_vec_a.size();
	for (int i = 2; i < end; i++)
		sep_a.push_back(' ');

	sep_a.push_back(0);
	return vertex_set(_vec_a, sep_a);
}

bool BaseXMLNode::vertex_set(STLVVec3& _vec_a, char _type, bool _closed)
{
	STLVInt sep_a;
	sep_a.push_back('M');
	sep_a.push_back(_type);
	for (int i = 1; i < _vec_a.size()-1; i++)
		sep_a.push_back(' ');
	if (_closed)
		sep_a.push_back('z');
	else
		sep_a.push_back(0);

	return vertex_set(_vec_a, sep_a);
}

bool BaseXMLNode::vertex_set(STLVVec3& _ver_a, STLVInt& _seperator)
{
	m_vertex_a = _ver_a;
	
	STLString vector_str = "";
	int s_c = 0;
	char buf[128];
	for (int i = 0; i < _ver_a.size(); i++)
	{
		if (_seperator[s_c] != '-')
			vector_str += (char)_seperator[s_c];
		if (s_c < _seperator.size() - 2 && _seperator[s_c + 1] == 'M') {
			s_c++;
			vector_str += (char)_seperator[s_c];
		}
		s_c++;
		for (int j = 0; j < 2; j++)
		{
			if(_ver_a[i][j] >= 0 && j==1)
				sprintf_s(buf, 128, " %2.2f", _ver_a[i][j]);
			else
				sprintf_s(buf, 128, "%2.2f", _ver_a[i][j]);
			vector_str += buf;
		}
	}
	if (_seperator[s_c])
		vector_str += (char)_seperator[s_c];

	set("d", vector_str);
	return true;
}

bool BaseXMLNode::transform_remove(PtMatrix4 _m4)
{
	STLString transform_str = get("transform");
	PtMatrix4 m4, m4R;
	m4 = _m4;
	BaseFile paser;
	bool ret = false;
	PtBound bnd;

	if (!transform_str.empty()
		&& matrix_get(transform_str, &m4R))
	{
			ret = true;
			m4 *= m4R;
	}

	STLString vector_str = get("d");
	char buf[128];
	if (!vector_str.empty())
	{
		STLVInt seperator;
		STLVVec3 vec_a = vertex_get(vector_str, &seperator);

		vector_str = "";
		for (int i = 0; i < vec_a.size(); i++)
			vec_a[i] *= m4;
		
		vertex_set(vec_a, seperator);
	}

	STLString x_value = get("x");
	STLString y_value = get("y");
	PtVector3 v3(0, 0, 0);

	if (!x_value.empty() && !y_value.empty())
	{
		
		v3.SetValue(BaseFile::get_float(x_value.c_str()), BaseFile::get_float(y_value.c_str()), 0);
		v3 *= m4;
		sprintf_s(buf, 128, "%f", v3.x);
		set("x", buf);
		sprintf_s(buf, 128, "%f", v3.y);
		set("y", buf);
	}

	float angle = m4.GetRotate(2) / PM_PI * -180.f;
	if (angle != 0 && name == "text")
	{
		sprintf_s(buf, 128, "rotate(%f %f,%f)", angle, v3.x, v3.y);
		set("transform", buf);
	}

	for (int i = 0; i < child_pa.size(); i++)
	{
		if (child_pa[i].transform_remove(m4))
		{
			for (int j = 0; j < child_pa[i].child_pa.size(); j++)
				child_pa.push_back(child_pa[i].child_pa[j]);
			child_pa[i].child_pa.clear();
		}
	}

	return ret;
}

bool BaseXMLNode::bound_child_get(PtMatrix4* _m4, PtBound* _bnd, STLString _name, int _value) const
{
	STLString transform_str = get("transform");
	PtMatrix4 m4, m4R;
	m4 = *_m4;

	if (!transform_str.empty() && matrix_get(transform_str, &m4R))
		m4 *= m4R;

	bool first = true;
	PtBound bnd;
	int _id;
	for (int i = 0; i < child_pa.size(); i++)
	{
		if (child_pa[i].int_get(_name.c_str(), &_id) 
			&& _id == _value
			&& child_pa[i].bound_get(&m4, &bnd))
		{
			if (first)
				*_bnd = bnd;
			else
				_bnd->merge(bnd);

			first = false;
		}
	}

	return !first;
}

bool BaseXMLNode::bound_get(PtMatrix4 *_m4, PtBound* _bnd) const 
{
	STLString transform_str = get("transform");
	PtMatrix4 m4, m4R;
	m4 = *_m4;
	BaseFile paser;
	bool first = true;
	PtBound bnd;

	if (!transform_str.empty() && matrix_get(transform_str, &m4R))
			m4 *= m4R;

	STLString vector_str = get("d");
	if (!vector_str.empty())
	{
		STLVVec3 vec_a = vertex_get(vector_str);

		if (_bnd->set(vec_a))
			first = false;
	}

	if (name != "rect")
	{
		STLString x_value = get("x");
		STLString y_value = get("y");
		if (!x_value.empty() && !y_value.empty())
		{
			PtVector3 v3;
			v3.SetValue(BaseFile::get_float(x_value.c_str()), BaseFile::get_float(y_value.c_str()), 0);
			if (first)
				(*_bnd)[0] = (*_bnd)[1] = v3;
			first = false;
		}

		if (name == "text")
		{
			STLString rot = get("transform");
			bool up = false;
			if (!rot.empty() && rot.substr(0, rot.find('(')) == "rotate")
			{
				up = true;
			}

			STLString width = get("font-size");
			if (!width.empty())
			{
				STLString text_ = text_get();
				float h;
				if (EOF == sscanf(width.c_str(), "%fpt", &h))
					if (EOF == sscanf(width.c_str(), "%fpt", &h))
						h = 0;

				if(h!= 0)
				{
					float w = text_.size() * h / 2.f;
					if (up) // rotated text
					{
						(*_bnd)[1] = (*_bnd)[0];
						(*_bnd)[0].x = (*_bnd)[1].x - h;
						(*_bnd)[0].y = (*_bnd)[1].y - w;
					}
					else {
						(*_bnd)[0].y -= h;
						(*_bnd)[1].x = (*_bnd)[0].x + w;
						(*_bnd)[1].y = (*_bnd)[0].y + h;
					}
				}
			}

		}
	}

	for (int i = 0; i < child_pa.size(); i++)
	{
		if (child_pa[i].bound_get(&m4, &bnd))
		{
			if (first)
				*_bnd = bnd;
			else
				_bnd->merge(bnd);

			first = false;
		}
	}

	return !first;
}

bool BaseXMLNode::set(const char* _name, int _value)
{
	char buf[128];
	sprintf_s(buf, 128, "%d", _value);
	return set(_name, buf);
}

bool BaseXMLNode::set(const char* _name, const char* _value)
{
	if (_name == NULL)
		return false;

	STLString name2 = _name;
	if (_value == NULL)
		return set(name2, "");

	STLString value = _value;
	return set(name2, value);
}

bool BaseXMLNode::set(STLString  _name, STLString  _option, STLString  _value)
{
	for (int i = 0; i < param_a.size(); i++)
	{
		if (param_a[i].name == _name)
		{
			BaseXMLParam& param = param_a[i];

			for (int j = 0; j < param.option_a.size(); j++)
			{
				if (param.option_a[j].name == _option)
				{
					if (_value.empty())
						param.option_a.erase(param.option_a.begin() + j);
					else
						param.option_a[j].value = _value;
					return true;
				}
			}

			if (!_value.empty())
			{
				BaseXMLOption option;
				option.name = _option;
				option.value = _value;
				param.option_a.push_back(option);
			}
			return true;
		}
	}

	return false;
}

bool BaseXMLNode::set(STLString _name, STLString _value)
{
	for (int i = 0; i < param_a.size(); i++)
	{
		if (param_a[i].name == _name)
		{
			if (_value.empty())
				param_a.erase(param_a.begin() + i);
			else
				param_a[i].value = _value;
			return true;
		}
	}

	if (!_value.empty())
	{
		BaseXMLParam param;
		param.name = _name;
		param.value = _value;
		param_a.push_back(param);
	}

	return false;
}

STLString BaseXMLNode::get(const char* _name, const char* _option) const
{
	for (int i = 0; i < param_a.size(); i++)
	{
		if (param_a[i].name == _name)
		{
			if (!_option)
				return param_a[i].value;
			else{
				const BaseXMLParam& param = param_a[i];

				for (int j = 0; j < param.option_a.size(); j++)
				{
					if (param.option_a[j].name == _option)
						return param.option_a[j].value;
				}
			}
		}
	}

	STLString str;
	return str;
}

bool polygonCollision(STLVVec3& _vec1_a, STLVVec3& _vec2_a, float _minAngle)
{
	float accu;

	PtVector3 up(0, 0, 1);
	for (int i = 0; i < _vec1_a.size(); i++)
	{
		accu = 0.f;
		PtVector3 v1 = _vec2_a[0] - _vec1_a[i], v2;
		v1.MakeNormal();
		for (int j = 1; j < _vec2_a.size(); j++)
		{
			v2 = _vec2_a[j] - _vec1_a[i];
			v2.MakeNormal();

			accu += v1.angle_inter(v2, up);
			v1 = v2;
		}

		if (accu < 0)
			accu *= -1;

		if (accu >= _minAngle)
			return true;

	}

	return false;
}

bool lineCollision(const STLVVec3& _vec1_a, const STLVVec3& _vec2_a, float _maxDist)
{
	float accu;

	PtVector3 up(0, 0, 1), ip;
	for (int i = 1; i < _vec1_a.size(); i++)
	{
		accu = 0.f;
		const PtVector3* line1 = &_vec1_a[i - 1];
		PtVector3	dist_v3;
		dist_v3 = line1[0] - line1[1];
		float rate = 0, rmax = _maxDist/dist_v3.Length();
		for (int j = 0; j < _vec2_a.size(); j++)
		{
			if ((_vec2_a[j].line_distance(line1, &rate) <= _maxDist)
				&& rate >= -rmax && rate <= 1+rmax)
				return true;

			if (j > 0)
			{
				const PtVector3* line2 = &_vec2_a[j - 1];
				if (PtVector3::line_intersection(line1[0], line1[1], line2[0], line2[1], &ip))
					return true;
			}
		}
	}

	return false;
}

bool BaseXMLNode::limitGet(const PtVector3& _dir, PtVector3* _limit) const
{
	STLVVec3 vec_a = vertex_get();

	if (vec_a.empty())
		return false;

	PtVector3 near_v3, pos_v3, dir_v3;
	float near_f = 9999999;

	pos_v3 = vec_a[0];
	for (int i = 1; i < vec_a.size(); i++)
	{
		dir_v3 = vec_a[i] - pos_v3;
		float dist = _dir.Dot(dir_v3);
		if (dist < near_f)
		{
			*_limit = vec_a[i];
			near_f = dist;
		}
	}

	return true;
}

bool BaseXMLNode::lineCollision(const STLVVec3& _vec_a, float _maxDist)
{
	STLVVec3 vec1_a = vertex_get(true);
	if (vec1_a.empty())
		return false;

	if (::lineCollision(vec1_a, _vec_a, _maxDist))
		return true;

	return false;
}

bool BaseXMLNode::lineCollision(BaseXMLNode* _node, float _maxDist)
{
	STLVVec3 vec1_a = vertex_get(true);
	if (vec1_a.empty())
		return false;

	STLVVec3 vec2_a = _node->vertex_get(true);
	if (vec2_a.empty())
		return false;

	if (::lineCollision(vec1_a, vec2_a, _maxDist))
		return true;

	return false;
}

bool BaseXMLNode::polygonCollision(BaseXMLNode* _node, float _minAngle)
{
	STLVVec3 vec1_a = vertex_get();
	if (vec1_a.empty())
		return false;
	
	STLVVec3 vec2_a = _node->vertex_get();
	if (vec2_a.empty())
		return false;

	if (::polygonCollision(vec1_a, vec2_a, _minAngle))
		return true;

	if (::polygonCollision(vec2_a, vec1_a, _minAngle))
		return true;
	return false;
}

bool BaseXMLNode::set(BaseFile* _file)
{
	m_vertex_a.clear();
	
	m_end = false;
	block_b = false;
	int last = 0;

	bool sep = _file->seperator_del(' ');
	bool del = _file->deletor_del(' ');
	do {
		text_tos += read_tag(_file);
		if (m_end)
			return false;
		last = _file->get_seperator_last();
	} while (last != '<');

	if (sep) _file->seperator_add(' ');
	if (del) _file->deletor_add(' ');

	XMLREAD(name);
	last = _file->get_seperator_last();
	BaseXMLParam param;
	while (last != '>' && last != '/') {
		if (param.set(_file))
		{
			param_a.push_back(param);

			if (param.name == "d")
			{
				m_vertex_a = vertex_get(param.value);
			}
		}
		last = _file->get_seperator_last();
		param.option_a.clear();
		param.value = "";
	}

	if (last == '>')
	{
		block_b = true;
		BaseXMLNode node;

		while (node.set(_file))
		{
			node.id = s_object_serial++;
			child_pa.push_back(node);
			node.child_pa.clear();
			node.param_a.clear();
		}

		if (!node.text_tos.empty())
			text = node.text_tos;

		if (_file->get_seperator_last() == '/') // terminal tag
		{
			STLString tag = read_tag(_file);
			if (tag != name || _file->get_seperator_last() != '>')
			{
				if (tag == name && _file->get_seperator_last() == ' ')
				{
					do {
						tag = read_tag(_file);
					} while (_file->get_seperator_last() == ' ');
				}
				if (_file->get_seperator_last() != '>')
					return false;
			}

			return true;
		}
	}
	else if (last != '/')
		return false;
	else {
		_file->read_asc_str(); // read last tag '>'
	}

	return true;
}

STLString BaseXMLNode::read_tag(BaseFile* _file)
{
	STLString str;
	str = _file->read_asc_str();

	while (str.empty() && _file->get_seperator_last() == 0 && !m_end)
	{
		if (!_file->read_asc_line())
			m_end = true;
		str = _file->read_asc_str();
	}
	return str;
}

STLString BaseXMLNode::get(int _tab)
{
	STLString format;
	if (name.empty())
		return format;

	XMLWTAB(_tab);		format += "<";
	format += name;
	for (int i = 0; i < param_a.size(); i++)
	{
		format += param_a[i].get();
	}
	if (!block_b)
		format += "/";
	format += ">";
	format += text;
	format += "\n";

	if (block_b)
	{
		for (int i = 0; i < child_pa.size(); i++)
		{
			format += child_pa[i].get(_tab + 1);
		}
		XMLWTAB(_tab);
		format += "</";
		format += name;
		format += ">\n";
	}
	return format;
}

BaseXML::~BaseXML()
{
	if (m_pFile)
		PT_OFree(m_pFile);
	m_pFile = NULL;
}

BaseXML::BaseXML()
{
	m_pFile = NULL;
	m_nSerialNumber = serial_create_();	// Unique number for each objects
	m_nObjectId = GetObjectId();
}

int BaseXML::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0) {
		s_iId = UniqHashStr::get_string_hash_code("BaseXML");
	}

	return s_iId;
}

STLString BaseXML::text_get()
{
	STLString out;
	for (int i = 0; i < m_root_a.size(); i++)
	{
		out += m_root_a[i].text_tos;
		out += m_root_a[i].text;
	}
	out += last;

	return out;
}

bool BaseXML::set(BaseFile* _file)
{
	bool ret_b = true;

	_file->set_asc_deletor(" \t");
	_file->set_asc_seperator(" \t<>/;=\"");
	last = "";

	BaseXMLNode node;
	while (node.set(_file))
	{
		node.id = s_object_serial++;
		m_root_a.push_back(node);
	}

	if (m_root_a.size() > 0)
		ret_b = true;
	else
		return false;

	if (!node.m_end)
	{
		_file->set_asc_seperator("");
		last = _file->read_asc_str();
	}

	return ret_b;
}

bool BaseXML::set(STLString& _stream)
{
	bool ret_b = true;

	BaseFile paser;

	paser.OpenFile((void*)_stream.data(), (int)_stream.size());

	return set(&paser);
}

bool BaseXML::Save()
{
	const char* strFilename = GetFileName();

	BaseFile file;

	if (file.OpenFile(strFilename, BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8) != 0)
		return false;

	for (int i = 0; i < m_root_a.size(); i++)
	{
		STLString str = m_root_a[i].get(0);

		file.write_asc_string(str.c_str(), 0);
		file.write_asc_line();
	}
	
	file.CloseFile();
	return true;
}

bool BaseXML::Load()
{
	if(BaseXMLParam::s_param_multi_s.size() == 0)
		BaseXMLParam::s_param_multi_s.insert(STRTOHASH("style"));

	const char* strFilename = GetFileName();

	BaseFile* pFile = NULL;
	PT_OAlloc(pFile, BaseFile);

	if (pFile->OpenFile(strFilename, BaseFile::OPEN_READ | BaseFile::OPEN_UTF8) != 0) {
		PT_OFree(pFile);
		return false;
	}

	m_pFile = pFile;
	set(m_pFile);

	pFile->CloseFile();
	PT_OFree(pFile);
	m_pFile = NULL;

	return true;
}

void BaseXML::Clear()
{
}

int BaseXML::read(int _nPos, bbyte* _data, int _nSize) {
	m_pFile->Seek(_nPos);
	return m_pFile->Read(_data, _nSize);
}

void BaseXML::root_set(BaseXMLNode* _root)
{
	m_root_a.clear();
	m_root_a.push_back(*_root);
}


bool BaseXML::fnMaxID(BaseXMLNode* _node, void* _data)
{
	int* max_p = (int*)_data;

	int id;
	if (_node->int_get("pn_id", &id) && id > *max_p)
	{
		*max_p = id;
	}
	return false;
}
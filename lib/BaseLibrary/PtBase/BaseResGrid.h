#pragma once
#include "BaseResResource.h"

class BaseResGridColumn {
public:
	STLVString	column_a;
};

typedef std::vector<BaseResGridColumn, PT_allocator<BaseResGridColumn> >	STLVGrid;

class BaseResGrid :
	public BaseResResource
{
public:
	BaseResGrid();
	~BaseResGrid();

	PtObjectHeader(BaseResGrid);
	static int GetObjectId();

	virtual bool Load(); // Load In Thread
	bool LoadJson();
	bool Save();

	const char* filename_get(const char *_name);
	virtual void Clear() {};

	int column_count();
	int column_search(const char *_label_name, const char* _search); // search ignore case and sub str
	STLVInt columns_search(const char *_label_name, const char* _search); // search ignore case and sub str
	int column_get(const char* _label_name, const char* _search); // search equal
	bool column_remove(int _column_index);
	const char* column_get(int _label_index, int _column_index);
	int label_find(const char* _label_name);
	int label_add(const char* _label_name);
	PtVector3 vector_get(int _column, const char* _label_name);
	const char*string_get(int _column, const char *_label_name);
	bool string_set(int _column, const char* _label_name, const char* _value);
	bool strings_add(STLVString _str_a);
	bool strings_set(const STLVString &_str_a);
	STLString label_get(int i);
	int label_count(){	return (int)m_label.size(); }

	bool VariableSet(const char* _variable, const STLString& _value_str);
	bool VariableGet(const char* _variable, STLString& _value_str);
	bool VariableSet(const char* _variable, float _value_f);
	bool VariableGet(const char* _variable, float &_value_f); 
	bool VariableSet(const char* _variable, int _value_n);
	bool VariableGet(const char* _variable, int& _value_n);
	INT64 ctid_next_get();
protected:
	STLVString	m_label;
	STLVGrid	m_grid_a;
	INT64	m_ctid_new;
};


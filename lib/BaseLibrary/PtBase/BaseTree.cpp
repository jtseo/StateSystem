#include "stdafx.h"
#include "BaseTree.h"

BaseTree::BaseTree(void)
{
}

BaseTree::~BaseTree(void)
{
	detech_all();
}

void BaseTree::detech_all()
{
	while(m_child_stlVp.size())
	{
		((BaseTree*)m_child_stlVp[0])->set_parent(0);
		((BaseTree*)m_child_stlVp[0])->dec_ref(this);
		m_child_stlVp.erase(m_child_stlVp.begin());
	}
}

void BaseTree::insert(int _nIndex, BaseTree *xpTree)
{
	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		if(i == _nIndex)
		{
			m_child_stlVp.insert(m_child_stlVp.begin()+i, xpTree);
			xpTree->set_parent(this);
			xpTree->inc_ref(this);
			return;
		}
	}

	m_child_stlVp.push_back(xpTree);
	xpTree->set_parent(this);
	xpTree->inc_ref(this);
}

BaseTree *BaseTree::get_next_depth_first()
{
	if(m_child_stlVp.size() > 0)
		return (BaseTree*)m_child_stlVp[0];

	if(get_parent() == NULL)
		return NULL;

	int nIndex	= get_index();
	return get_parent()->get_next_depth_first(nIndex);
}

BaseTree *BaseTree::get_next_depth_first(int _nIndex)
{
	if(_nIndex < (int)m_child_stlVp.size()-1)
	{
		return (BaseTree*)m_child_stlVp[_nIndex+1];
	}

	if(get_parent() == NULL)
		return NULL;

	return get_parent()->get_next_depth_first(get_index());
}

void BaseTree::attach_child(BaseTree *xpTree)
{
	m_child_stlVp.push_back(xpTree);
	xpTree->set_parent(this);
	xpTree->inc_ref(this);
}

void BaseTree::detech_child(BaseTree *xpTree)
{
	STLVpBaseObject::iterator it	= std::find(m_child_stlVp.begin(), m_child_stlVp.end(), xpTree);
	if(it != m_child_stlVp.end())
	{
		m_child_stlVp.erase(it);
		xpTree->dec_ref(this);
		xpTree->set_parent(0);
	}else{
#ifdef _DEBUG
		B_ASSERT(!"fail to detech child");
		//USER_CON_MSG(0, "Fail to detech child: P:%s, C:%s.", m_strName.c_str(),	xpTree->GetName().c_str());
#endif
	}
}

BaseTree *BaseTree::get_child(const char *_strName)
{
	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		const char *strName;
		strName	= m_child_stlVp[i]->get_name().c_str();
		if(_stricmp(_strName, strName) != 0)
			return (BaseTree*)m_child_stlVp[i];
	}

	return NULL;
}

// Move the specified child to first node.
void BaseTree::move_first(unsigned _nIndex)
{
	BaseObject	*pTree;

	if(_nIndex >= m_child_stlVp.size())
		return;

	pTree					= (BaseObject*)m_child_stlVp[_nIndex];
	m_child_stlVp[_nIndex]	= m_child_stlVp[0];
	m_child_stlVp[0]		= pTree;
}
// Move the specified child to last node.
void BaseTree::move_last(unsigned _nIndex)
{
	BaseObject	*pTree;
	size_t	nSize;

	nSize	= m_child_stlVp.size();
	if(_nIndex >= nSize)
		return;

	pTree					= (BaseObject*)m_child_stlVp[_nIndex];
	m_child_stlVp[_nIndex]	= m_child_stlVp[nSize-1];
	m_child_stlVp[nSize]	= pTree;
}
//EOF

int	BaseTree::get_index()
{
	if(get_parent()==NULL)
		return 0;

	unsigned i=0;
	for(i=0; i<get_parent()->get_childs().size(); i++)
	{
		if(get_parent()->get_child(i) == this)
			return i;
	}

	return i;
}

BaseTree *BaseTree::get_next_friend()
{
	BaseTree	*pTree	= NULL;

	if(get_parent())
	{
		return get_parent()->get_next_depth_first(get_index());
	}

	return pTree;
}
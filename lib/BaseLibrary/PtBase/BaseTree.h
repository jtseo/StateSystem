#pragma once
#include "BaseObject.h"

/** \brief 
*
* \par Project:
* Koongpa
*
* \par File:
* $Id: BaseTree.h,v 1.1 2009/03/13 07:32:56 jtseo Exp $
*
* \ingroup PtBase
* 
* \version 1.0
*
* \par History:
* $Date: 2009/03/13 07:32:56 $\n
* $Author: jtseo $\n
* $Locker:  $
*
* \par Description:
*
* \par license:
* Copyright (c) 2006 Ndoors Inc. All Rights Reserved.
* 
* \todo 
*
* \bug 
*
*/
class BaseTree :
	public BaseObject
{
public:
	BaseTree(void);
public:
	virtual ~BaseTree(void);

	virtual void insert(int _nIndex, BaseTree *xpTree);
	virtual void attach_child(BaseTree *xpTree);
	virtual void detech_child(BaseTree *xpTree);
	void detech_all();
	BaseTree *get_next_depth_first();	// Depth first traverse in child node
	BaseTree *get_next_depth_first(int _nIndex);	// Depth first traverse in friend node
	BaseTree *get_next_friend();	// Skip child traverse

	void set_parent(BaseTree *xpParent){
		if(xpParent)
			m_parent_stlVp.push_back(xpParent);
		else
			m_parent_stlVp.erase(m_parent_stlVp.begin());
	}
	const BaseTree *get_parent()const{
		if(m_parent_stlVp.size())
			return (const BaseTree*)m_parent_stlVp[0];
		return NULL;
	}

	BaseTree *get_root()
	{
		if(m_parent_stlVp.size())
			return ((BaseTree*)m_parent_stlVp[0])->get_root();

		return this;
	}

	BaseTree *get_parent(){
		if(m_parent_stlVp.size())
			return (BaseTree*)m_parent_stlVp[0];
		return NULL;
	}
	const STLVpBaseObject &get_childs() const{
		return m_child_stlVp;
	}

	size_t	get_num_child(){
		return m_child_stlVp.size();
	}

	BaseTree	*get_child(int _nIndex){
		return (BaseTree*)m_child_stlVp[_nIndex];
	}

	BaseTree	*get_child(const char *_strName);

	int	get_index();

	// Move the specified child to first node.
	void move_first(unsigned _nIndex);
	// Move the specified child to last node.
	void move_last(unsigned _nIndex);
protected:
	STLVpBaseObject	m_parent_stlVp;
	STLVpBaseObject	m_child_stlVp;
};


#pragma once

#include "BaseObject.h"

class BaseActionStack :
    public BaseObject
{
};

class BaseActionStackAction: public BaseObject
{
public:
    virtual void Do() = 0;
    virtual void Undo(int _serial) = 0;
    virtual void Redo(int _serial) = 0;
};
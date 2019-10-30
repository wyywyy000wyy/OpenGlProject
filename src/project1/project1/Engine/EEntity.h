
#ifndef __EENTITY_H__
#define __EENTITY_H__
#include "EObject.h"

class EEntity : public EObject
{
public:
	std::string Name() const { "EEntity"; }
	std::string TypeStr() const { "EEntity"; }
	EType Type() const { return ENTITY; }
};


#endif
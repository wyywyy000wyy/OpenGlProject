
#ifndef __ELIGHT_H__
#define __ELIGHT_H__
#include "EObject.h"

class ELight : public EObject
{
public:
	std::string Name() const { "ELight"; }
	std::string TypeStr() const { "ELight"; }
	EType Type() const { return LIGHT; }
};


#endif
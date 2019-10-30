#ifndef __EOBJECT_H__
#define __EOBJECT_H__
#include <string>

typedef int EGUID;

class EObject
{
public:
	enum EType
	{
		OBJECT,
		LIGHT,
		ENTITY,
		SCENE,

	};

	virtual std::string Name() const { return "EObject"; }
	virtual std::string TypeStr() const { return "EObject"; }
	virtual EType Type() const { return OBJECT; }

	EGUID m_guild;
};


#endif
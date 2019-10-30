
#ifndef __ESCENE_H__
#define __ESCENE_H__
#include "EObject.h"
#include <map>
#include <list>

class EScene : public EObject
{
public:
	std::string Name() const { return "EScene"; }
	std::string TypeStr() const { return "EScene"; }
	EType Type() const { return SCENE; }

	void AddObject(EObject* );

	//δ�������ObjectList
	std::list<EObject*> m_rawObjectList;
};


#endif
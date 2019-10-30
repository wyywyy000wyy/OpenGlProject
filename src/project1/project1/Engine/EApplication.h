#ifndef __EAPP_H__
#define __EAPP_H__
#include "EScene.h"

class EApplication
{
public:
	virtual void Start();
	virtual void Update();
	virtual void Render();

	EScene m_scene;
};

extern EApplication* App();

#endif
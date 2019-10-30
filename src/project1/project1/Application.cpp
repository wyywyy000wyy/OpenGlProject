#include "Application.h"

EApplication* App() {
	static WinApplication w;
	return &w;
}
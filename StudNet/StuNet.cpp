#include "CServer.h"
#include "interface.h"


#ifdef _DEBUG

#include <crtdbg.h>
#pragma comment(lib,"Msvcrtd.lib")
#else

#define _ASSERT
#endif // _DEBUG



int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nShowCmd);

	App *app  = new App(hInstance,nShowCmd); 
	//CServer *serv = new CServer();

return 0;
}
#include <windows.h>

int CALLBACK
WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
  MessageBox(NULL, "Hello, world!", "Prueba", MB_OK|MB_ICONINFORMATION);
  return(0);
}

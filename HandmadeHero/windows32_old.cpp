#include <windows.h>

LRESULT CALLBACK
MainWindowCallBack(HWND hwnd,
		   UINT uMsg,
		   WPARAM wParam,
		   LPARAM lParam)
{
  LRESULT Result = 0;
  
  switch(uMsg)
    {
    case WM_CREATE:
      {
	OutputDebugStringA("WM_CREATE\n");
      } break;

    case WM_SIZE:
      {
	OutputDebugStringA("WM_SIZE\n");
      } break;

    case WM_DESTROY:
      {
	OutputDebugStringA("WM_DESTROY\n");
      } break;

    case WM_CLOSE:
      {
	OutputDebugStringA("WM_CLOSE\n");
      } break;

    case WM_ACTIVATEAPP:
      {
	OutputDebugStringA("WM_ACTIVATEAPP\n");
      } break;

    case WM_PAINT:
      {
	PAINTSTRUCT Paint;
	HDC DeviceContext = BeginPaint(hwnd, &Paint);
	int X = Paint.rcPaint.left;
	int Y = Paint.rcPaint.top;
	int Width = Paint.rcPaint.right - Paint.rcPaint.left;
	int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
	static DWORD Operation = WHITENESS;
	PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
	EndPaint(hwnd, &Paint);
      } break;

    default:
      {
	//	OutputDebugStringA("default\n");
	Result = 9;
      } break;
    }
  
  return(Result);
}


int CALLBACK
WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
  WNDCLASS WindowClass = {};

  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallBack;
  WindowClass.hInstance = hInstance;
  WindowClass.lpszClassName = "Blahblah";

  if(RegisterClass(&WindowClass))
    {
      HWND WindowHandle =
	CreateWindowEx(
		       WS_EX_LEFT,
		       WindowClass.lpszClassName,
		       "HandMade Hero",
		       WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		       CW_USEDEFAULT,
		       CW_USEDEFAULT,
		       CW_USEDEFAULT,
		       CW_USEDEFAULT,
		       0,
		       0,
		       hInstance,
		       0
		       );
      if(WindowHandle)
	{
	  for(;;)
	    {
	      /*	      MSG Message;
	      BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
	      if(MessageResult > 0)
		{
		  TranslateMessage(&Message);
		  DispatchMessage(&Message);
		}
	      else
		{
		  break;
		  }*/
	    }
	}
      else
	{
	}; 
    }
  else
    {
    };
  
  return(0);
}

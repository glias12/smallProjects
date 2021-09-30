#include <windows.h>

LRESULT CALLBACK MainWindowCallback(HWND hwnd,
				    UINT uMsg,
				    WPARAM wParam,
				    LPARAM lParam)
{
  LRESULT Result = 0;

  //#if 0
  switch(uMsg)
    {
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
	PostQuitMessage(0);
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
	PatBlt(DeviceContext, X, Y, Width, Height, Operation);
	if(Operation == WHITENESS)
	  {
	    Operation = BLACKNESS;
	  }
	else
	  {
	    Operation = WHITENESS;
	  }
	EndPaint(hwnd, &Paint);
      } break;
      
    default:
      {
	//       	OutputDebugStringA("default\n");
	Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
      } break;
      
    }
  //#endif
  return(Result);
};
			    
int CALLBACK
WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
  WNDCLASS WindowClass = {};

  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = hInstance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";

  if (RegisterClass(&WindowClass))
    {
      HWND WindowHandle =
	CreateWindowEx(
		       WS_EX_LEFT,
		       WindowClass.lpszClassName,
		       "Handmade Hero",
		       WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		       CW_USEDEFAULT,
		       CW_USEDEFAULT,
		       CW_USEDEFAULT,
		       CW_USEDEFAULT,
		       NULL,
		       NULL,
		       hInstance,
		       NULL);
      DWORD CWError = GetLastError();
      if (WindowHandle)
	{
	  for(;;)
	    {
	      MSG Message;
	      BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
	      if (MessageResult > 0)
		{
		  TranslateMessage(&Message);
		  DispatchMessage(&Message);
		}
	      else
		{
		  break;
		}
	    }
	}
    }
  else
    {
    }
  

  
  return(0);
}

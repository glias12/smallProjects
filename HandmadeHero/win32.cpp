#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer
{
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;

struct win32_window_dimension
{
  int Width;
  int Height;
};

win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
  win32_window_dimension Result;

  // Rect the writable region of the window.
  RECT ClientRect;

  // The coordinates of the writable section of the window.
  GetClientRect(Window, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;
  
  return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueOffset, int GreenOffset)
{
  uint8 *Row = (uint8 *) Buffer.Memory;
  
  for (int Y = 0; Y < Buffer.Height; ++Y)
    {
      uint32 *Pixel = (uint32 *) Row;
      
      for (int X = 0; X < Buffer.Width; ++X)
	{
	  /*
	    Pixel in Memory: RR GG BB xx
	    Intel is little endian
	  */
	  uint8 Blue = (X + BlueOffset);
	  uint8 Green = (Y + GreenOffset);

	  *Pixel++ = ((Green << 8) | Blue);
	}
      Row += Buffer.Pitch;
    }
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{

  // We will call this function as many times as we need it.
  // So we check if the memory was already booked.
  if(Buffer->Memory)
    {
      VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

  // We fill the buffer structure with the data needed.
  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->BytesPerPixel = 4;

  // When the biHeight field is negative, this is the clue to
  // Windows to treat this bitmap as top-down, not bottom-up, meaning that
  // the first three bytes of the image are the color for the top left pixel
  // in the bitmap, not the bottom left.
  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  // We reserve the bitmap memory. We do it in this way to be able to resize it
  // Whenever we need a different size.
  // VirtualAlloc books in memory page sizes.
  // Pitch: The number of bytes needed to go from one row to the next one.
  int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  Buffer->Pitch = Width*Buffer->BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
			   int WindowWidth, int WindowHeight,
			   win32_offscreen_buffer Buffer,
			   int X, int Y, int Width, int Height)
{
  // This function stretches our backupbuffer up into or down into our display window.
  StretchDIBits(DeviceContext,
		/*
		X, Y, Width, Height,
		X, Y, Width, Height, 
		*/
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer.Width, Buffer.Height,
		Buffer.Memory,
		&Buffer.Info,
		DIB_RGB_COLORS, SRCCOPY);
}


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
	//	OutputDebugStringA("WM_SIZE\n");
      } break;

    case WM_CLOSE:
      {
	//PostQuitMessage(0);
	//OutputDebugStringA("WM_CLOSE\n");
	GlobalRunning = false;
      } break;

    case WM_ACTIVATEAPP:
      {
	OutputDebugStringA("WM_ACTIVATEAPP\n");
      } break;

    case WM_DESTROY:
      {
	//OutputDebugStringA("WM_DESTROY\n");
	GlobalRunning = false;
      } break;

    case WM_PAINT:
      {
	PAINTSTRUCT Paint;
	HDC DeviceContext = BeginPaint(hwnd, &Paint);
	int X = Paint.rcPaint.left;
	int Y = Paint.rcPaint.top;
	int Width = Paint.rcPaint.right - Paint.rcPaint.left;
	int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

	win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
	Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
				   GlobalBackbuffer, X, Y, Width, Height);
				   
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

//
// Entry point for the C Runtime library.
//
int CALLBACK
WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
  //
  // We initialize the WindowClass to 0.
  // Contains the window class attributes that are registered by RegisterClass function.
  //
  WNDCLASS WindowClass = {};

  // Redraws the full window (and not only the new part)
  // We initialize the Backbuffer
  Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

  // MainWindowCallBack: Procedure to know what the program should do when processing the mesasages.
  // There is notr a pointer in windows that uniquely identifies a memory position.
  // 'Therefore, to call our procedure we need to identify also the memory space to where this process
  // belongs. Thats the hInstance value.
  // The ClassName is for Windows to identify the window attributes. We only have one type of windows.
  WindowClass.style = CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = hInstance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";

  // Windows will make a new copy of the class when it is registered. So we dont need to keep the data in
  // WindowClass.
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
 
      if (WindowHandle)
	{
	  // Since we specified CS_OWNDC, we can just
	  // get one device context and use it forever because we
	  // are not sharing it with anyone.
	  HDC DeviceCOntext = GetDC(WindowHandle);
	  
	  int XOffset = 0;
	  int YOffset = 0;

	  GlobalRunning = true;
	  while(GlobalRunning)
	    {
	      // We create a space in the stack to store the messages send by the program.
	      MSG Message;

	      // PeekMessage: Will check the Windows stack to see if there is a new windows message.
	      while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
		  if(Message.message == WM_QUIT)
		    {
		      GlobalRunning = false;
		    }

		  // TranslateMessage: Will translate the windows messages.
		  // DispatchMessage: Will call the procedure function to deal with the messages.
		  TranslateMessage(&Message);
		  DispatchMessageA(&Message);
		}
	      
	      RenderWeirdGradient(GlobalBackbuffer, XOffset, YOffset);

	      // We get a handle to the window that allows us to issue draw calls.
	      HDC DeviceContext = GetDC(WindowHandle);

	      // We get the size of the writable section of the window.
	      win32_window_dimension Dimension = Win32GetWindowDimension(WindowHandle);

	      // We display the buffer in the window.
	      Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
					 GlobalBackbuffer,
					 0, 0, Dimension.Width, Dimension.Height);

	      // We release the handle.
	      ReleaseDC(WindowHandle, DeviceContext);
	       
	      ++XOffset;
	      YOffset += 1;
	    }
	}
    }
  else
    {
    }
  
  return(0);
}



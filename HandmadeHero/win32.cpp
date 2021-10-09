#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

#define Pif32 3.141592653589793238

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32_t bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#include <windows.h>
#include <stdio.h>
#include <xinput.h>
#include <dsound.h>

// We only need get_state and set_state.
// We are going to load the functions ourselves without asking the linker to do it for us.
// We define pointers to the functions to avoid the linker.
#include <math.h>

#include "handmade.cpp"


//global_variable bool32 GlobalRunning;
//global_variable win32_offscreen_buffer GlobalBackbuffer;
//global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

// XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dxUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
  return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

// XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dxUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
  return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

// This is global for now.
global_variable bool GlobalRunning;
//global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable game_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

// We load the library ourselves.
// xinput1_3.dll seems to be available on more computers and have a bigger compatibility.
internal void
Win32LoadXInput(void)
{
  HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
  if(!XInputLibrary)
    {
      XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }

  if(!XInputLibrary)
    {
      XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
  
  if(XInputLibrary)
    {
      // GetProcAddress returns a void * We need to cast it to the right type.
      XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
      if(!XInputGetState) {XInputGetState = XInputGetStateStub;}

      XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
      if(!XInputSetState) {XInputSetState = XInputSetStateStub;}
    }
  else
    {
      
    }
}

internal void
Win32InitDSound(HWND WindowHandle, int32 SamplesPerSecond, int32 BufferSize)
{
  // Load the library.
  HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

  if(DSoundLibrary)
    {
      // Get a directsound object. Cooperative.
      direct_sound_create *DirectSoundCreate = (direct_sound_create *)
	GetProcAddress(DSoundLibrary, "DirectSoundCreate");
      
      // SUCCEEDED is a macro to check if the object was rightly created.
      LPDIRECTSOUND DirectSound;
      if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
	{
	  WAVEFORMATEX WaveFormat = {};
	  WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	  WaveFormat.nChannels = 2;
	  WaveFormat.nSamplesPerSec = SamplesPerSecond;
	  WaveFormat.wBitsPerSample = 16;
	  WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
	  WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
	  WaveFormat.cbSize = 0;

	  // DirectSound is a pointer to the virtual table of the dsound.dll
	  if(SUCCEEDED(DirectSound->SetCooperativeLevel(WindowHandle, DSSCL_PRIORITY)))
	    {
	      DSBUFFERDESC BufferDescription = {};
	      BufferDescription.dwSize = sizeof(BufferDescription);
	      BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
	      
	      // "Create" a primary  buffer.
	      LPDIRECTSOUNDBUFFER PrimaryBuffer;
	      if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
		{
		  if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
		    {
		      // We have finally set the format.
		      OutputDebugStringA("Primary buffer format was set.\n");
		    }
		  else
		    {
		    }
		}
	      else
		{
		}
	    }
	  else
	    {
	    }
	  
	  DSBUFFERDESC BufferDescription = {};
	  BufferDescription.dwSize = sizeof(BufferDescription);
	  BufferDescription.dwFlags = 0;
	  BufferDescription.dwBufferBytes = BufferSize;
	  BufferDescription.lpwfxFormat = &WaveFormat;
	  if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0)))
	    {
	      // Start it playing.
	      OutputDebugStringA("Secondary buffer created successfully.\n");
	    }
	}
      else
	{
	  
	}
    }
}

internal win32_window_dimension
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
RenderWeirdGradient(win32_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
  uint8 *Row = (uint8 *) Buffer->Memory;
  
  for (int Y = 0; Y < Buffer->Height; ++Y)
    {
      uint32 *Pixel = (uint32 *) Row;
      
      for (int X = 0; X < Buffer->Width; ++X)
	{
	  /*
	    Pixel in Memory: RR GG BB xx
	    Intel is little endian
	  */
	  uint8 Blue = (X + BlueOffset);
	  uint8 Green = (Y + GreenOffset);

	  *Pixel++ = ((Green << 8) | Blue);
	}
      Row += Buffer->Pitch;
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
  
  int BytesPerPixel = 4;

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
  int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  Buffer->Pitch = Width*BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(game_offscreen_buffer *Buffer,
			   HDC DeviceContext, int WindowWidth, int WindowHeight)
{
  // This function stretches our backupbuffer up into or down into our display window.
  StretchDIBits(DeviceContext,
		/*
		X, Y, Width, Height,
		X, Y, Width, Height, 
		*/
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);
}


internal LRESULT
CALLBACK MainWindowCallback(HWND hwnd,
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

      // Keyboard Input
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
      {
	uint32 VKCode = wParam;
	bool WasDown = ((lParam & (1 << 30)) != 0);
	bool IsDown = ((lParam & (1 << 31)) == 0);
	if(WasDown != IsDown)
	  {
	    if(VKCode == 'W')
	      {
	      }
	    else if(VKCode == 'A')
	      {
	      }
	    else if(VKCode == 'S')
	      {
	      }
	    else if(VKCode == 'D')
	      {
	      }
	    else if(VKCode == 'Q')
	      {
	      }
	    else if(VKCode == 'E')
	      {
	      }
	    else if(VKCode == VK_UP)
	      {
	      }
	    else if(VKCode == VK_LEFT)
	      {
	      }
	    else if(VKCode == VK_DOWN)
	      {
	      }
	    else if(VKCode == VK_RIGHT)
	      {
	      }
	    else if(VKCode == VK_ESCAPE)
	      {
	      }
	    else if(VKCode == VK_SPACE)
	      {
	      }
	  }

	bool32 AltKeyWasDown = (lParam & (1 << 29));
	if((VKCode == VK_F4) && AltKeyWasDown)
	  {
	    GlobalRunning = false;
	  }
      } break;
      
    case WM_PAINT:
      {
	PAINTSTRUCT Paint;
	HDC DeviceContext = BeginPaint(hwnd, &Paint);
	win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
	Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
				   Dimension.Width, Dimension.Height);
	EndPaint(hwnd, &Paint);
      } break;
      
    default:
      {
	//       	OutputDebugStringA("default\n");
	Result = DefWindowProcA(hwnd, uMsg, wParam, lParam);
      } break;
      
    }
  //#endif
  return(Result);
};


internal void
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
  // Sound Buffer sample:
  // int16 int16 int16 ...
  // [LEFT RIGHT] [LEFT RIGHT] ...
  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;

  if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
					   &Region1, &Region1Size,
					   &Region2, &Region2Size,
					   0)))
    {
      uint8 *DestSample = (uint8 *)Region1;
      for(DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
	{
	  *DestSample++ = 0;
	}

      DestSample = (uint8 *)Region2;
      for(DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
	{
	  *DestSample++ = 0;
	}

      GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite,
		     game_sound_output_buffer *SourceBuffer)
{
  // Sound Buffer sample:
  // int16 int16 int16 ...
  // [LEFT RIGHT] [LEFT RIGHT] ...
  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;
		  
  if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
					   &Region1, &Region1Size,
					   &Region2, &Region2Size,
					   0)))
    {
      DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
      int16 *DestSample = (int16 *)Region1;
      int16 *SourceSample = SourceBuffer->Samples;
      
      for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
	{
	  *DestSample++ = *SourceSample++;
	  *DestSample++ = *SourceSample++;
	  ++SoundOutput->RunningSampleIndex;
	}
		  
      DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
      DestSample = (int16 *)Region2;
      for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
	{
	  *DestSample++ = *SourceSample++;
	  *DestSample++ = *SourceSample++;
	  ++SoundOutput->RunningSampleIndex;
	}
		  
      GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
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
  // Value of counts per second for the counter.
  LARGE_INTEGER PerfCountFrequencyResult;
  QueryPerformanceFrequency(&PerfCountFrequencyResult);
  int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
  
  // Initialization of the input procedures.
  Win32LoadXInput();
  
  //
  // We initialize the WindowClass to 0.
  // Contains the window class attributes that are registered by RegisterClass function.
  //
  WNDCLASSA WindowClass = {};

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
	  win32_sound_output SoundOutput = {};

	  SoundOutput.SamplesPerSecond = 48000;
	  //	  SoundOutput.RunningSampleIndex = 0;
	  SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
	  SoundOutput.BytesPerSample = sizeof(int16)*2;
	  SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
	  SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
	  Win32InitDSound(WindowHandle, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
	  Win32ClearBuffer(&SoundOutput);
	  GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
	  
	  GlobalRunning = true;

	  int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize,
						 MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	  
	  LARGE_INTEGER LastCounter;
	  QueryPerformanceCounter(&LastCounter);
	  uint64 LastCycleCount = __rdtsc();
	  
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
	      
	      // The input API need to be polled...
	      for(DWORD ControllerIndex = 8; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
		{
		  XINPUT_STATE ControllerState;
		  if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
		    {
		      // This controller is plugged in
		      XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

		      bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
		      bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
		      bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
		      bool Rigth = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
		      bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
		      bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
		      bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
		      bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
		      bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
		      bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
		      bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
		      bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

		      int16 StickX = Pad -> sThumbLX;
		      int16 StickY = Pad -> sThumbLY;

		      // We divide the Offsets by a certain number to avoid the deadzone of the controller.
		      // The controlled center is a range of values, not only 0.
		      XOffset += StickX / 4096;
		      YOffset += StickY / 4096;

		      SoundOutput.ToneHz = 512 * (int)(256.0f*((real32)StickY / 30000.0f));
		      SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
		    }
		  else
		    {
		      // The controller is not available
		    }
		}

	      // We turn on the vibration.
	      XINPUT_VIBRATION Vibration;
	      Vibration.wLeftMotorSpeed = 68888;
	      Vibration.wRightMotorSpeed = 68888;
	      XInputSetState(0, &Vibration);

	      DWORD ByteToLock;
	      DWORD TargetCursor;
	      DWORD BytesToWrite;
	      DWORD PlayCursor;
	      DWORD WriteCursor;
	      bool32 SoundIsValid = false;
	      if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
		{
		  ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize);

		  TargetCursor = ((PlayCursor - (SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);

		  if (ByteToLock > TargetCursor)
		    {
		      BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
		      BytesToWrite += TargetCursor;
		    }
		  else
		    {
		      BytesToWrite = TargetCursor - ByteToLock;
		    }
		  
		  SoundIsValid = true;
		}
	      
	      game_sound_output_buffer SoundBuffer = {};
	      SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
	      SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
	      SoundBuffer.Samples = Samples;
	      
	      game_offscreen_buffer Buffer = {};
	      Buffer.Memory = GlobalBackbuffer.Memory;
	      Buffer.Width = GlobalBackbuffer.Width;
	      Buffer.Height = GlobalBackbuffer.Height;
	      Buffer.Pitch = GlobalBackbuffer.Pitch;
	      
	      GameUpdateAndRender(&Buffer, XOffset, YOffset, &SoundBuffer, SoundOutput.ToneHz);
	      //	      RenderWeirdGradient(&GlobalBackbuffer, XOffset, YOffset);

	      if(SoundIsValid)
		{
		  Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
		}

	      // We get a handle to the window that allows us to issue draw calls.
	      HDC DeviceContext = GetDC(WindowHandle);

	      // We get the size of the writable section of the window.
	      win32_window_dimension Dimension = Win32GetWindowDimension(WindowHandle);

	      // We display the buffer in the window.
	      Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
					 Dimension.Width, Dimension.Height);

	      uint64 EndCycleCount = __rdtsc();
	      
	      // We release the handle.
	      ReleaseDC(WindowHandle, DeviceContext);
	       
	      ++XOffset;

	      LARGE_INTEGER EndCounter;
	      QueryPerformanceCounter(&EndCounter);

	      uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
	      int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
	      int32 MSPerFrame = (int32)((1000*CounterElapsed) / PerfCountFrequency);
	      int32 FPS = PerfCountFrequency / CounterElapsed;
	      int32 MCPF = CyclesElapsed / (1000 * 1000);

 #if 0
	      char Buffer[256];
	      wsprintf(Buffer, "%dms/f - %df/s - %dMc/f\n", MSPerFrame, FPS, MCPF);
	      OutputDebugStringA(Buffer);
#endif
	      
	      LastCounter = EndCounter;
	      LastCycleCount = EndCycleCount;
	    }
	}
    }
  else
    {
    }
  
  return(0);
}



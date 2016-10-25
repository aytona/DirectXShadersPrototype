#include "systemclass.h"

SystemClass::SystemClass()
{
	// Initialized to null because if these objects fail then the
	// Shutdown function will clean it up. If not null then it
	// assumes they were valid created objects. Also good practice.
	m_Input = 0;
	m_Graphics = 0;
}

SystemClass::SystemClass(const SystemClass& other)
{

}

SystemClass::~SystemClass()
{
	// Left empty because the author didn't trust deconstructors
	// to do object clean up. Instead has it in the Shutdown function
}

bool SystemClass::Initialize()
{
	// Basically initializes the window, input, and graphics objects
	int screenWidth, screenHeight;
	bool result;

	screenWidth = 0;
	screenHeight = 0;

    // Init windows api
	InitializeWindows(screenWidth, screenHeight);

	m_Input = new InputClass;
	if (!m_Input)
		return false;
	m_Input->Initialize();

    // Handles rendering all the graphics
	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
		return false;
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result)
		return false;

	return true;
}

void SystemClass::Shutdown()
{
	// Releases everything asociated with graphics and input object
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}
	if (m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}
	ShutdownWindows();
	return;
}

void SystemClass::Run()
{
	// Run loop until app quit
	MSG msg;
	bool done, result;

	ZeroMemory(&msg, sizeof(MSG));
	done = false;
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			done = true;
		else
		{
			result = Frame();
			if (!result)
				done = true;
		}
	}
	return;
}

bool SystemClass::Frame()
{
	// Where all he processing of the application is done
	bool result;
	if (m_Input->IsKeyDown(VK_ESCAPE))
		return false;
	result = m_Graphics->Frame();
	if (!result)
		return false;
	return true;
}

LRESULT CALLBACK SystemClass::MessageHandler
(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_KEYDOWN:
			m_Input->KeyDown((unsigned int)wparam);
			return 0;
		case WM_KEYUP:
			m_Input->KeyUp((unsigned int)wparam);
			return 0;
		default:
			return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	// Renders the window
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	ApplicationHandle = this;
	m_hinstance = GetModuleHandle(NULL);    // Get instance of this application
	m_applicationName = L"Engine";          // Name of application
	
    // Setup windows class with default settings
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (FULL_SCREEN)
	{
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize			= sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight	= (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel	= 32;
		dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		posX = posY = 0;	// Top-left corner
	}
	else
	{
		// 4 : 3 Ratio
		screenWidth = 1280;
		screenHeight = 960;

		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Creates window with screen settings and get handle to it
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName,
		m_applicationName, WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	WS_POPUP, posX, posY, screenWidth, screenHeight, NULL, NULL,
		m_hinstance, NULL);

    // Bring window up on screen and set as main focus
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

    // Mouse cursor
	ShowCursor(true);

	return;
}

void SystemClass::ShutdownWindows()
{
	// Returns screen settings back to normal and releases
	// window and handles associated wih it.
	ShowCursor(true);
	if (FULL_SCREEN)
		ChangeDisplaySettings(NULL, 0);
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;
	ApplicationHandle = NULL;
	return;	
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// NOTE: These cases were separated on the site,
		// but since they have the same exact thing, I grouped them
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return ApplicationHandle->MessageHandler(hwnd, umessage,
			wparam, lparam);
	}
}
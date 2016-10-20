#pragma once
#define WIN32_LEAN_AND_MEAN    // Speeds up build process
#include <Windows.h>    // Create and destroy windows
#include "inputclass.h"
#include "graphicsclass.h"

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	// Handles windows system messages that will get sent to the
	// application while it is running
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	// Pointers that will handle Input and Graphics respectively
	InputClass* m_Input;
	GraphicsClass* m_Graphics;
};

// To re-direct the windows system messaging into MessageHandler
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;

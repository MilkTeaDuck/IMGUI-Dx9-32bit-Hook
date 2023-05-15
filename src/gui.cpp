#include "gui.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"
#include "../ext/imgui/imgui_impl_dx9.h"

#include <stdexcept>

//GUI Winproc Handler

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);

// Window Process
LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);


bool gui::SetupWindowClass(const char* windowClassName) noexcept
{
	// population of window class
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = NULL;

	// register
	if (!RegisterClassEx(&windowClass))
		return false;

	return true;
}
void gui::DestroyWindowClass() noexcept
{
	UnregisterClass(
		windowClass.lpszClassName,
		windowClass.hInstance
	);
}

bool gui::SetupWindow(const char* windowName) noexcept
{
	// create temp window
	window = CreateWindow(
		windowClass.lpszClassName,
		windowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		100,
		100,
		0,
		0,
		windowClass.hInstance,
		0
	);

	// checking to make sure windowclass was init
	// if !window return false cause failed if not return true
	if (!window)
		return false;

	return true;
}

void gui::DestroyWindow() noexcept
{
	if (window)
		DestroyWindow(window);
}

bool gui::SetupDirectX() noexcept
{
	const auto handle = GetModuleHandle("d3d9.dll");

	// no handle = problemo joe so check and return false if we dont have 
	if (!handle)
		return false;

	// Sig DX Create Function
	// getting the d3d9 handle to the handle in cs
	using CreateFn = LPDIRECT3D9(__stdcall*)(UINT);

	// Returns the address so we can call it ourselves
	const auto create = reinterpret_cast<CreateFn>(GetProcAddress(
		handle,
		"Direct3DCreate9"
	));

	// check if success
	if (!create)
		return false;

	//creeate d3d9 

	d3d9 = create(D3D_SDK_VERSION);

	// check if success 

	if (!d3d9)
		return false;

	// creating direct x present params ///
	// create present called params
	// init it with nothing 
	// then we fill it out
	
	D3DPRESENT_PARAMETERS params = { };
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = D3DFMT_UNKNOWN;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = NULL;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = window;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = NULL;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	// d3d9 created now creating the device

	if (d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_NULLREF,
		window, //pass in window
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, // set flags
		&params, //address of our params 
		&device //pointer of the address where we want to store device so in devices -- makes little sense and much sense same time 
	) < 0) return false; //less than 0 = fail

	return true; // else greater = true and success 
}

void gui::DestroyDirectX() noexcept
{
	if (device)
	{
		device->Release();
		d3d9 = NULL;
	}
}

// ERROR TROUBLE SHOOTING FAILS N RUN TIME ERROR SCREENS XOXOXO RETURN THIS
// Yeah Yenrab see how i can return as many things as i want in a function and it doesnt break and i know where it fails 
// inb4 massive break
void gui::Setup()
{
	if (!SetupWindowClass("SetupWindowClass"))
		throw std::runtime_error("Failed to create window class.");

	if (!SetupWindow("SetupWindow"))
		throw std::runtime_error("Failed to create window oopsie.");

	if (!SetupDirectX())
		throw std::runtime_error("Failed to create device.");
	
	//Dont Need Classes Anymore so can destroy 
	DestroyWindow();
	DestroyWindowClass();
}

void gui::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept
{
	//create object n populate
	auto params = D3DDEVICE_CREATION_PARAMETERS{ };
	device->GetCreationParameters(&params);

	// get game window 
	window = params.hFocusWindow;

	//store games original window process 
	// casting all inside SWLP -> WNDPROC
	// Setting the games window process to our window process 
	originalWindowProcess = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
	);

	// Creating ImGui Context 
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Call init and pass games window
	ImGui_ImplWin32_Init(window);
	// Call Dx9 init and pass games device 
	ImGui_ImplDX9_Init(device);

	setup = true;

}
void gui::Destroy() noexcept
{
	//shutting down implementations we created
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// changed game window process to ours
	// so got to change it back / restore winproc
	SetWindowLongPtr(
		window,
		GWLP_WNDPROC,
		reinterpret_cast<LONG_PTR>(originalWindowProcess)
	);

	DestroyDirectX();

}

void gui::Render() noexcept
{
	// Rendering New Frames 
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Rendering Menu
	ImGui::Begin("PancakeMix", &open);
	ImGui::End();


	//Ending Render 
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

}


LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
)
{
	// Menu Toggle 
	///// 1 is so it will only take 1 input and if u hold it it wont spam menu open closed repeatedly 
	///// Then toggle just flips gui bool t to f and f to t 
	if (GetAsyncKeyState(VK_INSERT) & 1)
		gui::open = !gui::open;

	// pass messages to imgui 
	// only pass messages to imgui if window is open else ignore 
	if (gui::open && ImGui_ImplWin32_WndProcHandler(
		window,
		message,
		wideParam,
		longParam
	)) return 1L;

	// Returns Original Windows Messages so game still gets inputs when menu is open
	return CallWindowProc(
		gui::originalWindowProcess,
		window,
		message,
		wideParam,
		longParam
	);
}
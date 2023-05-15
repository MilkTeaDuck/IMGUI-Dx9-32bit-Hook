#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <thread>
#include <cstdint>

#include "hooks.h"



// setup function
void Setup(const HMODULE instance)
{
	//inits here 
	try 
	{
		//Setup WindowClass, Window, DirectX
		gui::Setup();
		//Init Minhook, EndScene, ResetOriginal
		hooks::Setup();
	}
	//if goes wrong gets caught here and throws a message
	catch (const std::exception& error)
	{
		MessageBeep(MB_ICONERROR);
		MessageBox(
			0,
			error.what(),
			"Pancake Mix Error",
			MB_OK | MB_ICONEXCLAMATION
		);

		goto UNLOAD;
	}

	//Panic UNLOAD Pancake Mix
	while (!GetAsyncKeyState(VK_END))
		std::this_thread::sleep_for(std::chrono::milliseconds(200));


UNLOAD:
	hooks::Destroy();
	gui::Destroy();

	FreeLibraryAndExitThread(instance, 0);

}







/////////////// ENTRY //////////////////

// Entry Point For DLL 

BOOL WINAPI DllMain(
	const HMODULE instance,
	const std::uintptr_t reason,
	const void* reserved
)

// THREAD_DLL_ATTACH 

{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(instance);

		//creating hack thread
		const auto thread = CreateThread(
			nullptr,
			0,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(Setup),
			instance,
			0,
			nullptr
		);

		if (thread)
			CloseHandle(thread);

	}


	return TRUE;
}
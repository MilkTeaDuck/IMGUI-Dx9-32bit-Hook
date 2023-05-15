#pragma once
#include "gui.h"

namespace hooks
{
	void Setup();
	void Destroy() noexcept;

	constexpr void* VirtualFunction(void* thisptr, size_t index) noexcept
	{
		return (*static_cast<void***>(thisptr))[index];
	}

	using EndSceneFn = long(__thiscall*)(void*, IDirect3DDevice9*) noexcept;
	// hooking original end scene so that we can return the game back to how it was  
	inline EndSceneFn EndSceneOriginal = nullptr;
	//defining hook function 
	long __stdcall EndScene(IDirect3DDevice9* device) noexcept;

	//hooking reset to reset imgui objects / information
	using ResetFn = HRESULT(__thiscall*)(void*, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*) noexcept;
	inline ResetFn ResetOriginal = nullptr;
	HRESULT __stdcall Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept;

}

#pragma once

#include <Windows.h>
#include <vector>
#include "MemoryFunctions.h"

namespace Tambouille
{
	int* isInGameMode;
	uintptr_t characterStateBasePtr;
	std::vector<unsigned int> characterStateOffsets = { 0x58, 0x118, 0x1EC };
	uintptr_t characterStatePtr;

	int doOnce = 0;
	int getCharacterStateRtn = 0x672A8D;
	int* characterState;
	void __declspec(naked) recoverCharacterStatePointer()
	{
		if (doOnce == 0)
		{
			characterStatePtr = characterStateBasePtr;
			for (unsigned int i = 0; i <= 2; ++i)
			{
				characterStatePtr = *(uintptr_t*)characterStatePtr;
				characterStatePtr += characterStateOffsets[i];
			}
			characterState = (int*)characterStatePtr;

			doOnce = 1;
		}

		__asm
		{
			jmp getCharacterStateRtn
		}
	}

	const int conversionCodeDeClavier[264] = { VK_ESCAPE /* Escape */,
										 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30 /* Numéros */,
										 VK_OEM_MINUS, VK_OEM_PLUS, VK_BACK, VK_TAB /* Minus, Equals, Backspace, Tab */,
										 0x41, 0x5A, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4F, 0x50 /* QWERTYUIOP */,
										 0xFF, 0xFF, VK_RETURN, VK_LCONTROL /* Left Bracket, Right Bracket, Enter, Left Control*/,
										 0x51, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C /* ASDFGHJKL */,
										 0xFF, 0xFF, 0xFF, VK_LSHIFT, 0xFF /* Semicolon, Apostrophe, (~) Console, Left Shift, Back Slash */,
										 0x57, 0x58, 0x43, 0x56, 0x42, 0x4E, 0x4D /* ZXCVBNM */,
										 VK_OEM_COMMA, VK_OEM_PERIOD, 0xFF, VK_RSHIFT, 0x90, VK_MENU, VK_SPACE, VK_CAPITAL /* Comma, Period, Forward Slash, Right Shift, Num*, Left Alt, Spacebar, Caps Lock */,
										 VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10 /* F1-10 */,
										 VK_NUMLOCK, VK_SCROLL /* Num lock, Scroll lock */,
										 VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9 /* Numpad 7 à 9 */,
										 0xFF, /* Numpad- */
										 VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6 /* Numpad 4 à 6 */,
										 0xFF /* Numpad+ */,
										 VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD0 /* Numpad 1 à 3 puis zero */,
										 0xFF /* Numpad . */,
										 VK_F11, VK_F12 /* F11 et 12 */,
										 0xFF /* Numpad Enter */,
										 VK_RCONTROL, 0xFF, 0xFF, VK_HOME /* Right Control, Numpad /, Right Alt, Home */,
										 VK_UP, VK_PRIOR, VK_LEFT, VK_RIGHT, VK_END, VK_DOWN, VK_NEXT, VK_INSERT, VK_DELETE, VK_LBUTTON, VK_RBUTTON, VK_MBUTTON
	};

	bool isControlPressed(uintptr_t controlKeyPtr)
	{
		unsigned char* controlKey = (unsigned char*)controlKeyPtr;
		return (GetAsyncKeyState(conversionCodeDeClavier[(unsigned int)*controlKey - 1]) & 1);
	}

	bool GetPlayerLanded()
	{
		return *characterState == 0;
	}

	void PlayerJump()
	{
		*characterState = 1;
	}

	bool GetInGameMode()
	{
		return *isInGameMode == 1;
	}

	int* getCharacterStatePtr()
	{
		return characterState;
	}

	void Init()
	{
		isInGameMode = (int*)((uintptr_t)GetModuleHandle(NULL) + 0x73341C);

		//Récupére le pointeur vers le Character State
		characterStateBasePtr = (uintptr_t)GetModuleHandle(NULL) + 0x7333B4;
		MemoryFunctions::WriteJump(0x672A86, 0x672A8D, (int)recoverCharacterStatePointer);
	}
}
#include "Windows.h"

namespace MemoryFunctions
{
	void SafeWrite8(int address, int data) {

		DWORD oldProtect;

		VirtualProtect((void*)address, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
		*((BYTE*)address) = data;
		VirtualProtect((void*)address, 4, oldProtect, &oldProtect);

	}

	bool WriteMemory(int address, char* data, int length)
	{
		DWORD oldProtect = 0;

		if (VirtualProtect((void*)address, length, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			memcpy((void*)address, data, length);
			if (VirtualProtect((void*)address, length, oldProtect, &oldProtect))
				return true;
		}

		return false;
	}


	void Nope(int address, int length)
	{
		char* nopArray = new char[length];
		memset(nopArray, 0x90, length);

		WriteMemory(address, nopArray, length);
		delete[] nopArray;
	}

	bool WriteJump(int addressFrom1, int addressFrom2, int addressTo)
	{
		DWORD oldProtect = 0;

		int len1 = addressFrom2 - addressFrom1;
		if (VirtualProtect((void*)addressFrom1, len1, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			*((unsigned char*)addressFrom1) = (unsigned char)0xE9;
			*((int*)(addressFrom1 + 1)) = (int)addressTo - addressFrom1 - 5;
			for (int i = 5; i < len1; i++)
				*((unsigned char*)(i + addressFrom1)) = (unsigned char)0x90;
			if (VirtualProtect((void*)addressFrom1, len1, oldProtect, &oldProtect))
				return true;
		}

		return false;
	}

}
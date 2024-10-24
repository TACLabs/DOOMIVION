#pragma once
namespace MemoryFunctions
{
	void Nope(int address, int length);
	bool WriteJump(int addressFrom1, int addressFrom2, int addressTo);
	void SafeWrite8(int address, int data);
	void SafeWrite32(UInt32 addr, UInt32 data);
}
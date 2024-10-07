#include "MemoryFunctions.h"
#include <stdint.h>
#include "Windows.h"

namespace GameplayTweaks
{
	double __cdecl returnFixedRunSpeed()
	{
		return 500;
	}


	int Calc_RunSpeed = 0x547D00;
	int hookRunSpeedRtn = 0x5E3830;
	int* comparisonPointer;
	void __declspec(naked) hookFixedRunSpeed()
	{
		_asm {
			fstp dword ptr[esp]
			mov comparisonPointer, esp
		}
		//IF C'EST LE PLAYER
		if ((unsigned)comparisonPointer == 1701536)
		{
			_asm {
				call returnFixedRunSpeed
				add esp, 0x18
				fstp dword ptr[esp + 0x8]
				jmp hookRunSpeedRtn
			}
		}
		//C'EST PAS LE PLAYER
		else
		{
			_asm {
				call Calc_RunSpeed
				add esp, 0x18
				fstp dword ptr[esp + 0x8]
				jmp hookRunSpeedRtn
			}
		}
	}

	float fixedJumpHeight = 200;
	int hookJumpHeightRtn = 0x65AB5A;
	void __declspec(naked)hookFixedJumpHeight()
	{
		_asm {
			fld dword ptr [fixedJumpHeight]
			push ecx
			fld dword ptr [fixedJumpHeight]
			jmp hookJumpHeightRtn
		}
	}

	void Init()
	{
		//Vitesse fixe pour le joueur
		MemoryFunctions::WriteJump(0x5E3821, 0x5E3830, (int)hookFixedRunSpeed);

		//Hauteur fixe pour le saut
		MemoryFunctions::WriteJump(0x65AB4D, 0x65AB54, (int)hookFixedJumpHeight);
	}
}
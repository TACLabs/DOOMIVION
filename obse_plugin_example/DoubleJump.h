#pragma once

#include "MemoryFunctions.h"
#include "SoundsManager.h"
#include "Tambouille.h"
#include "Windows.h"

namespace DoubleJump
{

	int cancelFallDamage = 0x5EFF04;
	int fallDamageRtn = 0x5EFE49;
	int* comparisonPointer;
	void __declspec(naked) NoFallDamageForThePlayer()
	{
		_asm {
			mov eax, [esi]
			mov edx, [eax + 0x380]
			mov comparisonPointer, eax
			mov ecx, esi
			call edx
			test eax, eax
			jne annulationFallDamage
			test bl, bl
			jne annulationFallDamage
		}
		//J'ai trouvé cette magouille pour discriminer le player des NPCs, alors voilà
		if ((unsigned)comparisonPointer == 10959372)
		{
			_asm {
				jmp cancelFallDamage
			}
		}
		else
		{

		}
		_asm {
			jmp fallDamageRtn
			annulationFallDamage :
			jmp cancelFallDamage
		}
	}


	int doubleJumpRtn = 0x89071A;
	int alternanceSon = 0;
	int nbJumps = 0;

	void __declspec(naked) doubleJump()
	{
		_asm {
			//Original code
			fstp dword ptr[ecx + 0x31C]
		}

		if (nbJumps == 1)
		{
			if (alternanceSon == 0)
			{
				SoundsManager::Play("DoubleJumpOne");
				Tambouille::PlayerJump();
				alternanceSon = 1;
			}
			else
			{
				SoundsManager::Play("DoubleJumpTwo");
				Tambouille::PlayerJump();
				alternanceSon = 0;
			}
			nbJumps += 1;
		}
		else
		{
			nbJumps += 1;
		}

		_asm {
			jmp doubleJumpRtn
		}
	}

	int resetJumpRtn = 0x89073B;
	void __declspec(naked) resetDoubleJump()
	{
		_asm
		{
			//mov[ecx + 0x1ec], eax
			//mov[ecx + 0x2a0], 0xB
			cmp eax, 0
			jne resetJumpRetourContinue
			mov comparisonPointer, ecx
			add comparisonPointer, 0x1ec
		}
		if (comparisonPointer == Tambouille::getCharacterStatePtr())
		{
			nbJumps = 0;
		}
		_asm {
		resetJumpRetourContinue:
			jmp resetJumpRtn
		}
	}


	void Init()
	{
		//Fix la baisse de stamina lors du premier saut, le Doom Slayer ne se fatigue jamais à sauter
		MemoryFunctions::Nope(0x672A9B, 5);

		//Retire les dégats de chute pour le joueur
		MemoryFunctions::WriteJump(0x5EFE2D, 0x5EFE49, (int)NoFallDamageForThePlayer);

		//Reinitialise l'état du double jump lors de l'atterrisage
		MemoryFunctions::WriteJump(0x890731, 0x89073B, (int)resetDoubleJump);

		//Permet LE Double Jump
		MemoryFunctions::WriteJump(0x890714, 0x89071A, (int)doubleJump);
	}

}
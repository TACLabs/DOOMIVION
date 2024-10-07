#pragma once

#include "MemoryFunctions.h"
#include "SoundsManager.h"
#include "Tambouille.h"
#include "Windows.h"
#include <vector>

namespace Dash
{
	//////////////////////Mon Dash doit :
	// 
	// [PAS CHECK]
	//Avoir un effet cin�tique autour de l'�cran (DirectX Shader??)
	// 
	// [PAS CHECK]
	//Ne pas subir les effets de friction avec le terrain (voir comment sont faites les attaques sp�ciales � mains nues, et les galipettes de la comp�tence Acrobatie)
	// 
	// [PAS CHECK]
	//Par contre quand le joueur est en l'air, on ajoute juste de la v�locit� au joueur dans la direction donn�e
	// 
	// [PAS CHECK]
	//Quand y'a PAS de direction, c'est de la v�locit� vers l'avant par d�faut
	// 
	// [CHECK]
	//Dans DOOM Eternal, c'est la touche Maj par d�faut pour le Dash, dans Oblivion, cette touche est r�serv�e � la touche "Run" qui ne sert � RIEN 
	//Il faut donc d�sactiver l'effet de cette touche dans le jeu de base 
	//
	// [CHECK]
	// en l'interceptant et en executant mon Dash � la place
	// 
	//J'ai tout dis

	int playerWantsToDash = 0;
	int firstDashReloadTime = 0;
	int dashAvailable = 2;

	HANDLE dashThreadHandle = NULL;

	DWORD WINAPI dashThread(HMODULE hModule)
	{
		if (playerWantsToDash)
		{
			playerWantsToDash = 0;

			if (dashAvailable > 0)
			{
				Actor* actor = OBLIVION_CAST(*g_thePlayer, TESObjectREFR, Actor);
				MiddleHighProcess* mhProc = OBLIVION_CAST(actor->process, BaseProcess, MiddleHighProcess);
				ahkCharacterProxy* proxy = (ahkCharacterProxy*)(mhProc->charProxy->hkObj);

				proxy->velocity.x = proxy->velocity.x * 6.9;
				proxy->velocity.y = proxy->velocity.y * 6.9;
				proxy->velocity.z = 50;

				dashAvailable--;

				if (dashAvailable == 1)
				{
					SoundsManager::Play("DashOne");

					while (dashAvailable == 1 && (!Tambouille::GetPlayerLanded() || !(Tambouille::GetInGameMode())))
					{
						Sleep(0);
					}
					//Si le joueur n'a pas redash entre temps
					if (dashAvailable == 1)
					{
						dashAvailable = 2;
						SoundsManager::Play("DashReloadOne");
					}
				}
				else if (dashAvailable == 0)
				{
					SoundsManager::Play("DashTwo");

					while (!(Tambouille::GetPlayerLanded()) || !(Tambouille::GetInGameMode()))
					{
						Sleep(0);
					}
					Sleep(650);
					SoundsManager::Play("DashReloadTwo");
					dashAvailable = 2;
					firstDashReloadTime = 0;
				}
			}
		}

		return 0;
	}

	void requestDash()
	{
		if (dashAvailable > 0 && !playerWantsToDash)
		{
			playerWantsToDash = 1;

			CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)dashThread, NULL, 0, nullptr));
		}
	}

	uintptr_t dashControlKeyBasePtr, dashControlKeyPtr;
	std::vector<unsigned int> dashControlKeyOffsets;

	int sub_5E65B0 = 0x5E65B0;
	int QueryControlState = 0x403520;
	int DashRtn = 0x67222A;

	int* ebxToRestore;
	int doOnce = 0;
	__declspec(naked) void DashHook()
	{
		_asm
		{
			//si je fais pas �a je crois que �a crash
			mov ebxToRestore, ebx
			//MEIN CODE
		}
		if (doOnce == 0)
		{
			dashControlKeyPtr = dashControlKeyBasePtr;
			for (unsigned int i = 0; i <= 1; ++i)
			{
				dashControlKeyPtr = *(uintptr_t*)dashControlKeyPtr;
				dashControlKeyPtr += dashControlKeyOffsets[i];
			}
			doOnce = 1;
		}
		if (Tambouille::isControlPressed(dashControlKeyPtr))
		{
			requestDash();
		}
		_asm
		{
			//le code que j'ai grand-remplac�, restitu�
			mov ecx, ebxToRestore
			call sub_5E65B0
			jmp DashRtn
		}
	}


	void Init()
	{
		//La magouille suivante emp�che que la touche Run fasse marcher le joueur, donc diminuer sa vitesse, donc vienne alt�rer mon Dash
		MemoryFunctions::SafeWrite8(0x6721C3, 0x10);
		MemoryFunctions::SafeWrite8(0x6721D7, 0x10);

		uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);
		dashControlKeyBasePtr = moduleBase + 0x733398;
		dashControlKeyOffsets = { 0x20, 0x1b7e + 0xa };
		MemoryFunctions::WriteJump(0x672223, 0x67222A, (int)DashHook);

	}
}
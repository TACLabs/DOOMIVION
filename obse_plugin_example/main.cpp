#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"

#include "obse/GameObjects.h"

#include "obse/Utilities.h"

#include "sound.h"


#include <Mmsystem.h>
#include "Windows.h"
#include <TlHelp32.h>

std::string doubleJumpSoundOneFilePath = GetOblivionDirectory() + "Data\\Sound\\fx\\DOOOOM\\DoubleJump\\doublejump1.wav";
BYTE* doubleJumpSoundOne;
DWORD doubleJumpSoundOneFileSize;

std::string doubleJumpSoundTwoFilePath = GetOblivionDirectory() + "Data\\Sound\\fx\\DOOOOM\\DoubleJump\\doublejump2.wav";;
BYTE* doubleJumpSoundTwo;
DWORD doubleJumpSoundTwoFileSize;

std::string dashSoundOneFilePath = GetOblivionDirectory() + "Data\\Sound\\fx\\DOOOOM\\Dash\\dash1.wav";
BYTE* dashSoundOne;
DWORD dashSoundOneFileSize;

std::string dashSoundTwoFilePath = GetOblivionDirectory() + "Data\\Sound\\fx\\DOOOOM\\Dash\\dash2.wav";
BYTE* dashSoundTwo;
DWORD dashSoundTwoFileSize;

std::string dashSoundOneReloadFilePath = GetOblivionDirectory() + "Data\\Sound\\fx\\DOOOOM\\Dash\\recharge.wav";
BYTE* dashSoundOneReload;
DWORD dashSoundOneReloadFileSize;

std::string dashSoundTwoReloadFilePath = GetOblivionDirectory() + "Data\\Sound\\fx\\DOOOOM\\Dash\\recharge_double.wav";
BYTE* dashSoundTwoReload;
DWORD dashSoundTwoReloadFileSize;


uintptr_t jumpStateBasePtr;
std::vector<unsigned int> jumpStateOffsets = { 0x58, 0x118, 0x1EC };
int jumpRetour = 0x672A8D;

uintptr_t jumpStatePtr;
int* jumpState;

int nbDashes = 0;


int nbSauts = 0;
int alternanceSon = 0;

int doOnce = 0;
int doneSuperJump = 0;

uintptr_t dashControlKeyBasePtr, dashControlKeyPtr;
std::vector<unsigned int> dashControlKeyOffsets;

//Trucs pour la jump height
uintptr_t fJumpHeightMinAddr, fJumpHeightMaxAddr;
float* fJumpHeightMin;
float* fJumpHeightMax;
float jumpHeight = 169.00;




int doubleJumpSoundRetour = 0x89071A;
void __declspec(naked) doubleJump()
{
	_asm {
		//Original code
		fstp dword ptr[ecx + 0x31C]
	}

	if (nbSauts == 1)
	{
		if (alternanceSon == 0)
		{
			//LeSoundControl->Play(doubleJumpTESSoundOne);
			PlaySound((LPCSTR)doubleJumpSoundOne, NULL, SND_MEMORY | SND_ASYNC);
			*jumpState = 1;
			alternanceSon = 1;
		}
		else
		{
			//LeSoundControl->Play(doubleJumpTESSoundTwo);
			PlaySound((LPCSTR)doubleJumpSoundTwo, NULL, SND_MEMORY | SND_ASYNC);
			*jumpState = 1;
			alternanceSon = 0;
		}
		nbSauts += 1;
	}
	else
	{
		nbSauts += 1;
	}

	_asm {
		jmp doubleJumpSoundRetour
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

int needToReloadLeDash = 0;
int* isInGameMode;
DWORD WINAPI dashThread(HMODULE hModule)
{
	/*
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	*/

	unsigned char* dashControlKey = (unsigned char*)dashControlKeyPtr;
	
	while (true) 
	{
		if (GetAsyncKeyState(conversionCodeDeClavier[(unsigned int)*dashControlKey - 1]) & 1) 
		{
			if (*isInGameMode == 1)
			{
				if (nbDashes < 2)
				{
					Actor* actor = OBLIVION_CAST(*g_thePlayer, TESObjectREFR, Actor);
					MiddleHighProcess* mhProc = OBLIVION_CAST(actor->process, BaseProcess, MiddleHighProcess);
					ahkCharacterProxy* proxy = (ahkCharacterProxy*)(mhProc->charProxy->hkObj);
					proxy->velocity.x = proxy->velocity.x * 6.9;
					proxy->velocity.y = proxy->velocity.y * 6.9;
					proxy->velocity.z = 50;

					if (nbDashes == 0)
					{
						PlaySound((LPCSTR)dashSoundOne, NULL, SND_MEMORY | SND_ASYNC);
					}
					else
					{
						PlaySound((LPCSTR)dashSoundTwo, NULL, SND_MEMORY | SND_ASYNC);
					}
					nbDashes += 1;
				}
				
			}
		}

		if (needToReloadLeDash == 1)
		{
			while (*jumpState != 0 && *isInGameMode != 1)
			{
				Sleep(0);
			}
			Sleep(650);
			PlaySound((LPCSTR)dashSoundTwoReload, NULL, SND_MEMORY | SND_ASYNC);
			nbDashes = 0;
			needToReloadLeDash = 0;
		}

	}
}

void __declspec(naked) initLesInformations()
{
	if (doOnce == 0)
	{
		jumpStatePtr = jumpStateBasePtr;
		for (unsigned int i = 0; i <= 2; ++i)
		{
			jumpStatePtr = *(uintptr_t*)jumpStatePtr;
			jumpStatePtr += jumpStateOffsets[i];
		}
		jumpState = (int*)jumpStatePtr;

		*fJumpHeightMin = jumpHeight;
		*fJumpHeightMax = jumpHeight;

		dashControlKeyPtr = dashControlKeyBasePtr;
		for (unsigned int i = 0; i <= 1; ++i)
		{
			dashControlKeyPtr = *(uintptr_t*)dashControlKeyPtr;
			dashControlKeyPtr += dashControlKeyOffsets[i];
		}

		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)dashThread, NULL, 0, nullptr));

		doOnce = 1;
	}

	__asm
	{
		jmp jumpRetour
	}
}

int resetJumpRetour = 0x89073B;
int* comparaisonPointeur;

void __declspec(naked) resetDoubleJump()
{
	_asm
	{
		//mov[ecx + 0x1ec], eax
		//mov[ecx + 0x2a0], 0xB
		cmp eax, 0
		jne resetJumpRetourContinue
		mov comparaisonPointeur, ecx
		add comparaisonPointeur, 0x1ec
	}
	if (comparaisonPointeur == jumpState)
	{
		doneSuperJump = 0;
		nbSauts = 0;
		if (nbDashes == 1)
		{
			nbDashes = 0;
			PlaySound((LPCSTR)dashSoundOneReload, NULL, SND_MEMORY | SND_ASYNC);
		}
		else if (nbDashes == 2)
		{
			needToReloadLeDash = 1;
		}
	}
	_asm {
	resetJumpRetourContinue:
		jmp resetJumpRetour
	}
}


int annuleFallDamage = 0x5EFF04;
int jumpRetourDuFallDamage = 0x5EFE49;
int* comparaisonPointeur2;
void __declspec(naked) NoFallDamagePourLePlayer()
{
	_asm {
		mov eax, [esi]
		mov edx, [eax + 0x380]
		mov comparaisonPointeur2, eax
		mov ecx, esi
		call edx
		test eax, eax
		jne annulationFallDamage
		test bl, bl
		jne annulationFallDamage
	}
	//J'ai trouvé cette magouille pour discriminer le player des NPCs, alors voilà
	if ((unsigned)comparaisonPointeur2 == 10959372)
	{
		//printf("CEST LE PLAYER");
		_asm {
			jmp annuleFallDamage
		}
	}
	else
	{
		//printf("CEST PAS LE PLAYER");
	}
	_asm {
		jmp jumpRetourDuFallDamage
		annulationFallDamage:
		jmp annuleFallDamage
	}
}

double __cdecl retourneVitesseQueJeVeux()
{
	return 500;
}


int Calc_RunSpeed = 0x547D00;
int retourDuHook = 0x5E3830;
int* comparaisonPointeur3;
void __declspec(naked) hookRunSpeed()
{
	_asm {
		fstp dword ptr[esp]
		mov comparaisonPointeur3, esp
	}
	if ((unsigned)comparaisonPointeur3 == 1701536)
	{
		//C'EST LE JOUEUR, ET LE JOUEUR A UNE VITESSE FIXE
		_asm {
			call retourneVitesseQueJeVeux
			add esp, 0x18
			fstp dword ptr[esp + 0x8]
			jmp retourDuHook
		}
	}
	else
	{
		_asm {
			call Calc_RunSpeed
			add esp, 0x18
			fstp dword ptr[esp + 0x8]
			jmp retourDuHook
		}
	}


}

void leTrucDesSounds()
{
	delete[] doubleJumpSoundOne;
	delete[] doubleJumpSoundTwo;
	delete[] dashSoundOne;
	delete[] dashSoundTwo;
	delete[] dashSoundOneReload;
	delete[] dashSoundTwoReload;


	float volume = getEffectsVolume();

	
	ReadWavFileIntoMemory(doubleJumpSoundOneFilePath, &doubleJumpSoundOne, &doubleJumpSoundOneFileSize);
	ReadWavFileIntoMemory(doubleJumpSoundTwoFilePath, &doubleJumpSoundTwo, &doubleJumpSoundTwoFileSize);
	patchVolume(&doubleJumpSoundOne, &doubleJumpSoundOneFileSize, volume);
	patchVolume(&doubleJumpSoundTwo, &doubleJumpSoundTwoFileSize, volume);

	
	ReadWavFileIntoMemory(dashSoundOneFilePath, &dashSoundOne, &dashSoundOneFileSize);
	ReadWavFileIntoMemory(dashSoundTwoFilePath, &dashSoundTwo, &dashSoundTwoFileSize);
	patchVolume(&dashSoundOne, &dashSoundOneFileSize, volume);
	patchVolume(&dashSoundTwo, &dashSoundTwoFileSize, volume);

	ReadWavFileIntoMemory(dashSoundOneReloadFilePath, &dashSoundOneReload, &dashSoundOneReloadFileSize);
	ReadWavFileIntoMemory(dashSoundTwoReloadFilePath, &dashSoundTwoReload, &dashSoundTwoReloadFileSize);
	patchVolume(&dashSoundOneReload, &dashSoundOneReloadFileSize, volume);
	patchVolume(&dashSoundTwoReload, &dashSoundTwoReloadFileSize, volume);
}

int doOnceleInitParContre = 0;
void InitLesSounds()
{
	if (doOnceleInitParContre == 0)
	{
		leTrucDesSounds();

		doOnceleInitParContre = 1;
	}
}

int puisLeRetour2 = 0x6AA274;
__declspec(naked) void InitLesSoundsHook()
{
	_asm {
		push eax
		push esi
		call edx
		pop edi
		pop esi
		call InitLesSounds
		jmp puisLeRetour2
	}
}

void UpdateLesSounds()
{
	leTrucDesSounds();
}

int leCallReplaced = 0x5BD610;
int puisLeRetour = 0x5957E9;
__declspec(naked) void UpdateLesSoundsHook()
{
	_asm {
		call UpdateLesSounds
		call leCallReplaced
		jmp puisLeRetour
	}
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


extern "C" {

bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	//_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "DASH AND DOUBLE JUMP";
	info->version = 1;

	// version checks
	if (obse->isEditor)
	{
		return false;
	}

	// version checks pass

	return true;
}

bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	_MESSAGE("load");

	

	// set up serialization callbacks when running in the runtime
	if(!obse->isEditor)
	{
		uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);

		jumpStateBasePtr = moduleBase + 0x7333B4;

		isInGameMode = (int*)(moduleBase + 0x73341C);

		fJumpHeightMinAddr = moduleBase + 0x737498;
		fJumpHeightMin = (float*)fJumpHeightMinAddr;
		fJumpHeightMaxAddr = moduleBase + 0x7374A0;
		fJumpHeightMax = (float*)fJumpHeightMaxAddr;

		dashControlKeyBasePtr = moduleBase + 0x733398;
		dashControlKeyOffsets = { 0x20, 0x1b7e + 0xa };

		//Petit coup de nope pour retirer la baisse de stamina lors du premier saut
		Nope(0x672A9B, 5);

		//Hook qui permet de init les informations dont j'ai besoin
		WriteJump(0x672A86, 0x672A8D, (int)initLesInformations);

		//Hook pour reinitialiser l'état du double jump
		WriteJump(0x890731, 0x89073B, (int)resetDoubleJump);

		//Hook vers ma fonction Double Jump
		WriteJump(0x890714, 0x89071A, (int)doubleJump);
		
		//Permet d'actualiser le volume des sons au lancement du jeu et lorsque les paramètres sonores sont modifiés
		WriteJump(0x6AA26E, 0x6AA274, (int)InitLesSoundsHook);
		WriteJump(0x5957E4, 0x5957E9, (int)UpdateLesSoundsHook);

		//Retire les dégats de chute pour le joueur
		WriteJump(0x5EFE2D, 0x5EFE49, (int)NoFallDamagePourLePlayer);

		//Permet au joueur d'avoir une vitesse FIXE tout le long du jeu, parce que je l'ai décidé ainsi, et il va RIEN faire
		WriteJump(0x5E3821, 0x5E3830, (int)hookRunSpeed);

	}

	

	return true;
}

};

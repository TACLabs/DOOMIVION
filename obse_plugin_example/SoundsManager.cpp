#include "Windows.h"
#include <TlHelp32.h>
#include <time.h>
#include <Mmsystem.h>
#include <fstream>
#include <iostream>
#include <map>
#include "MemoryFunctions.h"
#pragma comment(lib, "winmm.lib")

namespace SoundsManager
{

	std::map<std::string, std::string> soundsQueue;

	typedef struct {
		char* name;
		BYTE* data;
		DWORD filesize;
	} SoundEntry;

	typedef struct
	{
		SoundEntry* sounds;
		size_t count;
		size_t capacity;
	} SoundsCollection;

	typedef struct
	{
		char pad_0000[184]; //0x0000
		float masterVolume; //0x00B8
		char pad_00BC[8]; //0x00BC
		float effectVolume; //0x00C4
	} volumeData;

	void ReadWavFileIntoMemory(std::string fname, BYTE** pb, DWORD* fsize) {
		std::ifstream f(fname, std::ios::binary);

		f.seekg(0, std::ios::end);
		int lim = f.tellg();
		*fsize = lim;

		*pb = new BYTE[lim];
		f.seekg(0, std::ios::beg);

		f.read((char*)*pb, lim);

		f.close();
	}

	float getEffectsVolume()
	{
		uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);
		volumeData* volumes = *(volumeData**)(moduleBase + 0x73C0F0);

		return volumes->masterVolume * volumes->effectVolume;
	}

	void patchVolume(BYTE** pb, DWORD* fsize, float volume)
	{
		BYTE* pDataOffset = (*pb + 40);
		DWORD dwNumSampleBytes = *(DWORD*)(pDataOffset + 4);
		DWORD dwNumSamples = dwNumSampleBytes / 2;

		SHORT* pSample = (SHORT*)(pDataOffset + 8);

		for (DWORD dwIndex = 0; dwIndex < dwNumSamples; dwIndex++)
		{
			SHORT shSample = *pSample;
			shSample = (SHORT)(shSample * volume);
			*pSample = shSample;
			pSample++;


			if (((BYTE*)pSample) >= (*pb + *fsize - 1))
				break;
		}
	}

	

	void addSoundIntoQueue(const std::string& soundName, const std::string& OblivionDirectory) {
		soundsQueue[soundName] = OblivionDirectory + "Data\\Sound\\fx\\DOOOOM\\" + soundName + ".wav";
	}

	SoundsCollection collection;

	void initCollection(size_t capacity)
	{
		collection.sounds = (SoundEntry*)malloc(capacity * sizeof(SoundEntry));
		collection.count = 0;
		collection.capacity = capacity;
	}

	void resetSounds()
	{
		for (size_t i = 0; i < collection.count; ++i) {
			delete[] collection.sounds[i].data;
			delete[] collection.sounds[i].name;
		}
		delete[] collection.sounds;
	}

	BYTE* getDataByName(const char* name) {
		// Parcourir tous les sons dans la collection
		for (size_t i = 0; i < collection.count; i++) {
			// Comparer le champ name avec le nom recherché
			if (strcmp(collection.sounds[i].name, name) == 0) {
				// Retourner le pointeur data si le nom correspond
				return collection.sounds[i].data;
			}
		}

		// Si le son n'a pas été trouvé, retourner NULL
		return NULL;
	}

	void __stdcall Play(const char* name)
	{
		PlaySoundA((LPCSTR)getDataByName(name), NULL, SND_MEMORY | SND_ASYNC);
	}

	void updateSounds()
	{
		if (collection.count != 0)
		{
			resetSounds();
			collection.count = 0;
		}
		
		for (auto& sound : soundsQueue)
		{
			float volume = getEffectsVolume();

			ReadWavFileIntoMemory(sound.second, &collection.sounds[collection.count].data, &collection.sounds[collection.count].filesize);
			patchVolume(&collection.sounds[collection.count].data, &collection.sounds[collection.count].filesize, volume);

			char* c_str = new char[sound.first.size() + 1];
			strcpy_s(c_str, sound.first.size() + 1, sound.first.c_str());

			collection.sounds[collection.count].name = c_str;

			collection.count++;
		}
	}

	int doOnce = 0;
	void initSounds()
	{
		if (doOnce == 0)
		{
			updateSounds();

			doOnce = 1;
		}
	}

	int replacedCall = 0x5BD610;
	int UpdateSoundsRtn = 0x5957E9;
	__declspec(naked) void UpdateSoundsHook()
	{
		_asm {
			call updateSounds
			call replacedCall
			jmp UpdateSoundsRtn
		}
	}

	int InitSoundsRtn = 0x6AA274;
	__declspec(naked) void InitSoundsHook()
	{
		_asm {
			push eax
			push esi
			call edx
			pop edi
			pop esi
			call initSounds
			jmp InitSoundsRtn
		}
	}

	

	

	void Init(const char* OblivionDirectory)
	{
		//Sons du Double Jump
		addSoundIntoQueue("DoubleJumpOne", OblivionDirectory);
		addSoundIntoQueue("DoubleJumpTwo", OblivionDirectory);

		//Sons du Dash
		addSoundIntoQueue("DashOne", OblivionDirectory);
		addSoundIntoQueue("DashTwo", OblivionDirectory);
		addSoundIntoQueue("DashReloadOne", OblivionDirectory);
		addSoundIntoQueue("DashReloadTwo", OblivionDirectory);

		initCollection(soundsQueue.size());

		//Hook pour charger les sons avec le volume adéquat
		MemoryFunctions::WriteJump(0x6AA26E, 0x6AA274, (int)InitSoundsHook);
		

		//Hook pour recharger les sons quand le volume est modifié
		MemoryFunctions::WriteJump(0x5957E4, 0x5957E9, (int)UpdateSoundsHook);
	}
}
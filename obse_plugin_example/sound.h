#pragma once
#include "Windows.h"
#include <TlHelp32.h>
#include <time.h>
#include <Mmsystem.h>
#include <fstream>
#include <iostream>

class volumeData
{
public:
	char pad_0000[184]; //0x0000
	float masterVolume; //0x00B8
	char pad_00BC[8]; //0x00BC
	float effectVolume; //0x00C4
}; //Size: 0x0440

void ReadWavFileIntoMemory(std::string fname, BYTE** pb, DWORD* fsize);
void patchVolume(BYTE**pb, DWORD* fsize, float volume);
float getEffectsVolume();
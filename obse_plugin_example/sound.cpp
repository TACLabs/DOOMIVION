#include "sound.h"

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
    //printf("volumes->masterVolume : %f \n", volumes->masterVolume);
    //printf("volumes->effectVolume : %f \n", volumes->effectVolume);
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
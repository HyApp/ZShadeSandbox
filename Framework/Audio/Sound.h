//================================================================================================================
// Sound.h
//
//================================================================================================================
// History
//
// Created in August 2013 by Dustin Watson
// Updated on 10/9/2014 by Dustin Watson
//================================================================================================================
#ifndef __SOUND_H
#define __SOUND_H
//================================================================================================================
//================================================================================================================
//Includes
#include <Windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>
#include "GameDirectory.h"
#include "wave.h"
//#include "atlstr.h"
using namespace std;
//================================================================================================================
//================================================================================================================
//Linking
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")
//================================================================================================================
//================================================================================================================
class Sound
{
public:
	Sound(GameDirectory2D* gd, HWND hwnd);
	Sound(const Sound& s);
	~Sound();

	bool LoadSound(char* filename);
	void Play();

private:
	bool m_soundLoaded;

	GameDirectory2D* m_gd;

	IDirectSound8* m_DirectSound;
	IDirectSoundBuffer* m_primaryBuffer;
};
//================================================================================================================
//================================================================================================================
#endif//__SOUND_H
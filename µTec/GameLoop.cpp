// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "GameLoop.h"
#include <iostream>
#include <fstream>
#include "TextureLoader.h"
#include <ctime>
#include "Gametime.h"

using namespace µTec;

GameLoop::GameLoop(void)
{
}


GameLoop::~GameLoop(void)
{
}



void GameLoop::run()
{
	//double previous = (double)std::clock() / CLOCKS_PER_SEC;
	//double lag=0.0;

	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;

	QueryPerformanceFrequency(&Frequency);

	while(!IInput::breakLoop)
	{
		QueryPerformanceCounter(&StartingTime);
		double current = (double)std::clock() / CLOCKS_PER_SEC;
		//lag += GameTime::deltaTime;
		GameTime::timeFromStartup = current;
		update();
		draw();
		QueryPerformanceCounter(&EndingTime);
		ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		ElapsedMicroseconds.QuadPart *= 1000000;
		ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
		GameTime::deltaTime = (ElapsedMicroseconds.QuadPart / 1000.0)/1000.0;
	}
}


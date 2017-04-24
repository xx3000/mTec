// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once

#include "Renderer.h"

#define MAX_FRAMESKIP 0.03
extern bool breakLoop;

namespace µTec
{

class GameLoop
{
public:
	GameLoop(void);
	virtual ~GameLoop(void);

	virtual void init(int* argc, char** argv)=0;

	void run();
private:
		virtual void update()=0;
		virtual void draw()=0;
};


}

// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once

namespace µTec
{
class GameTime
{
public:

	static double deltaTime;
	static double timeFromStartup;
	GameTime(void);
	~GameTime(void);
};

}
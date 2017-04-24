// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)


#include "Game.h"

int main(int argc, char* argv[])
{
	Game game;

	game.init(&argc, argv);

	game.run();

	return 0;
}


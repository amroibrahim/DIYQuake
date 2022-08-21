#include "System.h"

#include <iostream>
#include <SDL.h>

System::System()
{
}

System::~System()
{
}

void System::Init()
{
	SDLInit();
}

void System::SDLInit()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "ERROR: SDL failed to initialize! SDL_Error: " << SDL_GetError() << std::endl;
	}
}

// DIYQuake.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <string>

#include "System.h"
#include "MemoryManager.h"

int main(int argc, char* argv[])
{
   MemoryManager memorymanager;
   memorymanager.Init(16 * 1024 * 1024);
   System system;
   system.Init();

   return 0;
}

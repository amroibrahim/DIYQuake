// DIYQuake.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <string>

#include "Common.h"
#include "Host.h"
#include "System.h"

int main(int argc, char* argv[])
{
   Parameters parameters;

   System system;
   Host host;

   parameters.ParseArguments(argc, argv);

   system.Init();
   host.Init(&parameters, &system);

   uint64_t OldTime, NewTime;
   double dTime;
   OldTime = system.GetTime();

   while (system.GetIsRunning())
   {
      NewTime = system.GetTime();
      dTime = (double)(NewTime - OldTime) / system.GetPerformanceFrequency();
      host.Frame(dTime);
      OldTime = NewTime;
   }

   return 0;
}

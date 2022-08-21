#pragma once

#include <algorithm>
#include <string>
#include <vector>

const static std::string PARAM_BASE_DIR = "-basedir";

struct Parameters
{
   std::string sCurrentWorkingDirectory;
   std::vector<std::string> ArgumentsList;

   void ParseArguments(int iArgumentsCount, char* szArgument[])
   {
      for (size_t i = 0; i < iArgumentsCount; ++i)
      {
         ArgumentsList.push_back(szArgument[i]);
      }
   }

   int FindArgument(const std::string& sArgument)
   {
      int iIndex = -1;
      std::vector<std::string>::iterator itr = std::find(ArgumentsList.begin(), ArgumentsList.end(), sArgument);
      if (itr != ArgumentsList.end())
      {
         iIndex = (int)std::distance(ArgumentsList.begin(), itr);
      }
      return iIndex;
   }
};

/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  prv2dim                                  *
 *           Paraver to Dimemas trace translator (old and new format)        *
 *****************************************************************************
 *     ___        This tool is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.12.1    *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This tool is distributed in hope that it will be         *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/#$:  File
  $Rev:: 478                                      $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 (jue, 28 oct #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "ParaverHeader.hpp"
#include "Macros.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
using std::cout;
using std::endl;

/*****************************************************************************
 * Public functions
 ****************************************************************************/

ParaverHeader::ParaverHeader(char* ASCIIHeader, INT32 HeaderLength)
{
  int   matches;
  char  InternalHeader[HeaderLength+1];
  char  TraceCreationDate[30];
  char  FinalTimeStr[25];   /* Can produce an overflow! */
  char* RsrcList = (char*) calloc((size_t) HeaderLength, sizeof(char));
  char* AppList  = (char*) calloc((size_t) HeaderLength, sizeof(char));
  
  /* Internal copy of ASCII header */
  InternalHeader[HeaderLength] = '\0';
  strcpy(InternalHeader, ASCIIHeader);
  
  /* strtok(InternalHeader, ')'); */
  
  matches = sscanf(ASCIIHeader,
                   "#Paraver (%[^)]):%[^:]:%[^:]:%d:%s\n",
                   TraceCreationDate,
                   FinalTimeStr,
                   RsrcList,
                   &AppNumber,
                   AppList);
  
  if (matches != 5)
  {
    SetError(true);
    SetErrorMessage ("Unknown trace format", "header not readable");
    return;
  }
  
  /*
  cout << "Creation date: " << TraceCreationDate << endl;
  */
  
  if (!ProcessFinalTime(FinalTimeStr))
  {
    SetError(true);
    SetErrorMessage ("Paraver trace header error", "final time not readable");
    return;
  }
  
  /*
  cout << "Final Time: " << FinalTime << " (" << TimeUnits << ")" << endl;
  */
  
  ResourceDescriptionPresent = false;
  RsrcList[strlen(RsrcList)] = '\0';
  if (!ProcessResourceList(RsrcList))
  {
    SetError(true);
    return;
  }
  
  /*
  cout << "Resources: " << ResourceNumber;
  cout << " (" << ResourceDescription.size() << ")" << endl;
  */
  
  if (!ProcessApplicationList(AppList))
  {
    SetError(true);
    return;
  }
  
  /*
  cout << "Number of applications: " << AppNumber;
  cout << " (" << AppsDescription.size() << ")" << endl;
  for (int i = 0; i < AppNumber; i++)
  {
    cout << "Application description (" << i << ")" << endl;
    cout << *AppsDescription[i];
  }
  */
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

bool
ParaverHeader::ProcessFinalTime(char* ASCIIFinalTime)
{
  if (strstr(ASCIIFinalTime, "_ns"))
  {
    ASCIIFinalTime[strlen(ASCIIFinalTime)-3] = '\0';
    FinalTime = strtoull(ASCIIFinalTime, NULL, 0);
    TimeUnits = NANOSECONDS;
  }
  else
  {
    FinalTime = strtoull(ASCIIFinalTime, NULL, 0);
    TimeUnits = MICROSECONDS;
  }
  
  return true;
}

bool
ParaverHeader::ProcessResourceList (char* ASCIIRsrcList)
{
  char* ResourceInfoStr = (char*) calloc(strlen(ASCIIRsrcList)+1,
                                          sizeof(char));
  char* CurrentCPUNum;
  
  if (ResourceInfoStr == NULL)
    return false;
  
  if (sscanf(ASCIIRsrcList,
             "%d(%[^)])",
             &ResourceNumber,
             ResourceInfoStr) == 2)
  {
    /* ResourceInforStr parsing */
    ResourceDescriptionPresent = true;
    
    CurrentCPUNum = strtok(ResourceInfoStr, ",");
    while (CurrentCPUNum != NULL)
    {
      ResourceDescription.push_back((INT32) atoi(CurrentCPUNum));
      CurrentCPUNum = strtok(NULL, ",");
    }
    
    return true;
  }
  else  if (sscanf(ASCIIRsrcList, "%d", &ResourceNumber) == 1)
    return true; /* No info about resources */
  else
  {
    SetErrorMessage ("Paraver trace header error",
                     "resource list not readable");
    return false;
  }
}

bool
ParaverHeader::ProcessApplicationList (char* ASCIIAppList)
{
  ApplicationDescription_t CurrentApp;
  bool end = false;
  char *AppsList = ASCIIAppList;
  char *CurrentAppInfo = (char*) calloc(strlen(ASCIIAppList)+1, sizeof(char));
  char *OtherApps = (char*) calloc(strlen(ASCIIAppList)+1, sizeof(char));
  char *CurrentAppStr;

  
  INT32 AppCount, CurrentTaskId, TaskCount, CommunicatorCount;
  INT32 ThreadCount, Node;
  
  AppCount = 0;
  CurrentAppStr = ASCIIAppList;
  while (!end)
  {
    if (sscanf(CurrentAppStr,
               "%d(%[^)]),%d:%s",
               &TaskCount,
               CurrentAppInfo,
               &CommunicatorCount,
               OtherApps) == 4)
    {
      CurrentApp = new ApplicationDescription(AppCount+1,
                                              TaskCount,
                                              CommunicatorCount,
                                              CurrentAppInfo);
      if (CurrentApp->GetError())
      {
        SetErrorMessage("Error parsing aaplication "+AppCount,
                        CurrentApp->GetLastError().c_str());
        return false;
      }
      AppCount++;
    }
    else if (sscanf(CurrentAppStr,
                    "%d(%[^)]),%d",
                    &TaskCount,
                    CurrentAppInfo,
                    &CommunicatorCount) == 3)
    {
      CurrentApp = new ApplicationDescription(AppCount+1,
                                              TaskCount,
                                              CommunicatorCount,
                                              CurrentAppInfo);
      if (CurrentApp->GetError())
      {
        SetErrorMessage("Error parsing aaplication "+AppCount,
                        CurrentApp->GetLastError().c_str());
        return false;
      }
      end = true;
      AppCount++;
    }
    else if (sscanf(CurrentAppStr,
                    "%d(%[^)])",
                    &TaskCount,
                    CurrentAppInfo) == 2)
    {
      CurrentApp = new ApplicationDescription(AppCount+1,
                                              TaskCount,
                                              0,
                                              CurrentAppInfo);
      if (CurrentApp->GetError())
      {
        SetErrorMessage("Error parsing aaplication "+AppCount,
                        CurrentApp->GetLastError().c_str());
        return false;
      }
      end = true;
      AppCount++;
    }
    else
    {
      char ErrorMessage[128];
      
      sprintf(ErrorMessage, "Error parsing application %d", AppCount);
      SetErrorMessage(ErrorMessage);
      return false;
    }
    AppsDescription.push_back(CurrentApp);
    if (AppCount == AppNumber)
      end = true;
  }

  if (AppCount != AppNumber)
    return false;
  
  return true;
}

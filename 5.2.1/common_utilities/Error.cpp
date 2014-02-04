/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                             ClusteringSuite                               *
 *   Infrastructure and tools to apply clustering analysis to Paraver and    *
 *                              Dimemas traces                               *
 *                                                                           * 
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
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

  $Id:: Error.cpp 23 2011-05-17 09:47:12Z jgonzal#$:  Id
  $Rev:: 23                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-05-17 11:47:12 +0200 (Tue, 17 May #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "Error.hpp"
using cepba_tools::Error;

/*****************************************************************************
 * Errors
 ****************************************************************************/

void Error::SetErrorMessage(string& UserMessage, string& SysMessage)
{
  LastError = UserMessage + " (" + SysMessage + ")";
  return;
}
    
void Error::SetErrorMessage(const char* UserMessage, const char* SysMessage)
{
  string UserError = UserMessage;
  string SysError  = SysMessage;

  SetErrorMessage(UserError, SysError);
};
    
void Error::SetErrorMessage(string& UserMessage, const char* SysMessage)
{
  string SysError = SysMessage;
  SetErrorMessage(UserMessage, SysError);
}
    
void Error::SetErrorMessage(const char* UserMessage, string SysMessage)
{
  string UserError = UserMessage;
  SetErrorMessage(UserError, SysMessage);
}
    
void Error::SetErrorMessage(string UserMessage)
{
  LastError = UserMessage;
}
    
void Error::SetErrorMessage(const char* UserMessage)
{
  string UserError = UserMessage;
  LastError        = UserError;
}

/*****************************************************************************
 * Warnings
 ****************************************************************************/

void Error::SetWarningMessage(string& UserMessage, string& SysMessage)
{
  LastWarning = UserMessage + " (" + SysMessage + ")";
  return;
}
    
void Error::SetWarningMessage(const char* UserMessage, const char* SysMessage)
{
  string UserError = UserMessage;
  string SysError  = SysMessage;

  SetWarningMessage(UserError, SysError);
};
    
void Error::SetWarningMessage(string& UserMessage, const char* SysMessage)
{
  string SysError = SysMessage;
  SetWarningMessage(UserMessage, SysError);
}
    
void Error::SetWarningMessage(const char* UserMessage, string SysMessage)
{
  string UserError = UserMessage;
  SetWarningMessage(UserError, SysMessage);
}
    
void Error::SetWarningMessage(string UserMessage)
{
  LastWarning = UserMessage;
}
    
void Error::SetWarningMessage(const char* UserMessage)
{
  string UserError = UserMessage;
  LastWarning        = UserError;
}

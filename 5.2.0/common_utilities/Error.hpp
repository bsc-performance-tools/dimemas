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

  $Id:: Error.hpp 70 2012-07-03 14:11:36Z jgonzal#$:  Id
  $Rev:: 70                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2012-07-03 16:11:36 +0200 (Tue, 03 Jul #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _ERROR_H
#define _ERROR_H

#include <string>
using std::string;

namespace cepba_tools
{
  class Error
  {
    protected:
      bool   _Error;
      string LastError;

      bool   _Warning;
      string LastWarning;

    public:
      Error(void) { _Error = false; _Warning = false; };

      bool GetError(void)        { return _Error; };
      void SetError(bool _Error) { this->_Error = _Error; };

      bool GetWarning(void)          { return _Warning; };
      void SetWarning(bool _Warning) { this->_Warning = _Warning; };

      string GetLastError(void)      { return LastError;   };
      string GetErrorMessage(void)   { return LastError;   };
      string GetLastWarning(void)    { return LastWarning; };
      string GetWarningMessage(void) { return LastWarning; };

      void SetErrorMessage(string& UserMessage, string& SysMessage);
      void SetErrorMessage(const char* UserMessage, const char* SysMessage);
      void SetErrorMessage(string& UserMessage, const char* SysMessage);
      void SetErrorMessage(const char* UserMessage, string SysMessage);
      void SetErrorMessage(string UserMessage);
      void SetErrorMessage(const char* UserMessage);

      void SetWarningMessage(string& UserMessage, string& SysMessage);
      void SetWarningMessage(const char* UserMessage, const char* SysMessage);
      void SetWarningMessage(string& UserMessage, const char* SysMessage);
      void SetWarningMessage(const char* UserMessage, string SysMessage);
      void SetWarningMessage(string UserMessage);
      void SetWarningMessage(const char* UserMessage);
  };

} /* End namespace */

/* Special prototype of 'error' function needed by R*-Tree. Impemented in
 * 'main.cpp'
extern void error (char* Message, bool Exit); */

#endif /* _ERROR_H */

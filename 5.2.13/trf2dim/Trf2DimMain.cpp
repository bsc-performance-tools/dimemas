/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  trf2trf                                  *
 *       Dimemas TRF to Dimemas DIM trace translator (old to new format)     *
 *****************************************************************************
 *     ___        This tool is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.12.1   *
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

  $URL:: https://svn.bsc.es/repos/ptools/trf2dim/trunk/src/Trf2DimMain.cpp $:

  $Rev:: 483                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-29 11:54:45 +0200#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "DimemasSDDFTranslator.hpp"
#include <basic_types.h>

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

/* Main variables */
char *InputTraceName;  /* Input trace name (old format) */
char *OutputTraceName; /* Output trace name (new format) */

bool  GenerateFirstIdle;

INT64 IprobeMissesThreshold; /* Maximun iprobe misses to discard iprobe burst */

INT32  BurstCounterType;
double BurstCounterFactor;

bool VerboseMode = false;

#define HELP \
"\n"\
"Usage: %s [-v] <input_trace> <output_trace>\n\n"\
"This application translates Dimemas traces on SDDF format to the new trace\n"\
"format\n\n"\
"(c) 2010 - BSC/CEPBA Tools Team\n\n"


void PrintUsage(char* ApplicationName)
{
  cout << "Usage: " << ApplicationName << " [-v] <input_trace> <output_trace>";
  cout << endl;
}

void
ReadArgs(int argc, char *argv[])
{
  if (argc == 2 && 
      ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
  {
    fprintf(stdout, HELP, argv[0]);
    exit(EXIT_SUCCESS);
  }
  
  /* Only 3/4 parameters: [app_name] <input_trace> <output_trace> */
  if (argc == 3)
  {
    /* Read Input & Output file names */
    InputTraceName  = argv[1];
    OutputTraceName = argv[2];
  }
  else if (argc == 4)
  {
    if ((strcmp(argv[1], "-v") != 0))
    {
      PrintUsage(argv[0]);
      exit(EXIT_FAILURE);
    }
    
    VerboseMode = true;
    /* Read Input & Output file names */
    InputTraceName  = argv[2];
    OutputTraceName = argv[3];
  }
  else
  {
    PrintUsage(argv[0]);
    exit(EXIT_FAILURE);
  }
  
  return;
}

int main(int argc, char *argv[])
{
  DimemasSDDFTranslator *Translator;

  ReadArgs(argc, argv);
  
  if (VerboseMode)
    cout << "* INITIALIZING TRANSLATOR" << endl;
  
  Translator = new DimemasSDDFTranslator(InputTraceName, OutputTraceName);
  
  if (Translator->GetError())
  {
    cerr << Translator->GetLastError() << endl;
    exit(EXIT_FAILURE);
  }
  
  if (!Translator->InitTranslator())
  {
    cerr << Translator->GetLastError() << endl;
    exit(EXIT_FAILURE);
  }
  
  if (VerboseMode)
    cout << "* TRANSLATING" << endl;
  
  if (!Translator->Translate())
  {
    cerr << endl;
    cerr << "Error: " << Translator->GetLastError() << endl;
    if (!Translator->EndTranslator())
      cerr << Translator->GetLastError() << endl;
    exit(EXIT_FAILURE);
  }
  
  if (!Translator->EndTranslator())
  {
    cerr << Translator->GetLastError() << endl;
    exit(EXIT_FAILURE);
  }
}

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
  $Rev:: 1044                                     $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2012-03-27 17:58:59 +0200 (Tue, 27 Mar #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "ParaverTraceTranslator.hpp"
#include "PCFGeneration.hpp"


#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

/* Main variables */
char *PrvTraceName;          /* Paraver (input) trace name */
char *TrfTraceName;          /* Dimemas (output) trace name */

bool  GenerateFirstIdle;

double IprobeMissesThreshold; /* Maximun iprobe misses to discard iprobe burst */

INT32  BurstCounterType;
double BurstCounterFactor;

#define HELP \
"Usage:\n"\
"  %s -i <iprobe_miss_threshold> -b <hw_counter_type>,<factor>\n"\
"  <paraver_trace> <dimemas_trace>\n\n"\
"  -h                            This help\n"\
"  -n                            No generate initial idle states\n"\
"  -i <iprobe_miss_threshold>    MPI_Iprobe miss rate (per milisecond) to \n"\
"                                discard Iprobe area CPU burst\n"\
"  -b <hw_counter_type>,<factor> Hardware counter type and factor used to\n"\
"                                generate burst durations\n"


void PrintUsage(char* ApplicationName)
{
  cout << "Usage: " << ApplicationName << " -n -i <iprobe_miss_threshold> ";
  cout << "-b <hw_counter_type>,<factor> ";
  cout << "<paraver_trace> <dimemas_trace>" << endl;
}

void ReadArgs(int argc, char *argv[])
{
  int   j = 1;
  char *BurstCounterTypeStr, *BurstCounterFactorStr;

  GenerateFirstIdle = true;

  if (argc == 2 &&
      ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
  {
    fprintf(stdout, HELP, argv[0]);
    exit(EXIT_SUCCESS);
  }

  if (argc > 6 || argc < 3 )
  {
    PrintUsage(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (argv[1][0] == '-')
  {
    for (j = 1; (j < argc - 1) && (argv[j][0] == '-'); j++)
    {
      switch (argv[j][1])
      {
        case 'n': /* No generate initial idles */
          GenerateFirstIdle = false;
          break;
        case 'i': /* Iprobe misses threshold */
          j++;
          IprobeMissesThreshold = atof(argv[j]);

          if (IprobeMissesThreshold == 0.0  && errno == EINVAL)
          {
            cerr << "Error: Invalid Iprobe threshold value " << endl;
            exit (EXIT_FAILURE);
          }
          break;
        case 'b': /* Burst duration counter */
          j++;

          BurstCounterTypeStr = strtok(argv[j], " ,\n");

          if (BurstCounterTypeStr == NULL)
          {
            cerr << "Error: Invalid hardware counter type" << endl;
            exit (EXIT_FAILURE);
          }

          BurstCounterType = atoi(BurstCounterTypeStr);

          BurstCounterFactorStr = strtok(NULL, " ,\n");

          if (BurstCounterFactorStr == NULL)
          {
            cerr << "Error: Invalid factor value to hardware" << endl;
            exit (EXIT_FAILURE);
          }

          BurstCounterFactor = atof(BurstCounterFactorStr);

          break;
        default:
          cerr << "Invalid parameter " << argv[j][1] << endl;
          PrintUsage(argv[0]);
          exit(EXIT_FAILURE);
          break;
      }
    }
  }

  if (j >= argc)
  {
    PrintUsage(argv[0]);
    exit (EXIT_FAILURE);
  }

  /* Read Input & Output file names */
  PrvTraceName = argv[j];
  j = j+1;
  TrfTraceName = argv[j];

  return;
}

int main(int argc, char *argv[])
{
  ParaverTraceTranslator *Translator;

  IprobeMissesThreshold = 0.0;
  BurstCounterType      = -1;
  BurstCounterFactor    = 0.0;

  PCFGeneration *PCFGenerator;

  ReadArgs(argc, argv);

  Translator = new ParaverTraceTranslator(PrvTraceName, TrfTraceName);

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

  if (!Translator->SplitCommunications())
  {
    cerr << Translator->GetLastError() << endl;
    if (!Translator->EndTranslator())
      cerr << Translator->GetLastError() << endl;
    exit(EXIT_FAILURE);
  }

  if (!Translator->Translate(GenerateFirstIdle,
                             IprobeMissesThreshold,
                             BurstCounterType,
                             BurstCounterFactor))
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

  PCFGenerator = new PCFGeneration(PrvTraceName, TrfTraceName);

  PCFGenerator->GeneratePCF(string(""));

  exit (EXIT_SUCCESS);
}

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

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>

#include <string>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <ezOptionParser.hpp>

#include <bsc_utils.hpp>
#include "ParaverTraceTranslator.hpp"
#include "PCFGeneration.hpp"

/* Main variables */
std::string PrvTraceName;          /* Paraver (input) trace name */
std::string DimTraceName;          /* Dimemas (output) trace name */

bool  GenerateFirstIdle;
bool  GenerateMPIInitBarrier;

double IprobeMissesThreshold; /* Maximun iprobe misses to discard iprobe burst */

INT32  BurstCounterType;
double BurstCounterFactor;

void Usage(ez::ezOptionParser& opt)
{
  std::string usage;
  opt.getUsage(usage);
  std::cout << usage;
};

bool ReadArgsNew(const int argc, const char *argv[])
{
  ez::ezOptionParser opt;

  opt.overview = "Paraver to Dimemas trace translator";
  opt.syntax = "prv2dim [OPTIONS] input_paraver_trace output_dimemas_trace";
  opt.example = "prv2dim -b 42000059,4,656e-7 trace_paraver.prv trace_dimemas.dim\n";

  opt.add(
      "", // Default.
      0, // Required?
      0, // Number of args expected.
      0, // Delimiter if expecting multiple args.
      "Display usage instructions.", // Help description.
      "-h",     // Flag token.
      "-help",  // Flag token.
      "--help", // Flag token.
      "--usage" // Flag token.
    );


  opt.add(
      "0",
      false,
      0,
      0,
      "Do not generate an initial idle state",
      "-n",
      "--initial-idle");

  opt.add(
      "",
      false,
      0,
      0,
      "Do not generate a synchronization primitive on MPI_Init calls",
      "-s",
      "--init-sync");

  ez::ezOptionValidator* vD = new ez::ezOptionValidator("d");
  opt.add(
      "",
      false,
      1,
      0,
      "MPI_Iprobe miss rate (per milisecond) to discard Iprobe area CPU burst\n",
      "-i",
      "--iprobe-miss-rate",
      vD);

  opt.add(
      "",
      false,
      2,
      ',',
      "Hardware counter type and factor used to generate burst durations",
      "-b",
      "--burst-counter-factor",
      vD);

  opt.parse(argc, argv);

  if (opt.isSet("-h"))
  {
    Usage(opt);

    exit(EXIT_SUCCESS);
  }

  std::vector<std::string> badOptions;

  if(!opt.gotRequired(badOptions))
  {
    for(int i=0; i < badOptions.size(); ++i)
    {
      std::cerr << "ERROR: Missing required option " << badOptions[i] << ".\n\n";
    }
    Usage(opt);

    return false;
  }

  if(!opt.gotExpected(badOptions))
  {
    for(int i=0; i < badOptions.size(); ++i)
    {
      std::cerr << "ERROR: Got unexpected number of arguments for option " << badOptions[i] << ".\n\n";
    }

    Usage(opt);

    return false;
  }

  std::vector<std::string> badArgs;
  if(!opt.gotValid(badOptions, badArgs))
  {
    for(int i=0; i < badOptions.size(); ++i)
    {
      std::cerr << "ERROR: Got invalid argument \"" << badArgs[i] << "\" for option " << badOptions[i] << ".\n\n";
    }
    //Usage(opt);

    return false;
  }

  if (opt.isSet("-n"))
  {
    GenerateFirstIdle = false;
  }
  else
  {
    GenerateFirstIdle = true;
  }

  if (opt.isSet("-s"))
  {
    GenerateMPIInitBarrier = false;
  }
  else
  {
    GenerateMPIInitBarrier = true;
  }

  if (opt.isSet("-i"))
  {
    std::string IprobeMissesThresholdStr;
    opt.get("-i")->getString(IprobeMissesThresholdStr);

    if (bsc_tools::isDouble(IprobeMissesThresholdStr))
    {
      opt.get("-i")->getDouble(IprobeMissesThreshold);
    }
    else
    {
      std::cerr << "ERROR: Got invalid argument \"" << IprobeMissesThresholdStr << "\" for option \"-i\".\n\n";
      return false;
    }
    /* Only valid in C++11
    try
    {
      IprobeMissesThreshold = std::stod(IprobeMissesThresholdStr);
    }
    catch (std:exception &e)
    {
      std::cerr << "ERROR: Got invalid argument \"" << IprobeMissesThresholdStr << "\" for option \"-i\".\n\n";
    }
    */
  }

  if (opt.isSet("-b"))
  {
    vector<string> Values;
    opt.get("-b")->getStrings(Values);

    if (Values.size() != 2)
    {
      std::cerr << "ERROR: Invalid number of arguments (" << Values.size() << ")";
      std::cerr << " for option \"-b\".\n\n";
    }

    if (bsc_tools::isLongInt(Values[0]))
    {
      cout << "'-b' Values[0] = " << Values[0] << endl;
      BurstCounterType = (INT32) bsc_tools::getLongInt(Values[0]);
    }
    else
    {
      std::cerr << "ERROR: Got invalid argument \"" << Values[0] << "\" for option \"-b\".\n\n";
      return false;
    }

    if (bsc_tools::isDouble(Values[1]))
    {
      BurstCounterFactor = bsc_tools::getDouble(Values[1]);
    }
    else
    {
      std::cerr << "ERROR: Got invalid argument \"" << Values[1] << "\" for option \"-b\".\n\n";
      return false;
    }

    cout << "BurstCounterType = " << BurstCounterType << endl;
    cout << "BurstCounterFactor = " << BurstCounterFactor << endl;
  }

  if (opt.lastArgs.size() != 2) {
    std::cerr << "ERROR: Missing input/output trace names\n\n";
    Usage(opt);
    return false;
  }
  else
  {
    PrvTraceName = *(opt.lastArgs[0]);
    DimTraceName = *(opt.lastArgs[1]);
  }

  return true;
}

/*
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
        case 'n': /* No generate initial idles *
          GenerateFirstIdle = false;
          break;
        case 'i': /* Iprobe misses threshold
          j++;
          IprobeMissesThreshold = atof(argv[j]);

          if (IprobeMissesThreshold == 0.0  && errno == EINVAL)
          {
            cerr << "Error: Invalid Iprobe threshold value " << endl;
            exit (EXIT_FAILURE);
          }
          break;
        case 'b': /* Burst duration counter *
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

  /* Read Input & Output file names *
  PrvTraceName = argv[j];
  j = j+1;
  DimTraceName = argv[j];

  return;
}
*/

void CopyRowFile(string InputTraceName, string OutputTraceName)
{
  string InputROWName, OutputROWName;
  FILE* InputROW = NULL, *OutputROW = NULL;

  string::size_type SubstrPosition;
  string ChoppedFileName;

  char*   line  = NULL;
  size_t  current_line_length = 0;

  ssize_t bytes_read;

  cout << "COPYING ROW FILE" << endl;

  SubstrPosition = InputTraceName.rfind(".prv");

  if (SubstrPosition == string::npos)
  {
    ChoppedFileName = InputTraceName;
  }
  else
  {
    ChoppedFileName = InputTraceName.substr(0, SubstrPosition);
  }

  InputROWName = ChoppedFileName+".row";

  if ( (InputROW = fopen(InputROWName.c_str(), "r")) == NULL)
  {
    cout << "-> No input trace ROW file found" << endl;
    InputROW = NULL;
    return;
  }

  /* Generate the output ROW name */
  SubstrPosition = OutputTraceName.rfind(".dim");

  if (SubstrPosition == string::npos)
  {
    ChoppedFileName = OutputTraceName;
  }
  else
  {
    ChoppedFileName = OutputTraceName.substr(0, SubstrPosition);
  }
  OutputROWName = ChoppedFileName+".row";

  if ( OutputROWName.compare(InputROWName) == 0)
  {
    cout << "-> Input and ouput ROW files have the same name. Please use it in your simulations" << endl;
    return;
  }

  if ( (OutputROW = fopen(OutputROWName.c_str(), "w")) == NULL)
  {
    cout << "-> Unable to open output ROW file" <<  strerror(errno) << endl;
    return;
  }

  while ( (bytes_read = getline(&line, &current_line_length, InputROW)) != -1)
  {
    fprintf(OutputROW, "%s", line);
    free(line);
    line = NULL;
  }

  if (!feof(InputROW))
  {
    cout << "   * Error reading input ROW file: " << strerror(errno) << endl;
    return;
  }

  fclose(InputROW);
  fclose(OutputROW);
  return;
}

int main(const int argc, const char *argv[])
{
  ParaverTraceTranslator *Translator;

  std::string CounterFactorStr;

  IprobeMissesThreshold = 0.0;
  BurstCounterType      = -1;
  BurstCounterFactor    = 0.0;

  PCFGeneration *PCFGenerator;

  if (!ReadArgsNew(argc, argv))
  {
    exit(EXIT_FAILURE);
  }

    Translator = new ParaverTraceTranslator(PrvTraceName, DimTraceName);

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
                             BurstCounterFactor,
                             GenerateMPIInitBarrier))
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

  PCFGenerator = new PCFGeneration(PrvTraceName, DimTraceName);

  PCFGenerator->GeneratePCF(string(""));

  CopyRowFile(PrvTraceName, DimTraceName);

  return EXIT_SUCCESS;
}

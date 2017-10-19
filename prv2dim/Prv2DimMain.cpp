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

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <string>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <bsc_utils.hpp>
#include "ParaverTraceTranslator.hpp"
#include "PCFGeneration.hpp"

using std::cout;
using std::cerr;
using std::endl;

std::string PrvTraceName;          /* Paraver (input) trace name */
std::string DimTraceName;          /* Dimemas (output) trace name */
std::string ExtraStatisticsName;   /* File that keeps some extra statistics */

bool extraStatistics = false;
bool  GenerateFirstIdle;
bool  GenerateMPIInitBarrier;
double IprobeMissesThreshold; /* Maximun iprobe misses to discard iprobe burst */
double TestMissesThreshold;
double BurstCounterFactor;

int  BurstCounterType;

bool ReadArgs(const int argc, const char *argv[])
{
    namespace po = boost::program_options;

    po::options_description mandatory("Mandatory options");
    mandatory.add_options()
        ("prv-trace,p", po::value<string>(&PrvTraceName)->required(),
            "Input paraver trace")
        ("dim-trace,d", po::value<string>(&DimTraceName)->required(),
            "Output dimemas trace")
    ;

    po::options_description optional("Other options");
    optional.add_options()
        ("burst-counter-type,b", po::value<int>(&BurstCounterType),
            "Hardware counter type used to generate burst duration")
        ("burst-counter-factor,f", po::value<double>(&BurstCounterFactor),
            "Factor applied to the hardware counter type value that "\
            "will generate the burst duration")
        ("iprobe-miss-rate,i", po::value<double>(&IprobeMissesThreshold),
            "MPI_Iprobe miss rate (ms) to iscard Iprobe area CPU burst")
        ("test-miss-rate,t", po::value<double>(&TestMissesThreshold),
            "MPI_Test miss rate (ms) to discard Test area CPU burst")
        ("initial-idle,n", po::bool_switch(&GenerateFirstIdle), 
            "Do not generate an initial idle state")
        ("init-sync,s", po::bool_switch(&GenerateMPIInitBarrier),
            "Do not generate a synchronization primitive on MPI_Init calls")
        ("deadlock-extra-statistics,e", po::bool_switch(&extraStatistics),
            "Generates a file with extra statistics for deadlock analysis")
    ;

    po::options_description miscellany("Miscellany options");
    miscellany.add_options()
        ("help,h", "Show this help message")
    ;

    po::options_description all("Allowed options");
    all.add(mandatory)
        .add(optional)
        .add(miscellany);

    po::positional_options_description pd;
    pd.add("prv-trace", 1)
        .add("dim-trace", 2);

    po::variables_map varmap;
    po::store(po::command_line_parser(argc, argv)
            .options(all)
            .positional(pd)
            .run(), varmap);

    // Options treatment
    //
    if (varmap.count("help"))
    {
        cout << endl;
        cout << "prv2dim - Paraver to Dimemas trace translator" << endl;
        cout << "Barcelona Supercomputer Center - Centro Nacional de Supercomputacion"
            << endl;
        cout << endl;
        cout << "USAGE: " << argv[0] 
            << " [--prv-trace] PRVTRACE [--dim-trace] DIMTRACE"
            << endl;
        cout << endl;
        cout << all << endl;
        exit(EXIT_SUCCESS);
    }

    try
    {
        po::notify(varmap);
    }
    catch(boost::program_options::required_option& e)
    {
        cout << "Error parsing arguments" << endl;
        cout << e.what() << endl;
        return false;
    }

    GenerateFirstIdle = !GenerateFirstIdle;
    GenerateMPIInitBarrier = !GenerateMPIInitBarrier;

    if (varmap.count("burst-counter-type") ^ varmap.count("burst-counter-factor"))
    {
        cout << "Error parsing arguments" << endl;
        cout << "\"--burst-counter-type\" and \"--burst-counter-factor\" should "\
            " appear together" << endl;
        exit(EXIT_FAILURE);
    }

    if (varmap.count("burst-counter-type"))
    {
        cout << "BURST COUNTER TYPE = " << BurstCounterType << endl;
        cout << "BURST COUNTER FACTOR = " << BurstCounterFactor << endl;
    }

    if (extraStatistics /*varmap.count("deadlock-extra-statistics")*/)
    {
        string ChoppedFileName;
        int SubstrPosition = DimTraceName.rfind(".dim");

        ChoppedFileName = DimTraceName.substr(0, SubstrPosition);
        ExtraStatisticsName = ChoppedFileName+".estats";

        cout << "EXTRA STATISTICS NAME = " << ExtraStatisticsName << endl;
    }
    return true;
}

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
  TestMissesThreshold = 0.0;
  BurstCounterType      = -1;
  BurstCounterFactor    = 0.0;

  PCFGeneration *PCFGenerator;

//  if (!ReadArgsNew(argc, argv))
  if (!ReadArgs(argc, argv))
  {
    exit(EXIT_FAILURE);
  }

  Translator = new ParaverTraceTranslator(PrvTraceName, DimTraceName, ExtraStatisticsName);

  if (Translator->GetError())
  {
    cerr << Translator->GetLastError() << endl;

    delete Translator;
    exit(EXIT_FAILURE);
  }

  if (!Translator->InitTranslator())
  {
    cerr << Translator->GetLastError() << endl;

    delete Translator;
    exit(EXIT_FAILURE);
  }

  if (!Translator->SplitCommunications())
  {
    cerr << Translator->GetLastError() << endl;
    if (!Translator->EndTranslator())
    {
      cerr << Translator->GetLastError() << endl;
    }

    delete Translator;
    exit(EXIT_FAILURE);
  }

  if (!Translator->Translate(GenerateFirstIdle,
                             IprobeMissesThreshold,
			                       TestMissesThreshold,
                             BurstCounterType,
                             BurstCounterFactor,
                             GenerateMPIInitBarrier))
  {
    cerr << endl;
    cerr << "Error: " << Translator->GetLastError() << endl;

    if (!Translator->EndTranslator())
    {
      cerr << Translator->GetLastError() << endl;
    }

    delete Translator;
    exit(EXIT_FAILURE);
  }

  if (!Translator->EndTranslator())
  {
    cerr << Translator->GetLastError() << endl;

    delete Translator;
    exit(EXIT_FAILURE);
  }

  PCFGenerator = new PCFGeneration(PrvTraceName, DimTraceName);

  PCFGenerator->GeneratePCF(string(""));

  CopyRowFile(PrvTraceName, DimTraceName);

  delete Translator;

  return EXIT_SUCCESS;
}

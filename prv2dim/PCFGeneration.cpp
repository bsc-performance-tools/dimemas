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
  $Rev:: 560                                      $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-03-22 10:15:54 +0100 (Tue, 22 Mar #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

extern "C" {
#include "pcf_defines.h"
}

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
using std::cout;
using std::endl;

#include "PCFGeneration.hpp"
#include <Dimemas2Prv.h>
#include <EventEncoding.h>

PCFGeneration::PCFGeneration(string InputTraceName,
                             string OutputTraceName)
{
  this->InputTraceName  = InputTraceName;
  this->OutputTraceName = OutputTraceName;
}

void PCFGeneration::GeneratePCF(string UserSuppliedInputPCF)
{
  string InputPCFName, OutputPCFName;
  FILE* InputPCF = NULL, *OutputPCF = NULL;

  string::size_type SubstrPosition;
  string ChoppedFileName;

  bool GenerateDefault = false;


  cout << "GENERATING PCF" << endl;

  /* Check the availability of user supplied PCF */
  if (UserSuppliedInputPCF.compare("") != 0)
  {
    InputPCFName = UserSuppliedInputPCF;

    if ( (InputPCF = fopen(InputPCFName.c_str(), "r")) == NULL)
    {
      cout << "-> User supplied PCF not found" << endl;
      InputPCF = NULL;
    }
  }

  /* If not available, check if a PCF is available with the same name of the
   * input trace */
  if (InputPCF == NULL)
  {
    SubstrPosition = InputTraceName.rfind(".prv");

    if (SubstrPosition == string::npos)
    {
      ChoppedFileName = InputTraceName;
    }
    else
    {
      ChoppedFileName = InputTraceName.substr(0, SubstrPosition);
    }

    InputPCFName = ChoppedFileName+".pcf";

    if ( (InputPCF = fopen(InputPCFName.c_str(), "r")) == NULL)
    {
      cout << "-> No input trace PCF found, generating default" << endl;
      InputPCF = NULL;
    }
  }

  /* Generate the output PCF name */
  SubstrPosition = OutputTraceName.rfind(".dim");

  if (SubstrPosition == string::npos)
  {
    ChoppedFileName = OutputTraceName;
  }
  else
  {
    ChoppedFileName = OutputTraceName.substr(0, SubstrPosition);
  }
  OutputPCFName = ChoppedFileName+".pcf";

  if ( InputPCF != NULL && (OutputPCFName.compare(InputPCFName) == 0))
  {
    cout << "-> Input PCF and output PCF have the same name. Please use it in your simulations" << endl;
    return;
  }

  if ( (OutputPCF = fopen(OutputPCFName.c_str(), "w")) == NULL)
  {
    cout << "-> Unable to open output PCF file" <<  strerror(errno) << endl;
    return;
  }

  if (InputPCF != NULL)
  {
    if (CopyInputPCF(InputPCF, OutputPCF))
    {
      cout << "   * Input PCF " << InputPCFName << " correctly copied to ";
      cout << OutputPCFName << endl;
    }
    else
    {
      cout << "   * WARNING: No output PCF generated" << endl;
    }
  }
  else
  {
    if (GenerateDefaultPCF(OutputPCF))
    {
      cout << "   * Default PCF written to " << OutputPCFName << endl;
      cout << "   * WARNING: it could not contain specific symbolic information" << endl;
    }
    else
    {
      cout << "   * WARNING: No output PCF generated" << endl;
    }
  }

  return;
}

bool PCFGeneration::CopyInputPCF(FILE* InputPCF, FILE* OutputPCF)
{
  char*   line  = NULL;
  size_t  current_line_length = 0;
  ssize_t bytes_read;

  while ( (bytes_read = getline(&line, &current_line_length, InputPCF)) != -1)
  {
    fprintf(OutputPCF, "%s", line);
    free(line);
    line = NULL;
  }

  if (!feof(InputPCF))
  {
    cout << "   * Error reading input PCF: " << strerror(errno) << endl;
    return false;
  }

  fclose(InputPCF);
  fclose(OutputPCF);
  return true;
}

bool PCFGeneration::GenerateDefaultPCF(FILE* OutputPCF)
{
  /* JGG: Escritura de la cabecera del PCF, estatica*/
  for (size_t i = 0; i < PCF_HEAD_LINES; i++)
  {
    fprintf(OutputPCF,"%s\n",pcf_head[i]);
  }

  /* JGG: Parte dinámica referente a los estados */
  fprintf(OutputPCF, "STATES\n");
  for (size_t i = 0; i < PRV_STATE_COUNT; i++)
  {
    fprintf(OutputPCF,
            "%d\t%s\n",
            (int) i,
            ParaverDefaultPalette[i].name);
  }

  /* Dimemas private states */
  for (size_t i = 0; i < DIMEMAS_STATE_COUNT; i++)
  {
    fprintf(OutputPCF,
            "%d\t%s\n",
            (int) i + PRV_STATE_COUNT,
            DimemasDefaultPalette[i].name);
  }

  fprintf(OutputPCF, "\nSTATES_COLOR\n");
  for (size_t i = 0; i < PRV_STATE_COUNT; i++)
  {
    fprintf(OutputPCF,
            "%d\t{%d,%d,%d}\n",
            (int) i,
            ParaverDefaultPalette[i].RGB[0],
            ParaverDefaultPalette[i].RGB[1],
            ParaverDefaultPalette[i].RGB[2]);
  }

  /* Dimemas private states */
  for ( size_t i = 0; i < DIMEMAS_STATE_COUNT; i++)
  {
    fprintf(OutputPCF,
            "%d\t{%d,%d,%d}\n",
            (int) i + PRV_STATE_COUNT,
            DimemasDefaultPalette[i].RGB[0],
            DimemasDefaultPalette[i].RGB[1],
            DimemasDefaultPalette[i].RGB[2]);
  }

  fprintf(OutputPCF, "\n");
  for(size_t i = 0; i < PCF_MIDDLE_LINES; i++)
  {
    fprintf(OutputPCF, "%s\n", pcf_middle[i]);
  }

  for (size_t i = 0; i < NUM_MPICALLS; i++)
  {
     MPIEventEncoding_EnableOperation( (MPI_Event_Values) i);
  }
  MPIEventEncoding_WriteEnabledOperations(OutputPCF);


  for (size_t i = 0; i < PCF_TAIL_LINES; i++)
  {
    fprintf(OutputPCF,"%s\n",pcf_tail[i]);
  }

  if (ferror(OutputPCF))
  {
    cout << "   * Error writing default PCF: " << strerror(errno) << endl;
    return false;
  }

  fflush(OutputPCF);
  fclose(OutputPCF);

  return true;
}

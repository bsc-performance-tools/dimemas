/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
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

  $URL:: https://svn.bsc.es/repos/DIMEMAS/trunk/s#$:  File
  $Rev:: 35                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2012-01-11 19:45:04 +0100 (Wed, 11 Jan #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <cstring>
#include <cerrno>
#include <cstdio>
#include <cstdlib>


/*
#5:
"modules information" {
    // Module identificator number
    int     "identificator";
    // Speed ratio for this module, 0 means instantaneous execution
    double  "execution_ratio";
};;
*/
bool ProcessModulesInformation(FILE  *CFGFile,
                               FILE  *CFGModifiedFile,
                               string CFGModifiedFileName)
{
  char*   line        = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;

  /*
   * Modules information
   */
  if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
  {
    cerr << "Error reading CFG file: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  if (strcmp(line, "\"modules information\" {\n") == 0)
  {
    fprintf(CFGModifiedFile, "%s", line);
  }
  else
  {
    cerr << "Wrong modules definition on input file. It won't be modified" << endl;
    fclose(CFGFile);
    fclose(CFGModifiedFile);

    unlink(CFGModifiedFileName.c_str());
    exit(EXIT_FAILURE);
  }

  /*
   * Id. comment
   */
  free(line);
  line = NULL;
  if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
  {
    cerr << "Error reading CFG file: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  if (strcmp(line, "\t// Module identificator number\n") == 0 ||
      strcmp(line, "   // Module identificator number\n") == 0)
  {
    fprintf(CFGModifiedFile, "\t// Module type\n");
  }
  else if (strcmp(line, "\t// Module type\n") == 0 ||
           strcmp(line, "   // Module type\n") == 0)
  {
    return false;
  }
  else
  {
    cerr << "Wrong modules definition on input file. It won't be modified" << endl;
    fclose(CFGFile);
    fclose(CFGModifiedFile);

    unlink(CFGModifiedFileName.c_str());
    exit(EXIT_FAILURE);
  }

  /*
   * Identificator
   */
  free(line);
  line = NULL;
  if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
  {
    cerr << "Error reading CFG file: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  if (strcmp(line, "\tint     \"identificator\";\n") == 0 ||
      strcmp(line, "   int     \"identificator\";\n") == 0)
  {
    fprintf(CFGModifiedFile, "\tint     \"type\";\n");
    fprintf(CFGModifiedFile, "\t// Module value\n");
    fprintf(CFGModifiedFile, "\tint     \"value\";\n");
    fprintf(CFGModifiedFile, "\t// Speed ratio for this module, 0 means instantaneous execution\n");
    fprintf(CFGModifiedFile, "\tdouble  \"execution_ratio\";\n");
    fprintf(CFGModifiedFile, "};;\n");

    /* Advance three lines! */
    if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
    {
      cerr << "Error reading CFG file: " << strerror(errno) << endl;
      fclose(CFGModifiedFile);
      unlink(CFGModifiedFileName.c_str());
      exit(EXIT_FAILURE);
    }
    if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
    {
      cerr << "Error reading CFG file: " << strerror(errno) << endl;
      fclose(CFGModifiedFile);
      unlink(CFGModifiedFileName.c_str());
      exit(EXIT_FAILURE);
    }

    if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
    {
      cerr << "Error reading CFG file: " << strerror(errno) << endl;
      fclose(CFGModifiedFile);
      unlink(CFGModifiedFileName.c_str());
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    cerr << "Wrong modules definition on input file. It won't be modified" << endl;
    fclose(CFGFile);
    fclose(CFGModifiedFile);

    unlink(CFGModifiedFileName.c_str());
    exit(EXIT_FAILURE);
  }

  return true;
}


int main(int argc, char *argv[])
{
  string CFGFileName, CFGModifiedFileName;
  FILE  *CFGFile,    *CFGModifiedFile;

  char*   line        = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;

  bool    ModuleDefinitions = false;
  bool    DefinedModules    = false;

  int    module_id;
  double module_ratio;

  if (argc != 2 )
  {
    cerr << "This application only receives the CFG parameter" << endl;
    exit(EXIT_FAILURE);
  }
  else
  {
    CFGFileName = argv[1];
  }

  if ( (CFGFile = fopen(CFGFileName.c_str(), "r") ) == NULL)
  {
    cerr << endl;
    cerr << "Unable to open CFG file '" << CFGFileName << "': ";
    cerr << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  CFGModifiedFileName = CFGFileName+".tmp";

  if ( (CFGModifiedFile = fopen(CFGModifiedFileName.c_str(), "w")) == NULL)
  {
    cerr << endl;
    cerr << "Unable to open temporal CFG file: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  while ((bytes_read = getline(&line, &line_length, CFGFile)) != -1)
  {
    // printf("Current line: '%s'", line);

    if ( strcmp(line, "#5:\n") == 0)
    {
      fprintf(CFGModifiedFile, "%s", line);

      ModuleDefinitions = true;

      if (!ProcessModulesInformation(CFGFile,
                                     CFGModifiedFile,
                                     CFGModifiedFileName))
      {
        cout << "CFG in new format. Not converted!" << endl;

        fclose(CFGFile);
        fclose(CFGModifiedFile);

        unlink(CFGModifiedFileName.c_str());
        exit(EXIT_SUCCESS);
      }

      free(line);
      line = NULL;

      continue;
    }

    if (sscanf(line, "\"modules information\" {%d, %lf }\n",
               &module_id,
               &module_ratio) == 2)
    {
      // cout << "Your input file has defined modules information";
      DefinedModules = true;
      fprintf (CFGModifiedFile, "// %s", line);

      free(line);
      line = NULL;
      continue;
    }

    fprintf(CFGModifiedFile, "%s", line);

    free(line);
    line = NULL;
  }

  if (!feof(CFGFile))
  {
    cerr << "Error reading CFG file: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  fclose(CFGFile);
  fclose(CFGModifiedFile);

  if (!ModuleDefinitions)
  {
    cerr << "Wrong format in the input file. Not converted!" << endl;
    unlink(CFGModifiedFileName.c_str());

    exit(EXIT_FAILURE);
  }

  if (rename(CFGFileName.c_str(), (CFGFileName+".OLD").c_str()) == -1)
  {
    cerr << "Unable to save input file backup" << endl;
    exit(EXIT_FAILURE);
  }

  if (rename(CFGModifiedFileName.c_str(), CFGFileName.c_str()) == -1)
  {
    cerr << "Unable to create definitive CFG file: " << strerror(errno) << endl;
    cerr << "Please recover the backup of the original file (" << CFGFileName+".OLD";
    cerr << ") and try again" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Update finished!" << endl;
  cout << "A backup of the original CFG is stored in " << CFGFileName+".OLD" << endl;

  if (DefinedModules)
  {
    cout << "WARNING: please check your manually defined modules" << endl;
  }

  exit(EXIT_SUCCESS);
}


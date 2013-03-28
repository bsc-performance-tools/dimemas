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

#include <unistd.h>

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


#define NEW_NODE_INFO_RECORD_BODY \
"\t\/\/ \"speed_ratio_instrumented_vs_simulated\" \"Relative processor speed\"\n" \
"\tdouble  \"speed_ratio_instrumented_vs_simulated\";\n" \
"\t\/\/ \"intra_node_startup\" \"Startup time (s) of intra-node communications model\"\n" \
"\tdouble  \"intra_node_startup\";\n" \
"\t\/\/ \"intra_node_bandwidth\" \"Bandwidth (MB/s) of intra-node communications model\"\n" \
"\t\/\/ \"0 means instantaneous communication\"\n" \
"\tdouble  \"intra_node_bandwidth\";\n" \
"\t\/\/ \"intra_node_buses\" \"Number of buses of intra-node communications model\"\n" \
"\t\/\/ \"0 means infinite buses\"\n" \
"\tint     \"intra_node_buses\";\n" \
"\t\/\/ \"intra_node_input_links\" \"Input links of intra-node communications model\"\n" \
"\tint     \"intra_node_input_links\";\n" \
"\t\/\/ \"intra_node_input_links\" \"Output links of intra-node communications model\"\n" \
"\tint     \"intra_node_output_links\";\n" \
"\t\/\/ \"intra_node_startup\" \"Startup time (s) of inter-node communications model\"\n" \
"\tdouble  \"inter_node_startup\";\n" \
"\t\/\/ \"inter_node_input_links\" \"Input links of inter-node communications model\"\n" \
"\tint     \"inter_node_input_links\";\n" \
"\t\/\/ \"inter_node_output_links\" \"Input links of intra-node communications model\"\n" \
"\tint     \"inter_node_output_links\";\n" \
"\t\/\/ \"wan_startup\" \"Startup time (s) of inter-machines (WAN) communications model\"\n" \
"\tdouble  \"wan_startup\";\n"


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
    fprintf(CFGModifiedFile, "%s", line);
    return false;
  }
  else if (strcmp(line, "\tint     \"type\";\n") == 0)
  {
    return false;
  }
  {
    cout << "line = " << line << endl;
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
    fprintf(CFGModifiedFile, "\t// Module tpye\n");
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

/*
#2:
"node information" {
  int "machine_id";
  // "node_id" "Node number"
  int     "node_id";
  // "simulated_architecture" "Architecture node name"
  char    "simulated_architecture"[];
  // "number_of_processors" "Number of processors within node"
  int     "number_of_processors";
  // "number_of_input_links" "Number of input links in node"
  int     "number_of_input_links";
  // "number_of_output_links" "Number of output links in node"
  int     "number_of_output_links";
  // "startup_on_local_communication" "Communication startup"
  double  "startup_on_local_communication";
  // "startup_on_remote_communication" "Communication startup"
  double  "startup_on_remote_communication";
  // "speed_ratio_instrumented_vs_simulated" "Relative processor speed"
  double  "speed_ratio_instrumented_vs_simulated";
  // "memory_bandwidth" "Data tranfer rate into node in Mbytes/s"
  // "0 means instantaneous communication"
  double  "memory_bandwidth";
  double "external_net_startup";
};;
*/



bool ProcessNodeInformation(FILE  *CFGFile,
                            FILE  *CFGModifiedFile,
                            string CFGModifiedFileName)
{
  char*   line        = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;
  size_t  field_count = 0;

  if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
  {
    cerr << "Error reading CFG file: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  if (strcmp(line, "\"node information\" {\n") == 0)
  {
    fprintf(CFGModifiedFile, "%s", line);
  }
  else
  {
    cerr << "Wrong node definition on input file. It won't be modified" << endl;
    fclose(CFGFile);
    fclose(CFGModifiedFile);

    unlink(CFGModifiedFileName.c_str());
    exit(EXIT_FAILURE);
  }

  /*
   * Identificator
   */

  field_count = 0;
  while (field_count < 7) // 4 seven lines before the new record order
  {
    free(line);
    line = NULL;
    if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
    {
      cerr << "Error reading CFG file: " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }

    fprintf(CFGModifiedFile, "%s", line);
    field_count++;
  }

  free(line);
  line = NULL;
  if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
  {
    cerr << __FUNCTION__ <<  "Error reading CFG file: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  // "number_of_input_links" "Number of input links in node"
  if (strcmp(line, "// \"number_of_input_links\" \"Number of input links in node\"\n") == 0 ||
      strcmp(line, "\t// \"number_of_input_links\" \"Number of input links in node\"\n") == 0)
  {
    fprintf(CFGModifiedFile, "%s", NEW_NODE_INFO_RECORD_BODY);

    field_count = 0;
    while (field_count < 13) // 13 lines of the previous record must be erased
    {
      free(line);
      line = NULL;
      if ((bytes_read = getline(&line, &line_length, CFGFile)) == -1)
      {
        cerr << "Error reading CFG file: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
      }

      // fprintf(CFGModifiedFile, "%s", line);
      field_count++;
    }

    return true;
  }

  return false;
}


int main(int argc, char *argv[])
{
  string CFGFileName, CFGModifiedFileName;
  FILE  *CFGFile,    *CFGModifiedFile;

  char*   line        = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;

  bool    OldModulesDefinition = false;
  bool    DefinedModules       = false;

  bool    OldNodeDefinition    = false;
  bool    DefinedNodes         = false;

  int    module_id;
  double module_ratio;

  char  *simulated_architecture;

  int       machine_id,
            node_id,
            no_processors,
            number_of_input_links,
            number_of_output_links;

  double    startup_on_local_communication,
            startup_on_remote_communication,
            speed_ratio_instrumented_vs_simulated,
            memory_bandwidth,
            external_net_startup;

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

    if ( strcmp(line, "#2:\n") == 0)
    {
      fprintf(CFGModifiedFile, "%s", line);

      OldNodeDefinition = ProcessNodeInformation(CFGFile,
                                                 CFGModifiedFile,
                                                 CFGModifiedFileName);

      free(line);
      line = NULL;

      continue;
    }

    if ( strcmp(line, "#5:\n") == 0)
    {
      fprintf(CFGModifiedFile, "%s", line);

      OldModulesDefinition = ProcessModulesInformation(CFGFile,
                                                       CFGModifiedFile,
                                                       CFGModifiedFileName);

      /*
      {
        // cout << "CFG in new format. Not converted!" << endl;

        fclose(CFGFile);
        fclose(CFGModifiedFile);

        unlink(CFGModifiedFileName.c_str());
        exit(EXIT_SUCCESS);
      }
      */

      free(line);
      line = NULL;

      continue;
    }

    if (OldNodeDefinition)
    {
      int subs;
      /* To avoid overflows! */
      simulated_architecture = (char*) malloc(strlen(line));

      if (subs = sscanf(line,
                 "\"node information\" {%d, %d, %[^,], %d, %d, %d, %lf, %lf, %lf, %lf, %lf };;\n",
                 &machine_id,
                 &node_id,
                 simulated_architecture,
                 &no_processors,
                 &number_of_input_links,
                 &number_of_output_links,
                 &startup_on_local_communication,
                 &startup_on_remote_communication,
                 &speed_ratio_instrumented_vs_simulated,
                 &memory_bandwidth,
                 &external_net_startup) == 11)
      {
        fprintf(CFGModifiedFile,
                "\"node information\" { %d, %d, %s, %d, %f, %f, %f, %d, 1, 0, %lf, %d, %d, %lf };;\n",
                machine_id,
                node_id,
                simulated_architecture,
                no_processors,
                speed_ratio_instrumented_vs_simulated,
                startup_on_local_communication,
                memory_bandwidth,
                no_processors, // The default translation considers as many buses as processors in the node
                startup_on_remote_communication,
                number_of_input_links,
                number_of_output_links,
                external_net_startup);

        free(simulated_architecture);
        free(line);
        line = NULL;
        continue;
      }

      free(simulated_architecture);
    }

    if (OldModulesDefinition)
    {
      if (sscanf(line, "\"modules information\" {%d, %lf };;\n",
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

  if (!OldNodeDefinition && !OldModulesDefinition)
  {
    cout << "CFG in new format. Not converted!" << endl;

    unlink(CFGModifiedFileName.c_str());
    exit(EXIT_SUCCESS);
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
    cout << "WARNING: please update your manually defined modules" << endl;
  }

  exit(EXIT_SUCCESS);
}


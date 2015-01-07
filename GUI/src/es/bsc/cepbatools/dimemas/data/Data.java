/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                               Dimemas GUI                                 *
 *                  GUI for the Dimemas simulation tool                      *
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

  $URL::              $:  File
  $Rev::              $:  Revision of last commit
  $Author::           $:  Author of last commit
  $Date::             $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

package es.bsc.cepbatools.dimemas.data;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import es.bsc.cepbatools.dimemas.tools.Tools;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.RandomAccessFile;
import javax.swing.*;

/*
* La clase Data ofrece acceso a todos los datos de la GUI.
*/
public class Data
{
  private JTextField currentConfigurationFile;
  public String instrumentedArchitecture = "";

  // Files included in the 'resource' directory of the JAR file
  static public final String  ICON_IMAGE              = "resources/dimemas_icon.png";
  static public final String  BSC_LOGO                = "resources/bsc_logo.jpg";
  static public final String  RECORDS_DEFINITION_FILE = "resources/records_definition.txt";
  // static public boolean       UNDEFINED_HOME  = false;
  // static public final String  HOME            = os();
  // static public final String  MACHINE_DB_FILE = HOME + "machines.db";
  // static public final String  NETWORK_DB_FILE = HOME + "networks.db";

  // 'Magic numbers' of the two Dimemas configuration files versions
  static final public String SDDFA_MAGIC       = "SDDFA";
  static final public String DIMEMAS_CFG_MAGIC = "#DIMEMAS_CONFIGURATION";

  // To check type of configuration file loaded
  static public final int NO_CFG_LOADED = -1;
  static public final int SDDFA         = 0;
  static public final int DIMEMAS_CFG   = 1;
  int ConfigurationFileType = Data.NO_CFG_LOADED;

  // Mapping constants
  static public final int NO_MAP                = -1;
  static public final int UNKNOWN_MAP           =  0;
  static public final int FILL_NODE_MAP         =  1;
  static public final int N_TASKS_PER_NODE_MAP  =  2;
  static public final int INTERLEAVE_MAP        =  3;

  static public final int NO_TASKS_DETECTED     = -1;

  // Mapping constants strings
  public static final String FILL_NODE_MAP_STR        = "FILL_NODES";
  public static final String N_TASKS_PER_NODE_MAP_STR = "TASKS_PER_NODE";
  public static final String INTERLEAVE_MAP_STR       = "INTERLEAVED";

  // Número de elementos que componen las COLLECTIVE OPERATIONS.
  static public final int DEFAULT_MPI_ITEMS = 14;

  static public final String WAN         = "\"wide area network information\" {";
  static public final String CONNECTION  = "\"dedicated connection information\" {";
  static public final String ENVIRONMENT = "\"environment information\" {";
  static public final String NODE        = "\"node information\" {";
  static public final String MULTINODE   = "\"multi node information\" {";
  static public final String MAPPING     = "\"mapping information\" {";
  static public final String PREDEF_MAP  = "\"predefined mapping information\" {";
  static public final String CONFIG      = "\"configuration files\" {";
  static public final String MODULE      = "\"modules information\" {";
  static public final String FILE_SYS    = "\"file system parameters\" {";

  // public MachineDataBase machineDB         = new MachineDataBase();
  // public NetworkDataBase netDB             = new NetworkDataBase();
  public WideAreaNetworkData     wan               = new WideAreaNetworkData();
  public DedicatedConnectionData dedicated         = new DedicatedConnectionData();
  public EnvironmentData         environment       = new EnvironmentData();
  public NodeData                nodes_information = new NodeData();
  public MappingData             map               = new MappingData(nodes_information);
  public ConfigurationData       config            = new ConfigurationData();
  public FileSystemData          fileSys           = new FileSystemData();
  public SimulatorCallData       simOptions        = new SimulatorCallData();
  public BlockData               block             = new BlockData();

  // To check if there are multiple mappings defined on the configuration file
  int MapsOnDisk = 0;

  // To check if multiple 'wan definition' records appear on the trace
  boolean wan_defined = false;



  /*
  * El método os genera una cadena con el path correcto para llegar a los
  * ficheros requeridos.
  *
  * @ret String: Cadena de caracteres que forman el path dependiendo del S.O.
  */
  /*
  private static String os()
  {
    if(System.getProperty("os.name").startsWith("Windows"))
    {
      return System.getProperty("USER_HOME") + "/DIMEMAS_defaults/";
    }
    else
    {
      String HomeDirectory = System.getenv("DIMEMAS_HOME");

      if (HomeDirectory == null || HomeDirectory.equals(""))
      {
        UNDEFINED_HOME = true;
        return "";
      }
      else
      {
        return HomeDirectory+"/share/dimemas_defaults/";
      }
    }
    */



    /*
    if(System.getProperty("os.name").startsWith("Windows"))
    {
      return System.getProperty("USER_HOME") + "/DIMEMAS_defaults/";
    }
    else
    {
      return System.getProperty("user.home") + "/.DIMEMAS_defaults/";
    }

  }
  */
  // Constructor de la clase Data.
  public Data(JTextField tf)
  {
    currentConfigurationFile = tf;
    currentConfigurationFile.setBorder(null);
    currentConfigurationFile.setEditable(false);
  }

  public static boolean checkConfigurationFiles()
  {
    /* Check if the application ICON and the RECORDS_DEFINITION file are
     * available */

    return true;
  }




  /*
  * Método que permite el acceso externo al nombre del fichero de configuración
  * actualmente en uso.
  *
  * @ret String: Cadena de caracteres con el nombre del fichero de config.
  */
  public String getCurrentConfigurationFileLabel()
  {
    return currentConfigurationFile.getText();
  }

  /*
  * Método que permite especificar el nombre del fichero de configuración usado.
  *
  * @param: String text -> nombre del fichero de config. que estará en uso.
  */
  public void setCurrentConfigurationFileLabel(String text)
  {
    currentConfigurationFile.setText(text);
  }

  /*
  * Método que genera una línea agrupando todos los datos pertenecientes a un
  * módulo de configuración (como era el caso del módulo MAPPING, en el que
  * prácticamente aparecía un dato por línea en el fichero de configuración!).
  *
  * @param: · String l -> línea parcial perteneciente a un módulo de config.
  *         · RandomAccessFile raf -> fichero fuente de configuración.
  *
  * @ret String: Línea con todos los datos de un módulo de configuración.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  private String splitLine(String l, RandomAccessFile raf) throws IOException
  {
    while(!l.endsWith("};;") && (raf.getFilePointer() != raf.length()))
    {
        l += raf.readLine();
    }

    return l;
  }

  /*
  * Método que permite cargar los datos relacionados con las COLLECTIVE OP. y
  * FLIGTH TIME que se encuentran ubicados en los ficheros de configuración
  * adicionales (EXTRA COMMUNICATION CONFIG FILE).
  *
  * @param: String filename -> nombre del fichero de configuración adicional.
  *
  * @ret boolean: TRUE si el núm. de máquinas del fichero de configuración
  *               adicional concuerda con el núm. de máquinas de la
  *               configuración en uso, FALSE en otro caso.
  *
  * @exc: Error de lectura del fichero, valor numérico no válido, etc.
  */
  public boolean loadCommunicationData(String filename) throws Exception
  {
    String line = null;
    int element;
    int machines = 0;
    RandomAccessFile source = new RandomAccessFile(new File(filename),"r");

    // Recorrer el fichero para obtener el núm. de máquinas.
    while(source.getFilePointer() != source.length())
    {
      line = source.readLine();

      if(line.startsWith("Machine globalop:"))
      {
        machines++;
      }
    }

    if(machines/DEFAULT_MPI_ITEMS != environment.getNumberOfMachines())
    {
      Tools.showInformationMessage("Number of machines mismatch.\nSelected file: "
                          + machines/DEFAULT_MPI_ITEMS + "\nConfiguration: "
                          + environment.getNumberOfMachines());

      source.close();
      return false;
    }
    else // Número correcto de máquinas.
    {
      line = null;
      source.seek(0);

      while(source.getFilePointer() != source.length())
      {
        line = source.readLine();

        if(line.startsWith("Flight time:")) // Datos FLIGHT TIME.
        {
          line = Tools.blanks(line.substring(line.lastIndexOf(":")+1,line.length()));
          environment.ftLoad(line);
        }
        else if(line.startsWith("Machine globalop:")) // Datos INTERNAL OP.
        {
          line = Tools.blanks(line.substring(line.indexOf(":")+1,line.length()));
          element = Integer.parseInt(line.substring(0,line.indexOf(" ")));
          line = Tools.blanks(line.substring(line.indexOf(" "),line.length()));

          if((element < environment.getNumberOfMachines()) &&
             element == Integer.parseInt(environment.machine[element].getId()))
          {
            environment.machine[element].mpiLoad(line);
          }
          else // Busqueda del elemento con el id equivalente a @element
          {
            for(int i = 0; i < environment.getNumberOfMachines(); i++)
            {
              if(element == Integer.parseInt(environment.machine[i].getId()))
              {
                environment.machine[i].mpiLoad(line);
                break;
              }
            }
          }
        } // END if(machine globalop)
        else if(line.startsWith("External globalop:")) // Datos EXTERNAL OP.
        {
          wan.mpiLoad(Tools.blanks(line.substring(line.lastIndexOf(":")+1,line.length())));
        }
      } // END while()
    } // END if(número correcto de máquinas)

    source.close();
    return true;
  }

  /*
  * Método que carga todos los datos de cada uno de los módulos que forman una
  * configuración.
  *
  * @param: String filename -> nombre del fichero de configuración.
  */
  public void loadFromDisk(String filename)
  {
    int first;
    int second;
    int lineCount = 0, lineSeek = 0;
    long seek         = 0;
    String line       = "";
    boolean oldFile   = true, validFile = false, malformedFile = false;
    RandomAccessFile source;

    try
    {
      source = new RandomAccessFile(new File(filename),"r");
    }
    catch(Throwable exc)
    {
      String message = "Unable to open configuration file \""+filename+"\"\n";
      message += exc.getMessage();
      
      Tools.showInformationMessage(message);
      return;
    }

    try
    {
      
      if (source.length() == 0)
      {
        Tools.showInformationMessage("The configuration file selected is empty");
        return;
      }

      // Check file type
      line = source.readLine();
      lineCount++;
      seek     = source.getFilePointer();
      lineSeek = lineCount;

      if(!validFile)
      {
        if(line.equalsIgnoreCase(Data.SDDFA_MAGIC) || line.equalsIgnoreCase(Data.DIMEMAS_CFG_MAGIC))
        {
          validFile  = true;
          MapsOnDisk = 0;
          wan.initialValues();
          dedicated.destroyConnections();
          environment.destroyMachines();
          nodes_information.destroyNodes();
          map.initialValues();
          config.initialValues();
          fileSys.initialValues();
          block.destroyFactors();

          if (line.equalsIgnoreCase(Data.SDDFA_MAGIC))
          {
            ConfigurationFileType = Data.SDDFA;
          }
          else
          {
            ConfigurationFileType = Data.DIMEMAS_CFG;
          }
        }
        else
        {
          Tools.showInformationMessage("The file you selected is not a valid Dimemas configuration file");
          return;
        }

        /* Now, the configuration file must always have the magic number
         * at the very beginning
        else if(!Tools.blanks(line).equalsIgnoreCase(""))
        {
          Tools.showInformationMessage("The file you selected is not a valid Dimemas configuration file");
          return;
        }
        */
      }

      /* Traverse the possible SDDF definition records or DIMEMAS_CONFIGURATION
       * records description */

      line = source.readLine().trim();
      while(source.getFilePointer() != source.length())
      {
        if (ConfigurationFileType == Data.SDDFA)
        {
          if(line.startsWith("#"))   // Configuration record start
          {
            if(line.endsWith("};;")) // Configuration record end
            {
              if(oldFile && (line.startsWith("#0") || line.startsWith("#7")))
              {
                oldFile = false;
              }

              seek      = source.getFilePointer();
              lineSeek  = lineCount;

              line = source.readLine().trim();
              lineCount++;
            }
            else                     // Módulo de configuración incompleto.
            {
              line += source.readLine().trim();
              lineCount++;
            }
          }
          else
          {
            line = source.readLine().trim();
            lineCount++;
          }
        }
        else
        {
          if (line.startsWith("/*")) // Records description start
          {
            if (line.endsWith("*/")) // Records description end
            {
              seek      = source.getFilePointer();
              lineSeek  = lineCount;

              line = source.readLine().trim();
              lineCount++;
            }
            else
            {
              line += source.readLine().trim();
              lineCount++;
            }
          }
          else
          {
            line = source.readLine().trim();
            lineCount++;
          }
        }
      }

      source.seek(seek);
      lineCount = lineSeek;

      wan_defined   = false;
      malformedFile = false;

      // Recorrer el fichero obteniendo los datos de cada uno de los módulos.
      while(!malformedFile && (source.getFilePointer() != source.length()))
      {
        line = source.readLine().trim();
        lineCount++;

        if(line.startsWith(WAN))              // Datos de WAN.
        {
          if (!wan_defined)
          {
            if (!wan.loadData(splitLine(line,source), lineCount))
            {
              malformedFile = true;
            }
            wan_defined = true;
          }
          else
          {
            Tools.showErrorMessage("Multiple 'wide area network information' records on file");
            malformedFile = true;
          }
        }
        else if(line.startsWith(CONNECTION))  // Datos de DEDICATED CONNECTION.
        {
          dedicated.setNumberOfConnections(Integer.parseInt(wan.getDedicated()));
          dedicated.createConnections();

          for(int i = 0; i < dedicated.getNumberOfConnections(); i++)
          {
            if (!dedicated.connection[i].loadData(splitLine(line,source),oldFile, lineCount))
            {
              malformedFile = true;
              break;
            }
            else
            {
              line = source.readLine();
            }
          }
        }
        else if(line.startsWith(ENVIRONMENT)) // Datos de MACHINES.
        {
          environment.setNumberOfMachines(Integer.parseInt(wan.getMachines()));

          if(instrumentedArchitecture.equalsIgnoreCase(""))
          {
            environment.createMachines(instrumentedArchitecture,environment.DEFAULT_BUSES);
          }
          else
          {
            boolean created = false;

            /*
            for(int i = netDB.getNumberOfNetworksInDB()-1; i >= 0; i--)
            {
              if(netDB.net[i].getLabel().equalsIgnoreCase(instrumentedArchitecture))
              {
                environment.createMachines(instrumentedArchitecture,netDB.net[i].getBuses());
                created = true;
                break;
              }
            }
            */

            if(!created)
            {
              environment.createMachines(instrumentedArchitecture,environment.DEFAULT_BUSES);
            }
          }

          int totalNodes = 0;

          for(int i = 0; i < environment.getNumberOfMachines(); i++)
          {
            if (!environment.machine[i].loadData(splitLine(line,source), oldFile, lineCount))
            {
              malformedFile = true;
              break;
            }
            else
            {
              totalNodes += Integer.parseInt(environment.machine[i].getNodes());
              line = source.readLine().trim();
              lineCount++;
            }
          }

          if (!malformedFile)
          {
            nodes_information.setNumberOfNodes(totalNodes);
            instrumentedArchitecture = environment.machine[0].getArchitecture(false);
          }
        }
        else if(line.startsWith(NODE))        // Datos de NODES.
        {
          // nodes_information.createNodes(environment,machineDB.machine);
          nodes_information.createNodes(environment);

          /* JGG (10/01/2014): This assumes that all node definitions come
             consecutive, change to line by line load!
          for(int i = 0; i < nodes_information.getNumberOfNodes(); i++)
          {
            if (!nodes_information.node[i].loadData(splitLine(line,source), oldFile, lineCount))
            {
              malformedFile = true;
              break;
            }
            else
            {
              line = source.readLine();
              lineCount++;
            }
          }
          */
          if (!nodes_information.loadSingleNodeData(splitLine(line,source), oldFile, lineCount))
          {
            malformedFile = true;
            break;
          }
        }
        else if(line.startsWith(MULTINODE))
        {
          nodes_information.createNodes(environment);
          if (!nodes_information.loadMultiNodeData(line, lineCount))
          {
            malformedFile = true;
            break;
          }
        }
        else if(line.startsWith(MAPPING))     // Datos de MAPPING.
        {
          if (MapsOnDisk == 0)
          {
            if (!map.loadTaskMapping(splitLine(line,source), lineCount))
            {
              malformedFile = true;
            }
          }

          MapsOnDisk++;
        }
        else if (line.startsWith(PREDEF_MAP))
        {
          if (MapsOnDisk == 0)
          {
            if (!map.loadPredefinedMapping(splitLine(line,source), lineCount))
            {
              malformedFile = true;
            }
          }

          MapsOnDisk++;
        }
        else if(line.startsWith(CONFIG))      // Datos de EXTRA CONFIG FILES.
        {
          if (!config.loadData(splitLine(line,source), lineCount))
          {
            malformedFile = true;
          }
          else
          {
            if(!config.getCommunication(false).equalsIgnoreCase(""))
            {
              loadCommunicationData(config.getCommunication(false));
            }
          }
        }
        else if(line.startsWith(MODULE))      // Datos de BLOCK FACTORS/MODULES.
        {
          if (!block.loadData(splitLine(line,source), lineCount))
          {
            malformedFile = true;
          }
        }
        else if(line.startsWith(FILE_SYS))    // Datos de FILE SYSTEM INFO.
        {
          if (!fileSys.loadData(splitLine(line,source), lineCount))
          {
            malformedFile = true;
          }
        }
      } // END while()
    }
    catch (Exception exc)
    {
      String message = "Unable to open configuration file \""+filename+"\"\n";
      message += exc.getMessage();
      
      Tools.showInformationMessage(message);
      return;
    }

    if (malformedFile)
    {
      // Reset all fields!
      wan.initialValues();
      dedicated.destroyConnections();
      environment.destroyMachines();
      nodes_information.destroyNodes();
      map.initialValues();
      config.initialValues();
      fileSys.initialValues();
      block.destroyFactors();
      currentConfigurationFile.setText("");

      Tools.showInformationMessage("Malformed configuration file. It won't be loaded");
    }
    else
    {
      if (MapsOnDisk > 1)
      {
        Tools.showInformationMessage("Current configuration file has multiple applications defined.\n\n"+
                                     "Only first application will be loaded");
      }

      for (int i = environment.getNumberOfMachines()-1; i >= 0; i--)
      {
        // Los nodos de una máquina tendrán su misma arquitectura.
        for(int j = 0; j < nodes_information.getNumberOfNodes(); j++)
        {
          if(nodes_information.node[j].getMachine_id().equalsIgnoreCase(environment.machine[i].getId()))
          {
            environment.machine[i].setNodeArchitecture(nodes_information.node[j].getArchitecture(false));
            break;
          }
        }

        // Fijar el índice que permitirá encontrar los datos de la arquitectura
        // en MachineDB.
        /*
        for(int k = machineDB.getNumberOfMachinesInDB()-1; k >= 0; k--)
        {
          if(machineDB.machine[k].getLabel().equalsIgnoreCase(environment.machine[i].getNodeArchitecture()))
          {
            environment.machine[i].setIndex(k);
            break;
          }
        }
        */
      }

      /* The file has been correctly loaded, set the name in the GUI window */
      currentConfigurationFile.setText(filename);
    }
  }

  /*
  * Método que graba, en un fichero con nombre @filename, todos los datos de
  * cada uno de los módulos que forman una configuración.
  *
  * @param: String filename -> nombre del fichero de configuración a generar.
  */
  public boolean saveToDisk(String filename)
  {
    RandomAccessFile source;
    RandomAccessFile target;

    String cfgCheckResult = checkCorrectConfiguration();

    if (cfgCheckResult != null)
    {
      Tools.showErrorDialog(cfgCheckResult+"\n\n"+
                            "Configuration file won't be saved");
      return false;
    }

    try
    {
      // source = new RandomAccessFile(new File(HEADER_FILE),"r");
      target = new RandomAccessFile(new File(filename),"rw");
      target.setLength(0);
      InputStream is = getClass().getClassLoader().getResourceAsStream(Data.RECORDS_DEFINITION_FILE);

      target.writeBytes(Data.DIMEMAS_CFG_MAGIC+"\n\n");

      if (is == null)
      {
        Tools.showInformationMessage("Unable to open record description file, it won't be included in configuration file\n"+
                                     "Please report this error to tools@bsc.es");
      }
      else
      {
        InputStreamReader isr = new InputStreamReader(is);
        BufferedReader br     = new BufferedReader(isr);
        String line;

        while ((line = br.readLine()) != null)
        {
          target.writeBytes(line+"\n");
        }
        br.close();
        isr.close();
        is.close();
        target.writeBytes("\n");
      }

      // WAN information record
      wan.saveData(target);
      target.writeBytes("\n");

      // DEDICATED CONNECTION records
      for(int i = 0; i < dedicated.getNumberOfConnections(); i++)
      {
        dedicated.connection[i].saveData(target);
      }

      target.writeBytes("\n");

      // MACHINE records
      if(environment.getNumberOfMachines() == 0)
      {
        environment.setNumberOfMachines(1);
        environment.createMachines(environment.DEFAULT_ARCHITECTURE,
                                   environment.DEFAULT_BUSES);
      }

      for(int i = 0; i < environment.getNumberOfMachines(); i++)
      {
        environment.machine[i].saveData(target);
      }

      target.writeBytes("\n");

      /* JGG 10/01/2014 Now 'nodes_information' object prints all the nodes data
      if(nodes_information.getNumberOfNodes() == 0)
      {
        nodes_information.setNumberOfNodes(environment.getNumberOfMachines());
        //nodes_information.createNodes(environment, machineDB.machine);
        nodes_information.createNodes(environment);
      }

      for(int i = 0; i < nodes_information.getNumberOfNodes(); i++)
      {
        nodes_information.node[i].saveData(target);
      }
      */
      nodes_information.saveData(target);

      target.writeBytes("\n");

      /*
      if(map.getTasks() == 0)
      {
        map.setTasks(1);
      }

      if(map.getMap(false).equalsIgnoreCase(""))
      {
        map.setMap(new int[0]);
      }
      */

      map.saveData(target);
      target.writeBytes("\n");

      // EXTRA CONFIG FILES record
      config.saveData(target);
      target.writeBytes("\n");

      // MODULES record
      block.saveData(target);
      target.writeBytes("\n");

      // FILE SYSTEM INFO record
      fileSys.saveData(target);
      target.close();
      
      // To ease Paraver <-> DimemasGUI communication
      System.out.println("Configuration file saved: "+filename);
    }
    catch(IOException exc)
    {
      Tools.showErrorDialog("Error writing configuration file\n"+exc.toString());
      return false;
    }
    catch(NumberFormatException exc)
    {
      Tools.showErrorDialog("Error creating default values, configuration file not written\n"+exc.toString());
      return false;
    }


    return true;
  }

  private String checkCorrectConfiguration()
  {
    if (nodes_information.getNumberOfNodes() == 0)
    {
      return "No nodes defined in current machine!";
    }

    if (map.getMapInfo() == Data.NO_MAP)
    {
      return "No tasks mapping defined!";
    }

    return null;
  }
}



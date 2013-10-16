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

package data;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import tools.*;
import javax.swing.*;
import java.io.*;
import java.lang.Throwable;

/*
* La clase Data ofrece acceso a todos los datos de la GUI.
*/
public class Data
{
  private JTextField currentConfigurationFile;
  public String instrumentedArchitecture = "";

  // Ficheros externos.
  static public boolean       UNDEFINED_HOME  = false;
  static public final String  HOME            = os();
  static public final String  ICON_IMAGE      = HOME + "icon.jpg";
  static public final String  HEADER_FILE     = HOME + "header.txt";
  // static public final String  MACHINE_DB_FILE = HOME + "machines.db";
  // static public final String  NETWORK_DB_FILE = HOME + "networks.db";

  // Constantes de Mapeo
  static public final int NO_MAP         = -1;
  static public final int UNKNOW_MAP     =  0;
  static public final int LINEAR_MAP     =  1;
  static public final int CHUNK_MAP      =  2;
  static public final int INTERLEAVE_MAP =  3;

  // Número de elementos que componen las COLLECTIVE OPERATIONS.
  static public final int DEFAULT_MPI_ITEMS = 14;

  static public final String WAN         = "\"wide area network information\" {";
  static public final String CONNECTION  = "\"dedicated connection information\" {";
  static public final String ENVIRONMENT = "\"environment information\" {";
  static public final String NODE        = "\"node information\" {";
  static public final String MAPPING     = "\"mapping information\" {";
  static public final String CONFIG      = "\"configuration files\" {";
  static public final String MODULE      = "\"modules information\" {";
  static public final String FILE_SYS    = "\"file system parameters\" {";

  // public MachineDataBase machineDB         = new MachineDataBase();
  // public NetworkDataBase netDB             = new NetworkDataBase();
  public WideAreaNetworkData     wan               = new WideAreaNetworkData();
  public DedicatedConnectionData dedicated         = new DedicatedConnectionData();
  public EnvironmentData         environment       = new EnvironmentData();
  public NodeData                nodes_information = new NodeData();
  public MappingData             map               = new MappingData();
  public ConfigurationData       config            = new ConfigurationData();
  public FileSystemData          fileSys           = new FileSystemData();
  public SimulatorCallData       simOptions        = new SimulatorCallData();
  public BlockData               block             = new BlockData();

  /*
  * El método os genera una cadena con el path correcto para llegar a los
  * ficheros requeridos.
  *
  * @ret String: Cadena de caracteres que forman el path dependiendo del S.O.
  */
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



    /*
    if(System.getProperty("os.name").startsWith("Windows"))
    {
      return System.getProperty("USER_HOME") + "/DIMEMAS_defaults/";
    }
    else
    {
      return System.getProperty("user.home") + "/.DIMEMAS_defaults/";
    }
    */
  }

  // Constructor de la clase Data.
  public Data(JTextField tf)
  {
    currentConfigurationFile = tf;
    currentConfigurationFile.setBorder(null);
    currentConfigurationFile.setEditable(false);
  }

  public static boolean checkConfigurationFiles()
  {
    RandomAccessFile test;

    if (UNDEFINED_HOME)
    {
      Tools.showErrorDialog("'DIMEMAS_HOME' environment variable not set!");
      return false;
    }

    /*
    try
    {
      test = new RandomAccessFile(new File(Data.MACHINE_DB_FILE),"r");
      test.close();
    }
    catch(Exception e)
    {
      Tools.showErrorDialog("Unable to open 'machine.db' file. Check existence and permissions on DIMEMAS_HOME directories");
      return false;
    }

    try
    {
      test = new RandomAccessFile(new File(Data.NETWORK_DB_FILE),"r");
      test.close();
    }
    catch(Exception e)
    {
      Tools.showErrorDialog("Unable to open 'network.db' file.  Check existence and permissions on DIMEMAS_HOME directories");
      return false;
    }
    */

    try
    {
      test = new RandomAccessFile(new File(Data.HEADER_FILE),"r");
      test.close();
    }
    catch(Exception e)
    {
      Tools.showErrorDialog("Unable to open 'header.txt' file.  Check existence and permissions on DIMEMAS_HOME directories");
      return false;
    }

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

      // Recorrer el fichero para ver si pertenece a la versión actual del GUI.
      while(source.getFilePointer() != source.length())
      {
        if(line.startsWith("#"))   // Empieza un módulo de configuración.
        {
          if(line.endsWith("};;")) // Acaba un módulo de configuración.
          {
            if(oldFile && (line.startsWith("#0") || line.startsWith("#7")))
            {
              oldFile = false;
            }

            seek = source.getFilePointer();
            lineSeek  = lineCount;

            line = source.readLine();
            lineCount++;
          }
          else                     // Módulo de configuración incompleto.
          {
            line += source.readLine();
            lineCount++;
          }
        }
        else                       // Comprobar que sea un fichero SDDFA válido.
        {
          line = source.readLine();
          lineCount++;

          if(!validFile)
          {
            if(line.equalsIgnoreCase("SDDFA"))
            {
              validFile = true;
              wan.initialValues();
              dedicated.destroyConnections();
              environment.destroyMachines();
              nodes_information.destroyNodes();
              map.initialValues();
              config.initialValues();
              fileSys.initialValues();
              block.destroyFactors();
            }
            else if(!Tools.blanks(line).equalsIgnoreCase(""))
            {
              Tools.showInformationMessage("The file you have selected is not a valid SDDFA file.\n"+
                                           "It won't be loaded");
              return;
            }
          } // END if(!validFile)
        } // END if(SDDFA válido)
      } // END while()

      source.seek(seek);
      lineCount = lineSeek;

      malformedFile = false;

      // Recorrer el fichero obteniendo los datos de cada uno de los módulos.
      while(!malformedFile && (source.getFilePointer() != source.length()))
      {
        line = source.readLine();
        lineCount++;

        if(line.startsWith(WAN))              // Datos de WAN.
        {
          if (!wan.loadData(splitLine(line,source), lineCount))
          {
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
              line = source.readLine();
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
        }
        else if(line.startsWith(MAPPING))     // Datos de MAPPING.
        {
          if (!map.loadData(splitLine(line,source), lineCount))
          {
            malformedFile = true;
          }
          else
          {
            map.setMapInfo(Data.UNKNOW_MAP);
          }
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

      source.close();
    }
    catch(Throwable exc)
    {
      Tools.showInformationMessage(exc.getMessage());
      exc.printStackTrace();
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
  public void saveToDisk(String filename)
  {
    RandomAccessFile source;
    RandomAccessFile target;

    try
    {
      source = new RandomAccessFile(new File(HEADER_FILE),"r");
      target = new RandomAccessFile(new File(filename),"rw");

      if(target.length() != 0)
      {
        target.setLength(0);
      }

      // Generación de la cabecera del nuevo fichero de configuración.
      while(source.getFilePointer() != source.length())
      {
        target.writeBytes(source.readLine() + "\n");
      }

      source.close();
      target.writeBytes("\n");

      // Datos de WAN.
      wan.saveData(target);
      target.writeBytes("\n");

      // Datos de DEDICATED CONNECTION.
      for(int i = 0; i < dedicated.getNumberOfConnections(); i++)
      {
        dedicated.connection[i].saveData(target);
      }

      target.writeBytes("\n");

      // Datos de MACHINES.
      if(environment.getNumberOfMachines() == 0)
      {
        environment.setNumberOfMachines(1);
        environment.createMachines(environment.DEFAULT_ARCHITECTURE,environment.DEFAULT_BUSES);
      }

      for(int i = 0; i < environment.getNumberOfMachines(); i++)
      {
        environment.machine[i].saveData(target);
      }

      target.writeBytes("\n");

      // Datos de NODES.
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

      target.writeBytes("\n");

      // Datos de MAPPING.
      if(map.getTasks() == 0)
      {
        map.setTasks("1");
      }

      if(map.getMap(false).equalsIgnoreCase(""))
      {
        map.setMap("0");
      }

      map.saveData(target);
      target.writeBytes("\n");

      // Datos de EXTRA CONFIG FILES.
      config.saveData(target);
      target.writeBytes("\n");

      // Datos de BLOCK FACTORS/MODULES.
      block.saveData(target);
      target.writeBytes("\n");

      // Datos de FILE SYSTEM INFO.
      fileSys.saveData(target);
      target.close();
    } catch(Throwable exc)
      {
        Tools.showInformationMessage(exc.toString());
      }
  }
}

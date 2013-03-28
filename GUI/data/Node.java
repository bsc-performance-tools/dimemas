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
 * TÃ­tulo:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Juan Gonzalez Garcia
 * @version 1.0
 */

import data.Data.*;
import tools.*;
import java.io.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

/*
* La clase Node provee la informaciÃ³n de un nodo.
*/
public class Node
{
  private String DEFAULT_MACHINE_ID;
  private String DEFAULT_NODE_ID;

  // Valores por defecto.
  static public final String DEFAULT_ARCHITECTURE         = "";
  static public final String DEFAULT_PROCESSORS           = "1";
  static public final String DEFAULT_CPU_RATIO            = "1.0";
  static public final String DEFAULT_INTRA_NODE_STARTUP   = "0.0";
  static public final String DEFAULT_INTRA_NODE_BANDWIDTH = "0.0";
  static public final String DEFAULT_INTRA_NODE_BUSES     = DEFAULT_PROCESSORS;
  static public final String DEFAULT_INTRA_NODE_IN_LINKS  = "1";
  static public final String DEFAULT_INTRA_NODE_OUT_LINKS = "0";
  static public final String DEFAULT_INTER_NODE_STARTUP   = "0.0";
  static public final String DEFAULT_INTER_NODE_IN_LINKS  = "1";
  static public final String DEFAULT_INTER_NODE_OUT_LINKS = "1";
  static public final String DEFAULT_WAN_STARTUP          = "0.0";


  static public final int    NODE_RECORD_FIELD_COUNT   = 14;

  private String machine_id;           // Machine to which belongs the node.
  private String node_id;              // Node identificator.
  private String architecture;         // Architecture name.
  private String processors;           // Number of processors within node.
  private String cpu_ratio;            // Relative processor speed.

  private String intra_node_startup;   // Startup on local communication.
  private String intra_node_bandwidth; // Node bandwidth.
  private String intra_node_buses;     // Number of memory buses
  private String intra_node_in_links;  // Number of memory input links
  private String intra_node_out_links; // Number of memory output links

  private String inter_node_startup;   // Startup on remote communication.
  private String inter_node_in_links;  // Number of input links.
  private String inter_node_out_links; // Number of output links.

  private String wan_startup;          // External net startup.

  // Constructor de la clase Node.
  public Node(String identificator, String machineId)
  {
    DEFAULT_NODE_ID    = identificator;
    DEFAULT_MACHINE_ID = machineId;
    initialValues();
  }

  // MÃ©todo que inicializa los datos con los valores por defecto.
  public void initialValues()
  {
    machine_id    = DEFAULT_MACHINE_ID;
    node_id       = DEFAULT_NODE_ID;
    architecture  = DEFAULT_ARCHITECTURE;
    processors    = DEFAULT_PROCESSORS;
    cpu_ratio     = DEFAULT_CPU_RATIO;

    intra_node_startup   = DEFAULT_INTRA_NODE_STARTUP;
    intra_node_bandwidth = DEFAULT_INTRA_NODE_BANDWIDTH;
    intra_node_buses     = DEFAULT_INTRA_NODE_BUSES;
    intra_node_in_links  = DEFAULT_INTRA_NODE_IN_LINKS;
    intra_node_out_links = DEFAULT_INTRA_NODE_OUT_LINKS;

    inter_node_startup   = DEFAULT_INTER_NODE_STARTUP;
    inter_node_in_links  = DEFAULT_INTER_NODE_IN_LINKS;
    inter_node_out_links = DEFAULT_INTER_NODE_OUT_LINKS;

    wan_startup          = DEFAULT_WAN_STARTUP;
  }

  /*
  * El mÃ©todo defaultValues permite comprobar si la totalidad de nodos tienen
  * todos sus datos inicializados con los valores por defecto.
  *
  * @ret boolean: TRUE si todos tienen el valor por defecto, FALSE en otro
  *               caso.
  */
  public boolean defaultValues()
  {
    if(machine_id.equalsIgnoreCase(DEFAULT_MACHINE_ID) &&
       node_id.equalsIgnoreCase(DEFAULT_NODE_ID) &&
       architecture.equalsIgnoreCase(DEFAULT_ARCHITECTURE) &&
       processors.equalsIgnoreCase(DEFAULT_PROCESSORS) &&
       cpu_ratio.equalsIgnoreCase(DEFAULT_CPU_RATIO) &&
       intra_node_startup.equalsIgnoreCase(DEFAULT_INTRA_NODE_STARTUP) &&
       intra_node_bandwidth.equalsIgnoreCase(DEFAULT_INTRA_NODE_BANDWIDTH) &&
       intra_node_buses.equalsIgnoreCase(DEFAULT_INTRA_NODE_BUSES) &&
       intra_node_in_links.equalsIgnoreCase(DEFAULT_INTRA_NODE_IN_LINKS) &&
       intra_node_out_links.equalsIgnoreCase(DEFAULT_INTRA_NODE_OUT_LINKS) &&
       inter_node_startup.equalsIgnoreCase(DEFAULT_INTER_NODE_STARTUP) &&
       inter_node_in_links.equalsIgnoreCase(DEFAULT_INTER_NODE_IN_LINKS) &&
       inter_node_out_links.equalsIgnoreCase(DEFAULT_INTER_NODE_OUT_LINKS) &&
       wan_startup.equalsIgnoreCase(DEFAULT_WAN_STARTUP))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  /*
  * El mÃ©todo loadData almacena los datos facilitados en @line en la
  * estructura de datos de un nodo.
  *
  * @param: Â· String line -> lÃ­nea con todos los datos de un nodo.
  *         Â· boolean oldFile -> TRUE si los datos provienen de un fichero de
  *                              configuraciÃ³n antiguo (menos datos), FALSE
  *                              en otro caso.
  *
  * @exc: Valor numÃ©rico no vÃ¡lido.
  */
  public boolean loadData(String line, boolean oldFile, int lineCount) throws Exception
  {
    Pattern pattern = Pattern.compile("\"node information\" \\{(.*)\\};;$");
    Matcher matcher = pattern.matcher(line);

    if (!matcher.matches())
    {
      Tools.showErrorDialog("Wrong node information record");
      return false;
    }

    String fields = matcher.group(1);

    int first = Data.NODE.length();
    int second = line.indexOf(",",first);

    String[] nodeFields = fields.split(",");

    if (nodeFields.length != NODE_RECORD_FIELD_COUNT)
    {
      Tools.showErrorDialog("Wrong number of fields of node information record (line "+lineCount+").\nTry to update the CFG file");
      return false;
    }

    setMachine_id(Tools.blanks(nodeFields[0]));
    setNode_id(Tools.blanks(nodeFields[1]));
    setArchitecture(Tools.blanks(nodeFields[2]));
    setProcessors(Tools.blanks(nodeFields[3]));
    setCPURatio(Tools.blanks(nodeFields[4]));
    setIntraNodeStartup(Tools.blanks(nodeFields[5]));
    setIntraNodeBandwidth(Tools.blanks(nodeFields[6]));
    setIntraNodeBuses(Tools.blanks(nodeFields[7]));
    setIntraNodeInLinks(Tools.blanks(nodeFields[8]));
    setIntraNodeOutLinks(Tools.blanks(nodeFields[9]));
    setInterNodeStartup(Tools.blanks(nodeFields[10]));
    setInterNodeInLinks(Tools.blanks(nodeFields[11]));
    setInterNodeOutLinks(Tools.blanks(nodeFields[12]));
    setWANStartup(Tools.blanks(nodeFields[13]));

    /*
    System.out.println("Number of node fields = "+nodeFields.length);
    System.out.println("First field = "+nodeFields[0]);


    if(!oldFile)
    {
      setMachine_id(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
    }

    setNode_id(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setArchitecture(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setProcessors(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setMemBuses(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setMemInLinks(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setMemOutLinks(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setInput(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setOutput(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setLocal(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setRemote(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setSpeed(Tools.blanks(line.substring(first,second)));

    first = second + 1;

    if(!oldFile)
    {
      second = line.indexOf(",",first);
      setBandwidth(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf("}",first);
      setLatency(Tools.blanks(line.substring(first,second)));
    }
    else
    {
      second = line.indexOf("}",first);
      setBandwidth(Tools.blanks(line.substring(first,second)));
    }
    */

    return true;
  }

  /*
  * El mÃ©todo saveData genera, en un fichero de configuraciÃ³n @target, la
  * informaciÃ³n correspondiente a un nodo.
  *
  * @param: RandomAccessFile target -> fichero de configuraciÃ³n destino.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  public void saveData(RandomAccessFile target) throws IOException
  {
    target.writeBytes(Data.NODE);
    target.writeBytes(getMachine_id() + ", ");
    target.writeBytes(getNode_id() + ", ");
    target.writeBytes(getArchitecture(true) + ", ");
    target.writeBytes(getProcessors() + ", ");
    target.writeBytes(getCPURatio() + ", ");
    target.writeBytes(getIntraNodeStartup() + ", ");
    target.writeBytes(getIntraNodeBandwidth() + ", ");
    target.writeBytes(getIntraNodeBuses() + ", ");
    target.writeBytes(getIntraNodeInLinks() + ", ");
    target.writeBytes(getIntraNodeOutLinks() + ", ");
    target.writeBytes(getInterNodeStartup() + ", ");
    target.writeBytes(getInterNodeInLinks() + ", ");
    target.writeBytes(getInterNodeOutLinks() + ", ");
    target.writeBytes(getWANStartup() + "};;\n");
  }

  // MÃ©todos GET: permiten el acceso externo a los datos de un nodo.
  public String getMachine_id()
  {
    return machine_id;
  }

  public String getNode_id()
  {
    return node_id;
  }

  public String getArchitecture(boolean toDisk)
  {
    if(toDisk) // Si el dato va a disco --> nombre entre comillas.
    {
      return "\"" + architecture + "\"";
    }

    return architecture;
  }

  public String getProcessors()
  {
    return processors;
  }

  public String getCPURatio()
  {
    return cpu_ratio;
  }

  public String getIntraNodeStartup()
  {
    return intra_node_startup;
  }

  public String getIntraNodeBandwidth()
  {
    return intra_node_bandwidth;
  }

  public String getIntraNodeBuses()
  {
    return intra_node_buses;
  }

  public String getIntraNodeInLinks()
  {
    return intra_node_in_links;
  }

  public String getIntraNodeOutLinks()
  {
    return intra_node_out_links;
  }

  public String getInterNodeStartup()
  {
    return inter_node_startup;
  }

  public String getInterNodeInLinks()
  {
    return inter_node_in_links;
  }

  public String getInterNodeOutLinks()
  {
    return inter_node_out_links;
  }

  public String getWANStartup()
  {
    return wan_startup;
  }

  // MÃ©todos SET: permiten la modificaciÃ³n, de forma externa, de los datos de
  // un nodo.
  public void setMachine_id(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      machine_id = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("MACHINE ID");
        throw e;
      }
  }

  public void setNode_id(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      node_id = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("NODE ID");
        throw e;
      }
  }

  public void setArchitecture(String value)
  {
    if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
    {
      architecture = value.substring(1,value.length()-1);
    }
    else
    {
      architecture = value;
    }
  }

  public void setProcessors(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      processors = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("NUMBER OF PROCESSORS");
        throw e;
      }
  }

  public void setCPURatio(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      cpu_ratio = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("RELATIVE PROCESSOR SPEED");
        throw e;
      }
  }

  public void setIntraNodeStartup(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      intra_node_startup = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG INTRA-NODE COMMUNICATIONS STARTUP VALUE");
        throw e;
      }
  }

  public void setIntraNodeBandwidth(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      intra_node_bandwidth = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG INTRA-NODE COMMUNICATIONS BANDWIDTH VALUE");
        throw e;
      }
  }

  public void setIntraNodeBuses(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      intra_node_buses = value;
    }
    catch(NumberFormatException e)
    {
      Tools.showErrorMessage("WRONG INTRA-NODE COMMUNICATIONS BUSES VALUE");
      throw e;
    }
  }

  public void setIntraNodeInLinks(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      intra_node_in_links = value;
    }
    catch(NumberFormatException e)
    {
      Tools.showErrorMessage("WRONG INTRA-NODE COMMUNICATIONS INPUT LINKS VALUE");
      throw e;
    }
  }

  public void setIntraNodeOutLinks(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      intra_node_out_links = value;
    }
    catch(NumberFormatException e)
    {
      Tools.showErrorMessage("WRONG INTRA-NODE COMMUNICATIONS OUTPUT LINKS VALUE");
      throw e;
    }
  }

  public void setInterNodeStartup(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      inter_node_startup = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG INTER-NODE COMMUNICATIONS STARTUP VALUE");
        throw e;
      }
  }

  public void setInterNodeInLinks(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      inter_node_in_links = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG INTER-NODE COMMUNICATIONS INPUT LINKS VALUE");
        throw e;
      }
  }

  public void setInterNodeOutLinks(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      inter_node_out_links = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG INTER-NODE COMMUNICATIONS OUTPUT LINKS VALUE");
        throw e;
      }
  }

  public void setWANStartup(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      wan_startup = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG INTER-MACHINES (WAN) COMMUNICATIONS STARTUP VALUE");
        throw e;
      }
  }
}

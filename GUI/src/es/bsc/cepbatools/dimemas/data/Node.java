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
 * TÃ­tulo:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Juan Gonzalez Garcia
 * @version 1.0
 */

import es.bsc.cepbatools.dimemas.data.Data.*;
import es.bsc.cepbatools.dimemas.tools.*;
import java.io.*;

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
  
  
  static public final String DEFAULT_ACC_STARTUP		  = "0.0";
  static public final String DEFAULT_ACC_MEM_STARTUP	  = "0.0";
  static public final String DEFAULT_ACC_BANDWIDTH		  = "0.0";
  static public final String DEFAULT_ACC_BUSES			  = DEFAULT_PROCESSORS;
  static public final String DEFAULT_ACC_RATIO		  	  = "1.0";
  static public final Boolean DEFAULT_ACC				  = false;
  

  static public final int    NODE_RECORD_NO_INTRA_NODE_FIELD_COUNT = 11;
  static public final int    NODE_RECORD_WITH_NODE_ID_FIELD_COUNT  = 14;
  static public final int    NODE_RECORD_FIELD_COUNT               = 13;
  
  static public final int	 ACC_NODE_RECORD_FIELD_COUNT = 6;

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
  
  private Boolean acc;
  private String acc_startup;
  private String acc_mem_startup;
  private String acc_bandwidth;
  private String acc_buses;
  private String acc_ratio;

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
    machine_id           = DEFAULT_MACHINE_ID;
    node_id              = DEFAULT_NODE_ID;
    architecture         = DEFAULT_ARCHITECTURE;
    processors           = DEFAULT_PROCESSORS;
    cpu_ratio            = DEFAULT_CPU_RATIO;

    intra_node_startup   = DEFAULT_INTRA_NODE_STARTUP;
    intra_node_bandwidth = DEFAULT_INTRA_NODE_BANDWIDTH;
    intra_node_buses     = DEFAULT_INTRA_NODE_BUSES;
    intra_node_in_links  = DEFAULT_INTRA_NODE_IN_LINKS;
    intra_node_out_links = DEFAULT_INTRA_NODE_OUT_LINKS;

    inter_node_startup   = DEFAULT_INTER_NODE_STARTUP;
    inter_node_in_links  = DEFAULT_INTER_NODE_IN_LINKS;
    inter_node_out_links = DEFAULT_INTER_NODE_OUT_LINKS;

    wan_startup          = DEFAULT_WAN_STARTUP;
    
    acc					 = DEFAULT_ACC;
    acc_startup			 = DEFAULT_ACC_STARTUP;
    acc_mem_startup 	 = DEFAULT_ACC_MEM_STARTUP;
    acc_bandwidth		 = DEFAULT_ACC_BANDWIDTH;
    acc_buses			 = DEFAULT_ACC_BUSES;
    acc_ratio			 = DEFAULT_ACC_RATIO;
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
       wan_startup.equalsIgnoreCase(DEFAULT_WAN_STARTUP) &&
       
       acc.equals(DEFAULT_ACC)							 &&
       acc_startup.equalsIgnoreCase(DEFAULT_ACC_STARTUP) &&
       acc_mem_startup.equalsIgnoreCase(DEFAULT_ACC_MEM_STARTUP) &&
       acc_bandwidth.equalsIgnoreCase(DEFAULT_ACC_BANDWIDTH) &&
       acc_buses.equalsIgnoreCase(DEFAULT_ACC_BUSES) &&
       acc_ratio.equalsIgnoreCase(DEFAULT_ACC_RATIO))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  /*
   * Method to compare two objects
   */
  public boolean equals(Object obj)
  {
    // if the two objects are equal in reference, they are equal
    if (this == obj)
    {
      return true;
    }
    else if (obj instanceof Node) 
    {
      Node otherNode = (Node) obj;
      
      if ( this.machine_id.equals(otherNode.getMachine_id())                  &&
           this.architecture.equals(otherNode.getArchitecture(false))         &&
           this.processors.equals(otherNode.getProcessors())                  &&
           this.cpu_ratio.equals(otherNode.getCPURatio())                     &&
           this.intra_node_startup.equals(otherNode.getIntraNodeStartup())    &&
           this.intra_node_buses.equals(otherNode.getIntraNodeBuses())        &&
           this.intra_node_in_links.equals(otherNode.getIntraNodeInLinks())   &&
           this.intra_node_out_links.equals(otherNode.getIntraNodeOutLinks()) &&
           this.inter_node_startup.equals(otherNode.getInterNodeStartup())    &&
           this.inter_node_in_links.equals(otherNode.getInterNodeInLinks())   &&
           this.inter_node_out_links.equals(otherNode.getInterNodeOutLinks()) &&
           this.wan_startup.equals(otherNode.getWANStartup())				  &&
           this.acc.equals(otherNode.getAcc())								  &&
           this.acc_startup.equals(otherNode.getAccStartup())				  &&
           this.acc_mem_startup.equals(otherNode.getAccMemStartup())		  &&
           this.acc_bandwidth.equals(otherNode.getAccBandwidth())			  &&
           this.acc_buses.equals(otherNode.getAccBuses())					  &&
           this.acc_ratio.equals(otherNode.getAccRatio()))
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
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
    // Node ID is no longer used. It will be generated sequentially
    // target.writeBytes(getNode_id() + ", ");
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
  
  
  /*
   * Prints accelerator information for this node.
   * Node id needed for mapping accelerator tasks in these nodes
   */
  public void saveAccData(RandomAccessFile target) throws IOException
  {
	  target.writeBytes(Data.ACC_NODE);
	  target.writeBytes(getNode_id() + ", ");
	  target.writeBytes(getAccStartup() + ", ");
	  target.writeBytes(getAccMemStartup() + ", ");
	  target.writeBytes(getAccBandwidth() + ", ");
	  target.writeBytes(getAccBuses() + ", ");
	  target.writeBytes(getAccRatio() + "};;\n");
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
  
  public Boolean getAcc()
  {
	  return acc;
  }
  
  public String getAccStartup()
  {
	  return acc_startup;
  }
  
  public String getAccMemStartup()
  {
	  return acc_mem_startup;
  }
  
  public String getAccBandwidth()
  {
	  return acc_bandwidth;
  }
  
  public String getAccBuses()
  {
	  return acc_buses;
  }
  
  public String getAccRatio()
  {
	  return acc_ratio;
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
  
  public void setAcc(Boolean value) throws Exception
  {
	  try 
	  {
		acc = value;
	  } catch(ValueException e)
	  {
		Tools.showErrorMessage("WRONG ACCELERATOR NODE");
	    throw e;
	  }
  }
  
  public void setAccStartup(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      acc_startup = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG ACCELERATOR STARTUP VALUE");
        throw e;
      }
  }
  
  public void setAccMemStartup(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      acc_mem_startup = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG ACCELERATOR MEMORY STARTUP VALUE");
        throw e;
      }
  }
  
  public void setAccBandwidth(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      acc_bandwidth = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG ACCELERATOR BANDWIDTH VALUE");
        throw e;
      }
  }
  
  public void setAccBuses(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      acc_buses = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG ACCELERATOR BUSES VALUE");
        throw e;
      }
  }
  
  public void setAccRatio(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      acc_ratio = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("WRONG ACCELERATOR SPEED RATIO");
        throw e;
      }
  }
}

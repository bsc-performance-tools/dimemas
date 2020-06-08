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

import data.Data.*;
import tools.Tools;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/*
* Clase que albergará los datos correspondientes a NODES.
*/
public class NodeData
{
  private int    nNodes = 0; // Número de nodos.
  public  Node[] node;       // Nodos.
  
  private boolean  nodesCreated       = false;
  private int      lastNodeIdAssigned = 0;
  
  
  /**
  * Loads a single node definition record described in @line and stores it 
  * in the array of nodes
  *
  * @param line      ASCII string with data a description of the node
  * @param oldFile   true if record is defined in a previous version of the 
  *                  configuration file (less fields), false otherwise
  * @param lineCount number of line in the configuration file where the
  *                  record appear
  * 
  * @return          true if node was successfully loaded, false otherwise
  */
  public boolean loadSingleNodeData(String line, boolean oldFile, int lineCount) throws Exception
  {
    int nodeId;
    
    Pattern pattern = Pattern.compile("\"node information\" \\{(.*)\\};;$");
    Matcher matcher = pattern.matcher(line);

    if (!matcher.matches())
    {
      Tools.showErrorDialog("Wrong node information record");
      return false;
    }

    String   fields = matcher.group(1);
    String[] nodeFields = fields.split(",");
    
    if (nodeFields.length != Node.NODE_RECORD_NO_INTRA_NODE_FIELD_COUNT &&
        nodeFields.length != Node.NODE_RECORD_WITH_NODE_ID_FIELD_COUNT  &&
        nodeFields.length != Node.NODE_RECORD_FIELD_COUNT)
    {
      Tools.showErrorDialog("Wrong number of fields of node information record (line "+lineCount+").\nTry to update the CFG file");
      return false;
    }

    if (nodeFields.length == Node.NODE_RECORD_NO_INTRA_NODE_FIELD_COUNT ||
        nodeFields.length == Node.NODE_RECORD_WITH_NODE_ID_FIELD_COUNT)
    {
      try
      {
        nodeId = Integer.parseInt(Tools.blanks(nodeFields[1]));
        if (nodeId != lastNodeIdAssigned)
        {
          Tools.showErrorDialog("Wrong node identifier in node information record. "+
                                "Expected: '"+lastNodeIdAssigned+"' Read: '"+nodeId+"' "+
                                "(line "+lineCount+")");
          return false;
        }
      }
      catch(NumberFormatException e)
      {
        Tools.showErrorDialog("Wrong node identifier in node information record (line "+lineCount+")");
        return false;
      }
    }
    else
    {
      nodeId = lastNodeIdAssigned;
    }
    
    if (nodeId >= node.length)
    {
      Tools.showErrorDialog("More nodes defined than actual machines nodes (line "+lineCount+")");
      return false;
    }
    
    if (nodeFields.length == Node.NODE_RECORD_FIELD_COUNT)
    {
      node[nodeId].setMachine_id(Tools.blanks(nodeFields[0]));
      node[nodeId].setNode_id(String.valueOf(nodeId));
      node[nodeId].setArchitecture(Tools.blanks(nodeFields[1]));
      node[nodeId].setProcessors(Tools.blanks(nodeFields[2]));
      node[nodeId].setCPURatio(Tools.blanks(nodeFields[3]));
      node[nodeId].setIntraNodeStartup(Tools.blanks(nodeFields[4]));
      node[nodeId].setIntraNodeBandwidth(Tools.blanks(nodeFields[5]));
      node[nodeId].setIntraNodeBuses(Tools.blanks(nodeFields[6]));
      node[nodeId].setIntraNodeInLinks(Tools.blanks(nodeFields[7]));
      node[nodeId].setIntraNodeOutLinks(Tools.blanks(nodeFields[8]));
      node[nodeId].setInterNodeStartup(Tools.blanks(nodeFields[9]));
      node[nodeId].setInterNodeInLinks(Tools.blanks(nodeFields[10]));
      node[nodeId].setInterNodeOutLinks(Tools.blanks(nodeFields[11]));
      node[nodeId].setWANStartup(Tools.blanks(nodeFields[12]));
    }
    else if (nodeFields.length == Node.NODE_RECORD_WITH_NODE_ID_FIELD_COUNT)
    {
      node[nodeId].setMachine_id(Tools.blanks(nodeFields[0]));
      node[nodeId].setNode_id(Tools.blanks(nodeFields[1]));
      node[nodeId].setArchitecture(Tools.blanks(nodeFields[2]));
      node[nodeId].setProcessors(Tools.blanks(nodeFields[3]));
      node[nodeId].setCPURatio(Tools.blanks(nodeFields[4]));
      node[nodeId].setIntraNodeStartup(Tools.blanks(nodeFields[5]));
      node[nodeId].setIntraNodeBandwidth(Tools.blanks(nodeFields[6]));
      node[nodeId].setIntraNodeBuses(Tools.blanks(nodeFields[7]));
      node[nodeId].setIntraNodeInLinks(Tools.blanks(nodeFields[8]));
      node[nodeId].setIntraNodeOutLinks(Tools.blanks(nodeFields[9]));
      node[nodeId].setInterNodeStartup(Tools.blanks(nodeFields[10]));
      node[nodeId].setInterNodeInLinks(Tools.blanks(nodeFields[11]));
      node[nodeId].setInterNodeOutLinks(Tools.blanks(nodeFields[12]));
      node[nodeId].setWANStartup(Tools.blanks(nodeFields[13]));
    }
    else
    {
      node[nodeId].setMachine_id(Tools.blanks(nodeFields[0]));
      node[nodeId].setNode_id(Tools.blanks(nodeFields[1]));
      node[nodeId].setArchitecture(Tools.blanks(nodeFields[2]));
      node[nodeId].setProcessors(Tools.blanks(nodeFields[3]));
      node[nodeId].setInterNodeInLinks(Tools.blanks(nodeFields[4]));
      node[nodeId].setInterNodeOutLinks(Tools.blanks(nodeFields[5]));
      node[nodeId].setIntraNodeStartup(Tools.blanks(nodeFields[6]));
      node[nodeId].setInterNodeStartup(Tools.blanks(nodeFields[7]));
      node[nodeId].setCPURatio(Tools.blanks(nodeFields[8]));
      node[nodeId].setIntraNodeBandwidth(Tools.blanks(nodeFields[9]));
      node[nodeId].setWANStartup(Tools.blanks(nodeFields[10]));
      
      node[nodeId].setIntraNodeBuses(Tools.blanks(nodeFields[3])); // Same intra-node buses as processors
      node[nodeId].setIntraNodeInLinks("1");
      node[nodeId].setIntraNodeOutLinks("0");
      
      node[nodeId].setAcc(false);
    }
    
    lastNodeIdAssigned++;
    
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
  
 /**
  * Loads a multiple node definition record described in @line and * stores it 
  * in the array of nodes
  *
  * @param line       ASCII string with data a description of multiple nodes
  * @param lineCount  number of line in the configuration file where the
  *                   record appear
  * 
  * @return           true if nodes were successfully loaded, false otherwise
  */
  public boolean loadMultiNodeData(String line, int lineCount) throws Exception
  {
    Pattern pattern = Pattern.compile("\"multi node information\" \\{(.*)\\};;$");
    Matcher matcher = pattern.matcher(line);
    
    int numberOfNodes;
    
    if (!matcher.matches())
    {
      Tools.showErrorDialog("Wrong multi node information record");
      return false;
    }
    
    String   fields     = matcher.group(1);
    String[] nodeFields = fields.split(",");
    
    if (nodeFields.length == 14)
    {
      try
      {
        numberOfNodes = Integer.parseInt(Tools.blanks(nodeFields[1]));
      }
      catch (NumberFormatException e)
      {
        Tools.showErrorDialog("Incorrect number of nodes in multi node definition record");
        return false;
      }
      
      if (lastNodeIdAssigned + numberOfNodes > node.length)
      {
        Tools.showErrorDialog("Wrong number of nodes defined");
        return false;
      }
      
      for (int i = 0; i < numberOfNodes; i++)
      {
        node[lastNodeIdAssigned].setMachine_id(Tools.blanks(nodeFields[0]));
        node[lastNodeIdAssigned].setNode_id(String.valueOf(lastNodeIdAssigned));
        node[lastNodeIdAssigned].setArchitecture(Tools.blanks(nodeFields[2]));
        node[lastNodeIdAssigned].setProcessors(Tools.blanks(nodeFields[3]));
        node[lastNodeIdAssigned].setCPURatio(Tools.blanks(nodeFields[4]));
        node[lastNodeIdAssigned].setIntraNodeStartup(Tools.blanks(nodeFields[5]));
        node[lastNodeIdAssigned].setIntraNodeBandwidth(Tools.blanks(nodeFields[6]));
        node[lastNodeIdAssigned].setIntraNodeBuses(Tools.blanks(nodeFields[7]));
        node[lastNodeIdAssigned].setIntraNodeInLinks(Tools.blanks(nodeFields[8]));
        node[lastNodeIdAssigned].setIntraNodeOutLinks(Tools.blanks(nodeFields[9]));
        node[lastNodeIdAssigned].setInterNodeStartup(Tools.blanks(nodeFields[10]));
        node[lastNodeIdAssigned].setInterNodeInLinks(Tools.blanks(nodeFields[11]));
        node[lastNodeIdAssigned].setInterNodeOutLinks(Tools.blanks(nodeFields[12]));
        node[lastNodeIdAssigned].setWANStartup(Tools.blanks(nodeFields[13]));
        
        node[lastNodeIdAssigned].setAcc(false);
        
        lastNodeIdAssigned++;
      }
    }
    else
    {
      Tools.showErrorDialog("Wrong multi node information record");
      return false;
    }
    
    return true;
  }
  
  public boolean loadAccNodeData(String line, boolean oldFile, int lineCount) throws Exception
  {
    int nodeId;
    
    Pattern pattern = Pattern.compile("\"accelerator node information\" \\{(.*)\\};;$");
    Matcher matcher = pattern.matcher(line);

    if (!matcher.matches())
    {
      Tools.showErrorDialog("Wrong accelerator node information record");
      return false;
    }

    String   fields = matcher.group(1);
    String[] accNodeFields = fields.split(",");
    
    if (accNodeFields.length != Node.ACC_NODE_RECORD_FIELD_COUNT)
    {
      Tools.showErrorDialog("Wrong number of fields of accelerator node information record (line "+lineCount+").\nTry to update the CFG file");
      return false;
    }

    try
	{
	  nodeId = Integer.parseInt(Tools.blanks(accNodeFields[0]));
	  if (nodeId >= node.length)
	  {
	    Tools.showErrorDialog("The accelerator node does not exist");
	    return false;
	  }
    
      node[nodeId].setAcc(true);
      node[nodeId].setAccNumber(Tools.blanks(accNodeFields[1]));
      node[nodeId].setAccStartup(Tools.blanks(accNodeFields[2]));
      node[nodeId].setAccMemStartup(Tools.blanks(accNodeFields[3]));
      node[nodeId].setAccBandwidth(Tools.blanks(accNodeFields[4]));
      node[nodeId].setAccBuses(Tools.blanks(accNodeFields[5]));
      node[nodeId].setAccRatio(Tools.blanks(accNodeFields[6]));
      
    } catch(NumberFormatException e)
    {
    	Tools.showErrorDialog("Wrong accelerator node information record (line "+lineCount+")");
    	return false;
    }

    return true;
  }

  /**
  * Stores the information of the nodes defined in the target
  * configuration file, merging consecutive nodes with same definition
  * in single "multi node information" record
  *
  * @param  target configuration file to be written
  *
  */
  public void saveData(RandomAccessFile target) throws IOException
  {
    int currentNode  = 0;
    int previousNode = 0;
    int summarized   = 0;
    
    while (currentNode < node.length)
    {
      if (node[currentNode].equals(node[previousNode]))
      {
        summarized++;
        currentNode++;
      }
      else
      {
        if (summarized == 1)
        {
          node[previousNode].saveData(target);
        }
        else
        {
          target.writeBytes(Data.MULTINODE);
          target.writeBytes(node[previousNode].getMachine_id() + ", ");
          target.writeBytes(String.valueOf(summarized) + ", ");
          target.writeBytes(node[previousNode].getArchitecture(true) + ", ");
          target.writeBytes(node[previousNode].getProcessors() + ", ");
          target.writeBytes(node[previousNode].getCPURatio() + ", ");
          target.writeBytes(node[previousNode].getIntraNodeStartup() + ", ");
          target.writeBytes(node[previousNode].getIntraNodeBandwidth() + ", ");
          target.writeBytes(node[previousNode].getIntraNodeBuses() + ", ");
          target.writeBytes(node[previousNode].getIntraNodeInLinks() + ", ");
          target.writeBytes(node[previousNode].getIntraNodeOutLinks() + ", ");
          target.writeBytes(node[previousNode].getInterNodeStartup() + ", ");
          target.writeBytes(node[previousNode].getInterNodeInLinks() + ", ");
          target.writeBytes(node[previousNode].getInterNodeOutLinks() + ", ");
          target.writeBytes(node[previousNode].getWANStartup() + "};;\n");
        }
        
        previousNode = currentNode;
        summarized   = 0;
      }
    }
    
    if (summarized == 1)
    {
      node[previousNode].saveData(target);
    }
    else
    {
      target.writeBytes(Data.MULTINODE);
      target.writeBytes(node[previousNode].getMachine_id() + ", ");
      target.writeBytes(String.valueOf(summarized) + ", ");
      target.writeBytes(node[previousNode].getArchitecture(true) + ", ");
      target.writeBytes(node[previousNode].getProcessors() + ", ");
      target.writeBytes(node[previousNode].getCPURatio() + ", ");
      target.writeBytes(node[previousNode].getIntraNodeStartup() + ", ");
      target.writeBytes(node[previousNode].getIntraNodeBandwidth() + ", ");
      target.writeBytes(node[previousNode].getIntraNodeBuses() + ", ");
      target.writeBytes(node[previousNode].getIntraNodeInLinks() + ", ");
      target.writeBytes(node[previousNode].getIntraNodeOutLinks() + ", ");
      target.writeBytes(node[previousNode].getInterNodeStartup() + ", ");
      target.writeBytes(node[previousNode].getInterNodeInLinks() + ", ");
      target.writeBytes(node[previousNode].getInterNodeOutLinks() + ", ");
      target.writeBytes(node[previousNode].getWANStartup() + "};;\n");
    }
    
    /* The accelerator node information (if defined) is printed
     * after node information, for each node
     */
    currentNode  = 0;
    target.writeBytes("\n");
    while (currentNode < node.length)
    {
    	if (node[currentNode].getAcc())
    	{
    		node[currentNode].saveAccData(target);
    	}
    	currentNode++;
    }
    
  }
  
  // Método que permite acceder al número de nodos fuera de la clase.
  public int getNumberOfNodes()
  {
    return nNodes;
  }
  
  public int[] getCpusPerNode()
  {
    int[] Result = new int[nNodes];
    
    for (int i = 0; i < nNodes; i++)
    {
      Result[i] = Integer.parseInt(node[i].getProcessors());
    }
    
    return Result;
  }

  // Método que permite fijar el número de nodos desde fuera de la clase.
  public void setNumberOfNodes(int value)
  {
    nNodes = value;
  }

  /*
  * El método defaultValues permite comprobar si la totalidad de los nodos
  * tienen sus datos inicializados con los valores por defecto.
  *
  * @ret boolean: TRUE si todos tienen el valor por defecto, FALSE en otro caso.
  */
  public boolean defaultValues()
  {
    for(int i = 0; i < nNodes; i++)
    {
      if(!node[i].defaultValues())
      {
        return false;
      }
    }
    
    lastNodeIdAssigned = 0;

    return true;
  }

  /*
  * El método verifyArchitecture comprueba que la arquitectura de la máquina a
  * la que pertenecen los nodos no haya cambiado (una máquina y sus nodos tienen
  * la misma arquitectura), en caso de variación se debe actualizar la
  * arquitectura de los nodos y los datos relacionados. También comprueba que el
  * usuario no haya cambiado el ID de la máquina a la que pertenecen los nodos y
  * lo actualiza si hubiera cambiado.
  *
  * @param: · String oldId -> anterior identificador de la máquina.
  *         · String newId -> nuevo identificador de la máquina.
  *         · MachineDataBase mDB -> base de datos de máquinas/arquitecturas.
  *         · EnvironmentData env -> máquinas de la configuración actual.
  *         · int index -> índice de la máquina a la que pertenecen los nodos a
  *                        revisar/actualizar.
  *
  * @exc: Valor numérico no válido.
  */
  //public void verifyArchitecture(String oldId, String newId, MachineDataBase mDB,
  //  EnvironmentData env, int index) throws Exception
  public void verifyArchitecture(String oldId,
                                 String newId,
                                 EnvironmentData env,
                                 int index) throws Exception
  {
    for(int i = nNodes-1; i >= 0; i--)
    {
      if(node[i].getMachine_id().equalsIgnoreCase(oldId))
      {
        node[i].setMachine_id(newId);

        /*
        if(!node[i].getArchitecture(false).equalsIgnoreCase(env.machine[index].getNodeArchitecture()))
        {
          node[i].setArchitecture(mDB.machine[env.machine[index].getIndex()].getLabel());
          node[i].setProcessors(mDB.machine[env.machine[index].getIndex()].getProcessors());
          node[i].setInput(mDB.machine[env.machine[index].getIndex()].getInputLinks());
          node[i].setOutput(mDB.machine[env.machine[index].getIndex()].getOutputLinks());
          node[i].setLocal(mDB.machine[env.machine[index].getIndex()].getLocalStartup());
          node[i].setRemote(mDB.machine[env.machine[index].getIndex()].getRemoteStartup());
          node[i].setBandwidth(mDB.machine[env.machine[index].getIndex()].getDataTransferRate());
        }
        */
      }
    }
  }

  /*
  * El método createNodes genera las estructuras que contendrán los datos de los
  * nodos.
  *
  * @param: · EnvironmentData env -> máquinas de la configuración actual.
  *         · MachineDataBase.Machine[] mDB -> máquina predefinida de la que se
  *                                            obtendrán los datos necesarios.
  *
  * @exc: Valor numérico no válido.
  */
  // public void createNodes(EnvironmentData env, MachineDataBase.Machine[] mDB) throws Exception
  public void createNodes(EnvironmentData env) throws Exception
  {
    if (!nodesCreated)
    {
      node    = new Node[nNodes];
      int aux = nNodes-1;

      for(int i = env.getNumberOfMachines()-1; i >= 0; i--)
      {
        for(int j = Integer.parseInt(env.machine[i].getNodes())-1; j >= 0; j--)
        {
          node[aux] = new Node(String.valueOf(aux),env.machine[i].getId());

          /*
          if(env.machine[i].getIndex() != -1)
          { // Datos de la arquitectura de la máquina a la que pertenece el nodo.
            node[aux].setArchitecture(mDB[env.machine[i].getIndex()].getLabel());
            node[aux].setProcessors(mDB[env.machine[i].getIndex()].getProcessors());
            node[aux].setInput(mDB[env.machine[i].getIndex()].getInputLinks());
            node[aux].setOutput(mDB[env.machine[i].getIndex()].getOutputLinks());
            node[aux].setLocal(mDB[env.machine[i].getIndex()].getLocalStartup());
            node[aux].setRemote(mDB[env.machine[i].getIndex()].getRemoteStartup());
            node[aux].setBandwidth(mDB[env.machine[i].getIndex()].getDataTransferRate());
          }
          */

          aux--;
        }
      }
      
      nodesCreated = true;
    }
  }

  // Método que borra los datos de los nodos.
  public void eraseNodeInfo()
  {
    for(int i = nNodes-1; i >= 0; i--)
    {
      node[i].initialValues();
    }
  }

  // Método que borra los nodos existentes.
  public void destroyNodes()
  {
    for(int i = nNodes-1; i >= 0; i--)
    {
      node[i] = null;
    }

    nNodes             = 0;
    node               = null;
    nodesCreated       = false;
    lastNodeIdAssigned = 0;
    System.gc();
  }

  /*
  * El método changeAtNodes comprueba que no haya cambiado el número de nodos,
  * lo que afectaría a las estructuras. En caso de variación, crearía las
  * estructuras adecuadas para albergar el nuevo número de nodos, preservando
  * los datos de las nodos existentes hasta ese instante (si hubiera alguno).
  *
  * @param: · EnvironmentData env -> máquinas de la configuración actual.
  *         · MachineDataBase.Machine[] mDB -> máquina predefinida de la que se
  *                                            obtendrán los datos necesarios.
  *
  * @exc: Valor numérico no válido.
  */
  //public void changeAtNodes(EnvironmentData env, MachineDataBase.Machine[] mDB) throws Exception
  public void changeAtNodes(EnvironmentData env) throws Exception
  {
    int k = nNodes-1;
    Node[] aux = new Node[nNodes];

    // Inicialización de cada uno de los nuevos nodos.
    for(int i = env.getNumberOfMachines()-1; i >= 0; i--)
    {
      for(int j = Integer.parseInt(env.machine[i].getNodes())-1; j >= 0; j--)
      {
        aux[k] = new Node(Integer.toString(k),env.machine[i].getId());
        k--;
      }
    }

    int l = 0;

    if(nNodes >= node.length)     // #nodos nuevos >= #nodos existentes.
    {
      for(int i = 0; i < nNodes; i++)
      {
        // Conservar los datos de un nodo existente.
        if((l < node.length) && (aux[i].getMachine_id().equalsIgnoreCase(node[l].getMachine_id())))
        {
          aux[i].setArchitecture(node[l].getArchitecture(false));
          aux[i].setProcessors(node[l].getProcessors());
          aux[i].setCPURatio(node[l].getCPURatio());
          aux[i].setIntraNodeStartup(node[l].getIntraNodeStartup());
          aux[i].setIntraNodeBandwidth(node[l].getIntraNodeBandwidth());
          aux[i].setIntraNodeInLinks(node[l].getIntraNodeInLinks());
          aux[i].setIntraNodeOutLinks(node[l].getIntraNodeOutLinks());
          aux[i].setInterNodeStartup(node[l].getInterNodeStartup());
          aux[i].setInterNodeInLinks(node[l].getInterNodeInLinks());
          aux[i].setInterNodeOutLinks(node[l].getInterNodeOutLinks());
          aux[i].setWANStartup(node[l].getWANStartup());
          
          aux[i].setAcc(node[l].getAcc());
          aux[i].setAccNumber(node[l].getAccNumber());
          aux[i].setAccStartup(node[l].getAccStartup());
          aux[i].setAccMemStartup(node[l].getAccMemStartup());
          aux[i].setAccBandwidth(node[l].getAccBandwidth());
          aux[i].setAccBuses(node[l].getAccBuses());
          aux[i].setAccRatio(node[l].getAccRatio());
          
          l++;
        }
        else // Nodo de máquina nuevo.
        {
          for(int j = env.getNumberOfMachines()-1; j >= 0; j--)
          {
            /*
            if(env.machine[j].getId().equalsIgnoreCase(aux[i].getMachine_id())
               && (env.machine[j].getIndex() != -1))
            {
              aux[i].setArchitecture(mDB[env.machine[j].getIndex()].getLabel());
              aux[i].setProcessors(mDB[env.machine[j].getIndex()].getProcessors());
              aux[i].setInput(mDB[env.machine[j].getIndex()].getInputLinks());
              aux[i].setOutput(mDB[env.machine[j].getIndex()].getOutputLinks());
              aux[i].setLocal(mDB[env.machine[j].getIndex()].getLocalStartup());
              aux[i].setRemote(mDB[env.machine[j].getIndex()].getRemoteStartup());
              aux[i].setBandwidth(mDB[env.machine[j].getIndex()].getDataTransferRate());
              break;
            }
            */
          }
        }
      }
    }
    else                          // #nodos nuevos < #nodos existentes.
    {
      for(int i = 0; i < node.length; i++)
      {
        if(node[i].getMachine_id().equalsIgnoreCase(aux[l].getMachine_id()))
        {
          aux[l].setArchitecture(node[i].getArchitecture(false));
          aux[l].setProcessors(node[i].getProcessors());
          aux[l].setCPURatio(node[i].getCPURatio());
          aux[l].setIntraNodeStartup(node[i].getIntraNodeStartup());
          aux[l].setIntraNodeBandwidth(node[i].getIntraNodeBandwidth());
          aux[l].setIntraNodeInLinks(node[i].getIntraNodeInLinks());
          aux[l].setIntraNodeOutLinks(node[i].getIntraNodeOutLinks());
          aux[l].setInterNodeStartup(node[i].getInterNodeStartup());
          aux[l].setInterNodeInLinks(node[i].getInterNodeInLinks());
          aux[l].setInterNodeOutLinks(node[i].getInterNodeOutLinks());
          aux[l].setWANStartup(node[i].getWANStartup());
          
          aux[l].setAcc(node[i].getAcc());
          aux[l].setAccNumber(node[i].getAccNumber());
          aux[l].setAccStartup(node[i].getAccStartup());
          aux[l].setAccMemStartup(node[i].getAccMemStartup());
          aux[l].setAccBandwidth(node[i].getAccBandwidth());
          aux[l].setAccBuses(node[i].getAccBuses());
          aux[l].setAccRatio(node[i].getAccRatio());
          
          l++;
        }
      }
    }

    node = null;
    node = aux;
    aux = null;
    System.gc();
  }
}

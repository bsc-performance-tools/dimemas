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
import tools.*;
import java.io.*;

/*
* Clase que albergará los datos correspondientes a NODES.
*/
public class NodeData
{
  private int nNodes = 0; // Número de nodos.
  public Node[] node;     // Nodos.

  // Método que permite acceder al número de nodos fuera de la clase.
  public int getNumberOfNodes()
  {
    return nNodes;
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
  public void verifyArchitecture(String oldId, String newId, EnvironmentData env, int index) throws Exception
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
    node = new Node[nNodes];
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

    nNodes = 0;
    node = null;
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
          aux[i].setInput(node[l].getInput());
          aux[i].setOutput(node[l].getOutput());
          aux[i].setLocal(node[l].getLocal());
          aux[i].setRemote(node[l].getRemote());
          aux[i].setSpeed(node[l].getSpeed());
          aux[i].setBandwidth(node[l].getBandwidth());
          aux[i].setLatency(node[l].getLatency());
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
          aux[l].setInput(node[i].getInput());
          aux[l].setOutput(node[i].getOutput());
          aux[l].setLocal(node[i].getLocal());
          aux[l].setRemote(node[i].getRemote());
          aux[l].setSpeed(node[i].getSpeed());
          aux[l].setBandwidth(node[i].getBandwidth());
          aux[l].setLatency(node[i].getLatency());
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

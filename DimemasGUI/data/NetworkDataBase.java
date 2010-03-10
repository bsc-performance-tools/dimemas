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
import java.awt.*;
import java.io.*;
import javax.swing.*;

/*
* Clase que albergará los datos correspondientes a las redes WAN predefinidas.
*/
public class NetworkDataBase
{
  static private final String NETWORK_DB = "network : ";

  private int nNetworks = 0; // Número de redes predefinidas.
  public Network[] net;      // Redes.

  // Constructor de la clase NetworkDataBase.
  public NetworkDataBase()
  {
    loadDB();
  }

  // Método que permite acceder al número de redes fuera de la clase.
  public int getNumberOfNetworksInDB()
  {
    return nNetworks;
  }

  /*
  * El método addElement permite añadir una nueva definición de red a la base de
  * datos con las especificaciones dadas en @info[].
  *
  * @param: String[] info -> listado con las propiedades de la nueva red.
  *
  * @exc: Valor numérico no válido.
  */
  public void addElement(String[] info) throws Exception
  {
    nNetworks++;
    Network[] aux = new Network[nNetworks];

    try
    {
      // Se añade el nuevo elemento (en último lugar).
      aux[nNetworks-1] = new Network();
      aux[nNetworks-1].setName(info[0]);
      aux[nNetworks-1].setLabel(info[1]);
      aux[nNetworks-1].setBandwidth(info[2]);
      aux[nNetworks-1].setBuses(info[3]);
    } catch(Exception e)
      {
        nNetworks--;
        aux = null;
        System.gc();
        throw e;
      }
    // Si todo ha ido bien, copiar el resto de la base de datos.
    for(int i = 0; i < nNetworks - 1; i++)
    {
      aux[i] = net[i];
      net[i] = null;
    }

    net = null;
    net = aux;
    System.gc();
    saveDB();
  }

  /*
  * El método deleteElement permite eliminar una red de la base de datos.
  *
  * @param: int index -> posición de la red a borrar.
  */
  public void deleteElement(int index)
  {
    nNetworks--;
    int i = nNetworks;
    int j = i-1;
    Network[] aux = new Network[nNetworks];

    while(i >= 0)
    {
      if(i != index)
      {
        aux[j] = net[i];
        j--;
      }

      net[i] = null;
      i--;
    }

    net = null;
    net = aux;
    System.gc();
    saveDB();
  }

  // Método que carga en memoria la base de datos de redes predefinidas.
  public void loadDB()
  {
    int first = 0;
    int second = 0;
    String line;
    RandomAccessFile source;

    try
    {
      source = new RandomAccessFile(new File(Data.NETWORK_DB_FILE),"r");
      nNetworks = 0;

      // Adquirir el número de redes definidas.
      while(source.getFilePointer() != source.length())
      {
        if(source.readLine().startsWith(NETWORK_DB))
        {
          nNetworks++;
        }
      }

      source.seek(0);
      net = new Network[nNetworks];

      // Carga de los datos desde el fichero a memoria.
      for(int i = 0; i < nNetworks; i++)
      {
        net[i] = new Network();
        line = source.readLine();
        first = NETWORK_DB.length();
        second = line.indexOf(" ", first);
        net[i].setLabel(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(" ", first);
        net[i].setName(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(" ", first);
        net[i].setBandwidth(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.length();
        net[i].setBuses(Tools.blanks(line.substring(first,second)));
      }

      source.close();
    } catch(Throwable exc)
      {
        Tools.showInformationMessage(exc.toString());
      }
  }

  // Método que salva en fichero la base de datos existente en memoria.
  public void saveDB()
  {
    RandomAccessFile target;
    File file = new File(Data.NETWORK_DB_FILE);

    try
    {
      target = new RandomAccessFile(file,"rw");
      target.setLength(0);

      for(int i = 0; i < nNetworks; i++)
      {
        target.writeBytes(NETWORK_DB + net[i].getLabel() + " " + net[i].getName()
                          + " " + net[i].getBandwidth() + " " + net[i].getBuses()
                          + "\n");
      }

      target.close();
    } catch(Throwable exc)
      {
        Tools.showInformationMessage(exc.toString());
      }
  }

  /*
  * La clase Network provee la información de una red predefinida.
  */
  public class Network
  {
    private String name;
    private String label;
    private String bandwidth;
    private String buses;

    // Métodos GET: permiten el acceso externo a los datos de una red.
    public String getName()
    {
      return name;
    }

    public String getLabel()
    {
      return label;
    }

    public String getBandwidth()
    {
      return bandwidth;
    }

    public String getBuses()
    {
      return buses;
    }

    // Métodos SET: permiten la modificación, de forma externa, de los datos de
    // una red predefinida.
    public void setName(String value)
    {
      if(!value.equalsIgnoreCase(" "))
      {
        name = Tools.blanks(value).replace(' ','_'); // Reemplazar espacios.
      }
      else
      {
        name = "_";
      }
    }

    public void setLabel(String value)
    {
      if(!value.equalsIgnoreCase(" "))
      {
        label = Tools.blanks(value).replace(' ','_'); // Reemplazar espacios.
      }
      else
      {
        label = "_";
      }
    }

    public void setBandwidth(String value) throws Exception
    {
      try
      {
        Double.parseDouble(value);
        bandwidth = Tools.filterForDouble(value);
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("BANDWIDTH");
          throw e;
        }
    }

    public void setBuses(String value) throws Exception
    {
      try
      {
        Integer.parseInt(value);
        buses = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("BUSES");
          throw e;
        }
    }
  }
}

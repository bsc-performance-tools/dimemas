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
import java.io.*;

/*
* Clase que albergará los datos correspondientes a BLOCK FACTORS.
*/
public class BlockData
{
  static public final String DEFAULT_VALUE = "1.0";
  static public final String BLOCK_LINE = "\"block definition\" {";

  private int nBlocks = 0;      // Número de BLOCK FACTORS.
  public Definition[] factors;  // BLOCK FACTORS.

  // Método que permite acceder al número de BLOCK FACTORS fuera de la clase.
  public int getNumberOfBlocks()
  {
    return nBlocks;
  }

  // Método que elimina la info. existente para poder albergar nuevos datos.
  public void destroyFactors()
  {
    for(int i = nBlocks-1; i >= 0; i--)
    {
      factors[i] = null;
    }

    nBlocks = 0;
    factors = null;
    System.gc();
  }

  /*
  * El método createFactors permitirá obtener el número de BLOCK FACTORS así
  * como parte de la información correspondiente a partir de @filename (fichero
  * de trazas).
  *
  * @param: String filename -> nombre del fichero de trazas actualmente en uso.
  */
  public void createFactors(String filename)
  {
    try
    {
      long seekPoint = 0;
      String line;
      RandomAccessFile source = new RandomAccessFile(new File(filename),"r");

      // Recorrido del fichero de trazas para obtener el núm. de BLOCK FACTORS.
      while(source.getFilePointer() != source.length())
      {
        line = Tools.blanks(source.readLine());

        if(!line.equalsIgnoreCase(""))
        {
          if(line.startsWith("#"))
          {
            line = splitLine(line,source);
            seekPoint = source.getFilePointer();
          }
          else if(line.startsWith(BLOCK_LINE))
          {
            line = splitLine(line,source);
            nBlocks++;
          }
          else if(line.startsWith("\"CPU burst\""))
          {
            break;
          }
        }
      }

      factors = new Definition[nBlocks];
      source.seek(seekPoint);
      int i = 0;
      int first = 0;
      int second = 0;

      // Obtención de los datos de los BLOCK FACTORS (nombre e identificador).
      while((i < nBlocks) && (source.getFilePointer() != source.length()))
      {
        line = Tools.blanks(source.readLine());

        if(line.startsWith(BLOCK_LINE))
        {
          line = splitLine(line,source);
          factors[i] = new Definition();
          first = line.indexOf("{")+1;
          second = line.indexOf(",",first);
          factors[i].setId(Tools.blanks(line.substring(first,second)));
          first = second+1;
          second = line.indexOf(",",first);
          factors[i].setName(Tools.blanks(line.substring(first,second)));
          i++;
        }
      }

      source.close();
    } catch(FileNotFoundException fne)
      {
        Tools.showInformationMessage(fne.toString());
      }
      catch(IOException ioe)
      {
        Tools.showInformationMessage(ioe.toString());
      }
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
  * El método loadData permite obtener el valor asociado a un BLOCK FACTOR
  * determinado, a partir de una cadena de caracteres @line extraida de un
  * fichero de configuración.
  *
  * @param: String line -> línea que contiene la tupla [id,valor] de un BLOCK
  *                         FACTOR.
  *
  * @exc: Valor numérico no válido.
  */
  public void loadData(String line) throws Exception
  {
    String id = Tools.blanks(line.substring(line.indexOf("{")+1,line.indexOf(",")));
    String value = Tools.blanks(line.substring(line.indexOf(",")+1,line.indexOf("}")));

    // Búsqueda del BLOCK FACTOR con el id. correspondiente.
    for(int i = 0; i < nBlocks; i++)
    {
      if(factors[i].getId().equalsIgnoreCase(id))
      {
        factors[i].setValue(value);
        break;
      }
    }
  }

  /*
  * El método saveData permite generar, en un fichero de configuración @target,
  * la información (modules information) de aquellos BLOCK FACTORS que tengan
  * asociado un valor diferente al valor por defecto.
  *
  * @param: RandomAccessFile target -> fichero de configuración destino.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  public void saveData(RandomAccessFile target) throws IOException
  {
    for(int i = 0; i < nBlocks; i++)
    {
      // Composición de la línea "modules information {id , valor};;".
      if(!factors[i].getValue().equalsIgnoreCase(DEFAULT_VALUE))
      {
        target.writeBytes(Data.MODULE);
        target.writeBytes(factors[i].getId() + ", ");
        target.writeBytes(factors[i].getValue() + "};;\n");
      }
    }
  }

  /*
  * El método defaultValues permite comprobar si la totalidad de BLOCK FACTORS
  * tienen asociado el valor por defecto en sus datos.
  *
  * @ret boolean: TRUE si todos tienen el valor por defecto, FALSE en otro caso.
  */
  public boolean defaultValues()
  {
    for(int i = 0; i < nBlocks; i++)
    {
      if(!factors[i].value.equalsIgnoreCase(DEFAULT_VALUE))
      {
        return false;
      }
    }

    return true;
  }

  /*
  * La clase Definition provee la información de un BLOCK FACTOR determinado:
  * Nombre, identificador y valor asociado.
  */
  public class Definition
  {
    private String name;   // Descripción del BLOCK FACTOR.
    private String id;     // Identificador del BLOCK FACTOR.
    private String value;  // Valor asociado al BLOCK FACTOR.

    // Constructor de la clase Definition.
    public Definition()
    {
      value = DEFAULT_VALUE;
    }

    // Métodos GET: Permiten el acceso externo a los datos de un BLOCK FACTOR.
    public String getName()
    {
      return name;
    }

    public String getId()
    {
      return id;
    }

    public String getValue()
    {
      return value;
    }

    // Métodos SET: Permiten la modificación de los datos de un BLOCK FACTOR.
    public void setName(String value)
    {
      name = value;
    }

    public void setId(String value)
    {
      try
      {
        Integer.parseInt(value);
        id = value;
      } catch(NumberFormatException e)
        {
          Tools.showInformationMessage(e.toString());
        }
    }

    public void setValue(String number) throws Exception
    {
      try
      {
        Double.parseDouble(number);
        value = Tools.filterForDouble(number);
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("Wrong value at :" + name);
          throw e;
        }
    }
  }
}

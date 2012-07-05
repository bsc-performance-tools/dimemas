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
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;
import java.util.HashSet;
import java.util.NoSuchElementException;


/*
* Clase que albergará los datos correspondientes a BLOCK FACTORS.
*/
public class BlockData
{
  private HashSet<ModuleInformation>     ModuleRatios = new HashSet<ModuleInformation>();
  private Iterator<ModuleInformation>    ExternalIterator;
  private ModuleInformation              ExternalModuleInformation;

  public BlockData()
  {
  }

  public int getNumberOfBlocks()
  {
    return ModuleRatios.size();
  }

  public boolean defaultValues()
  {
    if (ModuleRatios.size() == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // Método que elimina la info. existente para poder albergar nuevos datos.
  public void destroyFactors()
  {
    ModuleRatios.clear();
    System.gc();
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
  * JGG (2012/03/30): Updated! Now, block ratios are expressed as
  *                   'type value ratio'
  *
  * El método loadData permite obtener el valor asociado a un BLOCK FACTOR
  * determinado, a partir de una cadena de caracteres @line extraida de un
  * fichero de configuración.
  *
  * @param: String line -> línea que contiene la tupla [type,value,valor] de un
  *                        BLOCK FACTOR.
  *
  * @exc: Valor numérico no válido.
  */
  public void loadData(String line) throws Exception
  {
    String            type, value, ratio;
    int               first, second;
    ModuleInformation NewModuleInformation = new ModuleInformation();

    first  = line.indexOf("{")+1;
    second = line.indexOf(",",first);
    type   = Tools.blanks(line.substring(first,second));

    first  = second + 1;
    second = line.indexOf(",",first);

    /* Check if it is an old format */
    if (second == -1)
    {
      Tools.showInformationMessage("Old configuration file, modules information will not be extracted.");
      return;
    }

    value  = Tools.blanks(line.substring(first,second));


    first  = second + 1;
    second = line.indexOf("}",first);
    ratio  = Tools.blanks(line.substring(first,second));

    NewModuleInformation.setType(type);
    NewModuleInformation.setValue(value);
    NewModuleInformation.setRatio(ratio);

    if ( !ModuleRatios.contains(NewModuleInformation) )
    {
      ModuleRatios.add(NewModuleInformation);
    }
    else
    {
      Tools.showInformationMessage("Duplicated block "+NewModuleInformation+" not updated");
      return;
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
    /* JGG (2012/03/30): UPDATE! "modules information { type, value, ratio };; */

    Iterator<ModuleInformation> it = ModuleRatios.iterator();

    while ( it.hasNext() )
    {
      ModuleInformation CurrentModuleInformation = it.next();

      target.writeBytes(Data.MODULE);
      target.writeBytes(" "+CurrentModuleInformation.getType() + ", ");
      target.writeBytes(CurrentModuleInformation.getValue() + ", ");
      target.writeBytes(CurrentModuleInformation.getRatio() + " };;\n");
    }
  }


  public void addModuleRatio(String type, String value, String ratio)
  {
    ModuleInformation NewModuleInformation = new ModuleInformation();

    NewModuleInformation.setType(type);
    NewModuleInformation.setValue(value);
    NewModuleInformation.setRatio(ratio);

    if ( !ModuleRatios.contains(NewModuleInformation) )
    {
      ModuleRatios.add(NewModuleInformation);
    }
    else
    {
      Tools.showInformationMessage("Duplicated block "+NewModuleInformation+" not updated");
      return;
    }
  }

  public void startIterator()
  {
    ExternalIterator = ModuleRatios.iterator();
  }

  public boolean hasNext()
  {
    boolean result = ExternalIterator.hasNext();
    if (result)
    {
      ExternalModuleInformation = ExternalIterator.next();
    }
    else
    {
      ExternalModuleInformation = null;
    }

    return result;
  }

  public String getType()
  {
    if (ExternalModuleInformation != null)
    {
      return ExternalModuleInformation.getType();
    }
    else
    {
      throw new NoSuchElementException();
    }
  }

  public String getValue()
  {
    if (ExternalModuleInformation != null)
    {
      return ExternalModuleInformation.getValue();
    }
    else
    {
      throw new NoSuchElementException();
    }
  }

  public String getRatio()
  {
    if (ExternalModuleInformation != null)
    {
      return ExternalModuleInformation.getRatio();
    }
    else
    {
      throw new NoSuchElementException();
    }
  }

    /*
   * JGG: External container for module ratios
   */

  public class ModuleInformation
  {
    private int    type;
    private int    value;
    private double ratio;

    public ModuleInformation()
    {
      type  = 0;
      value = 0;
      ratio = 0;
    }

    public void setType(String type)
    {
      try
      {
        this.type = Integer.parseInt(type);
      }
      catch(NumberFormatException e)
      {
        Tools.showInformationMessage("Wrong format of 'Type' field (" + e.toString()+")");
      }
    }

    public void setValue(String value)
    {
      try
      {
        this.value = Integer.parseInt(value);
      }
      catch(NumberFormatException e)
      {
        Tools.showInformationMessage("Wrong format of 'Value' field ("+e.toString()+")");
      }
    }

    public void setRatio(String ratio)
    {
      try
      {
        this.ratio = Double.parseDouble(ratio);
      }
      catch(NumberFormatException e)
      {
        Tools.showInformationMessage("Wrong format of 'Ratio' field ("+e.toString()+")");
      }
    }

    public int hashCode()
    {
      return (type + value) * value + type;
    }

    public boolean equals(Object other)
    {
      if (other instanceof ModuleInformation)
      {
        ModuleInformation otherModuleInformation = (ModuleInformation) other;

        if ( (this.type  == otherModuleInformation.type) &&
             (this.value == otherModuleInformation.value))
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

    public String toString()
    {
      return "(" + type + ", " + value + ", " + ratio + ")";
    }

    public String getType()
    {
      return Integer.toString(type);
    }

    public String getValue()
    {
      return Integer.toString(value);
    }

    public String getRatio()
    {
      return Double.toString(ratio);
    }

  }
}

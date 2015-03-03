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

package es.bsc.cepbatools.dimemas.tools;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import org.apache.commons.io.FilenameUtils;
import java.awt.*;
import javax.swing.*;

public class Tools
{
  // Selector de ficheros utilizado en las operaciones "Load/Save"
  static public JFileChooser fc = new JFileChooser(System.getProperty("user.dir"));

  // Filtro para el selector de ficheros: archivos con extensión ".cfg"
  static public class CFGfilter extends javax.swing.filechooser.FileFilter
  {
    public boolean accept(java.io.File file)
    {
      if (file.isDirectory())
      {
        return true;
      }

      return FilenameUtils.getExtension(file.getPath()).equals("cfg");
    }

    public String getDescription()
    {
      return "CFG files (*.cfg)";
    }
  }


  // Filtro para el selector de ficheros: archivos con extensión ".out"
  static public class OUTfilter extends javax.swing.filechooser.FileFilter
  {
    public boolean accept(java.io.File file)
    {
      if (file.isDirectory())
      {
        return true;
      }

      return FilenameUtils.getExtension(file.getPath()).equals("out");

    }

    public String getDescription()
    {
      return "OUT files (*.out)";
    }
  }

  // Filtro para el selector de ficheros: archivos con extensión ".opt"
  static public class OPTfilter extends javax.swing.filechooser.FileFilter
  {
    public boolean accept(java.io.File file)
    {
      if (file.isDirectory())
      {
        return true;
      }

      return FilenameUtils.getExtension(file.getPath()).equals("opt");
    }

    public String getDescription()
    {
      return "OPT files (*.opt)";
    }
  }

  // Filtro para el selector de ficheros: archivos con extensión ".dim"
  static public class DIMfilter extends javax.swing.filechooser.FileFilter
  {
    public boolean accept(java.io.File file)
    {
      if (file.isDirectory())
      {
        return true;
      }

      return FilenameUtils.getExtension(file.getPath()).equals("dim");
    }

    public String getDescription()
    {
      return "DIM files (*.dim)";
    }
  }

  /*
  * La clase showConfirmationMessage provee una ventana de confirmación con las
  * opciones OK/CANCEL y el mensaje "@text", donde @text es el parámetro
  * facilitado.
  *
  * @param: String text --> mensaje que se mostrará en la ventana.
  */
  static public boolean showConfirmationMessage(String text)
  {
    int result = JOptionPane.showConfirmDialog((Component)null,text,
                 "Confirmation",JOptionPane.OK_CANCEL_OPTION);

    if(result == JOptionPane.OK_OPTION)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  /*
  * La clase showErrorMessage provee una ventana con el siguiente mensaje "Wrong
  * value at @text.", donde @text es el parámetro facilitado.
  *
  * @param: String text -> nombre del campo que contiene el valor no válido.
  */
  static public void showErrorMessage(String text)
  {
    JOptionPane.showMessageDialog((Component)null,
    "Wrong value at " + text + ".","Error",JOptionPane.ERROR_MESSAGE);
  }

  /*
  * La clase showErrorDialog provee una ventana con el mensaje @text.
  *
  * @param: String text -> nombre del campo que contiene el valor no válido.
  */
  static public void showErrorDialog(String text)
  {
    JOptionPane.showMessageDialog((Component) null,
    text,"Error",JOptionPane.ERROR_MESSAGE);
  }

  /*
  * La clase showWarningMessage provee una ventana con el siguiente mensaje
  * "@text is not set.", donde @text es el parámetro facilitado.
  *
  * @param: String text -> nombre del campo que no ha sido configurado.
  */
  static public void showWarningMessage(String text)
  {
    JOptionPane.showMessageDialog((Component)null,text + " is not set.",
    "Warning",JOptionPane.WARNING_MESSAGE);
  }

  /*
  * La clase showInformationMessage provee una ventana con el mensaje dado.
  *
  * @param: String text -> nombre del campo que no ha sido configurado.
  */
  static public void showInformationMessage(String text)
  {
    JOptionPane.showMessageDialog((Component)null,text,"Information",
    JOptionPane.INFORMATION_MESSAGE);
  }

  /*
  * La clase verifyData comprueba que todos los elementos de la lista @data
  * sean números enteros.
  *
  * @param: · String data -> lista de datos (elementos separados por comas).
  *         · String name -> nombre del campo al que pertenece la lista dada.
  *
  * @ret int: Devuelve el número de elementos de la lista facilitada, -1 en caso
  *           de error.
  */
  static public int verifyData(String data, String name)
  {
    if(data.startsWith(","))          //Desechar posible coma inicial.
    {
      data = data.substring(1);
      return verifyData(data,name);
    }
    else if(data.endsWith(","))       //Desechar posible coma final.
    {
      data = data.substring(0,data.length()-1);
      return verifyData(data,name);
    }

    int number = 0;
    int first = 0;
    int second = data.indexOf(",",first);

    try
    {
      while(second != -1)
      {
        Integer.parseInt(Tools.blanks(data.substring(first,second)));
        number++;
        first = second + 1;
        second = data.indexOf(",",first);
      }

      Integer.parseInt(Tools.blanks(data.substring(first)));
      number++;
      return number;
    } catch(NumberFormatException e)
      {
        showErrorMessage(name);
        return -1;
      }
  }

  /*
  * La clase blanks elimina los posibles espacios en blanco que pudiera tener la
  * cadena @line al principio y/o al final.
  *
  * @param: String line -> cadena de caracteres.
  *
  * @ret String: Devuelve la cadena @line sin espacios en blanco iniciales y/o
  *              finales.
  */
  static public String blanks(String line)
  {
    if(line.startsWith(" ") || line.startsWith("\t"))
    {
      return blanks(line.substring(1));
    }
    else if(line.endsWith(" ") || line.endsWith("\t"))
    {
      return blanks(line.substring(0,line.length()-1));
    }
    else
    {
      return line;
    }
  }

  /*
  * La clase filterForDouble comprueba que la cadena @value represente un valor
  * de tipo double.
  *
  * @param: String value -> dato que representa un valor numérico de tipo double.
  *
  * @ret String: Devuelve la cadena @value con la representación correcta de un
  *              número de tipo double.
  */
  static public String filterForDouble(String value)
  {
    /*
     * En Java, "d" y "f" al final de un valor numérico representa "double" y
     * "float" respectivamente, en C esta sintaxis no existe --> eliminar d/f.
     */
    if(value.endsWith("d") || value.endsWith("f"))
    {
      value = value.substring(0,value.length()-2);
    }

    if(value.indexOf(".") == -1)  //Si el valor es X --> X.0
    {
      value = value + ".0";
    }
    else
    {
      if(value.startsWith("."))   //Si el valor empieza por .X --> 0.X
      {
        value = "0" + value;
      }

      if(value.endsWith("."))     //Si el valor acaba en X. --> X.0
      {
        value = value + "0";
      }
    }

    return value;
  }

  /*
  * La clase mpiValue facilita el valor numérico (índice) correspondiente a la
  * cadena @value (aplicable en el caso de Collective Operations).
  *
  * @param: int column -> columna a la que pertenece @value
  *                          - columna 0 o 2 -> MODEL
  *                          - columna 1 o 3 -> SIZE
  *         String value -> cadena de la que se quiere obtener el índice
  *                         correspondiente.
  *
  * @ret int: Índice correspondiente a @value, retorna 0 en cualquier otro caso.
  */
  static public int mpiValue(int column, String value)
  {
    if((column == 0) || (column == 2))
    {
      if(value.equalsIgnoreCase("0"))
      {
        return 1;
      }
      else if(value.equalsIgnoreCase("LOG"))
      {
        return 2;
      }
      else if(value.equalsIgnoreCase("LIN"))
      {
        return 3;
      }
      else if(value.equalsIgnoreCase("CTE"))
      {
        return 4;
      }
    }
    else
    {
      if(value.equalsIgnoreCase("MIN"))
      {
        return 1;
      }
      else if(value.equalsIgnoreCase("MAX"))
      {
        return 2;
      }
      else if(value.equalsIgnoreCase("MEAN"))
      {
        return 3;
      }
      else if(value.equalsIgnoreCase("2MAX"))
      {
        return 4;
      }
      else if(value.equalsIgnoreCase("S+R"))
      {
        return 5;
      }
    }

    return 0;
  }

  /*
  * La clase mpiString facilita la cadena correspondiente al índice @value
  * (aplicable en el caso de COLLECTIVE OPERATIONS).
  *
  * @param: · int column -> columna a la que pertenece @value
  *                          - columna 0 o 2 -> columna MODEL
  *                          - columna 1 o 3 -> columna SIZE
  *         · int value -> índice del que se quiere obtener la cadena
  *                       correspondiente.
  *
  * @ret String: Cadena correspondiente a @value, retorna "0/MAX" en cualquier
  *              otro caso.
  */
  static public String mpiString(int column, int value)
  {
    if((column == 0) || (column == 2))
    {
      if(value == 1)
      {
        return "0";
      }
      else if(value == 2)
      {
        return "LOG";
      }
      else if(value == 3)
      {
        return "LIN";
      }
      else if(value == 4)
      {
        return "CTE";
      }
      else
      {
        return "0";
      }
    }
    else
    {
      if(value == 1)
      {
        return "MIN";
      }
      else if(value == 2)
      {
        return "MAX";
      }
      else if(value == 3)
      {
        return "MEAN";
      }
      else if(value == 4)
      {
        return "2MAX";
      }
      else if(value == 5)
      {
        return "S+R";
      }
      else
      {
        return "MAX";
      }
    }
  }
}

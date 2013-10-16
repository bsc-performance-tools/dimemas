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

package gui;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import data.*;
import tools.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de configuración de los nodos.
*/
public class NodeWindow extends GUIWindow
{
  public static final long serialVersionUID = 17L;
  
  private JButton b_left = createButton("<<<");
  private JButton b_right = createButton(">>>");
  private JButton b_save = createButton("Save");
  private JButton b_close = createButton("Close");
  private JButton b_same = createButton("Do all the same");

  private JTextField tf_number = createTextField(2);
  private JTextField tf_machine = createTextField(18);
  private JTextField tf_id = new JTextField(18);
  private JTextField tf_architecture = createTextField(18);
  private JTextField tf_processors = new JTextField(18);
  private JTextField tf_input = new JTextField(18);
  private JTextField tf_output = new JTextField(18);
  private JTextField tf_local = new JTextField(18);
  private JTextField tf_remote = new JTextField(18);
  private JTextField tf_speed = new JTextField(18);
  private JTextField tf_memory = new JTextField(18);
  private JTextField tf_latency = new JTextField(18);

  /*
  * El método createTextField genera un campo de texto Swing que actuará como
  * contador o indicador del #elemento que se está visualizando en ese momento.
  *
  * @param: int num -> tamaño del texfield.
  *
  * @ret JTextField: Campo de texto creado.
  */
  private JTextField createTextField(int num)
  {
    JTextField tf = new JTextField(num);

    tf.setBorder(null);
    tf.setEditable(false);

    return tf;
  }

  /*
  * El método fillInformation actualiza todos los componentes de la pantalla
  * para que muestren la información correspondiente al elemento indicado por el
  * parámetro @index.
  *
  * @param: int index -> índice del elemento del que se obtiene la información.
  */
  private void fillInformation(int index)
  {
    tf_number.setText(String.valueOf(index+1));
    tf_machine.setText(data.processor.node[index].getMachine_id());
    tf_id.setText(data.processor.node[index].getNode_id());

    if(data.processor.node[index].getArchitecture(false).equalsIgnoreCase(""))
    {
      tf_architecture.setText("Custom architecture");
    }
    else
    {
      tf_architecture.setText(data.processor.node[index].getArchitecture(false));
    }

    tf_processors.setText(data.processor.node[index].getProcessors());
    tf_input.setText(data.processor.node[index].getInput());
    tf_output.setText(data.processor.node[index].getOutput());
    tf_local.setText(data.processor.node[index].getLocal());
    tf_remote.setText(data.processor.node[index].getRemote());
    tf_speed.setText(data.processor.node[index].getSpeed());
    tf_memory.setText(data.processor.node[index].getBandwidth());
    tf_latency.setText(data.processor.node[index].getLatency());
  }

  /*
  * El método dataOK comprueba algunos aspectos que deben cumplir los datos que
  * se muestran en ese momento en pantalla (no puede haber campos vacíos, el
  * valor 0 no esta permitido en algunos casos, etc).
  */
  private boolean dataOK()
  {
    if(tf_machine.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Machine identificator");
      return false;
    }
    else if(tf_id.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Node identificator");
      return false;
    }
    else if(tf_processors.getText().equalsIgnoreCase("") || tf_processors.getText().equalsIgnoreCase("0"))
    {
      Tools.showWarningMessage("Number of processors");
      return false;
    }
    else if(tf_input.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Input links");
      return false;
    }
    else if(tf_output.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Output links");
      return false;
    }
    else if(tf_local.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Local startup");
      return false;
    }
    else if(tf_remote.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Remote startup");
      return false;
    }
    else if(tf_speed.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Relative processor speed");
      return false;
    }
    else if(tf_memory.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Memory bandwidth");
      return false;
    }
    else if(tf_latency.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("External net latency");
      return false;
    }

    return true;
  }

  /*
  * El método storeInformation salva la información que aparece en pantalla en
  * la estructura de datos correspondiente.
  *
  * @param: · int index -> índice que indica el elemento al que pertenecen los
  *                        datos a guardar.
  *         · boolean reply -> TRUE si la información debe ser copiada a todos
  *                            los elementos (el usuario ha pulsado el botón "DO
  *                            ALL THE SAME"), FALSE en otro caso.
  *
  * @ret boolean: TRUE si la operación ha concluido con éxito, FALSE en otro caso.
  */
  private boolean storeInformation(int index, boolean reply)
  {
    if(!dataOK())
    {
      return false;
    }

    try
    {
      if(!reply)
      {
        data.processor.node[index].setNode_id(tf_id.getText());
      }

      data.processor.node[index].setProcessors(tf_processors.getText());
      data.processor.node[index].setInput(tf_input.getText());
      data.processor.node[index].setOutput(tf_output.getText());
      data.processor.node[index].setLocal(tf_local.getText());
      data.processor.node[index].setRemote(tf_remote.getText());
      data.processor.node[index].setSpeed(tf_speed.getText());
      data.processor.node[index].setBandwidth(tf_memory.getText());
      data.processor.node[index].setLatency(tf_latency.getText());

      return true;
    } catch(Exception exc)
      {
        return false;
      }
  }

  // Constructor de la clase NodeWindow.
  public NodeWindow(Data d, int nodeNumber)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Node information");

    // Añadiendo información.
    fillInformation(nodeNumber);

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Node number"),b_left,tf_number,b_right});
    drawLine(new Component[] {new JLabel("Machine id"),tf_machine});
    drawLine(new Component[] {new JLabel("Simulated architecture"),tf_architecture});
    drawLine(new Component[] {new JLabel("Node id"),tf_id});
    drawLine(new Component[] {new JLabel("Number of processors"),tf_processors});
    drawLine(new Component[] {new JLabel("Number of input links"),tf_input});
    drawLine(new Component[] {new JLabel("Number of output links"),tf_output});
    drawLine(new Component[] {new JLabel("Startup on local comm [s]"),tf_local});
    drawLine(new Component[] {new JLabel("Startup on remote comm [s]"),tf_remote});
    drawLine(new Component[] {new JLabel("Relative processor speed [%]"),tf_speed});
    drawLine(new Component[] {new JLabel("Memory bandwidth [MByte/s]"),tf_memory});
    drawLine(new Component[] {new JLabel("External net latency [s]"),tf_latency});
    drawButtons(new Component[] {b_save,b_same,b_close},25,5);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+30,getHeight());
    pack();
    setVisible(true);
  }

  /*
  * El método increase incrementa el valor del contador que indica qué nodo se
  * esta visualizando por pantalla. Si se sobrepasa el número de elementos
  * entonces se volverá al primer elemento (cíclico).
  *
  * @param: String stz -> valor actual del contador.
  *
  * @ret String: Nuevo valor del contador.
  */
  private String increase(String stz)
  {
    int aux = Integer.parseInt(stz);

    if(aux == data.processor.getNumberOfNodes())
    {
      return "1";
    }
    else
    {
      return String.valueOf(++aux);
    }
  }

  /*
  * El método decrease decrementa el valor del contador que indica qué nodo se
  * esta visualizando por pantalla. Si se sobrepasa el número de elementos
  * entonces se volverá al último elemento (cíclico).
  *
  * @param: String stz -> valor actual del contador.
  *
  * @ret String: Nuevo valor del contador.
  */
  private String decrease(String stz)
  {
    int aux = Integer.parseInt(stz);

    if(aux == 1)
    {
      return String.valueOf(data.processor.getNumberOfNodes());
    }
    else
    {
      return String.valueOf(--aux);
    }
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_left)
    {
      if(storeInformation(Integer.parseInt(tf_number.getText())-1,false))
      {
        tf_number.setText(decrease(tf_number.getText()));
        fillInformation(Integer.parseInt(tf_number.getText())-1);
      }
    }
    if(e.getSource() == b_right)
    {
      if(storeInformation(Integer.parseInt(tf_number.getText())-1,false))
      {
        tf_number.setText(increase(tf_number.getText()));
        fillInformation(Integer.parseInt(tf_number.getText())-1);
      }
    }
    else if(e.getSource() == b_save)
    {
      if(storeInformation(Integer.parseInt(tf_number.getText())-1,false))
      {
        dispose();
      }
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
    else if(e.getSource() == b_same)
    {
      for(int i = data.processor.getNumberOfNodes()-1; i >= 0; i--)
      {
        if(!storeInformation(i,true))
        {
          break;
        }
      }
    }
  }
}

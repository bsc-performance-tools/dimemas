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

package es.bsc.cepbatools.dimemas.gui;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import es.bsc.cepbatools.dimemas.data.*;
import es.bsc.cepbatools.dimemas.tools.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de configuración de las conexiones dedicadas.
*/
public class DedicatedConnectionWindow extends GUIWindow
{
  public static final long serialVersionUID = 7L;
  
  private JButton b_left = createButton("<<<");
  private JButton b_right = createButton(">>>");
  private JButton b_save = createButton("Save");
  private JButton b_close = createButton("Close");
  private JButton b_same = createButton("Do all the same");

  private JComboBox cb_firstCond = createComboBox('c');
  private JComboBox cb_secondCond = createComboBox('c');
  private JComboBox cb_operation = createComboBox('o');
  private JComboBox cb_source = createComboBox('m');
  private JComboBox cb_target = createComboBox('m');

  private JTextField tf_number = createTextField(2,false,true);
  private JTextField tf_id = createTextField(20,true,false);
  private JTextField tf_source = createTextField(12,false,false);
  private JTextField tf_target = createTextField(12,false,false);
  private JTextField tf_bandwidth = createTextField(20,true,false);
  private JTextField tf_tags = createTextField(20,true,false);
  private JTextField tf_firstSize = createTextField(8,true,false);
  private JTextField tf_secondSize = createTextField(8,true,false);
  private JTextField tf_communicators = createTextField(20,true,false);
  private JTextField tf_startup = createTextField(20,true,false);
  private JTextField tf_flight = createTextField(20,true,false);

  /*
  * El método createComboBox genera el selector Swing con las opciones
  * requeridas según el parámetro @function.
  *
  * @param: char function -> función que desempeñará el selector.
  *                           · 'c': Comparador ("<","=",">")
  *                           · 'o': Operación ("OR","AND")
  *                           · 'm': Máquinas existentes (id. de las máquinas)
  *
  * @ret JComboBox: Selector Swing creado.
  */
  private JComboBox createComboBox(char function)
  {
    JComboBox cb = new JComboBox();

    switch(function)
    {
      case 'c': cb.addItem("Select");
                cb.addItem("<");
                cb.addItem("=");
                cb.addItem(">");
                break;

      case 'o': cb.addItem("Operation");
                cb.addItem("AND");
                cb.addItem("OR");
                break;

      case 'm': cb.addItem("Select");

                for(int i = 0; i < data.environment.getNumberOfMachines(); i++)
                {
                  cb.addItem(data.environment.machine[i].getId());
                }

                cb.addActionListener(this);

                break;

      default: break;
    }

    return cb;
  }

  /*
  * El método createTextField genera un campo de texto Swing con las caractrísticas
  * fijadas por los parámetros.
  *
  * @param: · int num -> tamaño del texfield.
  *         · boolean editable -> TRUE si se podrá editar el contenido del campo
  *                               de texto (en el caso de que @counter = TRUE),
  *                               FALSE en otro caso.
  *         · boolean counter -> TRUE si el campo de texto actuará como contador
  *                              (@editable = FALSE), FALSE en otro caso.
  *
  * @ret JTextField: Campo de texto creado.
  */
  private JTextField createTextField(int num, boolean editable, boolean counter)
  {
    JTextField tf = new JTextField(num);

    if(counter)
    {
      tf.setText("1");
      tf.setBorder(null);
    }

    tf.setEditable(editable);

    return tf;
  }

  /*
  * El método drawPanel crea el panel que contendrá los componentes destinados a
  * fijar el tamaño de mensajes para el uso de la conexión dedicada.
  */
  protected void drawPanel()
  {
    JPanel panel = new JPanel();

    panel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"Message size required to use the connection"));

    panel.add(cb_firstCond);
    panel.add(tf_firstSize);
    panel.add(cb_operation);
    panel.add(cb_secondCond);
    panel.add(tf_secondSize);

    super.addComponent(panel);
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
    tf_id.setText(data.dedicated.connection[index].getId());
    tf_bandwidth.setText(data.dedicated.connection[index].getBandwidth());
    tf_tags.setText(data.dedicated.connection[index].getTags(false));
    tf_communicators.setText(data.dedicated.connection[index].getCommunicators(false));
    tf_firstSize.setText(data.dedicated.connection[index].getFirstSize());
    tf_secondSize.setText(data.dedicated.connection[index].getSecondSize());
    tf_source.setText(data.dedicated.connection[index].getSource());
    tf_target.setText(data.dedicated.connection[index].getTarget());
    tf_startup.setText(data.dedicated.connection[index].getStartup());
    tf_flight.setText(data.dedicated.connection[index].getFlightTime());

    if(tf_source.getText().equalsIgnoreCase(""))
    {
      cb_source.setSelectedIndex(0);
    }
    else
    {
      for(int i = 1; i < cb_source.getItemCount(); i++)
      {
        if(tf_source.getText().equalsIgnoreCase((String)cb_source.getItemAt(i)))
        {
          cb_source.setSelectedIndex(i);
          break;
        }
      }
    }

    if(tf_target.getText().equalsIgnoreCase(""))
    {
      cb_target.setSelectedIndex(0);
    }
    else
    {
      for(int i = 1; i < cb_target.getItemCount(); i++)
      {
        if(tf_target.getText().equalsIgnoreCase((String)cb_target.getItemAt(i)))
        {
          cb_target.setSelectedIndex(i);
          break;
        }
      }
    }

    if(data.dedicated.connection[index].getFirstCond(false).equalsIgnoreCase("<"))
    {
      cb_firstCond.setSelectedIndex(1);
    }
    else if(data.dedicated.connection[index].getFirstCond(false).equalsIgnoreCase("="))
    {
      cb_firstCond.setSelectedIndex(2);
    }
    else if(data.dedicated.connection[index].getFirstCond(false).equalsIgnoreCase(">"))
    {
      cb_firstCond.setSelectedIndex(3);
    }
    else
    {
      cb_firstCond.setSelectedIndex(0);
    }

    if(data.dedicated.connection[index].getOperation(false).equalsIgnoreCase("&"))
    {
      cb_operation.setSelectedIndex(1);
    }
    else if(data.dedicated.connection[index].getOperation(false).equalsIgnoreCase("|"))
    {
      cb_operation.setSelectedIndex(2);
    }
    else
    {
      cb_operation.setSelectedIndex(0);
    }

    if(data.dedicated.connection[index].getSecondCond(false).equalsIgnoreCase("<"))
    {
      cb_secondCond.setSelectedIndex(1);
    }
    else if(data.dedicated.connection[index].getSecondCond(false).equalsIgnoreCase("="))
    {
      cb_secondCond.setSelectedIndex(2);
    }
    else if(data.dedicated.connection[index].getSecondCond(false).equalsIgnoreCase(">"))
    {
      cb_secondCond.setSelectedIndex(3);
    }
    else
    {
      cb_secondCond.setSelectedIndex(0);
    }
  }

  /*
  * El método dataOK comprueba algunos aspectos que deben cumplir los datos que
  * se muestran en ese momento en pantalla (no puede haber campos vacíos, el
  * valor 0 no esta permitido en algunos casos, etc).
  */
  private boolean dataOK()
  {
    if(tf_id.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("DEDICATED CONNECTION ID");
      return false;
    }
    else if(tf_source.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("SOURCE MACHINE");
      return false;
    }
    else if(tf_target.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("TARGET MACHINE");
      return false;
    }
    else if(tf_target.getText().equalsIgnoreCase(tf_source.getText()))
    {
      Tools.showInformationMessage("Source and target machine shouldn't have the same identificator.");
      return false;
    }
    else if(tf_bandwidth.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("BANDWIDTH");
      return false;
    }
    else if(tf_startup.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("STARTUP");
      return false;
    }
    else if(tf_flight.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("FLIGHT TIME");
      return false;
    }
    else
    {
      return true;
    }
  }

  /*
  * El método storeInformation salva la información que aparece en pantalla en
  * la estructura de datos correspondiente.
  *
  * @param: · int i -> índice que indica el elemento al que pertenecen los datos
  *                    a guardar.
  *         · boolean reply -> TRUE si la información debe ser copiada a todos
  *                            los elementos (el usuario ha pulsado el botón "DO
  *                            ALL THE SAME"), FALSE en otro caso.
  *
  * @ret boolean: TRUE si la operación ha concluido con éxito, FALSE en otro caso.
  */
  private boolean storeInformation(int i, boolean reply)
  {
    if(!dataOK())
    {
      return false;
    }

    try
    {
      if(!reply)
      {
        data.dedicated.connection[i].setId(tf_id.getText());
      }

      data.dedicated.connection[i].setSource(tf_source.getText());
      data.dedicated.connection[i].setTarget(tf_target.getText());
      data.dedicated.connection[i].setBandwidth(tf_bandwidth.getText());
      data.dedicated.connection[i].setTags(tf_tags.getText());
      data.dedicated.connection[i].setCommunicators(tf_communicators.getText());
      data.dedicated.connection[i].setFirstSize(tf_firstSize.getText());
      data.dedicated.connection[i].setSecondSize(tf_secondSize.getText());
      data.dedicated.connection[i].setStartup(tf_startup.getText());
      data.dedicated.connection[i].setFlightTime(tf_flight.getText());
    } catch(Exception e)
      {
        return false;
      }


    if(cb_firstCond.getSelectedIndex() == 0)
    {
      data.dedicated.connection[i].setFirstCond(">");
    }
    else if(cb_firstCond.getSelectedIndex() == 1)
    {
      data.dedicated.connection[i].setFirstCond("<");
    }
    else if(cb_firstCond.getSelectedIndex() == 2)
    {
      data.dedicated.connection[i].setFirstCond("=");
    }
    else if(cb_firstCond.getSelectedIndex() == 3)
    {
      data.dedicated.connection[i].setFirstCond(">");
    }

    if(cb_operation.getSelectedIndex() == 0)
    {
      data.dedicated.connection[i].setOperation("|");
    }
    if(cb_operation.getSelectedIndex() == 1)
    {
      data.dedicated.connection[i].setOperation("&");
    }
    else if(cb_operation.getSelectedIndex() == 2)
    {
      data.dedicated.connection[i].setOperation("|");
    }

    if(cb_secondCond.getSelectedIndex() == 0)
    {
      data.dedicated.connection[i].setSecondCond(">");
    }
    else if(cb_secondCond.getSelectedIndex() == 1)
    {
      data.dedicated.connection[i].setSecondCond("<");
    }
    else if(cb_secondCond.getSelectedIndex() == 2)
    {
      data.dedicated.connection[i].setSecondCond("=");
    }
    else if(cb_secondCond.getSelectedIndex() == 3)
    {
      data.dedicated.connection[i].setSecondCond(">");
    }

    return true;
  }

  // Constructor de la clase DedicatedConnectionWindow.
  public DedicatedConnectionWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Dedicated connection information");

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Dedicated connection number"),b_left,tf_number,b_right});
    drawLine(new Component[] {new JLabel("Connection id"),tf_id});
    drawLine(new Component[] {new JLabel("Source machine"),cb_source,tf_source});
    drawLine(new Component[] {new JLabel("Target machine"),cb_target,tf_target});
    drawLine(new Component[] {new JLabel("Connection bandwidth [MByte/s]"),tf_bandwidth});
    tf_tags.setToolTipText("Type TAGS between commas: \"T1,T2,...,Tn\"");
    drawLine(new Component[] {new JLabel("Tag list"),tf_tags});
    drawPanel();
    tf_communicators.setToolTipText("Type COMMUNICATORS between commas: \"C1,C2,...,Cn\"");
    drawLine(new Component[] {new JLabel("Communicators list"),tf_communicators});
    drawLine(new Component[] {new JLabel("Connection startup [s]"),tf_startup});
    drawLine(new Component[] {new JLabel("Flight time [s]"),tf_flight});
    drawButtons(new Component[] {b_save,b_same,b_close},25,5);

    // Obteniendo información.
    fillInformation(0);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+10,getHeight());
    pack();
    setVisible(true);
  }

  /*
  * El método increase incrementa el valor del contador que indica qué conexión
  * dedicada se esta visualizando por pantalla. Si se sobrepasa el número de
  * elementos entonces se volverá al primer elemento (cíclico).
  *
  * @param: String stz -> valor actual del contador.
  *
  * @ret String: Nuevo valor del contador.
  */
  private String increase(String stz)
  {
    int aux = Integer.parseInt(stz);

    if(aux == data.dedicated.getNumberOfConnections())
    {
      return "1";
    }
    else
    {
      return String.valueOf(++aux);
    }
  }

  /*
  * El método decrease decrementa el valor del contador que indica qué conexión
  * dedicada se esta visualizando por pantalla. Si se sobrepasa el número de
  * elementos entonces se volverá al último elemento (cíclico).
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
      return data.wan.getDedicated();
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
      for(int i = data.dedicated.getNumberOfConnections()-1; i >= 0 ; i--)
      {
        if(!storeInformation(i,true))
        {
          break;
        }
      }
    }
    else if(e.getSource() == cb_source)
    {
      if(cb_source.getSelectedIndex() != 0)
      {
        tf_source.setText((String)cb_source.getSelectedItem());
      }
      else
      {
        tf_source.setText("");
      }
    }
    else if(e.getSource() == cb_target)
    {
      if(cb_target.getSelectedIndex() != 0)
      {
        tf_target.setText((String)cb_target.getSelectedItem());
      }
      else
      {
        tf_target.setText("");
      }
    }
  }
}

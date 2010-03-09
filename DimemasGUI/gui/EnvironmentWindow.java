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
* Esta clase crea la ventana de configuración de las máquinas que forman la
* arquitectura a simular.
*/
public class EnvironmentWindow extends GUIWindow
{
  public static final long serialVersionUID = 8L;
  
  private JRadioButton lin;
  private JRadioButton log;
  private JRadioButton ct;
  private ButtonGroup boxGroup = createGroup();

  private JButton b_left = createButton("<<<");
  private JButton b_right = createButton(">>>");
  private JButton b_save = createButton("Save");
  private JButton b_close = createButton("Close");
  private JButton b_same = createButton("Do all the same");

  private JComboBox cb_architecture = createComboBox();

  private JTextField tf_architecture = new JTextField(10);
  private JTextField tf_number = createTextField(2);
  private JTextField tf_name = new JTextField(22);
  private JTextField tf_id = new JTextField(22);
  private JTextField tf_nodes = new JTextField(22);
  private JTextField tf_bandwidth = new JTextField(22);
  private JTextField tf_buses = new JTextField(22);

  /*
  * El método createComboBox genera un selector Swing que tiene como opciones
  * los nombres de las arquitecturas/máquinas predefinidas en la base de datos.
  *
  * @ret JComboBox: Selector Swing creado.
  */
  private JComboBox createComboBox()
  {
    JComboBox cb = new JComboBox();

    cb.addItem("Select");

    for(int i = 0; i < data.machineDB.getNumberOfMachinesInDB(); i++)
    {
      cb.addItem(data.machineDB.machine[i].getName());
    }

    cb.addActionListener(this);

    return cb;
  }

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

    tf.setText("1");
    tf.setBorder(null);
    tf.setEditable(false);

    return tf;
  }

  // Método que agrupa las opciones para que solo se pueda escoger una a la vez.
  private ButtonGroup createGroup()
  {
    ButtonGroup group = new ButtonGroup();

    log = new JRadioButton("LOG");
    lin = new JRadioButton("LIN");
    ct = new JRadioButton("CT");
    group.add(log);
    group.add(lin);
    group.add(ct);

    return group;
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
    if(data.environment.machine[index].getIndex() == -1)
    {
      cb_architecture.setSelectedIndex(0);
    }
    else
    {
      cb_architecture.setSelectedIndex(data.environment.machine[index].getIndex()+1);
    }

    switch(Integer.parseInt(data.environment.machine[index].getCommunication()))
    {
      case 1: ct.setSelected(true);
              break;
      case 2: lin.setSelected(true);
              break;
      case 3: log.setSelected(true);
              break;
    }

    tf_name.setText(data.environment.machine[index].getName(false));
    tf_id.setText(data.environment.machine[index].getId());
    tf_architecture.setText(data.environment.machine[index].getNodeArchitecture());
    tf_nodes.setText(data.environment.machine[index].getNodes());
    tf_bandwidth.setText(data.environment.machine[index].getBandwidth());
    tf_buses.setText(data.environment.machine[index].getBuses());
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
      Tools.showWarningMessage("MACHINE ID");
      return false;
    }
    else if(tf_nodes.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("NUMBER OF NODES");
      return false;
    }
    else if(tf_bandwidth.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("BANDWIDTH");
      return false;
    }
    else if(tf_buses.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("NUMBER OF BUSES");
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
  private boolean storeInformation(int index, boolean reply)
  {
    if(!dataOK())
    {
      return false;
    }

    try
    {
      data.environment.machine[index].setIndex(cb_architecture.getSelectedIndex()-1);
      data.environment.machine[index].setNodeArchitecture(tf_architecture.getText());
      data.environment.machine[index].setNodes(tf_nodes.getText());
      data.environment.machine[index].setBandwidth(tf_bandwidth.getText());
      data.environment.machine[index].setBuses(tf_buses.getText());

      String oldId = data.environment.machine[index].getId();

      if(reply)
      {
        data.processor.verifyArchitecture(oldId,String.valueOf(index),
          data.machineDB,data.environment,index);
      }
      else
      {
        data.environment.machine[index].setId(tf_id.getText());
        data.environment.machine[index].setName(tf_name.getText());
        data.processor.verifyArchitecture(oldId,tf_id.getText(),data.machineDB,
          data.environment,index);
      }

      if(ct.isSelected())
      {
        data.environment.machine[index].setCommunication(data.environment.COMM_CT);
      }
      else if(lin.isSelected())
      {
        data.environment.machine[index].setCommunication(data.environment.COMM_LIN);
      }
      else if(log.isSelected())
      {
        data.environment.machine[index].setCommunication(data.environment.COMM_LOG);
      }
    } catch(Exception e)
      {
        return false;
      }

    return true;
  }

  // Constructor de la clase EnvironmentWindow.
  public EnvironmentWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Environment information");

    // Obteniendo información.
    fillInformation(0);

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Machine number"),b_left,tf_number,b_right});
    drawLine(new Component[] {new JLabel("Machine name"),tf_name});
    drawLine(new Component[] {new JLabel("Machine id"),tf_id});
    drawLine(new Component[] {new JLabel("Simulated architecture"),cb_architecture,tf_architecture});
    drawLine(new Component[] {new JLabel("Number of nodes"),tf_nodes});
    drawLine(new Component[] {new JLabel("Network bandwidth [MByte/s]"),tf_bandwidth});
    drawLine(new Component[] {new JLabel("Number of buses"),tf_buses});
    drawLine(new Component[] {new JLabel("Communication group model"),log,lin,ct});
    drawButtons(new Component[] {b_save,b_same,b_close},25,5);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+25,getHeight());
    pack();
    setVisible(true);
  }

  /*
  * El método increase incrementa el valor del contador que indica qué máquina
  * se esta visualizando por pantalla. Si se sobrepasa el número de elementos
  * entonces se volverá al primer elemento (cíclico).
  *
  * @param: String stz -> valor actual del contador.
  *
  * @ret String: Nuevo valor del contador.
  */
  private String increase(String stz)
  {
    int aux = Integer.parseInt(stz);

    if(aux == data.environment.getNumberOfMachines())
    {
      return "1";
    }
    else
    {
      return String.valueOf(++aux);
    }
  }

  /*
  * El método decrease decrementa el valor del contador que indica qué máquina
  * se esta visualizando por pantalla. Si se sobrepasa el número de elementos
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
      return data.wan.getMachines();
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
    else if(e.getSource() == cb_architecture)
    {
      if(cb_architecture.getSelectedIndex() != 0)
      {
        tf_architecture.setEditable(false);
        tf_architecture.setText(data.machineDB.machine[cb_architecture.getSelectedIndex()-1].getLabel());
        tf_nodes.setText(data.machineDB.machine[cb_architecture.getSelectedIndex()-1].getProcessors());
      }
      else
      {
        tf_architecture.setEditable(true);
        tf_architecture.setText(data.environment.DEFAULT_ARCHITECTURE);
        tf_nodes.setText(data.environment.DEFAULT_NODES);
      }
    }
    else if(e.getSource() == b_save)
    {
      if(storeInformation(Integer.parseInt(tf_number.getText())-1,false))
      {
        int totalNodes = 0;

        for(int i = data.environment.getNumberOfMachines()-1; i >= 0; i--)
        {
          totalNodes += Integer.parseInt(data.environment.machine[i].getNodes());
        }

        try
        {
          data.processor.setNumberOfNodes(totalNodes);

          if(data.processor.node == null)
          {
            data.processor.createNodes(data.environment,data.machineDB.machine);
          }
          else
          {
            data.processor.changeAtNodes(data.environment,data.machineDB.machine);
          }
        } catch(Exception exc) {}

        dispose();
      }
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
    else if(e.getSource() == b_same)
    {
      for(int i = data.environment.getNumberOfMachines()-1; i >= 0; i--)
      {
        if(!storeInformation(i,true))
        {
          break;
        }
      }
    }
  }
}

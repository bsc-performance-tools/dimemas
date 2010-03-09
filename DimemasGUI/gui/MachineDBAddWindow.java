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
* Esta clase crea la ventana de configuración que permite definir una nueva
* arquitectura predefinida o redefinir una existente.
*/
public class MachineDBAddWindow extends GUIWindow
{
  public static final long serialVersionUID = 12L;
  
  // Valores por defecto
  private final String DEFAULT_PROCESSORS = "1";
  private final String DEFAULT_INPUT = "1";
  private final String DEFAULT_OUTPUT = "1";
  private final String DEFAULT_LOCAL = "0.0";
  private final String DEFAULT_REMOTE = "0.0";
  private final String DEFAULT_DATA_TR = "0.0";

  private int index;

  private JButton b_save = createButton("Save machine");
  private JButton b_cancel = createButton("Cancel operation");

  private JTextField tf_name = new JTextField(15);
  private JTextField tf_id = new JTextField(15);
  private JTextField tf_proc = new JTextField(15);
  private JTextField tf_input = new JTextField(15);
  private JTextField tf_output = new JTextField(15);
  private JTextField tf_local = new JTextField(15);
  private JTextField tf_remote = new JTextField(15);
  private JTextField tf_data = new JTextField(15);

  // Constructor de la clase MachineDBAddWindow.
  public MachineDBAddWindow(Data d, int i)
  {
    super(d);
    index = i;

    // Propiedades de la ventana.
    setTitle("Machine information");
    setLocation(25,150);

    // Añadiendo información.
    if(index == -1)
    {
      tf_proc.setText(DEFAULT_PROCESSORS);
      tf_input.setText(DEFAULT_INPUT);
      tf_output.setText(DEFAULT_OUTPUT);
      tf_local.setText(DEFAULT_LOCAL);
      tf_remote.setText(DEFAULT_REMOTE);
      tf_data.setText(DEFAULT_DATA_TR);
    }
    else
    {
      tf_name.setText(data.machineDB.machine[index].getName());
      tf_id.setText(data.machineDB.machine[index].getLabel());
      tf_proc.setText(data.machineDB.machine[index].getProcessors());
      tf_input.setText(data.machineDB.machine[index].getInputLinks());
      tf_output.setText(data.machineDB.machine[index].getOutputLinks());
      tf_local.setText(data.machineDB.machine[index].getLocalStartup());
      tf_remote.setText(data.machineDB.machine[index].getRemoteStartup());
      tf_data.setText(data.machineDB.machine[index].getDataTransferRate());
    }

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Name"),tf_name});
    drawLine(new Component[] {new JLabel("Identifier"),tf_id});
    drawLine(new Component[] {new JLabel("Processors"),tf_proc});
    drawLine(new Component[] {new JLabel("Inputs links"),tf_input});
    drawLine(new Component[] {new JLabel("Output links"),tf_output});
    drawLine(new Component[] {new JLabel("Local startup [s]"),tf_local});
    drawLine(new Component[] {new JLabel("Remote startup [s]"),tf_remote});
    drawLine(new Component[] {new JLabel("Data transfer rate [MByte/s]"),tf_data});
    drawButtons(new Component[] {b_save,b_cancel},60,1);

    // Más propiedades de ventana.
    pack();
    setVisible(true);
  }

  /*
  * El método dataOK comprueba algunos aspectos que deben cumplir los datos que
  * se muestran en ese momento en pantalla (no puede haber campos vacíos, el
  * valor 0 no esta permitido en algunos casos, etc).
  */
  public boolean dataOK()
  {
    if(tf_name.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("NAME");
      return false;
    }
    else if(tf_id.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("IDENTIFIER");
      return false;
    }
    else if(tf_proc.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("NUMBER OF PROCESSORS");
      return false;
    }
    else if(tf_input.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("INPUT LINKS");
      return false;
    }
    else if(tf_output.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("OUTPUT LINKS");
      return false;
    }
    else if(tf_local.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("LOCAL STARTUP");
      return false;
    }
    else if(tf_remote.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("REMOTE STARTUP");
      return false;
    }
    else if(tf_data.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("DATA TRANSFER RATE");
      return false;
    }

    return true;
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_save)
    {
      if(dataOK())
      {
        try
        {
          if(index == -1)
          {
            data.machineDB.addElement(new String[] {tf_name.getText(),
                                                    tf_id.getText(),
                                                    tf_proc.getText(),
                                                    tf_input.getText(),
                                                    tf_output.getText(),
                                                    tf_local.getText(),
                                                    tf_remote.getText(),
                                                    tf_data.getText()});
          }
          else
          {
            data.machineDB.machine[index].setName(tf_name.getText());
            data.machineDB.machine[index].setLabel(tf_id.getText());
            data.machineDB.machine[index].setProcessors(tf_proc.getText());
            data.machineDB.machine[index].setInputLinks(tf_input.getText());
            data.machineDB.machine[index].setOutputLinks(tf_output.getText());
            data.machineDB.machine[index].setLocalStartup(tf_local.getText());
            data.machineDB.machine[index].setRemoteStartup(tf_remote.getText());
            data.machineDB.machine[index].setDataTransferRate(tf_data.getText());
            data.machineDB.saveDB();
          }

          dispose();
        } catch(Exception exc) {}
      }
    }
    else if(e.getSource() == b_cancel)
    {
      dispose();
    }
  }
}

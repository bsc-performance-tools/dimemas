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
* red predefinida o redefinir una existente.
*/
public class NetworkDBAddWindow extends GUIWindow
{
  public static final long serialVersionUID = 15L;
  
  // Valores por defecto.
  private final String DEFAULT_BANDWIDTH = "0.0";
  private final String DEFAULT_BUSES = "1";

  private int index;

  private JButton b_save = createButton("Save network");
  private JButton b_cancel = createButton("Cancel operation");

  private JTextField tf_name = new JTextField(15);
  private JTextField tf_id = new JTextField(15);
  private JTextField tf_bandwidth = new JTextField(15);
  private JTextField tf_buses = new JTextField(15);

  // Constructor de la clase NetworkDBAddWindow.
  public NetworkDBAddWindow(Data d, int i)
  {
    super(d);
    index = i;

    // Propiedades de la ventana.
    setTitle("Network information");
    setLocation(25,150);

    // Añadiendo información.
    if(index == -1)
    {
      tf_bandwidth.setText(DEFAULT_BANDWIDTH);
      tf_buses.setText(DEFAULT_BUSES);
    }
    else
    {
      tf_name.setText(data.netDB.net[index].getName());
      tf_id.setText(data.netDB.net[index].getLabel());
      tf_bandwidth.setText(data.netDB.net[index].getBandwidth());
      tf_buses.setText(data.netDB.net[index].getBuses());
    }

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Name"),tf_name});
    drawLine(new Component[] {new JLabel("Identifier"),tf_id});
    drawLine(new Component[] {new JLabel("Bandwidth [MByte/s]"),tf_bandwidth});
    drawLine(new Component[] {new JLabel("Buses"),tf_buses});
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
    else if(tf_bandwidth.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("BANDWIDTH");
      return false;
    }
    else if(tf_buses.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("BUSES");
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
            data.netDB.addElement(new String[] {tf_name.getText(),
                                                tf_id.getText(),
                                                tf_bandwidth.getText(),
                                                tf_buses.getText()});
          }
          else
          {
            data.netDB.net[index].setName(tf_name.getText());
            data.netDB.net[index].setLabel(tf_id.getText());
            data.netDB.net[index].setBandwidth(tf_bandwidth.getText());
            data.netDB.net[index].setBuses(tf_buses.getText());
            data.netDB.saveDB();
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

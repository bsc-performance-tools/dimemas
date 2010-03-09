package gui;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import tools.*;
import data.*;
import javax.swing.*;
import java.io.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana que da acceso a los módulos de configuración de las
* comunicaciones: INTERNAL COLLECTIVE OP., EXTERNAL COLLECTIVE OP. y FLIGHT TIME.
*/
public class CommunicationWindow extends GUIWindow
{
  public static final long serialVersionUID = 4L;
  
  private JTextField tf_file;

  private JButton b_close = createButton("Close");
  private JButton b_save = createButton("Save to disk");
  private JButton b_int = createButton("Setup");
  private JButton b_ext = createButton("Setup");
  private JButton b_ft = createButton("Setup");

  // Constructor de la clase CommunicationWindow.
  public CommunicationWindow(Data d, JTextField tf)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Communication setup window");
    tf_file = tf;

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {b_ext,new JLabel("External collective operations")});
    drawLine(new Component[] {b_int,new JLabel("Internal collective operations")});
    drawLine(new Component[] {b_ft,new JLabel("Flight time information")});
    drawButtons(new Component[] {b_save,b_close},5,5);

    // Más propiedades de ventana.
    setBounds(500,150,getWidth()+35,getHeight());
    pack();
    setVisible(true);
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_close)
    {
      if(tf_file.getText().equalsIgnoreCase(""))
      {
        Tools.showInformationMessage("If you do not save this information to file,\n communication data will be lost.");
      }

      dispose();
    }
    else if(e.getSource() == b_save)
    {
      File f = null;
      Tools.fc.addChoosableFileFilter(new Tools.CFGfilter());
      int result = Tools.fc.showSaveDialog(null);

      if(result == JFileChooser.APPROVE_OPTION)
      {
        if(Tools.fc.getSelectedFile().getAbsolutePath().endsWith(".cfg"))
        {
          f = new File(Tools.fc.getSelectedFile().getAbsolutePath());
        }
        else
        {
          f = new File(Tools.fc.getSelectedFile().getAbsolutePath() + ".cfg");
        }

        try
        {
          RandomAccessFile raf = new RandomAccessFile(f,"rw");

          if(raf.length() != 0)
          {
            raf.setLength(0);
          }

          raf.writeBytes("Policy: FIFO\n");
          data.environment.ftSave(raf);  // Datos FLIGHT TIME.

          // Datos INTERNAL COLLECTIVE OP.
          for(int i = 0; i < data.environment.getNumberOfMachines(); i++)
          {
            data.environment.machine[i].mpiSave(raf);
          }

          data.wan.mpiSave(raf);         // Datos EXTERNAL COLLECTIVE OP.
          raf.close();
          data.config.setCommunication(f.getAbsolutePath());
          tf_file.setText(f.getAbsolutePath());
        } catch(Throwable t)
          {
            Tools.showInformationMessage(t.toString());
          }
      }

      Tools.fc.resetChoosableFileFilters();
      dispose();
    }
    else if(e.getSource() == b_int)
    {
      if(b_close.isEnabled() && b_save.isEnabled())
      {
        b_close.setEnabled(false);
        b_save.setEnabled(false);
      }

      b_int.setEnabled(false);
      new CollectiveOpWindow(data,true).addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            if(b_ext.isEnabled() && b_ft.isEnabled())
            {
              b_close.setEnabled(true);
              b_save.setEnabled(true);
            }

            b_int.setEnabled(true);
          }
        }
      );
    }
    else if(e.getSource() == b_ext)
    {
      if(b_close.isEnabled() && b_save.isEnabled())
      {
        b_close.setEnabled(false);
        b_save.setEnabled(false);
      }

      b_ext.setEnabled(false);
      new CollectiveOpWindow(data,false).addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            if(b_int.isEnabled() && b_ft.isEnabled())
            {
              b_close.setEnabled(true);
              b_save.setEnabled(true);
            }

            b_ext.setEnabled(true);
          }
        }
      );
    }
    else if(e.getSource() == b_ft)
    {
      if(data.environment.getNumberOfMachines() == 1)
      {
        Tools.showInformationMessage("Two or more machines are needed to configure these properties.");
      }
      else
      {
        if(b_close.isEnabled() && b_save.isEnabled())
        {
          b_close.setEnabled(false);
          b_save.setEnabled(false);
        }

        b_ft.setEnabled(false);
        new FlightTimeWindow(data).addWindowListener(
          new WindowAdapter()
          {
            public void windowClosed(WindowEvent we)
            {
              if(b_int.isEnabled() && b_ext.isEnabled())
              {
                b_close.setEnabled(true);
                b_save.setEnabled(true);
              }

              b_ft.setEnabled(true);
            }
          }
        );
      }
    }
  }
}

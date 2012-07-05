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
import java.io.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de configuración correspondiente a la arquitectura
* origen o INITIAL MACHINE.
*/
public class InitialMachineWindow extends GUIWindow
{
  public static final long serialVersionUID = 11L;

  // private JComboBox cb_architecture = createComboBox(false);
  // private JComboBox cb_tasks = createComboBox(true);

  private JTextField tf_tracefile        = new JTextField(33);
  private JTextField tf_tasks            = new JTextField(17);

  private JButton b_save   = createButton("Save");
  private JButton b_select = createButton("Select tracefile");
  private JButton b_close  = createButton("Close");

  // Constructor de la clase InitialMachineWindow.
  public InitialMachineWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Initial machine information");

    // Añadiendo información.
    tf_tracefile.setText(data.map.getTracefile(false));
    // tf_instrumentedArch.setText(data.instrumentedArchitecture);
    tf_tasks.setText(String.valueOf(data.map.getTasks()));

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Input tracefile name"),tf_tracefile});
    tf_tracefile.setEditable(false);

    /*
    drawLine(new Component[] {new JLabel("Architecture used to instrument"),
                              cb_architecture,
                              tf_instrumentedArch});
    */
    drawLine(new Component[] {new JLabel("Number of aplication tasks"),
                              // cb_tasks,
                              tf_tasks});
    tf_tasks.setEditable(false);

    drawButtons(new Component[] {b_save,b_select,b_close},20,5);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+20,getHeight());
    pack();
    setVisible(true);
  }

  /*
  * El método dataOK comprueba algunos aspectos que deben cumplir los datos que
  * se muestran en ese momento en pantalla (no puede haber campos vacíos, el
  * valor 0 no esta permitido en algunos casos, etc).
  */
  private boolean dataOK()
  {
    if(tf_tracefile.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("TRACEFILE");
      return false;
    }
    else
    {

    }

    if(tf_tasks.getText().equalsIgnoreCase("") || tf_tasks.getText().equalsIgnoreCase("0"))
    {
      Tools.showWarningMessage("NUMBER OF TASKS");
      return false;
    }
    else
    {
      return true;
    }
  }

  /*
  * La clase workUnit facilita el cálculo del número de tareas ubicadas en un
  * fichero de trazas.
  */
  class workUnit extends Thread
  {
    String line;
    int nTasks = 0;
    int first;
    int second;
    // progress prog;
    RandomAccessFile sourceFile;

    // Constructor de la clase workUnit.
    // public workUnit(File file, progress p)
    public workUnit(File file)
    {
      // prog = p;

      try
      {
        sourceFile = new RandomAccessFile(file,"r");

      }
      catch(FileNotFoundException fe)
      {
        Tools.showErrorDialog("Tracefile "+file.getName()+ " not found");
      }
    }

    // Método que permite acceder al número de tareas desde fuera de la clase.
    public String getNumberOfTasks()
    {
      return String.valueOf(nTasks);
    }

    public void run()
    {
      try
      {
        sleep(50);

        do
        {

          line = sourceFile.readLine();

        } while(line.equals(""));

        if (line.startsWith("#DIMEMAS"))
        {
          /* line contains the header! */
          String[] tokens = line.split("[:,()]+");

          /* 0: Magic!
           * 1: Trace Name
           * 2: Offsets Presence
           */
          if (tokens[2].equals("0"))
          {
            tf_tasks.setText(tokens[3]);
          }
          else if (tokens[2].equals("1"))
          {
            tf_tasks.setText(tokens[4]);
          }
          else
          {
            Tools.showErrorDialog("Wrong header in tracefile");
          }
        }
        else if (line.startsWith("SDDFA;;"))
        {
          Tools.showErrorDialog("This version does not support TRF tracefiles. Plase translate the trace");
          tf_tracefile.setText(null);
        }
        else
        {
          Tools.showErrorDialog("Unknown input trace format");
          tf_tracefile.setText(null);
        }

        sourceFile.close();
      }
      catch(InterruptedException ie)
      {
        Tools.showInformationMessage(ie.toString());
      }
      catch(IOException ioe)
      {
        Tools.showInformationMessage(ioe.toString());
      }

      // prog.interrupt();
    }
  }

  /*
  * La clase progress crea y mantiene la barra de progreso que indica el proceso
  * de cálculo del número de tareas de un fichero de trazas.
  */
  class progress extends Thread
  {
    int current;
    JProgressBar bar = new JProgressBar();
    JFrame frame = new JFrame("Work in progress");

    // Constructor de la clase progress.
    public progress(int length)
    {
      bar.setMinimum(0);
      bar.setMaximum(length);
      bar.setStringPainted(true);
      frame.setIconImage(Toolkit.getDefaultToolkit().createImage(Data.ICON_IMAGE));
      frame.getContentPane().setLayout(new BorderLayout());
      frame.getContentPane().add(new JLabel("Please wait",JLabel.CENTER),BorderLayout.CENTER);
      frame.getContentPane().add(bar,BorderLayout.SOUTH);
      frame.setResizable(false);
      frame.setBounds(150,200,frame.getWidth()+50,frame.getHeight()+20);
      frame.pack();
    }

    public void setCurrent(int value)
    {
      current = value;
    }

    public void run()
    {
      frame.setVisible(true);

      while(bar.getValue() < bar.getMaximum())
      {
        try
        {
          sleep(500);
          bar.setValue(current);
        } catch(InterruptedException ie)
          {
            bar.setValue(bar.getMaximum());
            break;
          }
      }

      frame.setVisible(false);
    }
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    /*
    if(e.getSource() == cb_architecture)
    {
      if(cb_architecture.getSelectedIndex() != 0)
      {
        tf_instrumentedArch.setEditable(false);
        tf_instrumentedArch.setText(data.machineDB.machine[cb_architecture.getSelectedIndex()-1].getName());
      }
      else
      {
        tf_instrumentedArch.setEditable(true);
        tf_instrumentedArch.setText("");
      }
    }
    else
    if (e.getSource() == cb_tasks)
    {
      if(cb_tasks.getSelectedIndex() != 0)
      {
        tf_tasks.setText((String)cb_tasks.getSelectedItem());
      }
      else
      {
        tf_tasks.setText(String.valueOf(data.map.DEFAULT_TASKS));
      }
    }
    else */
    if(e.getSource() == b_save)
    {
      if(dataOK())
      {
        try
        {
          data.map.setTasks(tf_tasks.getText());
          //data.instrumentedArchitecture = tf_instrumentedArch.getText();
          data.map.setTracefile(tf_tracefile.getText());

          for(int i = data.environment.getNumberOfMachines()-1; i >= 0; i--)
          {
            data.environment.machine[i].setArchitecture(data.instrumentedArchitecture);
          }

          dispose();
        } catch(Exception exc){}
      }
    }
    else if(e.getSource() == b_select)
    {
      Tools.fc.addChoosableFileFilter(new Tools.DIMfilter());

      int result = Tools.fc.showOpenDialog(null);

      if(result == JFileChooser.APPROVE_OPTION)
      {
        tf_tracefile.setText(Tools.fc.getSelectedFile().getAbsolutePath());
      }

      // Check the trace format and the number of tasks
      File source = new File(tf_tracefile.getText());
      workUnit wu = new workUnit(source);
      wu.start();

      Tools.fc.resetChoosableFileFilters();
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
  }
}

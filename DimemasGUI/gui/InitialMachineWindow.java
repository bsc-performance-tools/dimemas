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
  
  private JComboBox cb_architecture = createComboBox(false);
  private JComboBox cb_tasks = createComboBox(true);

  private JTextField tf_tracefile = new JTextField(33);
  private JTextField tf_instrumentedArch = new JTextField(17);
  private JTextField tf_tasks = new JTextField(17);

  private JButton b_save = createButton("Save");
  private JButton b_select = createButton("Select tracefile");
  private JButton b_tasks = createButton("Compute number of tasks");
  private JButton b_close = createButton("Close");

  /*
  * El método createComboBox genera un selector Swing que tiene como opciones
  * los nombres de las arquitecturas/máquinas predefinidas en la base de datos
  * o diversos números enteros (para fijar el número de tareas) según indique
  * el parámetroe @tasks.
  *
  * @param: boolean tasks -> TRUE si el selector irá destinado a escoger el
  *                          número de tareas, FALSE si el selector mostrará
  *                          nombres de arquitecturas predefinidas.
  *
  * @ret JComboBox: Selector Swing creado.
  */
  private JComboBox createComboBox(boolean tasks)
  {
    JComboBox cb = new JComboBox();

    cb.addItem("Edit");

    if(tasks)
    {
      cb.addItem("1");
      cb.addItem("2");
      cb.addItem("4");
      cb.addItem("8");
      cb.addItem("16");
      cb.addItem("32");
    }
    else
    {
      for(int i = 0; i <data.machineDB.getNumberOfMachinesInDB(); i++)
      {
        cb.addItem(data.machineDB.machine[i].getLabel());
      }
    }

    cb.addActionListener(this);

    return cb;
  }

  // Constructor de la clase InitialMachineWindow.
  public InitialMachineWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Initial machine information");

    // Añadiendo información.
    tf_tracefile.setText(data.map.getTracefile(false));
    tf_instrumentedArch.setText(data.instrumentedArchitecture);
    tf_tasks.setText(String.valueOf(data.map.getTasks()));

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Input tracefile name"),tf_tracefile});
    drawLine(new Component[] {new JLabel("Architecture used to instrument"),
                              cb_architecture,
                              tf_instrumentedArch});
    drawLine(new Component[] {new JLabel("Number of aplication tasks"),
                              cb_tasks,
                              tf_tasks});
    drawButtons(new Component[] {b_save,b_select,b_tasks,b_close},20,5);

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
    else if(tf_tasks.getText().equalsIgnoreCase("") || tf_tasks.getText().equalsIgnoreCase("0"))
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
    progress prog;
    RandomAccessFile sourceFile;

    // Constructor de la clase workUnit.
    public workUnit(File file, progress p)
    {
      prog = p;

      try
      {
        sourceFile = new RandomAccessFile(file,"r");
      } catch(FileNotFoundException fe)
        {
          Tools.showInformationMessage(fe.toString());
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

        while(sourceFile.getFilePointer() != sourceFile.length())
        {
          line = sourceFile.readLine();

          if(line.startsWith("\"CPU burst\" {") && line.endsWith(";;"))
          {
            first = line.indexOf("{")+1;
            second = line.indexOf(",");
            line = Tools.blanks(line.substring(first,second));

            if(!line.equalsIgnoreCase("") && nTasks < Integer.parseInt(line)+1)
            {
              nTasks = Integer.parseInt(line)+1;
            }
          }

          prog.setCurrent((int)sourceFile.getFilePointer());
        }

        sourceFile.close();
      } catch(InterruptedException ie)
        {
          Tools.showInformationMessage(ie.toString());
        }
        catch(IOException ioe)
        {
          Tools.showInformationMessage(ioe.toString());
        }

      prog.interrupt();
      tf_tasks.setText(String.valueOf(nTasks));
      b_tasks.setEnabled(true);
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
    else if (e.getSource() == cb_tasks)
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
    else if(e.getSource() == b_save)
    {
      if(dataOK())
      {
        try
        {
          data.block.destroyFactors();
          data.block.createFactors(tf_tracefile.getText());
          data.map.setTasks(tf_tasks.getText());
          data.instrumentedArchitecture = tf_instrumentedArch.getText();
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
      Tools.fc.addChoosableFileFilter(new Tools.TRFfilter());
      int result = Tools.fc.showOpenDialog(null);

      if(result == JFileChooser.APPROVE_OPTION)
      {
        tf_tracefile.setText(Tools.fc.getSelectedFile().getAbsolutePath());
      }

      Tools.fc.resetChoosableFileFilters();
    }
    else if(e.getSource() == b_tasks)
    {
      if(!tf_tracefile.getText().equals(""))
      {
        File source = new File(tf_tracefile.getText());
        b_tasks.setEnabled(false);
        progress p = new progress((int)source.length());
        workUnit wu = new workUnit(source,p);
        p.start();
        wu.start();
      }
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
  }
}

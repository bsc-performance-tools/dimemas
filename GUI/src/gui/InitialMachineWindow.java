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
 * @author Oscar Bardillo Luján
 */

import data.*;
import tools.*;
import javax.swing.*;
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

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

  // Constructor
  public InitialMachineWindow(Data d)
  {
    super(d);

    // Window properties
    setTitle("Application information");

    // Adding information
    tf_tracefile.setText(data.map.getTracefile(false));
    // tf_instrumentedArch.setText(data.instrumentedArchitecture);
    if (data.map.getTasks() == Data.NO_TASKS_DETECTED)
    {
      tf_tasks.setText("N/A");
    }
    else
    {
      tf_tasks.setText(String.valueOf(data.map.getTasks()));
    }

    // Window components
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

    // Window size
    setBounds(25,150,getWidth()+20,getHeight());
    pack();
    setVisible(true);
  }

  /**
   * Checks the status of some data
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

  /**
   * Checks the number of MPI processes in a trace
   */
  class workUnit extends Thread
  {
    String line;
    int nTasks = 0;
    int first;
    int second;
    // progress prog;
    RandomAccessFile sourceFile;

    // Constructor
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

        // Pattern p = Pattern.compile("#DIMEMAS:\"(\\d+)\"-(\\p{Alpha}+)-(\\d+) (\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)")

        if (line.startsWith("#DIMEMAS"))
        {
          /* line contains the header! */
          Pattern header_pattern = Pattern.compile("#DIMEMAS:\"(.*)\":(.*):(\\d+)\\((.*)$");
          Matcher header_matcher = header_pattern.matcher(line);

          // String[] tokens = line.split("#DIMEMAS:\"([^\"]+)\":([^:]+):([^\n]+)");
          // String[] tokens = line.split("#DIMEMAS:\"[^\"]+\":[^:]+:[^\n]+$");
          // String[] tokens = line.split("#DIMEMAS:\"([^\"]+)\":([^:]+):([.]+)$");
          // String[] tokens = line.split("#DIMEMAS:\"(.*)\":(.*):(.*)");
         //


          if (!header_matcher.matches())
          {
            Tools.showErrorDialog("Wrong header in tracefile");
          }
          else
          {
          /* 1: Trace Name
           * 2: Offsets
           * 3: # Tasks
           * 4: Rest of object definitions*/
            String objects = header_matcher.group(3);

            tf_tasks.setText(header_matcher.group(3));

           // System.out.println("Objects = "+objects);
          }

        }
        else if (line.startsWith("SDDFA;;"))
        {
          Tools.showErrorDialog("This version does not support TRF tracefiles. Plase convert the trace");
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

    }
        
  }

  /**
   * Progress bar when reading a trace file
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
      frame.setIconImage(Toolkit.getDefaultToolkit().createImage(getClass().getClassLoader().getResource(Data.ICON_IMAGE)));
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

  public void actionPerformed(ActionEvent e)
  {

    if(e.getSource() == b_save)
    {
      if(dataOK())
      {
        try
        {
          //data.map.setTasks(Integer.parseInt(tf_tasks.getText()));
          //data.instrumentedArchitecture = tf_instrumentedArch.getText();
          
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
      Tools.fc.setAcceptAllFileFilterUsed(false);
      Tools.fc.addChoosableFileFilter(new Tools.DIMfilter());

      int result = Tools.fc.showOpenDialog(null);

      if(result == JFileChooser.APPROVE_OPTION)
      {
        tf_tracefile.setText(Tools.fc.getSelectedFile().getAbsolutePath());
        
        data.map.setTracefile(tf_tracefile.getText());

        if (data.map.getTasks() != Data.NO_TASKS_DETECTED)
        {
          tf_tasks.setText(String.valueOf(data.map.getTasks()));
        }
        else
        {
          tf_tasks.setText("N/A");
        }
      }

      /* JGG: not needed any more Check the trace format and the number of tasks
      File source = new File(tf_tracefile.getText());
      workUnit wu = new workUnit(source);
      wu.start();
      */

      Tools.fc.resetChoosableFileFilters();
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
  }
}

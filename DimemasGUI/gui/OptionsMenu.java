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
import data.Data;
import javax.swing.*;
import java.io.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea el menu con todas las opciones del GUI.
*/
public class OptionsMenu extends JMenuBar implements ActionListener
{
  public static final long serialVersionUID = 18L;
  
  private Data data;

  private JMenu config = new JMenu("Configuration");
  private JMenu sim    = new JMenu("Simulator");
  private JMenu db     = new JMenu("Database");
  private JMenu info   = new JMenu("Information");

  private JMenuItem initial      = createMenuItem("Initial machine");
  private JMenuItem target       = createMenuItem("Target configuration");
  private JMenuItem loadConf     = createMenuItem("Load configuration");
  private JMenuItem saveConf     = createMenuItem("Save configuration");

  
  private JMenuItem exit         = createMenuItem("Exit");
  private JMenuItem dimemas      = createMenuItem("Dimemas");
  private JMenuItem critical     = createMenuItem("Critical path analysis");
  private JMenuItem perturbation = createMenuItem("Synthetic perturbation analysis");
  private JMenuItem loadOp       = createMenuItem("Load options");
  private JMenuItem saveOp       = createMenuItem("Save options");
  private JMenuItem machines     = createMenuItem("Machines");
  private JMenuItem networks     = createMenuItem("Networks");
  private JMenuItem about        = createMenuItem("About");

  ConfigurationOptionsWindow cow;

  // Método que crea un elemento del menú.
  private JMenuItem createMenuItem(String name)
  {
    JMenuItem mi = new JMenuItem(name);

    mi.addActionListener(this);

    return mi;
  }

  // Constructor de la clase OptionsMenu.
  public OptionsMenu(Data d)
  {
    data = d;

    critical.setEnabled(false);
    perturbation.setEnabled(false);

    // Añadiendo opciones al menú "Configuration".
    config.add(initial);
    config.add(target);
    config.addSeparator();
    config.add(loadConf);
    config.add(saveConf);
    config.addSeparator();
    config.add(exit);

    // Añadiendo opciones al menú "Simulator".
    sim.add(dimemas);
    sim.add(critical);
    sim.add(perturbation);
    sim.addSeparator();
    sim.add(loadOp);
    sim.add(saveOp);

    /// Añadiendo opciones al menú "Database".
    db.add(machines);
    db.add(networks);

    // Añadiendo opciones al menú "Information".
    info.add(about);

    // Creando la barra de menú.
    add(config);
    add(sim);
    add(db);
    add(info);
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == initial)
    {
      initial.setEnabled(false);

      new InitialMachineWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            initial.setEnabled(true);

            if(!target.isEnabled())
            {
              cow.refresh();
            }
          }
        }
      );
    }
    else if(e.getSource() == target)
    {
      target.setEnabled(false);

      cow = new ConfigurationOptionsWindow(data);

      cow.addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            target.setEnabled(true);
            cow = null;
          }
        }
      );
    }
    else if(e.getSource() == loadConf)
    {
      Tools.fc.addChoosableFileFilter(new Tools.CFGfilter());
      int result = Tools.fc.showOpenDialog(null);

      if(result == JFileChooser.APPROVE_OPTION)
      {
        data.loadFromDisk(Tools.fc.getSelectedFile().getAbsolutePath());

        if(!target.isEnabled())
        {
          cow.refresh();
        }
      }

      Tools.fc.resetChoosableFileFilters();
    }
    else if(e.getSource() == saveConf)
    {
      Tools.fc.addChoosableFileFilter(new Tools.CFGfilter());
      int result = Tools.fc.showSaveDialog(null);

      if(result == JFileChooser.APPROVE_OPTION)
      {
        if(Tools.fc.getSelectedFile().getAbsolutePath().endsWith(".cfg"))
        {
          data.saveToDisk(Tools.fc.getSelectedFile().getAbsolutePath());
          data.setCurrentConfigurationFileLabel(Tools.fc.getSelectedFile().getAbsolutePath());
        }
        else
        {
          data.saveToDisk(Tools.fc.getSelectedFile().getAbsolutePath() + ".cfg");
          data.setCurrentConfigurationFileLabel(Tools.fc.getSelectedFile().getAbsolutePath() + ".cfg");
        }
      }

      Tools.fc.resetChoosableFileFilters();
    }
    else if(e.getSource() == exit)
    {
      if ( 0 ==
        JOptionPane.showConfirmDialog(
          this,
          "Are you sure you want to quit?",
          "Dimemas Question",
          JOptionPane.YES_NO_OPTION
        ) 
      )
      {
        System.exit(0);
      }
    }
    else if(e.getSource() == dimemas)
    {
      dimemas.setEnabled(false);

      new SimulatorCallWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            dimemas.setEnabled(true);
          }
        }
      );
    }
    else if(e.getSource() == critical)
    {
      // Critical path analysis window
    }
    else if(e.getSource() == perturbation)
    {
      // Perturbation analysis window
    }
    else if(e.getSource() == loadOp)
    {
      Tools.fc.addChoosableFileFilter(new Tools.OPTfilter());
      int result = Tools.fc.showOpenDialog(null);

      if(result == JFileChooser.APPROVE_OPTION)
      {
        data.simOptions.loadFromDisk(Tools.fc.getSelectedFile());
      }

      Tools.fc.resetChoosableFileFilters();
    }
    else if(e.getSource() == saveOp)
    {
      Tools.fc.addChoosableFileFilter(new Tools.OPTfilter());
      int result = Tools.fc.showSaveDialog(null);

      if(result == JFileChooser.APPROVE_OPTION)
      {
        if(Tools.fc.getSelectedFile().getAbsolutePath().endsWith(".opt"))
        {
          data.simOptions.saveToDisk(Tools.fc.getSelectedFile().getAbsolutePath());
        }
        else
        {
          data.simOptions.saveToDisk(Tools.fc.getSelectedFile().getAbsolutePath() + ".opt");
        }
      }

      Tools.fc.resetChoosableFileFilters();
    }
    else if(e.getSource() == machines)
    {
      machines.setEnabled(false);

      new MachineDBWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            machines.setEnabled(true);
          }
        }
      );
    }
    else if(e.getSource() == networks)
    {
      networks.setEnabled(false);

      new NetworkDBWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            networks.setEnabled(true);
          }
        }
      );
    }
    else if(e.getSource() == about)
    {
      about.setEnabled(false);

      new AboutWindow().addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            about.setEnabled(true);
          }
        }
      );
    }
  }
}

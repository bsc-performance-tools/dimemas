/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import gui.*;
import data.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class Main extends JFrame
{
  public static final long serialVersionUID = 1000L;
  
  JTextField currentConfigurationFile = new JTextField("");
  Data data                           = new Data(currentConfigurationFile);
  OptionsMenu menu                    = new OptionsMenu(data);
  Container panelPrincipal            = getContentPane();

  public Main()
  {
    super("DIMEMAS");
    setIconImage(Toolkit.getDefaultToolkit().getImage(Data.ICON_IMAGE));
    setResizable(false);
    setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
    
    setJMenuBar(menu);
    panelPrincipal.setLayout(new BorderLayout());
    panelPrincipal.add("West",new JLabel("  Current configuration file: "));
    panelPrincipal.add("Center",currentConfigurationFile);
    
    setBounds(25,50,600,100);
    setVisible(true);
  }

  public static void main(String[] args)
  {
    final JFrame ventanaPrincipal = new Main();

    ventanaPrincipal.addWindowListener(new WindowAdapter()
      {
        public void windowClosing(WindowEvent e)
        {
          if ( 0 ==
            JOptionPane.showConfirmDialog(
              null,
              "Are you sure you want to quit?",
              "Dimemas Question",
              JOptionPane.YES_NO_OPTION
            )
          )
          {
            System.exit(0);
          }
          else
          {
            ventanaPrincipal.setVisible(true);
          }
        }
      });
  }
}

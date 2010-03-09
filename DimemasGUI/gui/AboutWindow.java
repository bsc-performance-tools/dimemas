package gui;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import data.Data;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de información de Dimemas.
*/
public class AboutWindow extends JFrame implements ActionListener
{
  public static final long serialVersionUID = 1L;
  
  private JButton b_close = createButton("Close");
  private Container panelPrincipal = getContentPane();

  /*
  * El método createButton genera un botón Swing con la etiqueta facilitada
  * @name y, además, añade al botón creado un ActionListener para poder capturar
  * las acciones del usuario sobre ese botón.
  *
  * @param: String name -> etiqueta que aperecerá en el botón.
  *
  * @ret JButton: Botón Swing con ActionListener.
  */
  private JButton createButton(String name)
  {
    JButton bt = new JButton(name);

    bt.addActionListener(this);

    return bt;
  }

  /*
  * El método aboutPanel crea un panel que contendrá la información referente a
  * Dimemas.
  *
  * @ret JPanel: Panel Swing con el texto referente a la información sobre
  *              Dimemas.
  */
  private JPanel aboutPanel()
  {
    JPanel panel = new JPanel(new GridLayout(11,1));

    panel.setBorder(BorderFactory.createEtchedBorder(1));
    panel.add(new JLabel(""));
    panel.add(new JLabel(
      "     DIMEMAS is a performance analysis tool for message passing     "));
    panel.add(new JLabel(
      "     programs.  It is designed,  developed  and  maintained  by  the"));
    panel.add(new JLabel(
      "     Centro Europeo de Paralelismo de Barcelona of the Technical"));
    panel.add(new JLabel("     University of Catalonia."));
    panel.add(new JLabel(""));
    panel.add(new JLabel("Contact:  support@cepba.upc.es",JLabel.CENTER));
    panel.add(new JLabel(""));
    panel.add(new JLabel(
      "For further information visit the Dimemas Web Site at:",JLabel.CENTER));
    panel.add(new JLabel("http://www.cepba.upc.es/dimemas",JLabel.CENTER));
    panel.add(new JLabel(""));

    return panel;
  }

  // Constructor de la clase AboutWindow.
  public AboutWindow()
  {
    // Propiedades de la ventana.
    setTitle("About DIMEMAS");
    setResizable(false);
    setLocation(200,200);
    setIconImage(Toolkit.getDefaultToolkit().getImage(Data.ICON_IMAGE));
    panelPrincipal.setLayout(new BorderLayout());

    // Se añaden los elementos de la ventana.
    panelPrincipal.add("West",new JLabel(new ImageIcon(Data.ICON_IMAGE)));
    panelPrincipal.add("Center",aboutPanel());
    panelPrincipal.add("South",b_close);

    // Más propiedades de ventana.
    pack();
    setVisible(true);
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_close)
    {
      dispose();
    }
  }
}

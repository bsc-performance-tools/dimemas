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
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de configuración de los BLOCK FACTORS.
*/
public class BlockFactorsWindow extends GUIWindow
{
  public static final long serialVersionUID = 2L;
  
  JButton b_save = createButton("Save");
  JButton b_close = createButton("Close");

  JPanel buttonPanel;
  JScrollPane infoPanel;
  JTextField[] blockValue;

  // Método que crea los paneles con los componentes correspondientes.
  private void createPanels()
  {
    // Panel de botones.
    buttonPanel = new JPanel(new FlowLayout());
    buttonPanel.add(b_save);
    buttonPanel.add(b_close);

    // Panel de información de BLOCK FACTORS (Nombre -- Valor).
    JPanel top = new JPanel(new GridLayout(data.block.getNumberOfBlocks(),2));
    blockValue = new JTextField[data.block.getNumberOfBlocks()];

    for(int i = 0; i < data.block.getNumberOfBlocks(); i++)
    {
      top.add(new JLabel(data.block.factors[i].getId() + " - " + data.block.factors[i].getName()));
      blockValue[i] = new JTextField(18);
      blockValue[i].setText(data.block.factors[i].getValue());
      top.add(blockValue[i]);
    }

    infoPanel = new JScrollPane(top,JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
  }

  // Constructor de la clase BlockFactorsWindow.
  public BlockFactorsWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    windowPanel.setLayout(new BorderLayout());
    setTitle("Block factors window");

    // Añadiendo los componentes a la ventana.
    createPanels();
    windowPanel.add(infoPanel,BorderLayout.CENTER);
    windowPanel.add(buttonPanel,BorderLayout.SOUTH);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+25,500);
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
    else if(e.getSource() == b_save)
    {
      try
      {
        for(int i = 0; i < data.block.getNumberOfBlocks(); i++)
        {
          data.block.factors[i].setValue(blockValue[i].getText());
        }

        dispose();
      } catch(Exception exc) {}
    }
  }
}

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
import tools.*;

/*
* Esta clase crea la ventana de configuración de las COLLECTIVE OPERATIONS.
*/
public class CollectiveOpWindow extends GUIWindow
{
  public static final long serialVersionUID = 3L;
  
  private JTextField tf_number = createTextField(2);

  private boolean internalOperations;

  private JButton b_same = createButton("Do all the same");
  private JButton b_save = createButton("Save");
  private JButton b_close = createButton("Close");
  private JButton b_left = createButton("<<<");
  private JButton b_right = createButton(">>>");

  private JComboBox[] col1 = createComboBox(true,data.DEFAULT_MPI_ITEMS);
  private JComboBox[] col2 = createComboBox(false,data.DEFAULT_MPI_ITEMS);
  private JComboBox[] col3 = createComboBox(true,data.DEFAULT_MPI_ITEMS);
  private JComboBox[] col4 = createComboBox(false,data.DEFAULT_MPI_ITEMS);
  private JComboBox[] cb_equals1 = createComboBox(true,1);
  private JComboBox[] cb_equals2 = createComboBox(false,1);
  private JComboBox[] cb_equals3 = createComboBox(true,1);
  private JComboBox[] cb_equals4 = createComboBox(false,1);

  private JPanel colModelIn;
  private JPanel colSizeIn;
  private JPanel colModelOut;
  private JPanel colSizeOut;
  private JPanel buttonPanelTop;
  private JPanel MPI_panel;
  private JPanel items;
  private JPanel buttonPanelBottom;

  /*
  * El método createComboBox genera el número de selectores Swing indicado por
  * @elementes, con las opciones requeridas según a la columna a la que irá
  * destinado dicho selector/es.
  *
  * @param: · boolean odd -> columna a la que va destinado el/los selector/es
  *                          creado/s.
  *                           - TRUE columna MODEL (0, LOG, LIN Y CT).
  *                           - FALSE columna SIZE (MIN, MAX, MEAN 2MAX y S+R).
  *         · int elements -> número de selectores a crear.
  *
  * @ret JComboBox[]: Selectores Swing creados.
  */
  private JComboBox[] createComboBox(boolean odd, int elements)
  {
    JComboBox[] cb = new JComboBox[elements];

    for(int i = 0; i < elements; i++)
    {
      cb[i] = new JComboBox();

      cb[i].addItem("Select");

      if(odd)
      {
        cb[i].addItem("0");
        cb[i].addItem("LOG");
        cb[i].addItem("LIN");
        cb[i].addItem("CT");
      }
      else
      {
        cb[i].addItem("MIN");
        cb[i].addItem("MAX");
        cb[i].addItem("MEAN");
        cb[i].addItem("2MAX");
        cb[i].addItem("S+R");
      }
    }

    if(elements == 1)
    {
      cb[0].addActionListener(this);
    }

    return cb;
  }

  /*
  * Método createTextField crea un campo de texto (<<< [contador] >>>) que
  * indica la operación colectiva se esta configurado en un momento dado (solo
  * aplicable en el caso de las operaciones colectivas internas).
  *
  * @param: int num -> tamaño del campo de texto.
  *
  * @ret JTextField: Campo de texto Swing que será usado a modo de contador.
  */
  private JTextField createTextField(int num)
  {
    JTextField tf = new JTextField(num);

    tf.setText("1");
    tf.setBorder(null);
    tf.setEditable(false);

    return tf;
  }

  // Método que crea los paneles con los componentes correspondientes.
  private void createPanels()
  {
    JPanel fan_in;
    JPanel fan_out;
    JPanel MPI_names;

    buttonPanelTop = new JPanel(new FlowLayout());
    buttonPanelTop.add(new JLabel("Machine number"));
    buttonPanelTop.add(b_left);
    buttonPanelTop.add(tf_number);
    buttonPanelTop.add(b_right);

    buttonPanelBottom = new JPanel(new FlowLayout());
    buttonPanelBottom.add(b_save);

    if(internalOperations)
    {
      buttonPanelBottom.add(b_same);
    }

    buttonPanelBottom.add(b_close);

    MPI_panel = new JPanel(new GridLayout(1,1));
    MPI_names = new JPanel(new GridLayout(16,1));

    fan_in = new JPanel(new GridLayout(1,2));
    fan_out = new JPanel(new GridLayout(1,2));

    colModelIn = new JPanel(new GridLayout(16,1));
    colSizeIn = new JPanel(new GridLayout(16,1));
    colModelOut = new JPanel(new GridLayout(16,1));
    colSizeOut = new JPanel(new GridLayout(16,1));

    fan_in.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"FAN IN"));
    fan_out.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"FAN OUT"));

    colModelIn.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"Model"));
    colSizeIn.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"Size"));
    colModelOut.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"Model"));
    colSizeOut.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"Size"));

    for(int i = 0; i < data.DEFAULT_MPI_ITEMS; i++)
    {
      colModelIn.add(col1[i]);
      colSizeIn.add(col2[i]);
      colModelOut.add(col3[i]);
      colSizeOut.add(col4[i]);
    }

    colModelIn.add(new JLabel("-------",JLabel.CENTER));
    colSizeIn.add(new JLabel("-------",JLabel.CENTER));
    colModelOut.add(new JLabel("-------",JLabel.CENTER));
    colSizeOut.add(new JLabel("-------",JLabel.CENTER));

    colModelIn.add(cb_equals1[0]);
    colSizeIn.add(cb_equals2[0]);
    colModelOut.add(cb_equals3[0]);
    colSizeOut.add(cb_equals4[0]);

    fan_in.add(colModelIn);
    fan_in.add(colSizeIn);
    fan_out.add(colModelOut);
    fan_out.add(colSizeOut);

    MPI_names.add(new JLabel("  MPI_Barrier",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Bcast",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Gather",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Gatherv",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Scatter",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Scatterv",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Allgather",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Allgatherv",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Alltoall",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Alltoallv",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Reduce",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Allreduce",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Reduce_Scatter",JLabel.LEFT));
    MPI_names.add(new JLabel("  MPI_Scan",JLabel.LEFT));
    MPI_names.add(new JLabel("--------------------",JLabel.CENTER));
    MPI_names.add(new JLabel("Apply to all:",JLabel.CENTER));
    MPI_names.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"Name"));

    MPI_panel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),"COLLECTIVE OP."));
    MPI_panel.add(MPI_names);

    items = new JPanel(new GridLayout(1,2));

    items.add(fan_in,BorderLayout.CENTER);
    items.add(fan_out,BorderLayout.CENTER);
  }

  // Constructor de la clase CollectiveOpWindow.
  public CollectiveOpWindow(Data d, boolean internal)
  {
    super(d);

    // Propiedades de la ventana.
    setResizable(true);
    windowPanel.setLayout(new BorderLayout());
    internalOperations = internal;
    createPanels();

    // Añadiendo los componentes a la ventana.
    if(internalOperations)
    {
      setTitle("Internal collective operations");
      windowPanel.add(buttonPanelTop,BorderLayout.NORTH);
      data.environment.mpiBackup();
    }
    else
    {
      setTitle("External collective operations");
    }

    windowPanel.add(MPI_panel,BorderLayout.WEST);
    windowPanel.add(items,BorderLayout.CENTER);
    windowPanel.add(buttonPanelBottom,BorderLayout.SOUTH);

    // Obteniendo información.
    fillInfo(0);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+25,getHeight());
    pack();
    setVisible(true);
  }

  /*
  * El método increase incrementa el valor del contador que indica qué operación
  * colectiva interna se esta visualizando por pantalla. Si se sobrepasa el
  * número de elementos entonces se volverá al primer elemento (cíclico).
  *
  * @param: String stz -> valor actual del contador.
  *
  * @ret String: Nuevo valor del contador.
  */
  private String increase(String stz)
  {
    int aux = Integer.parseInt(stz);

    if(aux == data.environment.getNumberOfMachines())
    {
      return "1";
    }
    else
    {
      return String.valueOf(++aux);
    }
  }

  /*
  * El método decrease decrementa el valor del contador que indica qué operación
  * colectiva interna se esta visualizando por pantalla. Si se sobrepasa el
  * número de elementos entonces se volverá al último elemento (cíclico).
  *
  * @param: String stz -> valor actual del contador.
  *
  * @ret String: Nuevo valor del contador.
  */
  private String decrease(String stz)
  {
    int aux = Integer.parseInt(stz);

    if(aux == 1)
    {
      return data.wan.getMachines();
    }
    else
    {
      return String.valueOf(--aux);
    }
  }

  /*
  * El método fillInfo actualiza todos los componentes de la pantalla para que
  * muestren la información correspondiente al elemento indicado por @index.
  *
  * @param: int index -> índice del elemento del que se obtiene la información.
  */
  private void fillInfo(int index)
  {
    cb_equals1[0].setSelectedIndex(0);
    cb_equals2[0].setSelectedIndex(0);
    cb_equals3[0].setSelectedIndex(0);
    cb_equals4[0].setSelectedIndex(0);

    for(int i = Data.DEFAULT_MPI_ITEMS-1; i >= 0; i--)
    {
      if(internalOperations) // Operaciones colectivas internas.
      {
        if(data.environment.machine[index].mpiConfigured())
        {
          col1[i].setSelectedIndex(data.environment.machine[index].mpiGetValue(1,i));
          col2[i].setSelectedIndex(data.environment.machine[index].mpiGetValue(2,i));
          col3[i].setSelectedIndex(data.environment.machine[index].mpiGetValue(3,i));
          col4[i].setSelectedIndex(data.environment.machine[index].mpiGetValue(4,i));
        }
        else // Op. colectivas no configuradas -> obtener info de la máquina.
        {
          if(data.environment.machine[index].getCommunication().equalsIgnoreCase(data.environment.COMM_CT))
          {
            col1[i].setSelectedIndex(4);
          }
          else if(data.environment.machine[index].getCommunication().equalsIgnoreCase(data.environment.COMM_LIN))
          {
            col1[i].setSelectedIndex(3);
          }
          else if(data.environment.machine[index].getCommunication().equalsIgnoreCase(data.environment.COMM_LOG))
          {
            col1[i].setSelectedIndex(2);
          }

          col2[i].setSelectedIndex(Tools.mpiValue(1,"MAX"));
          col3[i].setSelectedIndex(Tools.mpiValue(2,"0"));
          col4[i].setSelectedIndex(Tools.mpiValue(3,"MAX"));
        }
      }
      else                   // Operaciones colectivas externas.
      {
        if(data.wan.mpiGetValue(1,i) != 0)
        {
          col1[i].setSelectedIndex(data.wan.mpiGetValue(1,i));
          col2[i].setSelectedIndex(data.wan.mpiGetValue(2,i));
          col3[i].setSelectedIndex(data.wan.mpiGetValue(3,i));
          col4[i].setSelectedIndex(data.wan.mpiGetValue(4,i));
        }
        else // Op. colectivas no configuradas -> obtener info de la red WAN.
        {
          if(data.wan.getCommunication().equalsIgnoreCase(data.wan.COMM_GROUP_CT))
          {
            col1[i].setSelectedIndex(4);
          }
          else if(data.wan.getCommunication().equalsIgnoreCase(data.wan.COMM_GROUP_LIN))
          {
            col1[i].setSelectedIndex(3);
          }
          else if(data.wan.getCommunication().equalsIgnoreCase(data.wan.COMM_GROUP_LOG))
          {
            col1[i].setSelectedIndex(2);
          }

          col2[i].setSelectedIndex(Tools.mpiValue(1,"MAX"));
          col3[i].setSelectedIndex(Tools.mpiValue(2,"0"));
          col4[i].setSelectedIndex(Tools.mpiValue(3,"MAX"));
        }
      }
    }
  }

  /*
  * El método storeInfo salva los datos de las operaciones colectivas en las
  * estructuras de datos.
  *
  * @param: int index -> índice del elemento al que pertenece la información.
  *
  * @ret boolean: TRUE si la operación ha tenido éxito, FALSE en otro caso.
  */
  private boolean storeInfo(int index)
  {
    for(int i = Data.DEFAULT_MPI_ITEMS-1; i >= 0; i--)
    {
      if(col1[i].getSelectedIndex() == 0 || col2[i].getSelectedIndex() == 0 ||
         col3[i].getSelectedIndex() == 0 || col4[i].getSelectedIndex() == 0)
      {
        Tools.showInformationMessage("One or more values are not set.");
        return false;
      }
      else
      {
        if(internalOperations)
        {
          if(index >= data.environment.getNumberOfMachines())
          {
            return false;
          }

          data.environment.machine[index].mpiSetValue(1,i,col1[i].getSelectedIndex());
          data.environment.machine[index].mpiSetValue(2,i,col2[i].getSelectedIndex());
          data.environment.machine[index].mpiSetValue(3,i,col3[i].getSelectedIndex());
          data.environment.machine[index].mpiSetValue(4,i,col4[i].getSelectedIndex());
        }
        else
        {
          data.wan.mpiSetValue(1,i,col1[i].getSelectedIndex());
          data.wan.mpiSetValue(2,i,col2[i].getSelectedIndex());
          data.wan.mpiSetValue(3,i,col3[i].getSelectedIndex());
          data.wan.mpiSetValue(4,i,col4[i].getSelectedIndex());
        }
      }
    }

    return true;
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_left)
    {
      if(storeInfo(Integer.parseInt(tf_number.getText())-1))
      {
        tf_number.setText(decrease(tf_number.getText()));
        fillInfo(Integer.parseInt(tf_number.getText())-1);
      }
    }
    else if(e.getSource() == b_right)
    {
      if(storeInfo(Integer.parseInt(tf_number.getText())-1))
      {
        tf_number.setText(increase(tf_number.getText()));
        fillInfo(Integer.parseInt(tf_number.getText())-1);
      }
    }
    else if(e.getSource() == b_save)
    {
      if(storeInfo(Integer.parseInt(tf_number.getText())-1))
      {
        dispose();
      }
    }
    else if(e.getSource() == b_same)
    {
      for(int i = data.environment.getNumberOfMachines()-1; i >= 0; i--)
      {
        if(!storeInfo(i))
        {
          break;
        }
      }
    }
    else if(e.getSource() == b_close)
    {
      data.environment.mpiRestore();
      dispose();
    }
    else if(e.getSource() == cb_equals1[0])
    {
      for(int i = Data.DEFAULT_MPI_ITEMS-1; i >= 0; i--)
      {
        col1[i].setSelectedIndex(cb_equals1[0].getSelectedIndex());
      }
    }
    else if(e.getSource() == cb_equals2[0])
    {
      for(int i = Data.DEFAULT_MPI_ITEMS-1; i >= 0; i--)
      {
        col2[i].setSelectedIndex(cb_equals2[0].getSelectedIndex());
      }
    }
    else if(e.getSource() == cb_equals3[0])
    {
      for(int i = Data.DEFAULT_MPI_ITEMS-1; i >= 0; i--)
      {
        col3[i].setSelectedIndex(cb_equals3[0].getSelectedIndex());
      }
    }
    else if(e.getSource() == cb_equals4[0])
    {
      for(int i = Data.DEFAULT_MPI_ITEMS-1; i >= 0; i--)
      {
        col4[i].setSelectedIndex(cb_equals4[0].getSelectedIndex());
      }
    }
  }
}

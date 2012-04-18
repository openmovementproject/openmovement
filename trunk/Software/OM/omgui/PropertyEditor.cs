using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace OmGui
{
    public partial class PropertyEditor : Form
    {
        public object[] Items { get; protected set; }

        public PropertyEditor(string title, object item, bool canCancel)
            : this(title, new object[] { item }, canCancel)
        {
            ;
        }

        public PropertyEditor(string title, object[] items, bool canCancel)
        {
            InitializeComponent();
            Text = title;
            Items = items;
            propertyGrid.SelectedObjects = Items;
            buttonCancel.Visible = canCancel;
        }

        private void buttonOk_Click(object sender, EventArgs e)
        {
            DialogResult = System.Windows.Forms.DialogResult.OK;
            Close();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            DialogResult = System.Windows.Forms.DialogResult.Cancel;
            Close();
        }
    }
}

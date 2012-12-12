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
    public partial class OptionsDialog : Form
    {
        public OptionsDialog(string currentDefault)
        {
            InitializeComponent();

            btnOK.DialogResult = DialogResult.OK;
            btnCancel.DialogResult = DialogResult.Cancel;

            textBoxDefaultFolder.Text = currentDefault;
        }

        //Properties
        public String FolderName { get; set; }

        private void btnBrowse_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            DialogResult dr = fbd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                FolderName = fbd.SelectedPath;
                textBoxDefaultFolder.Text = FolderName;
            }
        }
    }
}

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
    public partial class ExportWavForm : Form
    {
        public ExportWavForm()
        {
            InitializeComponent();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void buttonResample_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void ExportWavForm_Load(object sender, EventArgs e)
        {

        }

        public int Rate { get { int rate = -1; int.TryParse(comboBoxRate.Text, out rate); return rate; } }
        public bool AutoCalibrate { get { return checkBoxAutoCalibrate.Checked; } }
    }
}

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
    public partial class ExportPaeeForm : Form
    {


        public ExportPaeeForm()
        {
            InitializeComponent();
            comboBox1.SelectedIndex = 0;
            comboBoxFilter.SelectedIndex = 1;
        }

        public string Settings
        {
            set
            {
                try
                {
                    Dictionary<string, string> settings = value.Split('&').ToDictionary(c => c.Split('=')[0], c => Uri.UnescapeDataString(c.Split('=')[1]));
                    int val;
                    if (settings.ContainsKey("epoch")) { comboBoxRate.Text = settings["epoch"]; }
                    if (settings.ContainsKey("model")) { comboBox1.Text = settings["model"]; }
                    if (int.TryParse(settings["filter"], out val)) { comboBoxFilter.SelectedIndex = val; }
                }
                catch (Exception e)
                {
                    Console.Out.WriteLine(e.StackTrace);
                }
            }
            get
            {
                IDictionary<string, string> settings = new Dictionary<string, string>();
                settings["epoch"] = comboBoxRate.SelectedIndex.ToString();
                settings["model"] = comboBox1.Text;
                settings["filter"] = comboBoxFilter.SelectedIndex.ToString();
                return string.Join("&", settings.Select((x) => x.Key + "=" + Uri.EscapeDataString(x.Value.ToString())).ToArray());
            }
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

        public int Epoch { get { int epoch = 1; int.TryParse(comboBoxRate.Text, out epoch); return epoch; } }
        public string Model { get { return comboBox1.Text; } }
        public int Filter { get { return comboBoxFilter.SelectedIndex; } }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void ExportPaeeForm_Load(object sender, EventArgs e)
        {

        }
    }
}

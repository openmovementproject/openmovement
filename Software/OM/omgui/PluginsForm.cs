using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace OmGui
{
    public partial class PluginsForm : Form
    {
        private List<Plugin> plugins = new List<Plugin>();
        public Plugin SelectedPlugin { get; set; }
        public Dictionary<string, string> SelectedParameters { get; set; }
        public string CWAFilename { get; set; }
        public RunPluginForm rpf { get; set; }

        public PluginsForm(List<Plugin> plugins, string fileName)
        {
            InitializeComponent();

            this.plugins = plugins;

            foreach (Plugin plugin in plugins)
            {
                pluginsComboBox.Items.Add(plugin.Type + " -- " + plugin.Name);
            }

            pluginsComboBox.SelectedIndex = 0;

            CWAFilename = fileName;
        }

        private void pluginsComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            //Chosen a plugin so show info.
            SelectedPlugin = plugins[pluginsComboBox.SelectedIndex];

            descriptionLabel.Text = SelectedPlugin.Description;

            if (SelectedPlugin.InputFile != "null")
            {
                inputFileLabel.Text = SelectedPlugin.InputFile;
            }
            else
            {
                inputFileLabel.Text = "No input file";
            }


            if (SelectedPlugin.OutputFile != "null")
            {
                outputFileLabel.Text = SelectedPlugin.InputFile;
            }
            else
            {
                outputFileLabel.Text = "No output file";
            }
            
        }

        private void btnRun_Click(object sender, EventArgs e)
        {
            //Want to pop up Input and Run Window.
            rpf = new RunPluginForm(SelectedPlugin, CWAFilename);
            rpf.ShowDialog();

            if (rpf.DialogResult == System.Windows.Forms.DialogResult.OK)
            {
                SelectedParameters = rpf.SelectedParameters;
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Close();

            //Also returns DialogResult.Cancel
        }
    }
}

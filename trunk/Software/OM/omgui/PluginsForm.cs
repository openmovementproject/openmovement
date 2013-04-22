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

        public float BlockStart { get; set; }
        public float BlockCount { get; set; }
        public string StartTime { get; set; }
        public string EndTime { get; set; }

        public PluginManager PluginManager { get; set; }
        
        public PluginsForm(PluginManager manager, string fileName, float blockStart, float blockCount, string startDateTime, string endDateTime)
        {
            InitializeComponent();

            PluginManager = manager;

            //Blocks from dataViewer...
            BlockStart = blockStart;
            BlockCount = blockCount;

            StartTime = null;
            EndTime = null;

            if(!startDateTime.Equals(""))
                StartTime = startDateTime;

            if(!endDateTime.Equals(""))
                EndTime = endDateTime;

            plugins = PluginManager.Plugins;

            if (plugins.Count < 1)
            {
                PluginManager.LoadPlugins();
                plugins = PluginManager.Plugins;
            }

            if (plugins.Count > 0)
            {
                foreach (Plugin plugin in plugins)
                {
                    pluginsComboBox.Items.Add(plugin.ReadableName);
                }

                pluginsComboBox.SelectedIndex = 0;

                CWAFilename = fileName;
            }
        }

        private void pluginsComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            //Chosen a plugin so show info.
            SelectedPlugin = plugins[pluginsComboBox.SelectedIndex];

            descriptionLabel.Text = SelectedPlugin.Description;

            //TS - Not worrying about input and output file anymore.
            //if (SelectedPlugin.InputFile != "null")
            //{
            //    inputFileLabel.Text = SelectedPlugin.InputFile;
            //}
            //else
            //{
            //    inputFileLabel.Text = "No input file";
            //}

            //if (SelectedPlugin.OutputFile != "null")
            //{
            //    outputFileLabel.Text = SelectedPlugin.InputFile;
            //}
            //else
            //{
            //    outputFileLabel.Text = "No output file";
            //}
        }

        private void btnRun_Click(object sender, EventArgs e)
        {
            //See now if we want the dataViewer selection.
            if (BlockCount > -1 && BlockStart > -1)
            {
                SelectedPlugin.SelectionBlockStart = BlockStart;
                SelectedPlugin.SelectionBlockCount = BlockCount;
            }

            if (StartTime != null && EndTime != null)
            {
                SelectedPlugin.SelectionDateTimeStart = StartTime;
                SelectedPlugin.SelectionDateTimeEnd = EndTime;
            }  

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

        private void buttonRefresh_Click(object sender, EventArgs e)
        {
            //Refresh the plugins in the manager.
            PluginManager.LoadPlugins();

            pluginsComboBox.Items.Clear();

            plugins = PluginManager.Plugins;

            foreach (Plugin plugin in plugins)
            {
                pluginsComboBox.Items.Add(plugin.ReadableName);
            }

            pluginsComboBox.SelectedIndex = 0;
        }
    }
}

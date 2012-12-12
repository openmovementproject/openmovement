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
    public partial class RunPluginForm : Form
    {
        Plugin Plugin { get; set; }
        public RunPluginForm(Plugin plugin)
        {
            InitializeComponent();

            Plugin = plugin;

            pluginNameLabel.Text = Plugin.Name;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.Close();

            //Also returns DialogResult.Cancel
        }

        private void okBtn_Click(object sender, EventArgs e)
        {
            //System.Diagnostics.Process.Start(Plugin.RunFile.FullName);
            System.Diagnostics.Process.Start("C:\\Program Files (x86)\\Notepad++\\notepad++.exe");

            //Also returns DialogResult.OK
        }

        private void RunPluginForm_Load(object sender, EventArgs e)
        {
            int x = 10;
            int y = 40;

            //Create text boxes.
            foreach (KeyValuePair<string, string> pair in Plugin.ActualParameters)
            {
                Label label = new Label();
                label.Text = pair.Key;
                label.Location = new Point(x, y);
                this.Controls.Add(label);

                x += 20;

                if (pair.Value == "string")
                {
                    TextBox textBox = new TextBox();
                    textBox.Location = new Point(x, y);
                    textBox.Size = new Size(184, 72);
                    this.Controls.Add(textBox);
                }
                else if (pair.Value == "combo")
                {
                    ComboBox comboBox = new ComboBox();
                }

                x -= 20;

                y += 30;
            }
        }
    }
}

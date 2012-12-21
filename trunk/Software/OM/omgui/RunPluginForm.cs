using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;

namespace OmGui
{
    public partial class RunPluginForm : Form
    {
        bool firstGone = false;
        
        Plugin Plugin { get; set; }

        public RunPluginForm(Plugin plugin)
        {
            InitializeComponent();

            Plugin = plugin;

            Go(Plugin.HTMLFile.FullName);
        }

        public void Go(String file)
        {
            string url = "file:///" + file.Replace("\\", "/");
            this.webBrowser1.Url = new System.Uri(url, System.UriKind.RelativeOrAbsolute);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.Close();

            //Also returns DialogResult.Cancel
        }

        private void okBtn_Click(object sender, EventArgs e)
        {
            //System.Diagnostics.Process.Start(Plugin.RunFile.FullName);
            

            //Also returns DialogResult.OK
        }

        private void RunPluginForm_Load(object sender, EventArgs e)
        {
            //int x = 10;
            //int y = 40;

            //TS - Old dynamic controls-adding - now using webbrowser.
            ////Create text boxes.
            //foreach (KeyValuePair<string, string> pair in Plugin.ActualParameters)
            //{
            //    Label label = new Label();
            //    label.Text = pair.Key;
            //    label.Location = new Point(x, y);
            //    this.Controls.Add(label);

            //    x += 20;

            //    if (pair.Value == "string")
            //    {
            //        TextBox textBox = new TextBox();
            //        textBox.Location = new Point(x, y);
            //        textBox.Size = new Size(184, 72);
            //        this.Controls.Add(textBox);
            //    }
            //    else if (pair.Value == "combo")
            //    {
            //        ComboBox comboBox = new ComboBox();
            //    }

            //    x -= 20;

            //    y += 30;
            //}
        }

        //TODO Got a problem that it will hit here twice.
        private void webBrowser1_Navigating(object sender, WebBrowserNavigatingEventArgs e)
        {
            Console.WriteLine("Location: " + e.Url.ToString());

            string[] url = e.Url.ToString().Split(new string[] { "%3F" }, StringSplitOptions.None);

            //Url will be of length 1 first time, after JavaScript 'enter' it will be length 2
            if (url.Length == 2)
            {
                //HACK - solve the hit here twice problem.
                if (firstGone)
                {
                    string[] keypairs = url[1].Split('&');

                    Dictionary<string, string> keypairdic = new Dictionary<string, string>();

                    string parametersStr = "";

                    foreach (string keypair in keypairs)
                    {
                        string[] keyvalue = keypair.Split('=');
                        keypairdic.Add(keyvalue[0], keyvalue[1]);

                        parametersStr += " " + keyvalue[1];
                    }

                    //Now we've got key value pairs we can run the exe
                    //System.Diagnostics.Process.Start(Plugin.RunFile.FullName + parametersStr);

                    Process p = new Process();
                    p.StartInfo.FileName = Plugin.RunFile.FullName;
                    p.StartInfo.Arguments = parametersStr;

                    p.Start();

                    p.WaitForExit();
                    p.Close();
                }
                else
                    firstGone = true;
            }
        }
    }
}
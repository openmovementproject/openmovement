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

                    RunProcess(parametersStr);
                }
                else
                    firstGone = true;
            }
        }

        private void RunProcess(string parametersAsString)
        {
            Process p = new Process();
            p.StartInfo.FileName = Plugin.RunFile.FullName;
            p.StartInfo.Arguments = parametersAsString;
            p.StartInfo.RedirectStandardOutput = true;
            p.StartInfo.RedirectStandardError = true;
            p.StartInfo.UseShellExecute = false;
            p.StartInfo.CreateNoWindow = true;

            p.Start();

            //READ STD OUT OF PLUGIN
            //We want to read the info and decide what to do based on it.
            //p - If it starts with p then percentage. [p 10%]
            //s - If it starts with s then status update. [status New Status Here]
            while (!p.HasExited)
            {
                string outputLine = p.StandardOutput.ReadLine();

                //OUTPUT
                if (outputLine != null)
                {
                    if (outputLine[0] == 'p')
                    {
                        string percentage = outputLine.Split(' ').Last();
                        runPluginProgressBar.Value = Int32.Parse(percentage);
                    }
                    else if (outputLine[0] == 's')
                    {
                        string message = outputLine.Split(new char[] { ' ' }, 2).Last();
                        labelStatus.Text = message;
                    }
                }

                Console.WriteLine("o: " + outputLine);

                runPluginProgressBar.Invalidate(true);
                labelStatus.Invalidate(true);
            }

            p.WaitForExit();
            p.Close();
        }
    }
}
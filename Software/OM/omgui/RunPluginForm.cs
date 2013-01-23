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

        private string CWAFilename { get; set; }
        private string TempCWAFilePath { get; set; }

        private string OutputName { get; set; }
        private string OriginalOutputName { get; set; }

        //Temp folder for Matlab Hack
        string destFolder = "C:\\OM\\PluginTemp\\";

        public RunPluginForm(Plugin plugin, string filename)
        {
            InitializeComponent();

            Plugin = plugin;

            Height = Plugin.Height;
            Width = Plugin.Width;

            CWAFilename = filename;

            TempCWAFilePath = System.IO.Path.Combine(destFolder, "input.cwa");

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

            string[] url = e.Url.ToString().Split(new string[] { "#" }, StringSplitOptions.None);

            //Url will be of length 1 first time, after JavaScript 'enter' it will be length 2
            if (url.Length == 2)
            {
                string[] keypairs = url[1].Split('&');

                Dictionary<string, string> keypairdic = new Dictionary<string, string>();

                string parametersStr = TempCWAFilePath;

                foreach (string keypair in keypairs)
                {
                    string[] keyvalue = keypair.Split('=');

                    if (keyvalue.Length > 1)
                    {
                        keypairdic.Add(keyvalue[0], keyvalue[1]);

                        //If there is an output file:
                        if (Plugin.OutputFile != "")
                        {
                            if (keyvalue[0].Equals("\"" + Plugin.OutputExt + "\""))
                            {
                                //Original name
                                OriginalOutputName = keyvalue[1];

                                parametersStr += " " + destFolder + "temp.csv";
                            }
                            else
                            {
                                parametersStr += " " + keyvalue[1];
                            }
                        }
                    }
                }
                    
                //Copy input and output file into directory with no spaces because of the Matlab hack.
                string sourceFile = CWAFilename;

                if (!System.IO.Directory.Exists(destFolder))
                    System.IO.Directory.CreateDirectory(destFolder);

                //Stick the .CWA in the temp folder
                System.IO.File.Copy(sourceFile, TempCWAFilePath, true);
                    
                RunProcess(parametersStr);
            }
        }

        private void RunProcess(string parametersAsString)
        {
            //TOM TODO - Add in so we can run PY and JAR files as well as EXE
            //if (Plugin.Ext == Plugin.ExtType.PY)
            //{
            //    p.StartInfo.FileName = "python " + Plugin.RunFile.FullName;
            //}

            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = Plugin.RunFile.FullName;
            //psi.FileName = @"H:\OM\Plugins\TestParameterApp.exe";

            psi.Arguments = parametersAsString;

            Console.WriteLine("arg: " + psi.Arguments);

            psi.RedirectStandardOutput = true;
            psi.RedirectStandardError = true;

            psi.UseShellExecute = false;
            psi.Verb = "runas";
            psi.CreateNoWindow = true;

            Process p = null;

            p = Process.Start(psi);

            //READ STD OUT OF PLUGIN
            //We want to read the info and decide what to do based on it.
            //p - If it starts with p then percentage. [p 10%]
            //s - If it starts with s then status update. [status New Status Here]
            while (!p.HasExited)
            {
                string outputLine = p.StandardOutput.ReadLine();

                parseMessage(outputLine);

                runPluginProgressBar.Invalidate(true);
                labelStatus.Invalidate(true);
            }

            p.WaitForExit();

            //Hack - Sometimes we don't get the last stdout line
            string lastLine = p.StandardOutput.ReadLine();
            parseMessage(lastLine);

            p.Close();

            //If there is an output file:
            if (Plugin.OutputFile != "")
            {
                //HACK - Copy the output file back into the working directory.
                string outputFileLocation = System.IO.Path.Combine(Properties.Settings.Default.CurrentWorkingFolder, OriginalOutputName);
                System.IO.File.Copy(destFolder + "temp.csv", outputFileLocation);

                //Delete the test csv and temp cwa
                System.IO.File.Delete(destFolder + "temp.csv");
                System.IO.File.Delete(TempCWAFilePath);
            }
        }

        private void parseMessage(string outputLine)
        {
            //OUTPUT
            if (outputLine != null)
            {
                if (outputLine[0] == 'p')
                {
                    string percentage = outputLine.Split(' ').ElementAt(1);
                    runPluginProgressBar.Value = Int32.Parse(percentage);
                }
                else if (outputLine[0] == 's')
                {
                    string message = outputLine.Split(new char[] { ' ' }, 2).Last();
                    labelStatus.Text = message;
                }
            }
            Console.WriteLine("o: " + outputLine);
        }
    }
}
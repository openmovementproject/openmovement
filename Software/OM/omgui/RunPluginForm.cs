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
using System.Xml;

namespace OmGui
{
    public partial class RunPluginForm : Form
    {
        bool firstGone = false;
        
        Plugin Plugin { get; set; }

        private string CWAFilename { get; set; }
        public string TempCWAFilePath { get; set; }

        private string OutputName { get; set; }
        public string OriginalOutputName { get; set; }
        public string ChosenExtension { get; set; }

        public string ParameterString { get; set; }

        public Dictionary<string, string> SelectedParameters { get; set; }

        //Temp folder for Matlab Hack
        public string destFolder = "C:\\OM\\PluginTemp\\";

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

            //Add block parameters if needed
            if (Plugin.CanSelection && Plugin.BlockStart > -1 && Plugin.BlockCount > -1)
            {
                url += "?";
                url += "blockStart=" + Plugin.BlockStart;
                url += "&blockCount=" + Plugin.BlockCount;
            }

            string profileString = loadXmlProfile();

            if (profileString != null)
            {
                url += profileString;
            }

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

                SelectedParameters = new Dictionary<string, string>();

                ParameterString = TempCWAFilePath;

                foreach (string keypair in keypairs)
                {
                    string[] keyvalue = keypair.Split('=');

                    if (keyvalue.Length > 1)
                    {
                        SelectedParameters.Add(keyvalue[0], keyvalue[1]);

                        //If there is an output file:
                        if (Plugin.OutputFile != "")
                        {
                            //Then get the extension...
                            
                            //If we can have multiple extensions then look for the extension key in the url
                            if (Plugin.OutputExts.Length > 1)
                            {
                                if (keyvalue[0].Equals("\"extension\""))
                                {
                                    ChosenExtension = keyvalue[1];
                                }
                                else if (keyvalue[0].Equals("\"outFileName\""))
                                {
                                    OriginalOutputName = keyvalue[1];
                                }
                                else
                                {
                                    ParameterString += " " + keyvalue[1];
                                }
                            }
                            else
                            {
                                if (keyvalue[0].Equals("\"" + Plugin.OutputExts[0] + "\""))
                                {
                                    //Original name
                                    OriginalOutputName = keyvalue[1];

                                    string ext = keyvalue[1].Split('.')[1];

                                    ParameterString += " " + destFolder + "temp." + ext;
                                }
                                else
                                {
                                    ParameterString += " " + keyvalue[1];
                                }
                            }

                            //For multiple output extension
                            if (Plugin.OutputExts.Length > 1)
                            {
                                if ((ChosenExtension != null || !ChosenExtension.Equals("")))
                                {
                                    ParameterString += " " + destFolder + "temp." + ChosenExtension;
                                }
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

                //Now we want to save the settings for this plugin
                saveXmlProfile();

                DialogResult = System.Windows.Forms.DialogResult.OK;
                return;
            }
        }

        private void saveXmlProfile()
        {
            XmlDocument xml = new XmlDocument();
            XmlNode outerNode = xml.CreateElement(Plugin.Name + "Profile");
            xml.AppendChild(outerNode);

            foreach (KeyValuePair<string, string> kvp in SelectedParameters)
            {
                string key = kvp.Key.Substring(1, kvp.Key.Length - 2);
                XmlNode node = xml.CreateElement(key);
                node.InnerText = kvp.Value;
                outerNode.AppendChild(node);
            }

            try
            {
                xml.Save(Properties.Settings.Default.CurrentWorkingFolder + "\\" + Plugin.Name + "_profile.xml");
            }
            catch (XmlException e)
            {
                Console.WriteLine("Xml Error: Could not save " + Plugin.Name + "_profile.xml - " + e.Message);
            }
            catch (Exception e)
            {
                Console.WriteLine("XML Saving Exception: " + e.Message);
                return;
            }
        }

        private string loadXmlProfile()
        {
            XmlDocument xmlDoc = new XmlDocument();

            bool first = true;

            string parameterString = null;
            
            if (File.Exists(Properties.Settings.Default.CurrentWorkingFolder + "\\" +  Plugin.Name + "_profile.xml"))
            {
                StreamReader recordProfile = new StreamReader(Properties.Settings.Default.CurrentWorkingFolder +
                    "\\" + Plugin.Name + "_profile.xml");
                String profileAsString = recordProfile.ReadToEnd();

                XmlDocument xmlDocument = new XmlDocument();
                xmlDocument.LoadXml(profileAsString);

                XmlNode rootNode = xmlDocument.DocumentElement;

                parameterString = "?";

                foreach (XmlNode node in rootNode.ChildNodes)
                {
                    if (!node.Name.Equals(Plugin.Name + "Profile"))
                    {
                        if (first)
                            first = false;
                        else
                            parameterString += "&";

                        parameterString += node.Name + "=" + node.InnerText;
                    }
                }
            }

            return parameterString;
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

                //labelStatus.Text = parseMessage(outputLine);

                //runPluginProgressBar.Invalidate(true);
                //labelStatus.Invalidate(true);
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
                    //runPluginProgressBar.Value = Int32.Parse(percentage);
                }
                else if (outputLine[0] == 's')
                {
                    string message = outputLine.Split(new char[] { ' ' }, 2).Last();
                    //labelStatus.Text = message;
                }
                else if (outputLine[0] == 'e')
                {
                    string message = outputLine.Split(new char[] { ' ' }, 2).Last();
                    //labelStatus.ForeColor = Color.Red;
                    //labelStatus.Text = message;
                }

                Console.WriteLine("o: " + outputLine);
            }
        }

        private void RunPluginForm_FormClosing(object sender, FormClosingEventArgs e)
        {
        
        }
    }
}
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Net;
using System.Xml;
using System.Runtime.Serialization;
using System.Drawing;

namespace OmGui
{
    public class Plugin
    {
        //Properties from XML
        public int Height { get; set; }
        public int Width { get; set; }
        public string RunFilePath { get; set; }
        public string HTMLFilePath { get; set; }
        public string SavedValuesFilePath { get; set; }
        public string iconFilePath { get; set; }
        public string Description { get; set; }
        public string FileName { get; set; }
        public string ReadableName { get; set; }
        public string[] OutputExtensions { get; set; }
        public string OutputFile { get; set; }
        public string InputFile { get; set; }
        public Dictionary<string, string> DefaultValues { get; set; }
        public string FilePath { get; set; }
        public bool WantMetaData { get; set; }
        public int NumberOfInputFiles { get; set; }
        public bool RequiresCWANames { get; set; }
        public bool CreatesOutput { get; set; }

        //Properties from Program
        public float SelectionBlockStart { get; set; }
        public float SelectionBlockCount { get; set; }
        public string SelectionDateTimeStart { get; set; }
        public string SelectionDateTimeEnd { get; set; }
        public Image Icon { get; set; }

        public static Plugin CreatePlugin(string pluginSettingsFolderPath)
        {
            Plugin plugin = null;

            Console.WriteLine("\nPlugin:" + Path.GetFileNameWithoutExtension(pluginSettingsFolderPath) + "\n");

            try
            {
                string[] files = Directory.GetFiles(pluginSettingsFolderPath, "*.plugin");

                if (files.Length > 0)
                {
                    //Get first because there should only be 1 .plugin file.
                    string pluginSettingsPath = files[0];

                    try
                    {
                        StreamReader sr = new StreamReader(pluginSettingsPath);
                        string pluginSettingsRaw = sr.ReadToEnd();

                        if (pluginSettingsRaw.Length > 0)
                        {
                            XmlDocument pluginSettingsXML = new XmlDocument();

                            try
                            {
                                plugin = new Plugin();

                                plugin.FilePath = pluginSettingsFolderPath;

                                pluginSettingsXML.LoadXml(pluginSettingsRaw);

                                //Go through XML and fill a Plugin() instance.
                                XmlNodeList heightXML = pluginSettingsXML.GetElementsByTagName("height");
                                XmlNodeList widthXML = pluginSettingsXML.GetElementsByTagName("width");
                                XmlNodeList runFilePathXML = pluginSettingsXML.GetElementsByTagName("runFilePath");
                                XmlNodeList htmlFilePathXML = pluginSettingsXML.GetElementsByTagName("htmlFilePath");
                                XmlNodeList savedValuesFilePathXML = pluginSettingsXML.GetElementsByTagName("savedValuesFilePath");
                                XmlNodeList iconFilePathXML = pluginSettingsXML.GetElementsByTagName("iconName");
                                XmlNodeList descriptionXML = pluginSettingsXML.GetElementsByTagName("description");
                                XmlNodeList readableNameXML = pluginSettingsXML.GetElementsByTagName("readableName");
                                XmlNodeList fileNameXML = pluginSettingsXML.GetElementsByTagName("fileName");
                                XmlNodeList outputExtensionsXML = pluginSettingsXML.GetElementsByTagName("outputExtensions");
                                XmlNodeList defaultValuesXML = pluginSettingsXML.GetElementsByTagName("defaultValues");
                                XmlNodeList outputFileXML = pluginSettingsXML.GetElementsByTagName("outputFile");
                                XmlNodeList inputFileXML = pluginSettingsXML.GetElementsByTagName("inputFile");
                                XmlNodeList wantMetaDataXML = pluginSettingsXML.GetElementsByTagName("wantMetadata");
                                XmlNodeList numberOfInputFilesXML = pluginSettingsXML.GetElementsByTagName("numberOfInputFiles");
                                XmlNodeList requiresCWANames = pluginSettingsXML.GetElementsByTagName("requiresCWANames");
                                XmlNodeList createsOutput = pluginSettingsXML.GetElementsByTagName("createsOutput");
                                
                                int height = 0;
                                if (heightXML.Count < 1 || !int.TryParse(heightXML[0].InnerText, out height))
                                    plugin.Height = 600;
                                else
                                    plugin.Height = height;

                                int width = 0;
                                if (widthXML.Count < 1 || !int.TryParse(widthXML[0].InnerText, out width))
                                    plugin.Width = 800;
                                else
                                    plugin.Width = width;

                                plugin.RunFilePath = "none";
                                if (runFilePathXML.Count > 0)
                                    plugin.RunFilePath = runFilePathXML[0].InnerText;
                                //TODO Else Throw new thingy

                                plugin.HTMLFilePath = "none";
                                if (htmlFilePathXML.Count > 0)
                                    plugin.HTMLFilePath = htmlFilePathXML[0].InnerText;
                                //TODO Else Throw new thingy

                                plugin.SavedValuesFilePath = "none";
                                if (savedValuesFilePathXML.Count > 0)
                                    plugin.SavedValuesFilePath = savedValuesFilePathXML[0].InnerText;

                                plugin.iconFilePath = "none";
                                if (iconFilePathXML.Count > 0)
                                {
                                    plugin.iconFilePath = iconFilePathXML[0].InnerText;

                                    try
                                    {
                                        plugin.loadIcon(plugin.iconFilePath);
                                    }
                                    catch (Exception)
                                    {
                                    }
                                }

                                plugin.Description = "none";
                                if (descriptionXML.Count > 0)
                                    plugin.Description = descriptionXML[0].InnerText;

                                plugin.ReadableName = "none";
                                if (readableNameXML.Count > 0)
                                    plugin.ReadableName = readableNameXML[0].InnerText;

                                plugin.FileName = "none";
                                if (fileNameXML.Count > 0)
                                    plugin.FileName = fileNameXML[0].InnerText;

                                if (outputExtensionsXML.Count > 0)
                                {
                                    plugin.OutputExtensions = new string[outputExtensionsXML[0].ChildNodes.Count];
                                    for (int i = 0; i < outputExtensionsXML[0].ChildNodes.Count; i++)
                                        plugin.OutputExtensions[i] = outputExtensionsXML[0].ChildNodes[i].InnerText;
                                }

                                plugin.DefaultValues = new Dictionary<string, string>();
                                if (defaultValuesXML.Count > 0)
                                    foreach (XmlNode node in defaultValuesXML[0].ChildNodes)
                                        plugin.DefaultValues.Add(node.Name, node.InnerText);

                                plugin.OutputFile = "none";
                                if (outputFileXML.Count > 0)
                                    plugin.OutputFile = outputFileXML[0].InnerText;

                                plugin.InputFile = "none";
                                if (inputFileXML.Count > 0)
                                    plugin.InputFile = inputFileXML[0].InnerText;

                                plugin.WantMetaData = false;
                                if (wantMetaDataXML.Count > 0)
                                    plugin.WantMetaData = wantMetaDataXML[0].InnerText.Equals("true") ? true : false;

                                plugin.NumberOfInputFiles = 1;
                                if (numberOfInputFilesXML.Count > 0)
                                    plugin.NumberOfInputFiles = Int32.Parse(numberOfInputFilesXML[0].InnerText);

                                plugin.RequiresCWANames = false;
                                if (requiresCWANames.Count > 0)
                                    plugin.RequiresCWANames = requiresCWANames[0].InnerText.Equals("true") ? true : false;

                                plugin.CreatesOutput = false;
                                if (createsOutput.Count > 0)
                                    plugin.CreatesOutput = createsOutput[0].InnerText.Equals("true") ? true : false;

                                Console.WriteLine("height: " + plugin.Height);
                                Console.WriteLine("width: " + plugin.Width);
                                Console.WriteLine("run file: " + plugin.RunFilePath);
                                Console.WriteLine("html file: " + plugin.HTMLFilePath);
                                Console.WriteLine("saved values file: " + plugin.SavedValuesFilePath);
                                Console.WriteLine("icon file: " + plugin.iconFilePath);
                                Console.WriteLine("description: " + plugin.Description);
                                Console.WriteLine("readable name: " + plugin.ReadableName);
                                Console.WriteLine("Want metadata: " + plugin.WantMetaData);
                                Console.WriteLine("Number of input files: " + plugin.NumberOfInputFiles);

                                Console.WriteLine("Output Extensions: ");
                                if (plugin.OutputExtensions != null && plugin.OutputExtensions.Length < 1)
                                    Console.Write("none");
                                else
                                    foreach (string outputExtension in plugin.OutputExtensions)
                                        Console.WriteLine("\tExtension: " + outputExtension);

                                Console.WriteLine("Default Values: ");
                                if (plugin.DefaultValues.Count < 1)
                                    Console.Write("none");
                                else
                                    foreach (KeyValuePair<string, string> kvp in plugin.DefaultValues)
                                        Console.WriteLine("\tDefault Value Name: " + kvp.Key + " | Value: " + kvp.Value);

                                //Now we want to look into the saved values.
                                Console.WriteLine("Saved Values: ");

                                Dictionary<string, string> savedValuesDictionary = GetSavedValues(Properties.Settings.Default.CurrentWorkingFolder + "\\" + plugin.ReadableName + "_" + plugin.SavedValuesFilePath);

                                if (savedValuesDictionary != null)
                                {
                                    foreach (KeyValuePair<string, string> kvp in savedValuesDictionary)
                                        Console.WriteLine("Saved Value Name: " + kvp.Key + " | Value: " + kvp.Value);
                                }
                                else
                                {
                                    Console.Write("No Saved Values file.\n");
                                }

                                plugin.SelectionBlockCount = -1;
                                plugin.SelectionBlockStart = -1;
                                plugin.SelectionDateTimeEnd = "";
                                plugin.SelectionDateTimeStart = "";
                                return plugin;
                            }
                            catch (XmlException)
                            {
                                Console.WriteLine("Error: Plugin Settings File is not valid XML.");
                            }
                        }

                        sr.Close();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("Error: Invalid Stream for Saved Values\r\n" + e.Message);
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Error: Could not find profile plugin: " + Path.GetFileNameWithoutExtension(pluginSettingsFolderPath) + "\r\n" + e.Message);
            }

            return null;
        }

        private static Dictionary<string, string> GetSavedValues(string savedValuesPath)
        {
            if (File.Exists(savedValuesPath))
            {
                try
                {
                    StreamReader sr = new StreamReader(savedValuesPath);
                    string savedValuesRaw = sr.ReadToEnd();

                    if (savedValuesRaw.Length > 0)
                    {
                        XmlDocument savedValuesXML = new XmlDocument();

                        try
                        {
                            Dictionary<string, string> savedValuesDictionary = new Dictionary<string, string>();

                            savedValuesXML.LoadXml(savedValuesRaw);

                            XmlNodeList xnl = savedValuesXML.GetElementsByTagName("SavedValues");

                            if (xnl.Count > 0)
                            {
                                XmlNodeList valuesList = xnl[0].ChildNodes;

                                foreach (XmlNode node in valuesList)
                                {
                                    savedValuesDictionary.Add(node.Name, node.InnerText);
                                }

                                return savedValuesDictionary;
                            }
                        }
                        catch (XmlException)
                        {
                            Console.WriteLine("Error: Invalid Saved Values XML File");
                        }
                    }

                    sr.Close();
                }
                catch (Exception e)
                {
                    Console.WriteLine("Error: Invalid Stream for Saved Values: " + e.Message);
                }
            }
            return null;
        }

        private void loadIcon(string name)
        {
            string filePath = FilePath + Path.DirectorySeparatorChar + name;

            try
            {
                Icon = Image.FromFile(filePath);
            }
            catch (System.IO.FileNotFoundException e)
            {
                Console.WriteLine("No Icon: " + e.Message);
            }
        }
    }

    [Serializable]
    public class PluginExtTypeException : Exception
    {
        public PluginExtTypeException() { }

        protected PluginExtTypeException(SerializationInfo info, StreamingContext ctxt)
            : base(info, ctxt)
        { }
    }

        /*
        //Enums
        public enum ExtType
        {
            EXE,
            M,
            PY,
            JAR
        }

        //Properties
        public string Description { get; set; }
        public string Name { get; set; }
        public string ReadableName { get; set; }
        public string Type { get; set; }
        public FileInfo XMLFile { get; set; }
        public FileInfo HTMLFile { get; set; }
        public FileInfo RunFile { get; set; }
        public string InputFile { get; set; }
        public string OutputFile { get; set; }
        public string[] OutputExts { get; set; }
        public int Height { get; set; }
        public int Width { get; set; }
        private Dictionary<string, string> RawParameters { get; set; }
        public Dictionary<string, string> ActualParameters { get; set; }
        public bool CanSelection { get; set; }
        public string IconName { get; set; }
        public Image Icon { get; set; }
        public ExtType Ext { get; set; }

        public float BlockStart { get; set; }
        public float BlockCount { get; set; }

        public string StartTimeString { get; set; }
        public string EndTimeString { get; set; }

        public Plugin(FileInfo run, FileInfo xml, FileInfo html)
        {
            //Default false
            CanSelection = false;
            BlockStart = -1;
            BlockCount = -1;

            //Set defaults:
            Height = 600;
            Width = 800;
            //end of set defaults

            XMLFile = xml;

            HTMLFile = html;

            RunFile = run;

            //Do Parse
            StreamReader file = new StreamReader(XMLFile.FullName);
            string fileAsString = file.ReadToEnd();

            //Now we can put it into XML Parser
            //TODO - Error/Exception handling
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(fileAsString);
            XmlNodeList items = xmlDoc.SelectNodes("Plugin/item");

            RawParameters = new Dictionary<string, string>();
            ActualParameters = new Dictionary<string, string>();

            foreach (XmlNode node in items)
            {
                //if (node.Attributes["input"] != null)
                //{
                //    ActualParameters.Add(node.InnerText, node.Attributes["type"].Value);
                //    RawParameters.Add(node.InnerText, node.Attributes["type"].Value);
                //}
                //else
                //{
                RawParameters.Add(node.Attributes["name"].Value, node.InnerText);

                if (node.Attributes["ext"] != null)
                {
                    string ext = node.Attributes["ext"].Value;

                    RawParameters.Add("ext", ext);
                }
                //}
            }

            //Now have all raw parameters to pull them out

            try
            {
                Ext = (ExtType)Enum.Parse(typeof(ExtType), RawParameters["ext"], true);
            }
            catch (Exception)
            {
                throw new PluginExtTypeException();
            }

            Description = RawParameters["description"];
            InputFile = RawParameters["inputFile"];

            if (RawParameters.ContainsKey("outputFile"))
                OutputFile = RawParameters["outputFile"];

            if (RawParameters.ContainsKey("outputExtension"))
            {
                string[] outputExts = RawParameters["outputExtension"].Split('/');

                OutputExts = outputExts;
            }

            if (RawParameters.ContainsKey("readableName"))
                ReadableName = RawParameters["readableName"];

            if (RawParameters.ContainsKey("iconName"))
                IconName = RawParameters["iconName"];

            //Load in icon;
            loadIcon(IconName);

            Type = RawParameters["type"];

            if (RawParameters.ContainsKey("height"))
                Height = int.Parse(RawParameters["height"]);

            if (RawParameters.ContainsKey("width"))
                Width = int.Parse(RawParameters["width"]);
            Name = Path.GetFileNameWithoutExtension(XMLFile.Name);

            if (RawParameters.ContainsKey("canSelection"))
                CanSelection = Boolean.Parse(RawParameters["canSelection"]);
        }

        private void loadIcon(string name)
        {
            string filePath = Properties.Settings.Default.CurrentPluginFolder + Path.DirectorySeparatorChar + name;
            try
            {
                Icon = Image.FromFile(filePath);
            }
            catch (System.IO.FileNotFoundException e)
            {
                Console.WriteLine("No Icon: " + e.Message);
            }
        }
    }

    [Serializable]
    public class PluginExtTypeException : Exception
    {
        public PluginExtTypeException() { }

        protected PluginExtTypeException(SerializationInfo info, StreamingContext ctxt)
            : base(info, ctxt)
        { }
    }*/
}

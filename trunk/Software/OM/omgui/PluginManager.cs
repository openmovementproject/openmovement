using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Windows.Forms;

namespace OmGui
{
    public class PluginManager
    {
        public List<Plugin> Plugins { get; set; }
        public List<Plugin> ProfilePlugins { get; set; }

        //private string profilePath = "";

        public PluginManager()
        {
            //profilePath = Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + "settings";

            Plugins = new List<Plugin>();

            ProfilePlugins = new List<Plugin>();
        }

        public void LoadPlugins()
        {
            Plugins.Clear();

            string pluginsFol = Properties.Settings.Default.CurrentPluginFolder;

            //MessageBox.Show("Current plugin folder: " + Properties.Settings.Default.CurrentPluginFolder);

            try
            {
                string[] pluginPaths = Directory.GetDirectories(Properties.Settings.Default.CurrentPluginFolder);

                foreach (string pluginPath in pluginPaths)
                {
                    Plugin p = Plugin.CreatePlugin(pluginPath);

                    if (p != null)
                        Plugins.Add(p);
                }
            }
            catch (DirectoryNotFoundException e)
            {
                MessageBox.Show("Could not find plugin directory (" + e.Message + "): " + Properties.Settings.Default.CurrentPluginFolder, "Plugins Error");
            }
        }

        public bool LoadProfilePlugins()
        {
            ProfilePlugins.Clear();
            string profilePath = Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + "settings";
            if (profilePath.Length > 0)
            {
                if (!Directory.Exists(profilePath))
                    Directory.CreateDirectory(profilePath);

                if(File.Exists(profilePath + Path.DirectorySeparatorChar + "pluginsProfile.profile"))
                {
                    StreamReader pluginProfile = new StreamReader(profilePath + Path.DirectorySeparatorChar + "pluginsProfile.profile");
                    string pluginProfileAsString = pluginProfile.ReadToEnd();

                    XmlDocument pluginsProfileXML = new XmlDocument();
                    
                    try
                    {
                        pluginsProfileXML.LoadXml(pluginProfileAsString);
                        
                        XmlNodeList items = pluginsProfileXML.SelectNodes("Profile/Plugin");

                        List<string> pluginNames = new List<string>();

                        foreach (XmlNode pluginNode in items)
                        {
                            if (pluginNode.ChildNodes.Count > 0 && pluginNode.ChildNodes[0].Name.Equals("name"))
                            {
                                pluginNames.Add(pluginNode.ChildNodes[0].InnerText);
                            }
                        }

                        if (pluginNames.Count < items.Count)
                        {
                            MessageBox.Show("Not all plugins from the profile could be found.", "Plugins Profile Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        }

                        foreach (string pluginName in pluginNames)
                        {
                            string pluginFolderPath = Properties.Settings.Default.CurrentPluginFolder + Path.DirectorySeparatorChar + pluginName;

                            ProfilePlugins.Add(Plugin.CreatePlugin(pluginFolderPath));
                        }

                        return true;
                    }
                    catch(XmlException)
                    {
                        Console.WriteLine("Error: pluginsProfile.profile is not valid. Please fix file.");

                        return false;
                    }
                }
            }

            return false;
        }
    }
}

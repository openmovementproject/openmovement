using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace OmGui
{
    class PluginQueueItem
    {
        public string Name { get; set; }
        public string[] Files { get; set; }
        public string Parameters { get; set; }
        public int Progress { get; set; }
        public Plugin Plugin { get; set; }
        public string OriginalOutputName { get; set; }
        public ProcessStartInfo StartInfo { get; set; }

        public string destFolder = "C:\\OM\\PluginTemp\\";

        public PluginQueueItem(Plugin plugin, string parameters, string[] files)
        {
            Plugin = plugin;
            Name = plugin.ReadableName;
            Parameters = parameters;
            Files = files;
        }
    }
}

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
        public string File { get; set; }
        public string Parameters { get; set; }
        public int Progress { get; set; }
        public Plugin Plugin { get; set; }
        public string OriginalOutputName { get; set; }
        public ProcessStartInfo StartInfo { get; set; }

        public string destFolder = "C:\\OM\\PluginTemp\\";

        public PluginQueueItem(Plugin plugin, string parameters, string file)
        {
            Plugin = plugin;
            Name = plugin.ReadableName;
            Parameters = parameters;
            File = file;
        }
    }
}

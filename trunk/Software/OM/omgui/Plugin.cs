using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Net;
using System.Xml;

namespace OmGui
{
    public class Plugin
    {
        //Properties
        public string Description { get; set; }
        public string Name { get; set; }
        public string Type { get; set; }
        public FileInfo ParamFile { get; set; }
        public FileInfo RunFile { get; set; }
        public string InputFile { get; set; }
        public string OutputFile { get; set; }
        private Dictionary<string, string> RawParameters { get; set; }
        public Dictionary<string, string> ActualParameters { get; set; }

        public Plugin(FileInfo run, FileInfo parameters)
        {
            ParamFile = parameters;
            RunFile = run;

            //Do Parse
            StreamReader file = new StreamReader(parameters.FullName);
            string fileAsString = file.ReadToEnd();

            //Now we can put it into XML Parser
            //TODO - Error/Exception handling
            XmlDocument xml = new XmlDocument();
            xml.LoadXml(fileAsString);
            XmlNodeList items = xml.SelectNodes("Plugin/item");

            RawParameters = new Dictionary<string, string>();
            ActualParameters = new Dictionary<string, string>();

            foreach (XmlNode node in items)
            {
                if (node.Attributes["input"] != null)
                {
                    ActualParameters.Add(node.InnerText, node.Attributes["type"].Value);
                    RawParameters.Add(node.InnerText, node.Attributes["type"].Value);
                }
                else
                {
                    RawParameters.Add(node.Attributes["name"].Value, node.InnerText);
                }
            }

            //Now have all raw parameters to pull them out
            
            //Get description
            Description = RawParameters["description"];
            InputFile = RawParameters["inputFile"];
            OutputFile = RawParameters["outputFile"];
            Type = RawParameters["type"];
            Name = Path.GetFileNameWithoutExtension(parameters.Name);
        }
    }
}

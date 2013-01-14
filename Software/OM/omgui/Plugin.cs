using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Net;
using System.Xml;
using System.Runtime.Serialization;

namespace OmGui
{
    public class Plugin
    {
        //Enums
        public enum ExtType
        {
            EXE,
            M,
            PY
        }

        //Properties
        public string Description { get; set; }
        public string Name { get; set; }
        public string Type { get; set; }
        public FileInfo XMLFile { get; set; }
        public FileInfo HTMLFile { get; set; }
        public FileInfo RunFile { get; set; }
        public string InputFile { get; set; }
        public string OutputFile { get; set; }
        public ExtType Ext { get; set; }
        public int Height { get; set; }
        public int Width { get; set; }
        private Dictionary<string, string> RawParameters { get; set; }
        public Dictionary<string, string> ActualParameters { get; set; }

        public Plugin(FileInfo run, FileInfo xml, FileInfo html)
        {
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
                Ext = (ExtType) Enum.Parse(typeof(ExtType), RawParameters["ext"], true);
            }
            catch(Exception)
            {
                throw new PluginExtTypeException();
            }
            
            Description = RawParameters["description"];
            InputFile = RawParameters["inputFile"];
            OutputFile = RawParameters["outputFile"];
            Type = RawParameters["type"];

            if(RawParameters.ContainsKey("height"))
                Height = int.Parse(RawParameters["height"]);

            if(RawParameters.ContainsKey("width"))
                Width = int.Parse(RawParameters["width"]);
            Name = Path.GetFileNameWithoutExtension(XMLFile.Name);
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
}

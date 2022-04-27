using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OmGui
{
    public class MetaDataEntry
    {
        public static Dictionary<string, string> shorthandMetaData;

        static MetaDataEntry()
        {
            shorthandMetaData = new Dictionary<string, string>();
            // At setup time
            shorthandMetaData.Add("_c", "Study Centre");
            shorthandMetaData.Add("_s", "Study Code");
            shorthandMetaData.Add("_i", "Investigator");
            shorthandMetaData.Add("_x", "Exercise Code");
            shorthandMetaData.Add("_v", "Volunteer Num");
            shorthandMetaData.Add("_p", "Body Location");
            shorthandMetaData.Add("_so", "Setup Operator");
            shorthandMetaData.Add("_n", "Notes");
            // At retrieval time
            shorthandMetaData.Add("_b", "Start time");
            shorthandMetaData.Add("_e", "End time");
            shorthandMetaData.Add("_ro", "Recovery Operator");
            shorthandMetaData.Add("_r", "Retrieval Time");
            shorthandMetaData.Add("_co", "Comments");
            shorthandMetaData.Add("_sc", "Subject Code");
            shorthandMetaData.Add("_se","Sex");
            shorthandMetaData.Add("_h","Height");
            shorthandMetaData.Add("_w","Weight");
            shorthandMetaData.Add("_ha","Handedness");
            shorthandMetaData.Add("_sn", "Subject Notes");
        }

        [System.ComponentModel.Browsable(false)]
        public string key;

        [System.ComponentModel.DisplayName("Name")]
        public string Name
        {
            get { return (shorthandMetaData.ContainsKey(key) ? shorthandMetaData[key] : key); }
            set
            {
                key = value;
                foreach (KeyValuePair<string, string> kvp in shorthandMetaData)
                {
                    if (kvp.Value.Equals(value, StringComparison.InvariantCultureIgnoreCase)) { key = kvp.Key; }
                }
            }
        }

        [System.ComponentModel.DisplayName("Value")]
        public string Value { get; set; }

        public MetaDataEntry(string name, string value) { this.Name = name; this.Value = value; }
        public MetaDataEntry() { this.Name = ""; this.Value = ""; }
    }
}

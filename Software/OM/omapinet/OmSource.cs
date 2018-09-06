using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace OmApiNet
{
    // CategoryAttribute, DescriptionAttribute, BrowsableAttribute, ReadOnlyAttribute, DefaultValueAttribute, DefaultPropertyAttribute, Browsable
    // [Editor(typeof(System.Windows.Forms.Design.FileNameEditor), typeof(System.Drawing.Design.UITypeEditor)), Description("Path to file"), Category("File")]

    //[DefaultPropertyAttribute("???")]
    public abstract class OmSource
    {
        public enum SourceCategory
        {
            Other,          // Unknown category

            NewData,        // Device attached, not downloading, non-empty data file, archive attribute set
            Downloading,    // Device attached, downloading
            Downloaded,     // Device attached, not downloading, non-empty data file, archive attribute cleared
            Charging,       // Device attached, not downloading, empty data file, zero session-id, charging
            Standby,        // Device attached, not downloading, empty data file, zero session-id, charged
            Outbox,         // Device attached, not downloading, empty data file, non-zero session id
            Removed,        // Device not attached

            File            // Downloaded data file
        }

        [Browsable(false)]
        public abstract SourceCategory Category { get; }

        [CategoryAttribute("Device"), DisplayName("Device"), DescriptionAttribute("Unique device ID."), DefaultValueAttribute(0), ReadOnly(true)]
        public abstract uint DeviceId { get; }

        [CategoryAttribute("Session"), DisplayName("Session ID"), DescriptionAttribute("Session ID."), DefaultValueAttribute(0), ReadOnly(true)]
        public abstract uint SessionId { get; }
    }
}

/*
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using OmApiNet;
using System.ComponentModel;

namespace OmGui
{
    // CategoryAttribute, DescriptionAttribute, BrowsableAttribute, ReadOnlyAttribute, DefaultValueAttribute, DefaultPropertyAttribute, Browsable
    // [Editor(typeof(System.Windows.Forms.Design.FileNameEditor), typeof(System.Drawing.Design.UITypeEditor)), Description("Path to file"), Category("File")]

    //[DefaultPropertyAttribute("???")]
    public class Device
    {
        [Browsable(false)]
        public bool IsDevice { get { return OmDevice != null; } }

        [Browsable(false)]
        public bool IsFile { get { return OmReader != null; } }

        [Browsable(false)]
        public OmDevice OmDevice { get; protected set; }

        [Browsable(false)]
        public OmReader OmReader { get; protected set; }


        // TODO: All of the below should be replaced by a generic interface over OmDevice and OmReader

        [CategoryAttribute("Device"), DisplayName("Device"), DescriptionAttribute("Unique device ID."), DefaultValueAttribute(0xffff), ReadOnly(true)]
        public int DeviceId { get { return IsDevice ? OmDevice.DeviceId : OmReader.DeviceId; } }

        [CategoryAttribute("Session"), DisplayName("Session ID"), DescriptionAttribute("Session ID."), DefaultValueAttribute(0), ReadOnly(true)]
        public uint SessionId { get { return IsDevice ? OmDevice.SessionId : OmReader.SessionId; } }



        public Device(OmDevice omDevice)
        {
            OmDevice = omDevice;
        }

        protected Device(OmReader reader)
        {
            OmReader = reader;
        }

        public static Device FromFile(string filename)
        {
            OmReader reader = OmReader.Open(filename);
            if (reader == null) { return null; }
            reader.Close();
            return new Device(reader);
        }

        [Browsable(false)]
        public bool Connected
        {
            get { return IsDevice && OmDevice.Connected; }
        }

        public void UpdateListViewItem(System.Windows.Forms.ListViewItem item)
        {
            int img = 8;
            string download = "-";
            string recording = "-";
            string deviceId = "-";
            string sessionId = "-";
            string battery = "-";

            if (item.SubItems.Count < 3)
            {
                item.SubItems.Clear();
                item.SubItems.Add("");                      // Device id
                item.SubItems.Add("");                      // Session id
                item.SubItems.Add("");                      // Battery level
                item.SubItems.Add("");                      // Download
                item.SubItems.Add("");                      // Recording
            }

            if (OmDevice != null)
            {
                img = (int)OmDevice.LedColor;
                if (img < 0 || img > 8) { img = 8; }

                if (OmDevice.Connected)
                {
                    item.Group = ((DeviceListView)item.ListView).groupDevices;
                    item.ForeColor = System.Drawing.SystemColors.WindowText;
                    item.Font = System.Drawing.SystemFonts.DefaultFont;
                }
                else
                {
                    item.Group = ((DeviceListView)item.ListView).groupRemoved;
                    item.ForeColor = System.Drawing.SystemColors.GrayText;
                    item.Font = new System.Drawing.Font(System.Drawing.SystemFonts.DefaultFont, System.Drawing.FontStyle.Italic);
                }

                if (OmDevice.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_CANCELLED) { download = "Cancelled"; }
                else if (OmDevice.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_COMPLETE) { download = "Complete"; }
                else if (OmDevice.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_ERROR) { download = string.Format("Error (0x{0:X})", OmDevice.DownloadValue); }
                else if (OmDevice.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS) { download = "" + OmDevice.DownloadValue + "%"; }
                //else if (OmDevice.downloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_NONE) { download = "-"; }

                if (OmDevice.StartTime >= OmDevice.StopTime) { recording = "Stopped"; }
                else if (OmDevice.StartTime == DateTime.MinValue && OmDevice.StopTime == DateTime.MaxValue) { recording = "Always"; }
                else
                {
                    recording = string.Format("Interval {0:dd/MM/yy HH:mm:ss}-{1:dd/MM/yy HH:mm:ss}", OmDevice.StartTime, OmDevice.StopTime);
                }

                deviceId = string.Format("{0:00000}", OmDevice.DeviceId);
                sessionId = (OmDevice.SessionId == uint.MinValue) ? "-" : OmDevice.SessionId.ToString();
                battery = (OmDevice.BatteryLevel == int.MinValue) ? "-" : OmDevice.BatteryLevel.ToString() + "%";
            }
            else if (OmReader != null)
            {
                img = 9;

                item.Group = ((DeviceListView)item.ListView).groupFiles;
                item.ForeColor = System.Drawing.SystemColors.GrayText;

                deviceId = string.Format("{0:00000}", OmReader.DeviceId);
                sessionId = (OmReader.SessionId == uint.MinValue) ? "-" : OmReader.SessionId.ToString();
                battery = "-";
                download = System.IO.Path.GetFileName(OmReader.Filename);
                recording = "-";
            }


            item.ImageIndex = img;
            item.SubItems[0].Text = deviceId;
            item.SubItems[1].Text = sessionId;
            item.SubItems[2].Text = battery;
            item.SubItems[3].Text = download;
            item.SubItems[4].Text = recording;

            item.Tag = this;
        }

    }
}
*/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace OmGui
{
    public partial class DeviceListView : ListView
    {
        // Components
        private IContainer components;
        private ImageList imageListDevice;

        // Columns
        public ColumnHeader columnDevice;
        public ColumnHeader columnSessionId;
        public ColumnHeader columnBattery;
        public ColumnHeader columnDownload;
        public ColumnHeader columnRecording;

        // Groups
        public ListViewGroup groupOther;
        public ListViewGroup groupNewData;
        public ListViewGroup groupDownloading;
        public ListViewGroup groupDownloaded;
        public ListViewGroup groupCharging;
        public ListViewGroup groupStandby;
        public ListViewGroup groupOutbox;
        public ListViewGroup groupRemoved;
        public ListViewGroup groupFile;

        public DeviceListView() : base()
        {
            InitializeComponent();

            // Properties
            AllowColumnReorder = true;
            Anchor = ((AnchorStyles)((((AnchorStyles.Top | AnchorStyles.Bottom) | AnchorStyles.Left) | AnchorStyles.Right)));
            FullRowSelect = true;
            GridLines = true;
            HideSelection = false;
            LabelWrap = false;
            Location = new System.Drawing.Point(12, 15);
            Size = new System.Drawing.Size(758, 233);
            Sorting = SortOrder.Ascending;
            SmallImageList = this.imageListDevice;
            LargeImageList = this.imageListDevice;
            TabIndex = 0;
            UseCompatibleStateImageBehavior = false;
            View = View.Details;

            // Columns
            columnDevice = new ColumnHeader()
            {
                Text = "Device",
                Width = 70
            };
            columnBattery = new ColumnHeader()
            {
                Text = "Battery",
                Width = 70
            };
            columnSessionId = new ColumnHeader()
            {
                Text = "Session Id",
                Width = 90
            };
            columnDownload = new ColumnHeader()
            {
                Text = "Download",
                Width = 90
            };
            columnRecording = new ColumnHeader()
            {
                Text = "Recording",
                Width = 280
            };
            Columns.AddRange(new ColumnHeader[] { columnDevice, columnSessionId, columnBattery, columnDownload, columnRecording });

            // Groups
            groupOther       = new ListViewGroup(OmApiNet.OmSource.SourceCategory.Other.ToString(),       "Devices"); 
            groupNewData     = new ListViewGroup(OmApiNet.OmSource.SourceCategory.NewData.ToString(),     "New Data");
            groupDownloaded  = new ListViewGroup(OmApiNet.OmSource.SourceCategory.Downloaded.ToString(),  "Downloaded");
            groupDownloading = new ListViewGroup(OmApiNet.OmSource.SourceCategory.Downloading.ToString(), "Downloading"); 
            groupCharging    = new ListViewGroup(OmApiNet.OmSource.SourceCategory.Charging.ToString(),    "Charging"); 
            groupStandby     = new ListViewGroup(OmApiNet.OmSource.SourceCategory.Standby.ToString(),     "Standby"); 
            groupOutbox      = new ListViewGroup(OmApiNet.OmSource.SourceCategory.Outbox.ToString(),      "Outbox"); 
            groupRemoved     = new ListViewGroup(OmApiNet.OmSource.SourceCategory.Removed.ToString(),     "Removed"); 
            groupFile        = new ListViewGroup(OmApiNet.OmSource.SourceCategory.File.ToString(),        "Files");

            Groups.AddRange(new ListViewGroup[] { groupOther, groupNewData, groupDownloading, groupDownloaded, groupCharging, groupStandby, groupOutbox, groupRemoved, groupFile });
        }

        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DeviceListView));
            this.imageListDevice = new System.Windows.Forms.ImageList(this.components);
            this.SuspendLayout();
            // 
            // imageListDevice
            // 
            this.imageListDevice.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageListDevice.ImageStream")));
            this.imageListDevice.TransparentColor = System.Drawing.Color.Transparent;
            this.imageListDevice.Images.SetKeyName(0, "Circle0.png");
            this.imageListDevice.Images.SetKeyName(1, "Circle1.png");
            this.imageListDevice.Images.SetKeyName(2, "Circle2.png");
            this.imageListDevice.Images.SetKeyName(3, "Circle3.png");
            this.imageListDevice.Images.SetKeyName(4, "Circle4.png");
            this.imageListDevice.Images.SetKeyName(5, "Circle5.png");
            this.imageListDevice.Images.SetKeyName(6, "Circle6.png");
            this.imageListDevice.Images.SetKeyName(7, "Circle7.png");
            this.imageListDevice.Images.SetKeyName(8, "Circle.png");
            this.imageListDevice.Images.SetKeyName(9, "Data.png");
            this.ResumeLayout(false);

        }

    }
}

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using OmApiNet;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.IO;
using System.Threading;
using System.Xml;

using AndreasJohansson.Win32.Shell;

namespace OmGui
{
    public partial class MainForm : Form
    {
        Om om;
        DeviceManager deviceManager;

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern uint RegisterWindowMessage(string lpString);

        public static uint queryCancelAutoPlayID = 0;

        public static string ADVICE = "\r\n\r\n(If device communication problems persist, please disconnect, wait, then reconnect the device.)";

        //PluginQueue
        Queue<PluginQueueItem> pluginQueueItems = new Queue<PluginQueueItem>();

        protected override void WndProc(ref Message msg)
        {
            base.WndProc(ref msg);
            if (MainForm.queryCancelAutoPlayID != 0 && msg.Msg == MainForm.queryCancelAutoPlayID)
            {
                msg.Result = (IntPtr)1;    // Cancel autoplay
            }
        }

        private bool QueryAdminElevate(bool always)
        {
            WindowsIdentity id = WindowsIdentity.GetCurrent();
            WindowsPrincipal principal = new WindowsPrincipal(id);
            if (principal.IsInRole(WindowsBuiltInRole.Administrator))
            {
                return true;
            }

            bool elevate;
            if (always)
            {
                elevate = true;
            }
            else
            {
                DialogResult dr = MessageBox.Show(this, "If you are connecting a large number of devices, \r\nto prevent drive letter exhaustion through remounting, \r\nyou must run this as an administrator.\r\n\r\nAttempt to elevate user level?", "Elevate to Administrator?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);
                if (dr == DialogResult.Cancel)
                {
                    Environment.Exit(-1); // Application.Exit(); // this.Close();
                }
                elevate = (dr == DialogResult.Yes);
            }

            if (elevate)
            {
                try
                {
                    // Start a new instance
                    ProcessStartInfo process = new ProcessStartInfo();
                    process.FileName = Application.ExecutablePath;
                    process.WorkingDirectory = Environment.CurrentDirectory;
                    process.UseShellExecute = true;
                    process.Verb = "runas";
                    Process.Start(process);

                    // Exit this process
                    Environment.Exit(-1); // Application.Exit(); // this.Close();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, ex.Message, "Error Elevating", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            return false;
        }

        public MainForm(int uac)
        {
            if (uac == 1)
            {
                QueryAdminElevate(false);
            }
            else if (uac == 2)
            {
                QueryAdminElevate(true);
            }
            else if (uac == 3)
            {
                if (!QueryAdminElevate(true))
                {
                    Environment.Exit(-1);
                }
            }

            // Capture any output prior to InitializeComponent
            TextBox initialLog = new TextBox();
            (new TextBoxStreamWriter(initialLog)).SetConsoleOut();

            // Get OM instance
            try
            {
                om = Om.Instance;
            }
            catch (Exception ex)
            {
                MessageBox.Show(this, "Error starting OMAPI (" + ex.Message + ")\r\n\r\nCheck the required .dll files are present and the correct versions:\r\n\r\nOmApiNet.dll\r\nlibomapi.dll\r\n", "OMAPI Startup Failed", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                Environment.Exit(-1);
            }

            

            // Initialize the component
            InitializeComponent();
            this.Text += " [V" + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString() + "]";

            // Set output redirection
            textBoxLog.Text = initialLog.Text;
            (new TextBoxStreamWriter(textBoxLog)).SetConsoleOut();
            Console.WriteLine("Started.");
            Trace.Listeners.Add(new ConsoleTraceListener());
            //Trace.WriteLine("Trace started.");

            // Cancel Windows auto-play
            queryCancelAutoPlayID = RegisterWindowMessage("QueryCancelAutoPlay");

            // Update status bar
            toolStripStatusLabelMain.Text = "";

            // Device manager
            deviceManager = new DeviceManager(om);
            deviceManager.DeviceUpdate += deviceManager_DeviceUpdate;
            deviceManager.FindInitiallyAttached();

            // Update view options and selection
            View_CheckChanged(null, null);
            listViewDevices_SelectedIndexChanged(null, null);

            // Start refresh timer
            EnableBackgroundTasks();

            // Forward mouse wheel events to DataViewer (controls have to have focus for mouse wheel events)
            //this.MouseWheel += (o, e) =>
            //{
            //    Control control = this.GetChildAtPoint(p);
            //    Control lastControl = control;
            //    while (control != null)
            //    {
            //        if (control == dataViewer)
            //        {
            //            Console.WriteLine(e.Delta);
            //            dataViewer.DataViewer_MouseWheel(o, e);
            //        }
            //        lastControl = control;
            //        p.X -= control.Location.X;
            //        p.Y -= control.Location.Y;
            //        control = control.GetChildAtPoint(p);
            //    }
            //};

            filesListView.FullRowSelect = true;
        }

        #region List View Column Sorting

        private ListViewColumnSorter lwColumnSorterFiles = new ListViewColumnSorter(false);
        private ListViewColumnSorter lwColumnSorterQueue = new ListViewColumnSorter(false);
        private ListViewColumnSorter lwColumnSorterOutput = new ListViewColumnSorter(false);
        private ListViewColumnSorter lwColumnSorterOutputDate = new ListViewColumnSorter(true);

        private void setupColumnSorter()
        {
            filesListView.ListViewItemSorter = lwColumnSorterFiles;
            queueListViewItems2.ListViewItemSorter = lwColumnSorterQueue;
            outputListView.ListViewItemSorter = lwColumnSorterOutput;
        }

        bool[] filesSortOrderFlags = { true, true, true, true };
        private void filesListView_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            lwColumnSorterFiles.SortColumn = e.Column;

            if (filesSortOrderFlags[e.Column])
            {
                lwColumnSorterFiles.Order = SortOrder.Descending;
            }
            else
            {
                lwColumnSorterFiles.Order = SortOrder.Ascending;
            }

            filesSortOrderFlags[e.Column] = !filesSortOrderFlags[e.Column];

            filesListView.Sort();
        }

        bool[] queueSortOrderFlags = { true, true, true, true };
        private void queueListViewItems2_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            lwColumnSorterQueue.SortColumn = e.Column;

            if (queueSortOrderFlags[e.Column])
            {
                lwColumnSorterQueue.Order = SortOrder.Descending;
            }
            else
            {
                lwColumnSorterQueue.Order = SortOrder.Ascending;
            }
            
            queueSortOrderFlags[e.Column] = !queueSortOrderFlags[e.Column];

            queueListViewItems2.Sort();
        }

        bool[] outputSortOrderFlags = { true, true, true, true };
        private void outputListView_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            lwColumnSorterOutput.SortColumn = e.Column;

            //TODO - if the column is 'date modified' then use the other sorter.
            if (e.Column == 3)
            {
                outputListView.ListViewItemSorter = lwColumnSorterOutputDate;
            }
            else
            {
                outputListView.ListViewItemSorter = lwColumnSorterOutput;
            }

            if (outputSortOrderFlags[e.Column])
            {
                lwColumnSorterOutput.Order = SortOrder.Descending;
            }
            else
            {
                lwColumnSorterOutput.Order = SortOrder.Ascending;
            }

            outputSortOrderFlags[e.Column] = !outputSortOrderFlags[e.Column];

            outputListView.Sort();
        }

        #endregion

        IDictionary<ushort, ListViewItem> listViewDevices = new Dictionary<ushort, ListViewItem>();
        IDictionary<string, ListViewItem> listViewFiles = new Dictionary<string, ListViewItem>();
        IDictionary<string, ListViewItem> listViewOutputFiles = new Dictionary<string, ListViewItem>();

        //TS - [P] - Updates the DeviceListViewItem's entry
        //TS - TODO - Work out exactly how this works.
        public void DeviceListViewItemUpdate(System.Windows.Forms.ListViewItem item)
        {
            item.SubItems.Clear();

            //TS - Actually does all the forecolors etc.
            DeviceListViewCreateItem((OmSource)item.Tag, item);

            //TS - [P] - Get and set the category for the list
            //TS - Don't need this anymore because there aren't groups in one list view but a list view each.
            //OmSource.SourceCategory category = source.Category;
            //item.Group = ((DeviceListView)item.ListView).Groups[category.ToString()];
            //if (category == OmSource.SourceCategory.Removed)
            //{
            //    item.ForeColor = System.Drawing.SystemColors.GrayText;
            //    item.Font = new System.Drawing.Font(System.Drawing.SystemFonts.DefaultFont, System.Drawing.FontStyle.Italic);
            //}
            //else if (category == OmSource.SourceCategory.File)
            //{
            //    item.ForeColor = System.Drawing.SystemColors.GrayText;
            //}
            //else
            //{
            //    item.ForeColor = System.Drawing.SystemColors.WindowText;
            //    item.Font = System.Drawing.SystemFonts.DefaultFont;
            //}    
        }

        //Logic for the device list view item.
        private void DeviceListViewCreateItem(OmSource source, ListViewItem item)
        {
            OmDevice device = source as OmDevice;
            OmReader reader = source as OmReader;

            string download = "";
            string battery = "";
            string recording = "";

            //OmSource.SourceCategory category = source.Category;
            //item.Group = ((DeviceListView)item.ListView).Groups[category.ToString()];
            //if (category == OmSource.SourceCategory.Removed)
            //{
            //    item.ForeColor = System.Drawing.SystemColors.GrayText;
            //    item.Font = new System.Drawing.Font(System.Drawing.SystemFonts.DefaultFont, System.Drawing.FontStyle.Italic);
            //}
            //else if (category == OmSource.SourceCategory.File)
            //{
            //    item.ForeColor = System.Drawing.SystemColors.GrayText;
            //}
            //else
            //{
            //    item.ForeColor = System.Drawing.SystemColors.WindowText;
            //    item.Font = System.Drawing.SystemFonts.DefaultFont;
            //}    

            int img = 8;

            if (device != null)
            {
                img = (int)device.LedColor;
                if (img < 0 || img > 8) { img = 8; }

                if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_CANCELLED) { download = "Cancelled"; }
                else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_COMPLETE) { download = "Complete"; }
                else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_ERROR) { download = string.Format("Error (0x{0:X})", device.DownloadValue); }
                else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS) { download = "" + device.DownloadValue + "%"; }
                //else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_NONE) { download = "-"; }

                if (device.StartTime >= device.StopTime) { recording = "Stopped"; }
                else if (device.StartTime == DateTime.MinValue && device.StopTime == DateTime.MaxValue) { recording = "Always"; }
                else
                {
                    recording = string.Format("Interval {0:dd/MM/yy HH:mm:ss}-{1:dd/MM/yy HH:mm:ss}", device.StartTime, device.StopTime);
                }

                if (device.HasData)
                {
                    recording += " (with data)";
                }

                battery = (device.BatteryLevel < 0) ? "-" : device.BatteryLevel.ToString() + "%";
            }
            else if (reader != null)
            {
                img = 9;

                battery = "-";
                download = "-";
                recording = System.IO.Path.GetFileName(reader.Filename);
            }
            else
            {
                battery = download = recording = null;
            }

            item.ImageIndex = img;

            string deviceText = string.Format("{0:00000}", source.DeviceId);
            string deviceSessionID = (source.SessionId == uint.MaxValue) ? "-" : source.SessionId.ToString();
            string deviceBattery = battery;
            string deviceDownload = download;
            string deviceRecording = recording;

            item.UseItemStyleForSubItems = false;


            item.Text = deviceText;
            item.SubItems.Add(deviceSessionID, Color.Black, Color.White, DefaultFont);
            item.SubItems.Add(deviceBattery);
            item.SubItems.Add(deviceDownload);
            item.SubItems.Add(deviceRecording);

            //TS - Colours for each
            //TS - BATTERY
            //Console.WriteLine("battery: " + device.BatteryLevel);
            if (device.BatteryLevel >= 0 && device.BatteryLevel < 33)
            {
                item.SubItems[2].ForeColor = Color.Red;

            }
            else if (device.BatteryLevel >= 33 && device.BatteryLevel < 66)
            {
                item.SubItems[2].ForeColor = Color.Orange;
            }
            else
            {
                item.SubItems[2].ForeColor = Color.Green;
            }

            //TS - DOWNLOAD
            if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_CANCELLED || device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_ERROR)
            {
                item.SubItems[3].ForeColor = Color.Red;
            }
            else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS)
            {
                item.SubItems[3].ForeColor = Color.Orange;
            }
            else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_COMPLETE)
            {
                item.SubItems[3].ForeColor = Color.Green;
            }

            //TS - RECORDING
            if (device.IsRecording == OmDevice.RecordStatus.Stopped)
            {
                item.SubItems[4].ForeColor = Color.Red;
            }
            else
            {
                item.SubItems[4].ForeColor = Color.Green;
            }
        }

        //TS - [P] - Hit if the deviceManager needs to update a device (add to the list etc.)
        void deviceManager_DeviceUpdate(object sender, DeviceEventArgs e)
        {
            if (InvokeRequired)
            {
                BeginInvoke(new DeviceManager.DeviceEventHandler(deviceManager_DeviceUpdate), new object[] { sender, e });
                return;
            }

            // Values
            ushort deviceId = e.Device.DeviceId;
            //bool connected = e.Device.Connected;

            ListViewItem item;
            if (!listViewDevices.ContainsKey(deviceId))
            {
                item = new ListViewItem();
                item.Tag = e.Device;
                listViewDevices[deviceId] = item;
                devicesListView.Items.Add(item);
                DeviceListViewItemUpdate(item);
            }
            else
            {
                item = listViewDevices[deviceId];
                UpdateDevice(item);

                devicesListViewUpdateEnabled();
            }

        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            (new AboutBox()).Show(this);
            //string message = Application.ProductName + " " + Application.ProductVersion + "\r\n" + Application.CompanyName;
            //MessageBox.Show(this, message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void View_CheckChanged(object sender, EventArgs e)
        {
            toolStripMain.Visible = toolbarToolStripMenuItem.Checked;
            statusStripMain.Visible = statusBarToolStripMenuItem.Checked;
            splitContainerLog.Panel2Collapsed = !logToolStripMenuItem.Checked;
            splitContainerPreview.Panel1Collapsed = !previewToolStripMenuItem.Checked;
            splitContainerDevices.Panel2Collapsed = !propertiesToolStripMenuItem.Checked;
        }

        //private void UpdateFile(ListViewItem item)
        //{
        //    OmSource device = (OmSource)item.Tag;
        //    UpdateListViewItem(item);

        //    UpdateEnabled();
        //}

        //TS - [P] - Update the device in terms of the listView
        private void UpdateDevice(ListViewItem item)
        {
            OmSource device = (OmSource)item.Tag;
            DeviceListViewItemUpdate(item);

            //If the device has been removed.
            if (device is OmDevice && !((OmDevice)device).Connected)
            {
                //TODO - Dataviewer clear.

                listViewDevices.Remove(device.DeviceId);
                devicesListView.Items.Remove(item);
            }

            //UpdateEnabled();
        }

        // Update the device from its ID
        private void UpdateDeviceId(int deviceId)
        {
            if (InvokeRequired)
            {
                this.Invoke((MethodInvoker) delegate 
                {
                    UpdateDeviceId(deviceId);
                });
                return;
            }

            foreach (ListViewItem item in devicesListView.Items)
            {
                OmSource source = (OmSource)item.Tag;
                if (source is OmDevice)
                {
                    OmDevice device = (OmDevice)source;
                    if (device.DeviceId == deviceId)
                    {
                        UpdateDevice(item);
                    }
                }
            }
        }


        //TS - Update file in listView
        private void UpdateFile(ListViewItem item, string path)
        {
            string fileName = System.IO.Path.GetFileName(path);

            System.IO.FileInfo info = new System.IO.FileInfo(path);
            //MB
            string fileSize = ((double)((double)info.Length / 1024.0 / 1024.0)).ToString("F");
            string dateModified = info.LastWriteTime.ToString("dd/MM/yy HH:mm:ss");

            item.SubItems.Clear();

            item.SubItems.Add(path);
            item.SubItems.Add(fileSize);
            item.SubItems.Add(dateModified);

            item.Text = fileName;
        }


        private ListViewItem UpdateOutputFile(string path)
        {
            ListViewItem item = new ListViewItem();

            string fileName = System.IO.Path.GetFileName(path);
            System.IO.FileInfo info = new FileInfo(path);

            string fileSize = "";
            string dateModified = "";

            if (info.Exists)
            {
                fileSize = ((double)((double)info.Length / 1024.0 / 1024.0)).ToString("F");
                dateModified = info.CreationTime.ToString("dd/MM/yy HH:mm:ss");
            }

            item.SubItems.Clear();

            item.SubItems.Add(path);
            item.SubItems.Add(fileSize);
            item.SubItems.Add(dateModified);

            item.Text = fileName;

            item.Tag = path;

            return item;
        }

        public static string GetPath(string template)
        {
            template = template.Replace("{MyDocuments}", Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments));
            template = template.Replace("{Desktop}", Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory));
            template = template.Replace("{LocalApplicationData}", Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
            template = template.Replace("{ApplicationData}", Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData));
            template = template.Replace("{CommonApplicationData}", Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData));
            if (!template.EndsWith("\\")) { template += "\\"; }
            return template;
        }


        private void toolStripButtonDownload_Click(object sender, EventArgs e)
        {
            //TS - Recording how many are downloading from selected items to display in prompt.
            int numDevicesDownloaded = 0;
            //int numDevicesNotDownloaded = 0;
            List<string> fileNamesDownloaded = new List<string>();

            Dictionary<string, string> devicesAndErrors = new Dictionary<string, string>();

            foreach (ListViewItem i in devicesListView.SelectedItems)
            {
                OmDevice device = (OmDevice)i.Tag;
                OmSource source = (OmSource)device;
                string deviceError = null;

                if (deviceError == null && device == null)
                {
                    //Couldn't download so hasn't downloaded
                    //Device null.
                    //numDevicesNotDownloaded++;
                    deviceError = "Unknown error";
                }

                if (deviceError == null && device.IsDownloading)
                {
                    //is downloading
                    deviceError = "device is already downloading";
                }

                //Console.WriteLine(device.StartTime.Ticks + " - " + device.StopTime.Ticks + " - " + device.IsRecording);
                if (deviceError == null && device.IsRecording != OmDevice.RecordStatus.Stopped)
                {
                    //is still recording
                    deviceError = "device is recording";
                }

                if (deviceError == null && !device.HasData)
                {
                    //Has no data
                    deviceError = "device has no data";
                }

                if (deviceError == null)
                {
                    string folder = GetPath(OmGui.Properties.Settings.Default.CurrentWorkingFolder);
                    System.IO.Directory.CreateDirectory(folder);
                    string prefix = folder + string.Format("{0:00000}_{1:0000000000}", device.DeviceId, device.SessionId);
                    string finalFilename = prefix + ".cwa";
                    string downloadFilename = finalFilename;

                    downloadFilename += ".part";
                    if (deviceError == null && System.IO.File.Exists(downloadFilename))
                    {
                        DialogResult dr = MessageBox.Show(this, "Download file already exists:\r\n\r\n    " + downloadFilename + "\r\n\r\nOverwrite existing file?", "Overwrite File?", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                        if (dr != DialogResult.OK)
                        {
                            deviceError = "Not overwriting existing download file.";
                        }
                        else { System.IO.File.Delete(downloadFilename); }
                    }

                    if (deviceError == null && System.IO.File.Exists(finalFilename))
                    {
                        DialogResult dr = MessageBox.Show(this, "File already exists:\r\n\r\n    " + downloadFilename + "\r\n\r\nOverwrite existing file?", "Overwrite File?", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                        if (dr != DialogResult.OK)
                        {
                            deviceError = "Not overwriting existing data file.";

                            //TS - Device not downloaded because user clicked cancel on overwrite.
                        }
                        else { System.IO.File.Delete(finalFilename); }
                    }

                    //Clicked OK and want to download.
                    if (deviceError == null)
                    {
                        device.BeginDownloading(downloadFilename, finalFilename);

                        //TS - Device downloaded because user clicked OK.
                        numDevicesDownloaded++;
                        fileNamesDownloaded.Add(finalFilename);
                    }
                }


                if (deviceError != null)
                {
                    string deviceText = string.Format("{0:00000}", source.DeviceId);
                    devicesAndErrors.Add(deviceText, deviceError);
                }

            }


            // Show any problems to the user
            if (numDevicesDownloaded != devicesListView.SelectedItems.Count)
            {
                string message = numDevicesDownloaded + " devices downloading:\r\n";

                foreach (KeyValuePair<string, string> kvp in devicesAndErrors)
                {
                    message += "\r\nDevice: " + kvp.Key + " - Status: " + kvp.Value;
                }

                MessageBox.Show(message, "Download Status", MessageBoxButtons.OK);
            }

            //TS - If multiple devices selected then show which downloaded and which didnt.
            //if (devicesListView.SelectedItems.Count > 0)
            //{
            //    string message = numDevicesDownloaded + " devices downloaded from a selection of " + (int)(numDevicesNotDownloaded + numDevicesDownloaded) + " devices." + "\r\nFiles:";

            //    foreach (string fileName in fileNamesDownloaded)
            //    {
            //        message += "\r\n" + fileName;
            //    }

            //    MessageBox.Show(message, "Download Status", MessageBoxButtons.OK);
            //}
        }

        //TS - [P] - Updates toolstrip buttons based on what has been selected in devicesListView
        public OmSource[] devicesListViewUpdateEnabled()
        {
            List<OmSource> selected = new List<OmSource>();

            DevicesResetToolStripButtons();

            /*
            //TS - If a single device has been selected
            if (devicesListView.SelectedItems.Count == 1)
            {
                bool downloading = false, hasData = false, isRecording = false, isSetupForRecord = false;

                OmDevice device = (OmDevice)devicesListView.SelectedItems[0].Tag;

                //Is Downloading
                if (((OmDevice)device).DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS)
                {
                    downloading = true;
                }

                //Has Data
                if (((OmDevice)device).HasData)
                {
                    hasData = true;
                }

                if (device.StartTime > DateTime.Now && (device.StartTime != DateTime.MaxValue)) { isSetupForRecord = true; };

                //TS - [P] - Taken from somewhere else, check if device is recording or not.
                if (device.StartTime >= device.StopTime) { isRecording = false; }
                else if (device.StartTime == DateTime.MinValue && device.StopTime == DateTime.MaxValue) { isRecording = true; }

                //Can Download/Clear
                toolStripButtonDownload.Enabled = !downloading && hasData && !isRecording && !isSetupForRecord; //If has data and not downloading
                toolStripButtonClear.Enabled = !downloading && hasData && !isRecording && !isSetupForRecord; //If not downloading but have data then can clear

                //Can Cancel Download
                toolStripButtonCancelDownload.Enabled = downloading; //If downloading then can cancel download

                //Can sync time
                //toolStripButtonSyncTime.Enabled = true; //If selected any then can sync time //TS - TODO - Ask Dan can this be done if downloading/recording etc.

                toolStripButtonStopRecording.Enabled = isRecording || isSetupForRecord; //If single and recording/setup for record then can stop recording

                toolStripButtonInterval.Enabled = !downloading && (!isRecording && !isSetupForRecord); //If any selected and not downloading then can set up interval.

                devicesToolStripButtonIdentify.Enabled = true;

                selected.Add(device);
            }
            //TS - Multiple rows logic - Only indicates of some or all are doing something so the buttons will actually have to do some logic as it iterates through each selected.
            else if (devicesListView.SelectedItems.Count > 1)
            {
                bool allDownloading = true, someDownloading = false, someHaveData = false, allRecording = true, someRecording = false, allSetupForRecord = true, someSetupForRecord = false;

                foreach (ListViewItem item in devicesListView.SelectedItems)
                {
                    OmDevice device = (OmDevice)item.Tag;

                    //DOWNLOADING
                    //If it is downloading then set downloading.
                    if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS)
                    {
                        someDownloading = true;
                    }
                    //If it isn't downloading and allDownloading is true then set it false.
                    else if (allDownloading)
                    {
                        allDownloading = false;
                    };

                    //HASDATA
                    //If it has data then some have data.
                    if (device.HasData)
                    {
                        someHaveData = true;
                    }

                    //ALL RECORDING
                    if (device.StartTime < device.StopTime)
                    {
                        someRecording = true;
                    }
                    else if (allRecording)
                    {
                        allRecording = false;
                    }

                    //SETUP FOR RECORD
                    if (device.StartTime > DateTime.Now && (device.StartTime != DateTime.MaxValue))
                    {
                        someSetupForRecord = true;
                    }
                    else if (allSetupForRecord)
                    {
                        allSetupForRecord = false;
                    }

                    selected.Add(device);
                }

                //TS - ToolStrip Logic for multiple

                //Can Download/Clear
                toolStripButtonDownload.Enabled = someHaveData && !allRecording; //If has data and not downloading
                toolStripButtonClear.Enabled = (someDownloading || !allDownloading) && someHaveData && !allRecording; //If not downloading but have data then can clear

                //Can Cancel Download
                toolStripButtonCancelDownload.Enabled = someDownloading || allDownloading; //If downloading then can cancel download

                //Can sync time
                //toolStripButtonSyncTime.Enabled = true; //If selected any then can sync time //TS - TODO - Ask Dan can this be done if downloading/recording etc.

                toolStripButtonStopRecording.Enabled = someRecording || allRecording || someSetupForRecord || allSetupForRecord; //If single and recording/setup for record then can stop recording

                //toolStripButtonRecord.Enabled = !allDownloading && !allRecording && !allSetupForRecord; //If any selected and not downloading then can record.
                toolStripButtonInterval.Enabled = !allDownloading && !allRecording && !allSetupForRecord; //If any selected and not downloading then can set up interval.
                //toolStripButtonSessionId.Enabled = !allDownloading && !allRecording && !allSetupForRecord; //If multiple or single and none are downloading then can set session ID.

                devicesToolStripButtonIdentify.Enabled = true;
            }
            */




            {
                int numTotal = 0;           // Total number of selected devices
                int numData = 0;            // Devices with some data
                int numRecording = 0;       // Devices not stopped
                int numDownloading = 0;     // Devices background downloading
                int numDataAndStoppedOrNoDataAndConfigured = 0;  // Devices with some and stopped, or no data but set to record

                foreach (ListViewItem item in devicesListView.SelectedItems)
                {
                    OmDevice device = (OmDevice)item.Tag;

                    numTotal++;

                    //DOWNLOADING
                    //If it is downloading then set downloading.
                    if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS)
                    {
                        numDownloading++;
                    }
 
                    //HASDATA
                    //If it has data then some have data.
                    if (device.HasData)
                    {
                        numData++;
                    }

                    //ALL RECORDING
                    if (device.StartTime < device.StopTime)
                    {
                        numRecording++;
                    }

                    //SETUP FOR RECORD
                    //if (device.StartTime > DateTime.Now && (device.StartTime != DateTime.MaxValue))
                    //{
                    //    numFutureRecording++;
                    //}

                    if ((device.HasData && device.StartTime >= device.StopTime) || (!device.HasData && device.StartTime < device.StopTime))
                    {
                        numDataAndStoppedOrNoDataAndConfigured++;
                    }

                    selected.Add(device);
                }

                //TS - ToolStrip Logic for multiple

                // Download: all have data & none recording & none downloading
                toolStripButtonDownload.Enabled = (numTotal > 0) && (numData == numTotal) && (numRecording == 0) && (numDownloading == 0); //If has data and not downloading

                // Clear: some have-data-or-recording & none downloading // OLD: [removed: all have data &] none recording & none downloading
                toolStripButtonClear.Enabled = (numTotal > 0) && (numDataAndStoppedOrNoDataAndConfigured == numTotal) && (numDownloading == 0); // /*(numData == numTotal) &&*/ (numRecording == 0) && (numDownloading == 0) //If not downloading but have data then can clear

                // Cancel: some downloading
                toolStripButtonCancelDownload.Enabled = (numTotal > 0) && (numDownloading > 0); //If downloading then can cancel download

                // Stop: some recording and none downloading // OLD: all recording & none downloading
                toolStripButtonStopRecording.Enabled = (numTotal > 0) && (numRecording > 0) && (numDownloading == 0); //If single and recording/setup for record then can stop recording

                // Interval(Record): none have data & none downloading
                toolStripButtonInterval.Enabled = (numTotal > 0) && (numData == 0) && (numDownloading == 0); //If any selected and not downloading then can set up interval.

                // Identify: someDevices
                devicesToolStripButtonIdentify.Enabled = (numTotal > 0);

            }


            //TS - DEBUG - Record always lit
            //toolStripButtonInterval.Enabled = true;

            return selected.ToArray();
        }

        private void DevicesResetToolStripButtons()
        {
            toolStripButtonDownload.Enabled = false;
            toolStripButtonClear.Enabled = false;

            toolStripButtonCancelDownload.Enabled = false;

            //toolStripButtonSyncTime.Enabled = false;

            toolStripButtonStopRecording.Enabled = false;
            //toolStripButtonRecord.Enabled = false;
            toolStripButtonInterval.Enabled = false;
            //toolStripButtonSessionId.Enabled = false;

            devicesToolStripButtonIdentify.Enabled = false;
        }


        private void listViewDevices_SelectedIndexChanged(object sender, EventArgs e)
        {
            //TS - [P] - Get the list of selected sources and in turn change the toolbar enabled/disabled.
            OmSource[] selected = devicesListViewUpdateEnabled();

            //TS - Deselect any files from listview if devices selected
            if (selected.Length > 0)
            {
                filesListView.SelectedItems.Clear();
            }

            //TS - [P] - TODO - Ask Dan why we have a property window, it isn't used yet.
            propertyGridDevice.SelectedObjects = selected;

            //TS - [P] - If there is one device selected then open it in the dataViewer
            if (selected.Length == 1 && selected[0] is OmDevice && ((OmDevice)selected[0]).Connected)
            {
                dataViewer.Open(selected[0].DeviceId);
            }
            else
            {
                dataViewer.Close();
            }

        }

        private void listViewDevices_ItemActivate(object sender, EventArgs e)
        {
            //            MessageBox.Show("");
        }


        private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            // Save properties
            Properties.Settings.Default.Save();
        }

        private void optionsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OptionsDialog optionsDialog = new OptionsDialog();
            DialogResult dr = optionsDialog.ShowDialog(this);

            //PropertyEditor propertyEditor = new PropertyEditor("Options", OmGui.Properties.Settings.Default, true);
            //DialogResult dr = propertyEditor.ShowDialog(this);

            if (dr == DialogResult.OK)
            {
                OmGui.Properties.Settings.Default.CurrentPluginFolder = optionsDialog.DefaultPluginName;

                Console.WriteLine(Properties.Settings.Default.CurrentPluginFolder);
                OmGui.Properties.Settings.Default.Save();
            }
            else
            {
                OmGui.Properties.Settings.Default.Reload();
            }
        }


        //public void DataFileRemove(string file)
        //{
        //    if (listViewFiles.ContainsKey(file))
        //    {
        //        devicesListView.Items.Remove(listViewFiles[file]);
        //        listViewFiles.Remove(file);
        //    }
        //}

        //TS - [P] - Old add function for joint listview, now seperate
        //public void DataFileAdd(string file)
        //{
        //    OmSource device = OmReader.Open(file);
        //    if (device != null)
        //    {
        //        ListViewItem listViewItem = new ListViewItem();
        //        listViewItem.Tag = device;
        //        devicesListView.Items.Add(listViewItem);
        //        listViewFiles[file] = listViewItem;
        //        UpdateDevice(listViewItem);
        //    }
        //}

        public void FileListViewRemove(string file, bool delete)
        {
            bool isDeleted = false;

            dataViewer.CancelPreview();
            dataViewer.Close();

            if (delete)
            {
                try
                {
                    System.IO.File.Delete(file);
                    isDeleted = true;
                }
                catch (Exception e)
                {
                    MessageBox.Show(e.Message);
                }
            }

            //If just removing or we are deleting and the delete worked.
            if (!delete || (delete && isDeleted))
            {
                if (listViewFiles.ContainsKey(file))
                {
                    filesListView.Items.Remove(listViewFiles[file]);
                    listViewFiles.Remove(file);


                }
            }
        }

        public void FileListViewAdd(string file)
        {
            OmReader device = null;
            try
            {
                device = OmReader.Open(file);
            }
            catch (Exception) { Console.Error.WriteLine("ERROR: Problem reading file: " + file); }

            if (device != null)
            {
                ListViewItem item = new ListViewItem();
                item.Tag = device;

                UpdateFile(item, file);

                filesListView.Items.Add(item);
                listViewFiles[file] = item;

                device.Close();
            }
        }

        private void fileSystemWatcher_Renamed(object sender, System.IO.RenamedEventArgs e)
        {
            FileListViewRemove(e.OldFullPath, false);
            if (e.FullPath.EndsWith(fileSystemWatcher.Filter.Substring(1), StringComparison.InvariantCultureIgnoreCase))
            {
                FileListViewAdd(e.FullPath);
            }
        }

        private void fileSystemWatcher_Changed(object sender, FileSystemEventArgs e)
        {
            FileListViewRemove(e.FullPath, false);
            FileListViewAdd(e.FullPath);
        }

        private void fileSystemWatcher_Created(object sender, System.IO.FileSystemEventArgs e)
        {
            FileListViewAdd(e.FullPath);
        }

        private void fileSystemWatcher_Deleted(object sender, System.IO.FileSystemEventArgs e)
        {
            FileListViewRemove(e.FullPath, false);
        }

        private void selectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in devicesListView.Items)
            {
                item.Selected = true;
            }
        }

        private void toolStripButtonCancelDownload_Click(object sender, EventArgs e)
        {
            //TS - Recording how many are canceling from selected items to display in prompt.
            //int devicesCancelling = 0;

            Cursor.Current = Cursors.WaitCursor;

            foreach (ListViewItem i in devicesListView.SelectedItems)
            {
                OmSource device = (OmSource)i.Tag;
                if (device is OmDevice && ((OmDevice)device).IsDownloading)
                {
                    ((OmDevice)device).CancelDownload();
                }
            }

            Cursor.Current = Cursors.Default;

            //TS - Show messagebox as there are multiple.
            //if (devicesCancelling != devicesListView.SelectedItems.Count)
            {
               // MessageBox.Show(devicesCancelling + " devices cancelled downloading from a selection of " + devicesListView.SelectedItems.Count + " devices.", "Cancel Status", MessageBoxButtons.OK);
            }
        }

        public bool EnsureNoSelectedDownloading()
        {
            int downloading = 0, total = 0;
            foreach (ListViewItem i in devicesListView.SelectedItems)
            {
                OmSource device = (OmSource)i.Tag;
                if (device is OmDevice && ((OmDevice)device).IsDownloading) { downloading++; }
                total++;
            }
            if (downloading > 0)
            {
                MessageBox.Show(this, "Download in progress for " + downloading + " (of " + total + " selected) device(s) -- cannot change configuration of these devices until download complete or cancelled.", "Download in progress", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return false;
            }
            return true;
        }

        private void toolStripButtonStop_Click(object sender, EventArgs e)
        {
            OmDevice[] devices = new OmDevice[devicesListView.SelectedItems.Count];
            for (int i = 0; i < devicesListView.SelectedItems.Count; i++)
            {
                devices[i] = (OmDevice)devicesListView.SelectedItems[i].Tag;
            }

            int devicesStopped = 0;

            dataViewer.CancelPreview();

            Cursor.Current = Cursors.WaitCursor;

            BackgroundWorker stopBackgroundWorker = new BackgroundWorker();
            stopBackgroundWorker.WorkerReportsProgress = true;
            stopBackgroundWorker.WorkerSupportsCancellation = true;
            stopBackgroundWorker.DoWork += (s, ea) =>
            {
                List<string> fails = new List<string>();
                int i = 0;

                //Set up all the devices
                foreach (OmDevice device in devices)
                {
                    bool error = false;

                    if (device is OmDevice && !((OmDevice)device).IsDownloading && ((OmDevice)device).IsRecording != OmDevice.RecordStatus.Stopped)
                    {
                        stopBackgroundWorker.ReportProgress(-1, "Stopping device " + (i + 1) + " of " + devices.Length + ".");

                        if (!((OmDevice)device).NeverRecord())
                        {
                            error = true;
                        }
                        else
                        {
                            devicesStopped++;
                        }
                    }

                    if (error) { fails.Add(device.DeviceId.ToString()); }

                    i++;
                }

                stopBackgroundWorker.ReportProgress(100, "Done");
                if (fails.Count > 0)
                {
                    this.Invoke(new Action(() =>
                    MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()) + ADVICE, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1)
                    ));
                }

            };

            ShowProgressWithBackground("Stopping", "Stopping devices...", stopBackgroundWorker);
            
            devicesListViewUpdateEnabled();

            Cursor.Current = Cursors.Default;
        }

        //private void toolStripButtonRecord_Click(object sender, EventArgs e)
        //{
        //    //TS - [P] - If nothing is downloading then cancel preview of dataViewer and turn on AlwaysRecord() for each device selected.
        //    if (EnsureNoSelectedDownloading())
        //    {
        //        List<string> fails = new List<string>();
        //        dataViewer.CancelPreview();
        //        Cursor.Current = Cursors.WaitCursor;
        //        foreach (ListViewItem i in devicesListView.SelectedItems)
        //        {
        //            OmSource device = (OmSource)i.Tag;
        //            if (device is OmDevice && !((OmDevice)device).IsDownloading)
        //            {
        //                if (!((OmDevice)device).AlwaysRecord())
        //                    fails.Add(device.DeviceId.ToString());
        //            }
        //        }
        //        Cursor.Current = Cursors.Default;
        //        if (fails.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
        //    }

        //    devicesListViewUpdateEnabled();
        //}

        private void toolStripButtonRecord_Click(object sender, EventArgs e)
        {
            OmDevice[] devices = new OmDevice[devicesListView.SelectedItems.Count];
            for (int i = 0; i < devicesListView.SelectedItems.Count; i++)
            {
                devices[i] = (OmDevice) devicesListView.SelectedItems[i].Tag;
            }

            DateRangeForm rangeForm = new DateRangeForm("Recording Settings", devices);
            DialogResult dr = rangeForm.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                Cursor.Current = Cursors.WaitCursor;

                DateTime start = DateTime.MinValue;
                DateTime stop = DateTime.MaxValue;

                //List<string> fails = new List<string>();
                //List<string> fails2 = new List<string>();

                dataViewer.CancelPreview();
                //Cursor.Current = Cursors.WaitCursor;

                BackgroundWorker recordBackgroundWorker = new BackgroundWorker();
                recordBackgroundWorker.WorkerReportsProgress = true;
                recordBackgroundWorker.WorkerSupportsCancellation = true;
                recordBackgroundWorker.DoWork += (s, ea) =>
                {
                    Dictionary<string,string> fails = new Dictionary<string,string>();

                    int i = 0;

                    //Set up all the devices
                    foreach (OmDevice device in devices)
                    {
                       string error = null;

                       if (device is OmDevice && !((OmDevice)device).IsDownloading)
                       {
                           // Nothing wrong so far...
                           string message = "Configuring device " + (i + 1) + " of " + devices.Length + ".... ";

                           recordBackgroundWorker.ReportProgress((100 * (5 * i + 0) / (devices.Length * 5)), message + "(session)");

                           //Set SessionID
                           if (error == null && !((OmDevice)device).SetSessionId((uint)rangeForm.SessionID, false))
                               error = "Failed to set session ID";

                           recordBackgroundWorker.ReportProgress((100 * (5 * i + 1) / (devices.Length * 5)), message + "(metadata)");

                           // Safe to do this as metadata cleared when device cleared
                           if (error == null && rangeForm.metaData.Length > 0)
                           {
                               if (OmApi.OM_FAILED(OmApi.OmSetMetadata(device.DeviceId, rangeForm.metaData, rangeForm.metaData.Length)))
                                   error = "Metadata set failed";
                           }

                           // Check 'max samples' is always zero
                           OmApi.OmSetMaxSamples(device.DeviceId, 0);

                           recordBackgroundWorker.ReportProgress((100 * (5 * i + 2) / (devices.Length * 5)), message + "(config)");

                           //Sampling Freq and Range
                           if (error == null && OmApi.OM_FAILED(OmApi.OmSetAccelConfig(device.DeviceId, (int)rangeForm.SamplingFrequency, rangeForm.Range)))
                               error = "Accel. config failed";

                           recordBackgroundWorker.ReportProgress((100 * (5 * i + 3) / (devices.Length * 5)), message + "(time sync)");

                           //Do Sync Time
                           if (error == null && rangeForm.SyncTime != DateRangeForm.SyncTimeType.None)
                           {
                               //Cursor.Current = Cursors.WaitCursor;
                               //foreach (ListViewItem i in devicesListView.SelectedItems)
                               //{
                               //    OmSource device = (OmSource)i.Tag;
                               if (device is OmDevice && ((OmDevice)device).Connected)
                               {
                                   if (!((OmDevice)device).SyncTime((int)rangeForm.SyncTime))
                                       error = "Time sync. failed";
                               }
                           }

                           recordBackgroundWorker.ReportProgress((100 * (5 * i + 4) / (devices.Length * 5)), message + "(interval)");

                           // Set the devices to record -- IMPORTANT: This also 'commits' the settings to the device
                           if (error == null)
                           {
                               if (rangeForm.Always)
                               {
                                   //TS - Taken from Record button
                                   if (!((OmDevice)device).AlwaysRecord())
                                       error = "Set interval (always) failed";
                               }
                               else
                               {
                                   //Do datetime intervals
                                   start = rangeForm.StartDate;
                                   stop = rangeForm.EndDate;

                                   if (!((OmDevice)device).SetInterval(start, stop))
                                       error = "Set interval failed";
                               }
                           }


                       }
                       else
                       {
                           error = "Device is downloading";
                       }
                        
                        if (error != null) { 
                            fails.Add(device.DeviceId.ToString(), error); 
                        }

                        i++;
                    }

                    recordBackgroundWorker.ReportProgress(100, "Done");
                    Thread.Sleep(100);
                    if (fails.Count > 0)
                    {
                        StringBuilder errorMessage = new StringBuilder();
                        errorMessage.Append("Failed operation on " + fails.Count + " device(s):\r\n");
                        foreach (KeyValuePair<string, string> kvp in fails)
                        {
                            errorMessage.Append("" + kvp.Key + ": " + kvp.Value + "\r\n");
                        }
                        this.Invoke(new Action(() =>
                            MessageBox.Show(this, errorMessage.ToString() + ADVICE, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1)
                        ));
                    }

                };

                ShowProgressWithBackground("Configuring", "Configuring devices...", recordBackgroundWorker);

                Cursor.Current = Cursors.Default;
            }
        }

        /*
        private void toolStripButtonInterval_Click(object sender, EventArgs e)
        {
            //if (EnsureNoSelectedDownloading())
            //{
            List<string> fails = new List<string>();

            DateTime start = DateTime.MinValue;
            DateTime stop = DateTime.MaxValue;

            OmDevice device = (OmDevice)devicesListView.SelectedItems[0].Tag;

            DateRangeForm rangeForm = new DateRangeForm("Recording Settings", new OmDevice[] {device});
            DialogResult dr = rangeForm.ShowDialog();
            
            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                start = rangeForm.FromDate;
                stop = rangeForm.UntilDate;

                dataViewer.CancelPreview();
                Cursor.Current = Cursors.WaitCursor;

                //Don't want it to be able to work on multiple do we because of metadata??
                //foreach (ListViewItem i in devicesListView.SelectedItems)
                //{                        
                if (device is OmDevice && !((OmDevice)device).IsDownloading)
                {
                    //Set Date
                    if (rangeForm.Always)
                    {
                        //TS - Taken from Record button
                        if (!((OmDevice)device).AlwaysRecord())
                            fails.Add(device.DeviceId.ToString());
                    }
                    else
                    {
                        if (!((OmDevice)device).SetInterval(start, stop))
                            fails.Add(device.DeviceId.ToString());
                    }

                    //Set SessionID
                    if (!((OmDevice)device).SetSessionId((uint)rangeForm.SessionID))
                        fails.Add(device.DeviceId.ToString());
                }

                //Sampling Freq and Range
                OmApi.OmSetAccelConfig(device.DeviceId, (int) rangeForm.SamplingFrequency, rangeForm.Range);
            }
            Cursor.Current = Cursors.Default;

            //if (fails.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
            //}

            //TS - If more than 1 device selected then show which could and couldn't be set up.
            //if (devicesListView.SelectedItems.Count > 1)
            //{
            //    MessageBox.Show(devicesCanRecord + " devices setup record from a selection of " + devicesListView.SelectedItems.Count + " devices.", "Record Status", MessageBoxButtons.OK);
            //}

            //Do Sync Time
            if (rangeForm.SyncTime != DateRangeForm.SyncTimeType.None)
            {
                List<string> fails2 = new List<string>();
                Cursor.Current = Cursors.WaitCursor;
                //foreach (ListViewItem i in devicesListView.SelectedItems)
                //{
                //    OmSource device = (OmSource)i.Tag;
                if (device is OmDevice && ((OmDevice)device).Connected)
                {
                    if (!((OmDevice)device).SyncTime((int)rangeForm.SyncTime))
                        fails2.Add(device.DeviceId.ToString());
                }
                //}
                Cursor.Current = Cursors.Default;
                if (fails2.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails2.Count + " device(s):\r\n" + string.Join("; ", fails2.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
            }

            devicesListViewUpdateEnabled();
        }
         */

        /*
        private void toolStripButtonSessionId_Click(object sender, EventArgs e)
        {
            if (EnsureNoSelectedDownloading())
            {
                InputBox inputBox = new InputBox("Session ID", "Session ID", "0");
                DialogResult dr = inputBox.ShowDialog();
                if (dr == System.Windows.Forms.DialogResult.OK)
                {
                    List<string> fails = new List<string>();
                    uint value = 0;
                    if (!uint.TryParse(inputBox.Value, out value)) { return; }

                    dataViewer.CancelPreview();
                    Cursor.Current = Cursors.WaitCursor;
                    foreach (ListViewItem i in devicesListView.SelectedItems)
                    {
                        OmSource device = (OmSource)i.Tag;
                        if (device is OmDevice && !((OmDevice)device).IsDownloading)
                        {
                            if (!((OmDevice)device).SetSessionId(value))
                                fails.Add(device.DeviceId.ToString());
                        }
                    }
                    Cursor.Current = Cursors.Default;
                    if (fails.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
                }
            }
        }
        */

        private void toolStripButtonClear_Click(object sender, EventArgs e)
        {
            //TS - Record which are cleared and stopped if multiple selected.
            //            int devicesClearing = 0;

            bool wipe = ((Form.ModifierKeys & Keys.Shift) == 0);

            if (EnsureNoSelectedDownloading())
            {
                DialogResult dr = MessageBox.Show(this, (wipe ? "Wipe" : "Clear") + " " + devicesListView.SelectedItems.Count + " device(s)?", Text, MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                if (dr == System.Windows.Forms.DialogResult.OK)
                {
                    //Cursor.Current = Cursors.WaitCursor;

                    dataViewer.CancelPreview();
                    dataViewer.Reader = null;
                    dataViewer.Close();

                    List<OmDevice> devices = new List<OmDevice>();
                    foreach (ListViewItem i in devicesListView.SelectedItems)
                    {
                        OmSource device = (OmSource)i.Tag;
                        if (device is OmDevice && !((OmDevice)device).IsDownloading)
                        {
                            devices.Add((OmDevice)device);
                        }
                    }

                    BackgroundWorker clearBackgroundWorker = new BackgroundWorker();
                    clearBackgroundWorker.WorkerReportsProgress = true;
                    clearBackgroundWorker.WorkerSupportsCancellation = true;
                    clearBackgroundWorker.DoWork += (s, ea) =>
                    {
                        List<string> fails = new List<string>();
                        int i = 0;
                        foreach (OmDevice device in devices)
                        {
                            clearBackgroundWorker.ReportProgress(-1, (wipe ? "Wiping" : "Clearing") + " device " + (i + 1) + " of " + devices.Count + ".");
                            if (!((OmDevice)device).Clear(wipe))
                                fails.Add(device.DeviceId.ToString());
                            i++;
                        }
                        clearBackgroundWorker.ReportProgress(100, "Done");
                        if (fails.Count > 0)
                        {
                            this.Invoke(new Action(() =>
                                MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()) + ADVICE, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1)
                            ));
                        }
                    };

                    ShowProgressWithBackground((wipe ? "Wiping" : "Clearing"), (wipe ? "Wiping" : "Clearing") + " devices...", clearBackgroundWorker);

                    //Cursor.Current = Cursors.Default;
                }
            }
        }

        private void ShowProgressWithBackground(string title, string message, BackgroundWorker backgroundWorker)
        {
            BlockBackgroundTasks();

            ProgressBox progressBox = new ProgressBox(title, message, backgroundWorker);
            progressBox.ShowDialog(this);

            // Re-enable refreshes
            EnableBackgroundTasks();

        }

        private void openDownloadFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // "explorer.exe /select, <filename>"
            System.Diagnostics.Process.Start(GetPath(OmGui.Properties.Settings.Default.CurrentWorkingFolder));
        }

        private void ExportDataConstruct()
        {
            float blockStart = -1;
            float blockCount = -1;

            // Selection
            if (dataViewer != null)
            {
                if (dataViewer.HasSelection)
                {
                    blockStart = dataViewer.SelectionBeginBlock + dataViewer.OffsetBlocks;
                    blockCount = dataViewer.SelectionEndBlock - dataViewer.SelectionBeginBlock;
                }
            }

            List<string> files = new List<string>();

            //TS - Record how many can be exported. (Little different to the other buttons because this method is used for both files and devices.
            int devicesSelected = -1; //-1 means files.
            if (devicesListView.SelectedItems.Count > 0)
            {
                devicesSelected = devicesListView.SelectedItems.Count;
            }

            //TS - TODO - Will need to have array of devices for batch export.
            if (devicesListView.SelectedItems.Count > 0)
            {
                foreach (ListViewItem item in devicesListView.SelectedItems)
                {
                    //TS - Don't want to try and export from a device that is running.
                    OmDevice device = (OmDevice)item.Tag;

                    //TS - If the device isn't recording or downloading then we can export.
                    if (device.IsRecording == OmDevice.RecordStatus.Stopped && !device.IsDownloading)
                    {
                        files.Add(device.Filename);
                    }
                }
            }
            else if (filesListView.SelectedItems.Count > 0)
            {
                foreach (ListViewItem item in filesListView.SelectedItems)
                {
                    OmReader reader = (OmReader)item.Tag;
                    files.Add(reader.Filename);
                }
            }

            //TS - [P] - Old now -> When files and devices were both in same list view.
            //foreach (ListViewItem i in devicesListView.SelectedItems)
            //{
            //    OmSource device = (OmSource)i.Tag;
            //    if (!(device is OmDevice) || !((OmDevice)device).Connected)
            //    {
            //        if (device is OmReader)
            //        {
            //            files.Add(((OmReader)device).Filename);
            //            numFiles++;
            //        }
            //    }
            //    else
            //    {
            //        if (device is OmDevice)
            //        {
            //            files.Add(((OmDevice)device).Filename);
            //            numDevices++;
            //        }
            //    }
            //}

            //Export files
            //If we are chosing from the dataViewer then we only have one else we can have many.
            if (blockStart > -1 && blockCount > -1)
                ExportData(files[0], blockStart, blockCount);
            else
                ExportData(files, devicesSelected);
        }

        private void ExportData(string fileName, float blockStart, float blockCount)
        {
            string folder = GetPath(Properties.Settings.Default.CurrentWorkingFolder);
            ExportForm exportForm = new ExportForm(fileName, folder, blockStart, blockCount);
            DialogResult result = exportForm.ShowDialog(this);
        }

        //Exporting many.
        private void ExportData(List<string> files, int numDevicesSelected)
        {
            string folder = GetPath(Properties.Settings.Default.CurrentWorkingFolder);

            ExportForm exportForm;

            foreach (string fileName in files)
            {
                exportForm = new ExportForm(fileName, folder, -1, -1);
                exportForm.ShowDialog();
            }

            //If 'numDevicesSelected' > -1 then we are dealing with devices otherwise we are dealing with files.
            if (numDevicesSelected > 1)
            {
                //TS - Display messagebox of which exported from those selected.
                MessageBox.Show(files.Count + " device data exported from a selection of " + numDevicesSelected + " devices.", "Export Status", MessageBoxButtons.OK);
            }
        }

        #region Working Folder Paradigm

        //TS - Properties
        public string CurrentWorkingFolder { get; set; }

        //TS - Fields
        public bool inWorkingFolder = false;
        string defaultTitleText = "Open Movement (Beta Testing Version )" + " [V" + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString() + "]";

        private void workingFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            fbd.ShowNewFolderButton = true;
            fbd.SelectedPath = Properties.Settings.Default.CurrentWorkingFolder;
            fbd.Description = "Choose or Create New Working Folder";

            DialogResult dr = fbd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                Properties.Settings.Default.CurrentWorkingFolder = fbd.SelectedPath;
            }

            LoadWorkingFolder();

            //Add to recent folders.
            if (!Properties.Settings.Default.RecentFolders.Contains(fbd.SelectedPath))
            {
                if (Properties.Settings.Default.RecentFolders.Count == 5)
                {
                    Properties.Settings.Default.RecentFolders.RemoveAt(4);
                }

                Properties.Settings.Default.RecentFolders.Insert(0, fbd.SelectedPath);
            }
            else
            {
                //If we have it we should move it up the list so it is the most recent.
                Properties.Settings.Default.RecentFolders.Remove(fbd.SelectedPath);
                Properties.Settings.Default.RecentFolders.Insert(0, fbd.SelectedPath);
            }

            UpdateRecentFoldersInGUI();
        }

        private void UpdateRecentFoldersInGUI()
        {
            recentFoldersToolStripMenuItem.DropDownItems.Clear();

            for (int i = 0; i < Properties.Settings.Default.RecentFolders.Count; i++)
            {
                recentFoldersToolStripMenuItem.DropDownItems.Add(Properties.Settings.Default.RecentFolders[i]);
                recentFoldersToolStripMenuItem.DropDownItems[i].Click += new EventHandler(MainForm_Click);
            }

            //If we haven't got the button on there yet then add it.
            if (recentFoldersToolStripMenuItem.DropDownItems.Count == Properties.Settings.Default.RecentFolders.Count)
            {
                recentFoldersToolStripMenuItem.DropDownItems.Add(new ToolStripSeparator());

                ToolStripButton t = new ToolStripButton();
                t.Text = "Clear Recent Folders...";
                t.Click += new EventHandler(t_Click);

                recentFoldersToolStripMenuItem.DropDownItems.Add(t);
            }
        }

        void t_Click(object sender, EventArgs e)
        {
            for (int i = recentFoldersToolStripMenuItem.DropDownItems.Count - 3; i >= 0; i--)
            {
                ToolStripItem t = recentFoldersToolStripMenuItem.DropDownItems[i];
                if (!t.Text.Equals(Properties.Settings.Default.CurrentWorkingFolder) && (t.GetType() != typeof(ToolStripButton) || t.GetType() != typeof(ToolStripSeparator)))
                {
                    recentFoldersToolStripMenuItem.DropDownItems.Remove(t);
                    Properties.Settings.Default.RecentFolders.Remove(t.Text);
                }


            }
        }

        void MainForm_Click(object sender, EventArgs e)
        {
            ToolStripItem t = (ToolStripItem)sender;

            Properties.Settings.Default.CurrentWorkingFolder = t.Text;

            Properties.Settings.Default.RecentFolders.Remove(t.Text);
            Properties.Settings.Default.RecentFolders.Insert(0, t.Text);

            LoadWorkingFolder();

            UpdateRecentFoldersInGUI();
        }

        private void LoadWorkingFolder()
        {
            //Load Plugin
            //Properties.Settings.Default.CurrentPluginFolder = Properties.Settings.Default.CurrentWorkingFolder;

            inWorkingFolder = true;

            Text = defaultTitleText + " - " + Properties.Settings.Default.CurrentWorkingFolder;

            //Find files
            try
            {
                string folder = GetPath(OmGui.Properties.Settings.Default.CurrentWorkingFolder);

                if (!System.IO.Directory.Exists(folder))
                {
                    System.IO.Directory.CreateDirectory(folder);
                }

                fileSystemWatcherOutput.Path = folder;
                fileListViewOutputRefreshList();

                //Files Watcher
                fileSystemWatcher.Path = folder;
                fileListViewRefreshList();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }

            //Add files in project to list.
            //if (downloadedFiles.Length > 0)
            //{
            //    foreach (string fileName in downloadedFiles)
            //    {
            //        FileListViewAdd(Properties.Settings.Default.CurrentWorkingFolder + "\\" + fileName);
            //    }
            //}

            //Set watcher path
            fileSystemWatcher.Path = OmGui.Properties.Settings.Default.CurrentWorkingFolder;

            //Profile Plugins
            AddProfilePluginsToToolStrip();
        }

        private void fileListViewRefreshList()
        {
            filesListView.Items.Clear();
            listViewFiles.Clear();

            string[] filePaths = System.IO.Directory.GetFiles(fileSystemWatcher.Path, fileSystemWatcher.Filter, System.IO.SearchOption.TopDirectoryOnly);
            foreach (string f in filePaths)
            {
                FileListViewAdd(f);
            }
        }

        private void fileListViewOutputRefreshList()
        {
            outputListView.Items.Clear();
            listViewOutputFiles.Clear();

            string[] filePaths = Directory.GetFiles(fileSystemWatcherOutput.Path, fileSystemWatcherOutput.Filter, System.IO.SearchOption.TopDirectoryOnly);
            foreach (string f in filePaths)
            {
                if(!Path.GetExtension(f).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase))
                    outputListAddItem(f);
            }
        }

        private void UnloadWorkingFolder()
        {
            Properties.Settings.Default.Reset();

            Text = defaultTitleText;

            inWorkingFolder = false;
        }

        #endregion

        //TS - TODO - This is hard-coded at the moment for testing purposes.
        private const string PLUGIN_PROFILE_FILE = "profile.xml";

        public PluginManager pluginManager;

        private void MainForm_Load(object sender, EventArgs e)
        {
            FilesResetToolStripButtons();

            //Create PluginManager
            pluginManager = new PluginManager();

            //TS - Working Folder logic
            //If current plugin folder is empty then make it My Documents.
            //if (Properties.Settings.Default.CurrentPluginFolder == "")
            //    Properties.Settings.Default.CurrentPluginFolder = GetPath("C:\\OM\\DefaultPlugins\\");

            if (Properties.Settings.Default.CurrentPluginFolder.Length == 0)
            {
                string test = Directory.GetCurrentDirectory() + Path.DirectorySeparatorChar + "Plugins";
                Console.WriteLine("TEST: " + test);
                Properties.Settings.Default.CurrentPluginFolder = Directory.GetCurrentDirectory() + Path.DirectorySeparatorChar + "Plugins";
            }

            Console.WriteLine("Current: " + Properties.Settings.Default.CurrentWorkingFolder);
            Console.WriteLine("Current Plugin: " + Properties.Settings.Default.CurrentPluginFolder);

            if (Properties.Settings.Default.CurrentWorkingFolder.Equals(""))
                Properties.Settings.Default.CurrentWorkingFolder = GetPath("{MyDocuments}");

            LoadWorkingFolder();

            //Recent Folders
            if (Properties.Settings.Default.RecentFolders.Count == 0)
            {
                Properties.Settings.Default.RecentFolders.Add(Properties.Settings.Default.CurrentWorkingFolder);
            }

            UpdateRecentFoldersInGUI();

            //Setup Column Sorting
            setupColumnSorter();
        }

        private void AddProfilePluginsToToolStrip()
        {
            for(int i = toolStripFiles.Items.Count; i > 4; i--)
            {
                toolStripFiles.Items.RemoveAt(toolStripFiles.Items.Count-1);
            }
            
            if (pluginManager.LoadProfilePlugins())
            {
                bool isFirst = true;

                foreach (Plugin plugin in pluginManager.ProfilePlugins)
                {
                    if (plugin != null)
                    {
                        ToolStripButton tsb = new ToolStripButton();
                        tsb.Text = plugin.ReadableName;
                        tsb.Tag = plugin;

                        if (plugin.Icon != null)
                            tsb.Image = plugin.Icon;

                        tsb.Click += new EventHandler(tsb_Click);

                        if (isFirst)
                        {
                            toolStripFiles.Items.Add(new ToolStripSeparator());
                            isFirst = false;
                        }

                        toolStripFiles.Items.Add(tsb);
                        toolStripFiles.Items[toolStripFiles.Items.Count - 1].Enabled = false;
                    }
                }
            }

            //TODO - Old code, delete after testing code above.
            //TS - Add tool strip buttons for "default" profiles
            /*try
            {
                StreamReader pluginProfile = new StreamReader(Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + PLUGIN_PROFILE_FILE);
                string pluginProfileAsString = pluginProfile.ReadToEnd();

                //Parse
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.LoadXml(pluginProfileAsString);
                XmlNodeList items = xmlDoc.SelectNodes("Profile/Plugin");

                //Used to add seperator.
                bool isFirst = true;

                //Each <Plugin>
                foreach (XmlNode node in items)
                {
                    string name = "";
                    string files = "";

                    //Each < /> inside <Plugin>
                    foreach (XmlNode childNode in node.ChildNodes)
                    {
                        switch (childNode.Name)
                        {
                            case "name":
                                name = childNode.InnerText;
                                break;
                            case "files":
                                files = childNode.InnerText;
                                break;
                        }
                    }

                    FileInfo[] pluginInfo = getPluginInfo(files);

                    if (pluginInfo != null)
                    {
                        if (pluginInfo[0] != null && pluginInfo[1] != null && pluginInfo[2] != null)
                        {
                            Plugin plugin = new Plugin(pluginInfo[0], pluginInfo[1], pluginInfo[2]);

                            ToolStripButton tsb = new ToolStripButton();
                            tsb.Text = name;
                            tsb.Tag = plugin;

                            if (plugin.Icon != null)
                                tsb.Image = plugin.Icon;

                            tsb.Click += new EventHandler(tsb_Click);

                            if (isFirst)
                            {
                                toolStripFiles.Items.Add(new ToolStripSeparator());
                                isFirst = false;
                            }

                            toolStripFiles.Items.Add(tsb);
                            toolStripFiles.Items[toolStripFiles.Items.Count - 1].Enabled = false;
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }*/
        }

        //Click event handler for the tool strip default plugins that comes from the profile file...
        void tsb_Click(object sender, EventArgs e)
        {
            ToolStripButton tsb = sender as ToolStripButton;

            if (tsb != null && tsb.Tag != null)
            {
                if (filesListView.SelectedItems.Count > 0)
                {
                    string[] CWAFilenames = new string[filesListView.SelectedItems.Count];

                    for (int i = 0; i < filesListView.SelectedItems.Count; i++)
                    {
                        OmReader reader = (OmReader)filesListView.SelectedItems[0].Tag;

                        CWAFilenames[i] = reader.Filename;
                    }

                    Plugin p = (Plugin)tsb.Tag;

                    float blockStart = -1;
                    float blockCount = -1;
                    float blockEnd = -1;

                    DateTime startDateTime;
                    DateTime endDateTime;

                    if (dataViewer.HasSelection)
                    {
                        blockStart = dataViewer.SelectionBeginBlock + dataViewer.OffsetBlocks;
                        blockCount = dataViewer.SelectionEndBlock - dataViewer.SelectionBeginBlock;
                        blockEnd = blockStart + blockCount;

                        startDateTime = dataViewer.TimeForBlock(blockStart);
                        endDateTime = dataViewer.TimeForBlock(blockEnd);
                        string startDateTimeString = startDateTime.ToString("dd/MM/yyyy/_HH:mm:ss");
                        string endDateTimeString = endDateTime.ToString("dd/MM/yyyy/_HH:mm:ss");

                        //See now if we want the dataViewer selection.
                        if (blockCount > -1 && blockStart > -1)
                        {
                            p.SelectionBlockStart = blockStart;
                            p.SelectionBlockCount = blockCount;
                        }

                        if (startDateTimeString != null && endDateTimeString != null)
                        {
                            p.SelectionDateTimeStart = startDateTimeString;
                            p.SelectionDateTimeEnd = endDateTimeString;
                        }
                    }

                    RunPluginForm rpf = new RunPluginForm(p, CWAFilenames);
                    rpf.ShowDialog();

                    if (rpf.DialogResult == System.Windows.Forms.DialogResult.OK)
                    {
                        //if the plugin has an output file    
                        RunProcess2(rpf.ParameterString, p, rpf.OriginalOutputName, CWAFilenames);

                        //Change index to the queue
                        tabControlFiles.SelectedIndex = 1;
                    }
                }
                else
                {
                    MessageBox.Show("Please choose a file to run a plugin on.");
                }
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            Om.Instance.Dispose();
        }

        private void filesListView_SelectedIndexChanged(object sender, EventArgs e)
        {
            //TS - If a file has been selected then deselect all rows of Devices list.
            if (filesListView.SelectedItems.Count > 0)
            {
                //TS - TODO - The text goes from Red to Black...
                devicesListView.SelectedItems.Clear();

                //Enable plugins
                for (int i = 1; i < toolStripFiles.Items.Count; i++)
                {
                    toolStripFiles.Items[i].Enabled = true;
                }
            }
            else
            {
                FilesResetToolStripButtons();
            }

            //Open DataViewer
            if (filesListView.SelectedItems.Count == 1)
            {
                OmReader reader = (OmReader)filesListView.SelectedItems[0].Tag;
                dataViewer.Open(reader.Filename);
            }
            else
            {
                dataViewer.CancelPreview();
                dataViewer.Close();
            }
        }

        private void FilesResetToolStripButtons()
        {
            //Disable buttons
            for (int i = 1; i < toolStripFiles.Items.Count; i++)
            {
                toolStripFiles.Items[i].Enabled = false;
            }
        }

        private void DeleteFilesToolStripButton_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show("Are you sure?", "Delete Data File", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button2);

            if (dr == System.Windows.Forms.DialogResult.Yes)
            {
                foreach (ListViewItem item in filesListView.SelectedItems)
                {
                    OmReader reader = (OmReader)item.Tag;
                    FileListViewRemove(reader.Filename, true);
                }
            }
        }

        List<OmDevice> identifyDevices = new List<OmDevice>();
        private void devicesToolStripButtonIdentify_Click(object sender, EventArgs e)
        {
            Cursor.Current = Cursors.WaitCursor;

            lock (identifyDevices)
            {
                identifyDevices.Clear();

                foreach (ListViewItem item in devicesListView.SelectedItems)
                {
                    OmDevice device = (OmDevice)item.Tag;
                    //device.SetLed(OmApi.OM_LED_STATE.OM_LED_BLUE);
                    identifyDevices.Add(device);
                }

                if (devicesListView.SelectedItems.Count > 0)
                {
                    //Start timer to turn off led.
                    identifyTicks = 10;     // counts down
                }
            }

            Cursor.Current = Cursors.Default;
        }


        //TS - Now the plugins button
        private void devicesToolStripButtonExport_Click(object sender, EventArgs e)
        {
            RunPluginsProcess(devicesListView.SelectedItems);
        }

        #region Plugin Stuff

        private FileInfo[] getPluginInfo(string pluginName)
        {
            FileInfo[] pluginInfo = new FileInfo[3];

            string folder = Properties.Settings.Default.CurrentPluginFolder;

            DirectoryInfo d = new DirectoryInfo(folder);

            FileInfo[] allFiles = d.GetFiles("*.*");

            foreach (FileInfo f in allFiles)
            {
                Console.WriteLine("name: " + f.Name);
                if (Path.GetFileNameWithoutExtension(f.Name).Equals(pluginName))
                {
                    //We've found our plugin.
                    if (f.Extension.Equals(".html"))
                    {
                        pluginInfo[2] = f;
                    }
                    else if (f.Extension.Equals(".xml"))
                    {
                        pluginInfo[1] = f;
                    }
                    else if (f.Extension.Equals(".exe"))
                    {
                        pluginInfo[0] = f;
                    }
                }
            }

            //We should have our FileInfo array by now
            if (pluginInfo[0] != null && pluginInfo[1] != null && pluginInfo[2] != null)
            {
                return pluginInfo;
            }

            return null;
        }

        private void RunPluginsProcess(ListView.SelectedListViewItemCollection listItems)
        {
            if (filesListView.SelectedItems.Count > 0)
            {
                //MessageBox.Show("files selected");
                if (pluginManager.Plugins.Count == 0)
                {
                    //MessageBox.Show("plugins exist");
                    pluginManager.LoadPlugins();
                }
                if (pluginManager.Plugins.Count > 0)
                {
                    //MessageBox.Show("plugins count > 0");
                    string[] CWAFilenames = new string[listItems.Count];

                    for (int i = 0; i < listItems.Count; i++)
                    {
                        OmReader reader = (OmReader)listItems[i].Tag;

                        CWAFilenames[i] = reader.Filename;
                    }

                    float blockStart = -1;
                    float blockCount = -1;
                    float blockEnd = -1;

                    DateTime startDateTime;
                    DateTime endDateTime;

                    string startDateTimeString = "";
                    string endDateTimeString = "";

                    if (dataViewer.HasSelection)
                    {
                        blockStart = dataViewer.SelectionBeginBlock;
                        blockCount = dataViewer.SelectionEndBlock - dataViewer.SelectionBeginBlock;
                        blockEnd = blockStart + blockCount;

                        startDateTime = dataViewer.TimeForBlock(blockStart);
                        endDateTime = dataViewer.TimeForBlock(blockEnd);

                        startDateTimeString = startDateTime.ToString("dd/MM/yyyy/_HH:mm:ss");
                        endDateTimeString = endDateTime.ToString("dd/MM/yyyy/_HH:mm:ss");
                    }

                    //MessageBox.Show("launching plugins form");
                    PluginsForm pluginsForm = new PluginsForm(pluginManager, CWAFilenames, blockStart, blockCount, startDateTimeString, endDateTimeString);

                    pluginsForm.ShowDialog();

                    if (pluginsForm.DialogResult == System.Windows.Forms.DialogResult.OK)
                    {
                        //if the plugin has an output file    
                        RunProcess2(pluginsForm.rpf.ParameterString, pluginsForm.SelectedPlugin, pluginsForm.rpf.OriginalOutputName, CWAFilenames);

                        //Change index to the queue
                        tabControlFiles.SelectedIndex = 1;
                    }
                }
                else
                {
                    MessageBox.Show("There are no plugins in this folder.\r\nPlease add plugin folders or change your Plugin folder in Options.", "No Plugins Found", MessageBoxButtons.OK, MessageBoxIcon.Error);    
                }
            }
            else
            {
                MessageBox.Show("Please choose a file to run a plugin on.");
            }
        }

            //TS -  Old version where plugin finding was done on the fly as it is here. This was a waste and now we use the PluginManager.
            //Find plugins and build form.
            /*if (listItems.Count > 0)
            {
                ListViewItem i = listItems[0];

                OmReader reader = (OmReader)i.Tag;

                //TODO - put new plugins code here using PluginManager.

                List<Plugin> plugins = new List<Plugin>();

                string folder = Properties.Settings.Default.CurrentPluginFolder;

                DirectoryInfo d = new DirectoryInfo(folder);

                FileInfo[] jarFiles = d.GetFiles("*.jar");
                FileInfo[] pythonFiles = d.GetFiles("*.py");
                FileInfo[] matlabFiles = d.GetFiles("*.m");
                FileInfo[] exeFiles = d.GetFiles("*.exe");
                FileInfo[] htmlFiles = d.GetFiles("*.html");
                FileInfo[] xmlFiles = d.GetFiles("*.xml");

                foreach (FileInfo x in xmlFiles)
                {
                    foreach (FileInfo h in htmlFiles)
                    {
                        //If the html name is the same as the exe name look for the xml
                        if (Path.GetFileNameWithoutExtension(h.Name).Equals(Path.GetFileNameWithoutExtension(x.Name)))
                        {
                            foreach (FileInfo e in exeFiles)
                            {
                                //We've found a plugin!
                                if (Path.GetFileNameWithoutExtension(h.Name).Equals(Path.GetFileNameWithoutExtension(e.Name)))
                                {
                                    plugins.Add(new Plugin(e, x, h));
                                }
                            }

                            foreach (FileInfo j in jarFiles)
                            {
                                //We've found a plugin!
                                if (Path.GetFileNameWithoutExtension(h.Name).Equals(Path.GetFileNameWithoutExtension(j.Name)))
                                {
                                    plugins.Add(new Plugin(j, x, h));
                                }
                            }

                            foreach (FileInfo p in pythonFiles)
                            {
                                //We've found a plugin!
                                if (Path.GetFileNameWithoutExtension(h.Name).Equals(Path.GetFileNameWithoutExtension(p.Name)))
                                {
                                    plugins.Add(new Plugin(p, x, h));
                                }
                            }

                            foreach (FileInfo m in matlabFiles)
                            {
                                //We've found a plugin!
                                if (Path.GetFileNameWithoutExtension(h.Name).Equals(Path.GetFileNameWithoutExtension(m.Name)))
                                {
                                    plugins.Add(new Plugin(m, x, h));
                                }
                            }
                        }
                    }
                }

                //    //Find XML files


                //    List<FileInfo> htmlFiles = new List<FileInfo>();

                //    foreach (FileInfo f in files)
                //    {
                //        if (f.Extension == ".html")
                //            htmlFiles.Add(f);
                //    }

                //    //Find matching other files and add to plugins dictionary
                //    foreach (FileInfo f in files)
                //    {
                //        if (f.Extension != ".html" && f.Extension != ".xml")
                //            foreach (FileInfo htmlFile in htmlFiles)
                //            {
                //                string name = Path.GetFileNameWithoutExtension(f.Name);
                //                string xmlName = Path.GetDirectoryName(f.FullName) + "\\" + name + ".xml";

                //                FileInfo xmlFile = new FileInfo(xmlName);

                //                if (Path.GetFileNameWithoutExtension(f.Name).Equals(Path.GetFileNameWithoutExtension(htmlFile.Name)))
                //                    plugins.Add(new Plugin(f, xmlFile, htmlFile));
                //            }
                //    }
                //}
                //catch (PluginExtTypeException)
                //{
                //    MessageBox.Show("Malformed Plugin file, plugins cannot be loaded until this is resolved.", "Malformed Plugin", MessageBoxButtons.OK, MessageBoxIcon.Error);
                //}

                //If there is more than 1 plugin then show the form otherwise give error.
                if (plugins.Count > 0)
                {
                    float blockStart = -1;
                    float blockCount = -1;
                    float blockEnd = -1;

                    DateTime startDateTime;
                    DateTime endDateTime;

                    PluginsForm pluginsForm;

                    //Now see if we've got a selection on the dataViewer
                    if (dataViewer.HasSelection)
                    {
                        blockStart = dataViewer.SelectionBeginBlock + dataViewer.OffsetBlocks;
                        blockCount = dataViewer.SelectionEndBlock - dataViewer.SelectionBeginBlock;
                        blockEnd = blockStart + blockCount;

                        startDateTime = dataViewer.TimeForBlock(blockStart);
                        endDateTime = dataViewer.TimeForBlock(blockEnd);
                        string startDateTimeString = startDateTime.ToString("dd/MM/yyyy/_HH:mm:ss");
                        string endDateTimeString = endDateTime.ToString("dd/MM/yyyy/_HH:mm:ss");

                        pluginsForm = new PluginsForm(pluginManager, reader.Filename, blockStart, blockCount, startDateTimeString, endDateTimeString);
                        pluginsForm.ShowDialog(this);

                        //Run the process
                        if (pluginsForm.DialogResult == System.Windows.Forms.DialogResult.OK)
                        {
                            //if the plugin has an output file    
                            RunProcess(pluginsForm.SelectedPlugin, pluginsForm.rpf.ParameterString, pluginsForm.rpf.OriginalOutputName, reader.Filename);
                        }
                    }
                    else
                    {
                        //Now that we have our plugins, open the plugin form
                        pluginsForm = new PluginsForm(pluginManager, reader.Filename, -1, -1, null, null);
                        pluginsForm.ShowDialog(this);

                        //Run the process
                        if (pluginsForm.DialogResult == System.Windows.Forms.DialogResult.OK)
                        {
                            //if the plugin has an output file    
                            RunProcess(pluginsForm.SelectedPlugin, pluginsForm.rpf.ParameterString, pluginsForm.rpf.OriginalOutputName, reader.Filename);
                        }
                    }
                }
                //No files so do a dialog.
                else
                {
                    MessageBox.Show("No plugins found, put plugins in the current plugins folder or change your plugin folder in the Options Menu.", "No Plugins Found", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                MessageBox.Show("Please choose a file to run a plugin on.");
            }*/

        private void RunProcess2(string parameterString, Plugin p, string outputName, string[] inputNames)
        {
            ProcessStartInfo psi = new ProcessStartInfo();

            psi.FileName = p.FilePath + Path.DirectorySeparatorChar + p.RunFilePath;

            //TODO - Idea to try and do the outfile as a filepath rather than filename.
            //Find arguments and replace the output file with the full file path.
            
            
            parameterString = parameterString.Replace(outputName, Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + outputName);
            
            psi.Arguments = parameterString;

            Console.WriteLine("Arguments: " + parameterString);

            psi.UseShellExecute = false;
            psi.RedirectStandardError = false;
            psi.RedirectStandardOutput = true;

            psi.Verb = "runas";
            psi.CreateNoWindow = false;

            PluginQueueItem pqi = new PluginQueueItem(p, parameterString, inputNames);
            pqi.StartInfo = psi;
            pqi.OriginalOutputName = outputName;

            //Add PQI to file queue
            string filesStr = pqi.Files[0];
            for (int i = 1; i < pqi.Files.Length; i++)
                filesStr += "  |  " + pqi.Files[i];

            ListViewItem lvi = new ListViewItem(new string[] { pqi.Name, filesStr, "0" });

            BackgroundWorker pluginQueueWorker = new BackgroundWorker();
            pluginQueueWorker.WorkerSupportsCancellation = true;

            pluginQueueWorker.DoWork += new DoWorkEventHandler(pluginQueueWorker_DoWork);
            pluginQueueWorker.ProgressChanged += new ProgressChangedEventHandler(pluginQueueWorker_ProgressChanged);
            pluginQueueWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(pluginQueueWorker_RunWorkerCompleted);

            lvi.Tag = pluginQueueWorker;

            queueListViewItems2.Items.Add(lvi);

            pluginQueueWorker.RunWorkerAsync(pqi);
        }

        List<ListViewItem> queueListViewItems = new List<ListViewItem>();
        //PLUGIN PROCESS RUNNING

        void pluginQueueWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            Console.WriteLine("run worker completed");
            BackgroundWorker worker = (BackgroundWorker)sender;
            if (e.Result != null)
            {
                string s = (string)e.Result;

                if (s.Equals("done"))
                {
                    foreach (ListViewItem lvi in queueListViewItems2.Items)
                    {
                        BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                        if (worker.Equals(bw))
                        {
                            this.Invoke((MethodInvoker)delegate
                            {
                                lvi.SubItems[2].Text = "Complete";

                                //Make clear button completed.
                                toolStripButtonClearCompleted.Enabled = true;
                            });
                        }
                    }
                }
                else if(s.Equals("fail"))
                {
                    foreach (ListViewItem lvi in queueListViewItems2.Items)
                    {
                        BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                        if (worker.Equals(bw))
                        {
                            this.Invoke((MethodInvoker)delegate
                            {
                                lvi.SubItems[2].Text = "Error";

                                //Make clear button completed.
                                toolStripButtonClearCompleted.Enabled = true;
                            });
                        }
                    }
                }
            }
            else if (e.Cancelled)
            {
                //TS - TODO - Do we need a cancel message to pop up?
            }
            else if (e.Error != null)
            {
                MessageBox.Show("An error has occured in the plugin:\n" + e.Error.Message, "Error in Plugin", MessageBoxButtons.OK);

            }
            else
            {
                MessageBox.Show("Unknown error - please try again", "Error in Plugin", MessageBoxButtons.OK);
            }
        }

        void pluginQueueWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            Console.WriteLine("Progress Percentage: " + e.ProgressPercentage);
        }

        void pluginQueueWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            PluginQueueItem pqi = (PluginQueueItem)e.Argument;
            BackgroundWorker worker = (BackgroundWorker)sender;

            try
            {
                Process p = Process.Start(pqi.StartInfo);

                //TS - TODO - This is where we need to get the info and put it into progress bar...
                //Hack - Sometimes we don't get the last stdout line

                //parseMessage(lastLine);

                while (!p.HasExited)
                {
                    string outputLine = p.StandardOutput.ReadLine();

                    int progress = -1;
                    string statusMessage = "";
                    string errorMessage = "";

                    parseMessage(outputLine, out progress, out statusMessage, out errorMessage);

                    if (progress > -1)
                    {
                        this.Invoke((MethodInvoker)delegate
                        {
                            foreach (ListViewItem lvi in queueListViewItems2.Items)
                            {
                                BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                                if (worker.Equals(bw))
                                {
                                    lvi.SubItems[2].Text = progress.ToString();
                                }
                            }
                        });
                    }

                    if (errorMessage.Length > 0)
                    {
                        this.Invoke((MethodInvoker)delegate
                        {
                            foreach (ListViewItem lvi in queueListViewItems2.Items)
                            {
                                BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                                if (worker.Equals(bw))
                                {
                                    lvi.SubItems[2].Text = "Error: " + errorMessage;
                                }
                            }
                        });

                        e.Result = "fail";
                    }
                    else if (statusMessage.Length > 0)
                    {
                        this.Invoke((MethodInvoker)delegate
                        {
                            foreach (ListViewItem lvi in queueListViewItems2.Items)
                            {
                                BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                                if (worker.Equals(bw))
                                {
                                    lvi.SubItems[2].Text = "Status: " + statusMessage;
                                }
                            }
                        });
                    }

                    //labelStatus.Text = parseMessage(outputLine);

                    //runPluginProgressBar.Invalidate(true);
                    //labelStatus.Invalidate(true);
                }
                /*    int progress = -1;

                    parseMessage(outputLine, out progress);

                    if (progress > -1)
                    {
                        this.Invoke((MethodInvoker)delegate
                        {
                            foreach (ListViewItem lvi in queueListViewItems2.Items)
                            {
                                BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                                if (worker.Equals(bw))
                                {
                                    lvi.SubItems[1].Text = progress.ToString();
                                }
                            }
                        });
                    }*/


                //labelStatus.Text = parseMessage(outputLine);

                //runPluginProgressBar.Invalidate(true);
                //labelStatus.Invalidate(true);
                //}

                p.WaitForExit();

                Console.WriteLine("EXITED");

                if (e.Result == null || !e.Result.Equals("fail"))
                    e.Result = "done";

                p.Close();

                //HACK - Copy the output file back into the working directory.
                //If there is an output file:
                /*if (pqi.OriginalOutputName != "")
                {
                    string outputFileLocation = System.IO.Path.Combine(Properties.Settings.Default.CurrentWorkingFolder + "\\output", pqi.OriginalOutputName);
                    System.IO.File.Copy(pqi.destFolder + "temp.csv", outputFileLocation);

                    //Delete the test csv and temp cwa
                    System.IO.File.Delete(pqi.destFolder + "temp.csv");

                    //Set result
                    e.Result = "done";
                }*/

                if (pqi.Plugin.CreatesOutput)
                {
                    try
                    {
                        //File.Move(Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + pqi.OriginalOutputName, Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + pqi.OriginalOutputName + "a");
                        //File.Move(Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + pqi.OriginalOutputName + "a", Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + pqi.OriginalOutputName);
                    }
                    catch (Exception ue)
                    {
                        MessageBox.Show("Plugin Error: Unknown plugin error: " + ue.Message, "Plugin Error", MessageBoxButtons.OK);
                    }
                }
            }
            catch (InvalidOperationException ioe)
            {
                this.Invoke((MethodInvoker)delegate
                {
                    foreach (ListViewItem lvi in queueListViewItems2.Items)
                    {
                        BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                        if (worker.Equals(bw))
                        {
                            queueListViewItems2.Items.Remove(lvi);

                            Console.WriteLine("ioe: " + ioe.Message);
                            MessageBox.Show("Plugin Error: " + ioe.Message, "Plugin Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                    }
                });
            }
            catch (System.ComponentModel.Win32Exception w32e)
            {
                this.Invoke((MethodInvoker)delegate
                {
                    foreach (ListViewItem lvi in queueListViewItems2.Items)
                    {
                        BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                        if (worker.Equals(bw))
                        {
                            queueListViewItems2.Items.Remove(lvi);
                        }
                    }

                    Console.WriteLine("w32e: " + w32e.Message);
                    MessageBox.Show("Plugin Error: " + "Plugin EXE not present", "Plugin Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                });
            }
            catch (ArgumentNullException ane)
            {
                MessageBox.Show("Argument null: " + ane.Message);
            }
            catch (Exception e2)
            {
                this.Invoke((MethodInvoker)delegate
                {
                    foreach (ListViewItem lvi in queueListViewItems2.Items)
                    {
                        BackgroundWorker bw = (BackgroundWorker)lvi.Tag;
                        if (worker.Equals(bw))
                        {
                            this.Invoke((MethodInvoker)delegate
                            {
                                queueListViewItems2.Items.Remove(lvi);
                            });

                        }
                    }

                    Console.WriteLine("e2: " + e2.Message);
                    MessageBox.Show("Plugin Error: " + e2.Message, "Plugin Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                });
            }
        }

        private void parseMessage(string outputLine, out int progress, out string statusMessage, out string errorMessage)
        {
            progress = -1;
            statusMessage = "";
            errorMessage = "";

            //OUTPUT
            if (outputLine != null && outputLine.Length > 1)
            {
                if (outputLine[0] == 'p' || outputLine[0] == 'P')
                {
                    string percentage = outputLine.Split(' ').ElementAt(1);

                    progress = Int32.Parse(percentage);
                    //runPluginProgressBar.Value = Int32.Parse(percentage);
                }
                //else if (outputLine[0] == 's' || outputLine[0] == 'S')
                //{
                //    statusMessage = outputLine.Split(new char[] { ' ' }, 2).Last();
                //    //labelStatus.Text = message;
                //}
                else if (outputLine[0] == 'e' || outputLine[0] == 'E')
                {
                    errorMessage = outputLine.Split(new char[] { ' ' }, 2).Last();
                }


                Console.WriteLine("o: " + outputLine);
            }
        }

        #endregion

        //TS - This will become a plugin so keep the code here for now just so we can see the Form.
        //ExportDataConstruct(OmSource.SourceCategory.File);

        private void gotoDefaultFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LoadWorkingFolder();
        }

        private void setDefaultWorkingFolderToCurrentWorkingFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
        }

        private void pluginsToolStripButton_Click(object sender, EventArgs e)
        {
            RunPluginsProcess(filesListView.SelectedItems);
        }

        private void openCurrentWorkingFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Process.Start(Properties.Settings.Default.CurrentWorkingFolder);
        }

        private void toolStripMain_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {

        }

        private void DeleteFilesToolStripButton_Click_1(object sender, EventArgs e)
        {
            if (filesListView.SelectedItems.Count > 0)
            {
                DialogResult d = MessageBox.Show("Are you sure you want to delete?", "Delete", MessageBoxButtons.OKCancel);

                if (d == System.Windows.Forms.DialogResult.OK)
                {
                    for (int i = 0; i < filesListView.SelectedItems.Count; i++)
                    {
                        OmReader r = (OmReader)filesListView.SelectedItems[i].Tag;

                        FileListViewRemove(r.Filename, true);
                    }
                }
            }
        }

        #region Dynamic ToolStrip Files Buttons

        //TS - If we have clicked over to the queue tab then change the toolstrip items
        //private void rebuildQueueToolStripItems()
        //{
        //    toolStripFiles.Items.Clear();
        //    ToolStripButton tsb = new ToolStripButton("Cancel", Properties.Resources.DeleteHS);
        //    tsb.Click += new EventHandler(tsb_Click2);
        //    tsb.Enabled = false;

        //    toolStripFiles.Items.Add(tsb);
        //}


        //Tool Strip Cancel button


        //private void rebuildFilesToolStripItems()
        //{
        //    toolStripFiles.Items.Clear();
        //    ToolStripButton tsbPlugins = new ToolStripButton("Plugins", Properties.Resources.Export);
        //    tsbPlugins.Click += new EventHandler(devicesToolStripButtonExport_Click);
        //    tsbPlugins.Enabled = false;

        //    ToolStripButton tsbDelete = new ToolStripButton("Delete", Properties.Resources.DeleteHS);
        //    tsbDelete.Click += new EventHandler(DeleteFilesToolStripButton_Click);
        //    tsbDelete.Enabled = false;

        //    toolStripFiles.Items.Add(tsbPlugins);
        //    toolStripFiles.Items.Add(tsbDelete);

        //    AddProfilePluginsToToolStrip();
        //}

        #endregion

        #region Queue ToolStrip Buttons
        private void queueListViewItems2_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (queueListViewItems2.SelectedItems.Count > 0)
            {
                toolStripQueueButtonCancel.Enabled = true;
            }
            else
            {
                toolStripQueueButtonCancel.Enabled = false;
            }
        }

        private void toolStripQueueButtonCancel_Click(object sender, EventArgs e)
        {
            //Delete
            if (queueListViewItems2.SelectedItems.Count > 0)
            {
                foreach (ListViewItem lvi in queueListViewItems2.SelectedItems)
                {
                    queueListViewItems2.Items.Remove(lvi);

                    BackgroundWorker bw = (BackgroundWorker)lvi.Tag;

                    //TS - TODO - Need to actually know when it is killed or if it is...
                    bw.CancelAsync();
                }
            }

            //clear all
            if (queueListViewItems2.Items.Count == 0)
            {
                toolStripQueue.Enabled = false;
            }
        }
        #endregion

        private void fileSystemWatcherOutput_Created(object sender, FileSystemEventArgs e)
        {
            if(!Path.GetExtension(e.FullPath).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase))
                outputListAddItem(e.FullPath);
        }

        private void outputListAddItem(string filePath)
        {
            ListViewItem item = UpdateOutputFile(filePath);

            outputListView.Items.Add(item);
            listViewOutputFiles[filePath] = item;
        }

        private void fileSystemWatcherOutput_Deleted(object sender, FileSystemEventArgs e)
        {
            if (!Path.GetExtension(e.FullPath).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase))
                outputListRemoveItem(e.FullPath, false);
        }

        private void outputListRemoveItem(string file, bool delete)
        {
            if (listViewOutputFiles.ContainsKey(file))
            {
                outputListView.Items.Remove(listViewOutputFiles[file]);
                listViewOutputFiles.Remove(file);

                if (delete)
                {
                    try
                    {
                        System.IO.File.Delete(file);
                    }
                    catch (Exception e)
                    {
                        MessageBox.Show(e.Message);
                    }
                }
            }
        }

        private void fileSystemWatcherOutput_Renamed(object sender, RenamedEventArgs e)
        {
            if (!Path.GetExtension(e.FullPath).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase))
            {
                outputListRemoveItem(e.OldFullPath, false);
                outputListAddItem(e.FullPath);
            }
        }

        private void outputListView_DoubleClick(object sender, EventArgs e)
        {
            ListView lv = sender as ListView;

            ListViewItem item = lv.SelectedItems[0];
            string filePath = item.Tag.ToString();

            System.Diagnostics.Process.Start(filePath);
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ExportDataConstruct();
        }

        bool showAll = true;
        private void toolStripButtonShowFiles_Click(object sender, EventArgs e)
        {
            if (showAll)
            {
                fileSystemWatcher.Filter = "*.*";
                fileListViewRefreshList();
                toolStripButtonShowFiles.Text = "Show *.cwa Only";
                showAll = false;
            }
            else
            {
                fileSystemWatcher.Filter = "*.cwa";
                fileListViewRefreshList();
                toolStripButtonShowFiles.Text = "Show All Files";
                showAll = true;
            }
        }

        #region Shell Context Menu

        ShellContextMenu scm = new ShellContextMenu();
        private void outputListView_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                ListView lv = sender as ListView;
                FileInfo[] fileInfos = new FileInfo[lv.SelectedItems.Count];

                int i = 0;
                foreach (ListViewItem item in lv.SelectedItems)
                {
                    string filePath = item.Tag.ToString();

                    fileInfos[i] = new FileInfo(filePath);
                    i++;
                }

                scm.ShowContextMenu(this.Handle, fileInfos, Cursor.Position);
            }
        }

        private void filesListView_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                ListView lv = sender as ListView;
                FileInfo[] fileInfos = new FileInfo[lv.SelectedItems.Count];

                int i = 0;
                foreach (ListViewItem item in lv.SelectedItems)
                {
                    string filePath = item.SubItems[1].Text.ToString();

                    fileInfos[i] = new FileInfo(filePath);
                    i++;
                }

                scm.ShowContextMenu(this.Handle, fileInfos, Cursor.Position);
            }
        }

        #endregion

        private void toolStripQueueButtonClearAll_Click(object sender, EventArgs e)
        {
            queueListViewItems2.Items.Clear();
        }

        private void tabControlFiles_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (tabControlFiles.SelectedIndex == 0)
            {
                queueListViewItems2.SelectedItems.Clear();
                outputListView.SelectedItems.Clear();
            }
            else if (tabControlFiles.SelectedIndex == 1)
            {
                filesListView.SelectedItems.Clear();
                outputListView.SelectedItems.Clear();
                dataViewer.CancelPreview();
                dataViewer.Close();
            }
            else if (tabControlFiles.SelectedIndex == 2)
            {
                queueListViewItems2.SelectedItems.Clear();
                filesListView.SelectedItems.Clear();
                dataViewer.CancelPreview();
                dataViewer.Close();
            }
        }

        private void toolStripButtonClearCompleted_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem lvi in queueListViewItems2.Items)
            {
                string text = lvi.SubItems[2].Text;
                int res = 0;
                if (!Int32.TryParse(text, out res))
                    queueListViewItems2.Items.Remove(lvi);
            }

            toolStripButtonClearCompleted.Enabled = false;
        }

        private void filesListView_ItemDrag(object sender, ItemDragEventArgs e)
        {
            ListViewItem lvi = (ListViewItem)e.Item;

            DataObject data = new DataObject(DataFormats.FileDrop, new string[] { lvi.SubItems[1].Text });

            filesListView.DoDragDrop(data, DragDropEffects.Copy);
        }

        private void outputListView_ItemDrag(object sender, ItemDragEventArgs e)
        {
            ListViewItem lvi = (ListViewItem)e.Item;

            DataObject data = new DataObject(DataFormats.FileDrop, new string[] { lvi.SubItems[1].Text });

            outputListView.DoDragDrop(data, DragDropEffects.Copy);
       }




        private void BlockBackgroundTasks()
        {
            // No more refreshes
            refreshTimer.Enabled = false;
            if (backgroundWorkerUpdate.IsBusy)
            {
                Cursor.Current = Cursors.WaitCursor;

                // Spin while waiting for background task (not great, but tasks should be short-lived)
                while (backgroundWorkerUpdate.IsBusy)
                {
Application.DoEvents();
                    Thread.Sleep(50);
                }
                Cursor.Current = Cursors.Default;
            }
        }

        private void EnableBackgroundTasks()
        {
            refreshTimer.Enabled = true;
        }

        int identifyTicks = 0;
        int refreshIndex = 0;
        int refreshCounter = 0;
        bool doIdentifyTask = false;
        private void refreshTimer_Tick(object sender, EventArgs e)
        {
            refreshCounter++;

            if ((refreshCounter % 5) == 0) { doIdentifyTask = true; } // Latch state at 2 Hz

            if (!backgroundWorkerUpdate.IsBusy)
            {
                if (doIdentifyTask && identifyTicks > 0)
                {
                    doIdentifyTask = false;
                    backgroundWorkerUpdate.RunWorkerAsync(null);
                }
                else
                {
                    if (refreshIndex >= devicesListView.Items.Count) { refreshIndex = 0; }
                    if (refreshIndex < devicesListView.Items.Count)
                    {
                        ListViewItem item = devicesListView.Items[refreshIndex];
                        OmSource source = (OmSource)item.Tag;
                        if (source is OmDevice)
                        {
                            OmDevice device = (OmDevice)source;
                            BackgroundTaskStatus(true);
                            backgroundWorkerUpdate.RunWorkerAsync(device);
                        }
                    }
                    refreshIndex++;
                }
            }
        }


        private void BackgroundTaskStatus(bool active)
        {
            if (InvokeRequired)
            {
                this.Invoke((MethodInvoker)delegate
                {
                    BackgroundTaskStatus(active);
                });
                return;
            }
            toolStripBackgroundTask.Visible = active;
        }

        private void backgroundWorkerUpdate_DoWork(object sender, DoWorkEventArgs e)
        {
            // If identify task
            if (e.Argument == null)
            {
                identifyTicks--;

                //Turn off lights
                lock (identifyDevices)
                {
                    foreach (OmDevice device in identifyDevices)
                    {
                        OmApi.OM_LED_STATE state;

                        if (identifyTicks <= 0)
                        {
                            state = OmApi.OM_LED_STATE.OM_LED_AUTO;
                        }
                        else
                        {
                            bool turnOn = ((identifyTicks & 1) == 1);
                            state = (turnOn) ? OmApi.OM_LED_STATE.OM_LED_BLUE : OmApi.OM_LED_STATE.OM_LED_MAGENTA;
                        }

                        device.SetLed(state);
                    }
                }

            }
            else
            {
                OmDevice device = (OmDevice)e.Argument;
                bool changed = device.Update();
                if (changed)
                {
                    UpdateDeviceId(device.DeviceId);
                }
            }
            BackgroundTaskStatus(false);
        } 

    }
}
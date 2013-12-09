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

        private uint queryCancelAutoPlayID;

        //PluginQueue
        Queue<PluginQueueItem> pluginQueueItems = new Queue<PluginQueueItem>();

        protected override void WndProc(ref Message msg)
        {
            base.WndProc(ref msg);
            if (msg.Msg == queryCancelAutoPlayID)
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
                MessageBox.Show(this, "Error starting OMAPI (" + ex.Message + ")\r\n\r\nCheck the required .dll files are present the correct versions:\r\n\r\nOmApiNet.dll\r\nlibomapi.dll\r\n", "OMAPI Startup Failed", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
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
            refreshTimer.Enabled = true;

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

        private ListViewColumnSorter lwColumnSorterFiles = new ListViewColumnSorter();
        private ListViewColumnSorter lwColumnSorterQueue = new ListViewColumnSorter();
        private ListViewColumnSorter lwColumnSorterOutput = new ListViewColumnSorter();

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
            string deviceSessionID = (source.SessionId == uint.MinValue) ? "-" : source.SessionId.ToString();
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

        int refreshIndex = 0;
        private void refreshTimer_Tick(object sender, EventArgs e)
        {
            if (refreshIndex >= devicesListView.Items.Count) { refreshIndex = 0; }
            if (refreshIndex < devicesListView.Items.Count)
            {
                ListViewItem item = devicesListView.Items[refreshIndex];
                OmSource device = (OmSource)item.Tag;
                if (device is OmDevice && ((OmDevice)device).Update())
                {
                    UpdateDevice(item);
                }
            }
            refreshIndex++;
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
            int numDevicesNotDownloaded = 0;
            List<string> fileNamesDownloaded = new List<string>();

            Dictionary<string, string> devicesAndErrors = new Dictionary<string, string>();

            foreach (ListViewItem i in devicesListView.SelectedItems)
            {
                OmDevice device = (OmDevice)i.Tag;
                OmSource source = (OmSource)device;
                string deviceText = string.Format("{0:00000}", source.DeviceId);

                if (device != null)
                {
                    if (!device.IsDownloading)
                    {
                        //Console.WriteLine(device.StartTime.Ticks + " - " + device.StopTime.Ticks + " - " + device.IsRecording);
                        if (device.IsRecording == OmDevice.RecordStatus.Stopped)
                        {
                            if (device.HasData)
                            {
                                bool ok = true;
                                string folder = GetPath(OmGui.Properties.Settings.Default.CurrentWorkingFolder);
                                System.IO.Directory.CreateDirectory(folder);
                                string prefix = folder + string.Format("{0:00000}_{1:0000000000}", device.DeviceId, device.SessionId);
                                string finalFilename = prefix + ".cwa";
                                string downloadFilename = finalFilename;

                                downloadFilename += ".part";
                                if (System.IO.File.Exists(downloadFilename))
                                {
                                    DialogResult dr = MessageBox.Show(this, "Download file already exists:\r\n\r\n    " + downloadFilename + "\r\n\r\nOverwrite existing file?", "Overwrite File?", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                                    if (dr != DialogResult.OK)
                                    {
                                        ok = false;

                                        //TS - Device not downloaded because user clicked cancel on overwrite.
                                        numDevicesNotDownloaded++;
                                    }
                                    else { System.IO.File.Delete(downloadFilename); }


                                }

                                if (ok && System.IO.File.Exists(finalFilename))
                                {
                                    DialogResult dr = MessageBox.Show(this, "File already exists:\r\n\r\n    " + downloadFilename + "\r\n\r\nOverwrite existing file?", "Overwrite File?", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                                    if (dr != DialogResult.OK)
                                    {
                                        ok = false;

                                        //TS - Device not downloaded because user clicked cancel on overwrite.
                                        numDevicesNotDownloaded++;
                                    }
                                    else { System.IO.File.Delete(finalFilename); }
                                }

                                //Clicked OK and want to download.
                                if (ok)
                                {
                                    device.BeginDownloading(downloadFilename, finalFilename);

                                    //TS - Device downloaded because user clicked OK.
                                    numDevicesDownloaded++;
                                    fileNamesDownloaded.Add(finalFilename);

                                    devicesAndErrors.Add(deviceText, "Device downloaded");
                                }
                            }
                            else
                            {
                                //Has no data
                                devicesAndErrors.Add(deviceText, "device has no data");
                            }
                        }
                        else
                        {
                            //is still recording
                            devicesAndErrors.Add(deviceText, "device is recording");
                        }
                    }
                    else
                    {
                        //is downloading
                        devicesAndErrors.Add(deviceText, "device is downloading");
                    }
                }
                else
                {
                    //Couldn't download so hasn't downloaded
                    //Device null.
                    //numDevicesNotDownloaded++;
                    devicesAndErrors.Add(deviceText, "Unknown error");
                }
            }

            string message = numDevicesDownloaded + " devices downloaded:\r\n";

            foreach (KeyValuePair<string, string> kvp in devicesAndErrors)
            {
                message += "\r\nDevice: " + kvp.Key + " - Status: " + kvp.Value;
            }

            MessageBox.Show(message, "Download Status", MessageBoxButtons.OK);

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
                OmGui.Properties.Settings.Default.DefaultWorkingFolder = optionsDialog.DefaultFolderName;
                OmGui.Properties.Settings.Default.CurrentPluginFolder = optionsDialog.DefaultPluginName;

                Console.WriteLine(Properties.Settings.Default.DefaultWorkingFolder);
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
            int devicesCancelling = 0;

            Cursor.Current = Cursors.WaitCursor;

            foreach (ListViewItem i in devicesListView.SelectedItems)
            {
                OmSource device = (OmSource)i.Tag;
                if (device is OmDevice && ((OmDevice)device).IsDownloading)
                {
                    ((OmDevice)device).CancelDownload();

                    devicesCancelling++;
                }
            }

            Cursor.Current = Cursors.Default;

            //TS - Show messagebox as there are multiple.
            if (devicesListView.SelectedItems.Count > 1)
            {
                MessageBox.Show(devicesCancelling + " devices cancelled downloading from a selection of " + devicesListView.SelectedItems.Count + " devices.", "Cancel Status", MessageBoxButtons.OK);
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
            //TS - Record which devices stopped recording out of the selected.
            int devicesStoppedRecording = 0;

            //TS - [P] - Removed this check as can now just give prompt box.
            //if (EnsureNoSelectedDownloading())
            //{
            List<string> fails = new List<string>();
            dataViewer.CancelPreview();
            Cursor.Current = Cursors.WaitCursor;
            foreach (ListViewItem i in devicesListView.SelectedItems)
            {
                OmSource device = (OmSource)i.Tag;
                if (device is OmDevice && !((OmDevice)device).IsDownloading && ((OmDevice)device).IsRecording != OmDevice.RecordStatus.Stopped)
                {
                    if (!((OmDevice)device).NeverRecord())
                    {
                        fails.Add(device.DeviceId.ToString());
                    }
                    else
                    {
                        //TS - Record device stopped recording.
                        devicesStoppedRecording++;
                    }
                }
            }
            Cursor.Current = Cursors.Default;
            if (fails.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
            //}

            //TS - If multiple devices then show which stopped and which didn't.
            if (devicesListView.SelectedItems.Count > 1)
            {
                MessageBox.Show(devicesStoppedRecording + " devices stopped recording from a selection of " + devicesListView.SelectedItems.Count + " devices.", "Stop Record Status", MessageBoxButtons.OK);
            }

            devicesListViewUpdateEnabled();
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
                DateTime start = DateTime.MaxValue;
                DateTime stop = DateTime.MaxValue;

                List<string> fails = new List<string>();
                List<string> fails2 = new List<string>();

                dataViewer.CancelPreview();
                Cursor.Current = Cursors.WaitCursor;

                foreach (OmDevice device in devices)
                {
                    if (device is OmDevice && !((OmDevice)device).IsDownloading)
                    {
                        if (rangeForm.Always)
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
                    }

                    //Sampling Freq and Range
                    OmApi.OmSetAccelConfig(device.DeviceId, (int)rangeForm.SamplingFrequency, rangeForm.Range);

                    //Do Sync Time
                    if (rangeForm.SyncTime != DateRangeForm.SyncTimeType.None)
                    {
                        Cursor.Current = Cursors.WaitCursor;
                        //foreach (ListViewItem i in devicesListView.SelectedItems)
                        //{
                        //    OmSource device = (OmSource)i.Tag;
                        if (device is OmDevice && ((OmDevice)device).Connected)
                        {
                            if (!((OmDevice)device).SyncTime((int)rangeForm.SyncTime))
                                fails2.Add(device.DeviceId.ToString());
                        }
                    }
                }

                if (fails2.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails2.Count + " device(s):\r\n" + string.Join("; ", fails2.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }

                Cursor.Current = Cursors.Default;
            }
        }

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

        private void toolStripButtonClear_Click(object sender, EventArgs e)
        {
            //TS - Record which are cleared and stopped if multiple selected.
            //            int devicesClearing = 0;

            if (EnsureNoSelectedDownloading())
            {
                DialogResult dr = MessageBox.Show(this, "Clear " + devicesListView.SelectedItems.Count + " device(s)?", Text, MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
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
                            clearBackgroundWorker.ReportProgress(-1, "Clearing device " + (i + 1) + " of " + devices.Count + ".");
                            if (!((OmDevice)device).Clear())
                                fails.Add(device.DeviceId.ToString());
                            i++;
                        }
                        clearBackgroundWorker.ReportProgress(100, "Done");
                        if (fails.Count > 0)
                        {
                            this.Invoke(new Action(() =>
                                MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1)
                            ));
                        }
                    };

                    ProgressBox progressBox = new ProgressBox("Clearing", "Clearing devices...", clearBackgroundWorker);
                    progressBox.ShowDialog(this);

                    //Cursor.Current = Cursors.Default;
                }
            }
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

            LoadWorkingFolder(false);
        }

        private void LoadWorkingFolder(bool defaultFolder)
        {
            //If first time then change to my documents
            if (Properties.Settings.Default.DefaultWorkingFolder.Equals("C:\\"))
            {
                Properties.Settings.Default.DefaultWorkingFolder = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            }

            if (defaultFolder)
                Properties.Settings.Default.CurrentWorkingFolder = Properties.Settings.Default.DefaultWorkingFolder;

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

            //If there is no default folder then make it My Documents
            if (Properties.Settings.Default.DefaultWorkingFolder == "")
                Properties.Settings.Default.DefaultWorkingFolder = GetPath("{MyDocuments}");

            if (Properties.Settings.Default.CurrentWorkingFolder != "" && Properties.Settings.Default.CurrentWorkingFolder != Properties.Settings.Default.DefaultWorkingFolder)
            {
                DialogResult dr = MessageBox.Show("Do you want to load the last opened Working Folder?\r\n\r\nLast Folder:\r\n" + Properties.Settings.Default.CurrentWorkingFolder, "Open Last Working Folder?", MessageBoxButtons.YesNo, MessageBoxIcon.Question);

                if (dr == System.Windows.Forms.DialogResult.Yes)
                {
                    LoadWorkingFolder(false);
                }
                else
                {
                    LoadWorkingFolder(true);
                }
            }
            else
            {
                LoadWorkingFolder(true);
            }

            Console.WriteLine("Default: " + Properties.Settings.Default.DefaultWorkingFolder);
            Console.WriteLine("Current: " + Properties.Settings.Default.CurrentWorkingFolder);
            Console.WriteLine("Current Plugin: " + Properties.Settings.Default.CurrentPluginFolder);

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
                    OmReader reader = (OmReader)filesListView.SelectedItems[0].Tag;

                    string CWAFilename = reader.Filename;
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

                    RunPluginForm rpf = new RunPluginForm(p, CWAFilename);
                    rpf.ShowDialog();

                    if (rpf.DialogResult == System.Windows.Forms.DialogResult.OK)
                    {
                        //if the plugin has an output file    
                        RunProcess2(rpf.ParameterString, p, rpf.OriginalOutputName, reader.Filename);

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
            //TS - If project exists and isn't the default then ask if its properties want to be saved to be automatically opened next time.
            if (Properties.Settings.Default.CurrentWorkingFolder != "" && Properties.Settings.Default.CurrentWorkingFolder != Properties.Settings.Default.DefaultWorkingFolder)
            {
                DialogResult dr = MessageBox.Show("The current working folder isn't the default.\r\n\r\nLoad up into this Working Folder next time?", "Keep Working Folder Properties?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1);
                if (dr == System.Windows.Forms.DialogResult.Yes)
                {
                    //Save properties for next opening.
                    Properties.Settings.Default.Save();
                }
                else if (dr == System.Windows.Forms.DialogResult.No)
                {
                    Properties.Settings.Default.Reset();
                }
                else
                {
                    e.Cancel = true;
                    return;
                }
            }
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

        List<OmDevice> identifyDevices;
        private void devicesToolStripButtonIdentify_Click(object sender, EventArgs e)
        {
            identifyDevices = new List<OmDevice>();

            foreach (ListViewItem item in devicesListView.SelectedItems)
            {
                OmDevice device = (OmDevice)item.Tag;

                device.SetLed(OmApi.OM_LED_STATE.OM_LED_BLUE);

                identifyDevices.Add(device);
            }

            if (devicesListView.SelectedItems.Count > 0)
            {
                //Start timer to turn off led.
                identifyTimer.Enabled = true;
            }
        }

        bool turnOn = true;
        int ticks = 0;
        private void identifyTimer_Tick(object sender, EventArgs e)
        {
            if (ticks <= 10)
            {
                //Turn off lights
                foreach (OmDevice device in identifyDevices)
                {
                    if (turnOn)
                    {
                        device.SetLed(OmApi.OM_LED_STATE.OM_LED_BLUE);
                        turnOn = false;
                    }
                    else
                    {
                        device.SetLed(OmApi.OM_LED_STATE.OM_LED_OFF);
                        turnOn = true;
                    }
                }

                ticks++;
            }
            else
            {
                foreach (OmDevice device in identifyDevices)
                {
                    device.SetLed(OmApi.OM_LED_STATE.OM_LED_AUTO);
                }
                ticks = 0;
                turnOn = true;
                identifyTimer.Enabled = false;
            }
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
                if (pluginManager.Plugins.Count == 0)
                    pluginManager.LoadPlugins();
                if (pluginManager.Plugins.Count > 0)
                {
                    OmReader reader = (OmReader)listItems[0].Tag;

                    string CWAFilename = reader.Filename;

                    float blockStart = -1;
                    float blockCount = -1;
                    float blockEnd = -1;

                    DateTime startDateTime;
                    DateTime endDateTime;

                    string startDateTimeString = "";
                    string endDateTimeString = "";

                    if (dataViewer.HasSelection)
                    {
                        blockStart = dataViewer.SelectionBeginBlock + dataViewer.OffsetBlocks;
                        blockCount = dataViewer.SelectionEndBlock - dataViewer.SelectionBeginBlock;
                        blockEnd = blockStart + blockCount;

                        startDateTime = dataViewer.TimeForBlock(blockStart);
                        endDateTime = dataViewer.TimeForBlock(blockEnd);

                        startDateTimeString = startDateTime.ToString("dd/MM/yyyy/_HH:mm:ss");
                        endDateTimeString = endDateTime.ToString("dd/MM/yyyy/_HH:mm:ss");
                    }

                    PluginsForm pluginsForm = new PluginsForm(pluginManager, CWAFilename, blockStart, blockCount, startDateTimeString, endDateTimeString);

                    pluginsForm.ShowDialog();

                    if (pluginsForm.DialogResult == System.Windows.Forms.DialogResult.OK)
                    {
                        //if the plugin has an output file    
                        RunProcess2(pluginsForm.rpf.ParameterString, pluginsForm.SelectedPlugin, pluginsForm.rpf.OriginalOutputName, CWAFilename);

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

        private void RunProcess2(string parameterString, Plugin p, string outputName, string inputName)
        {
            ProcessStartInfo psi = new ProcessStartInfo();

            psi.FileName = p.FilePath + Path.DirectorySeparatorChar + p.RunFilePath;

            //TODO - Idea to try and do the outfile as a filepath rather than filename but it didn't like it.
            //Find arguments and replace the output file with the full file path.
            parameterString = parameterString.Replace(outputName, "\"" + Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + outputName + "\"");

            psi.Arguments = parameterString;

            Console.WriteLine("ARguments: " + parameterString);

            psi.UseShellExecute = true;
            psi.RedirectStandardError = false;
            psi.RedirectStandardOutput = false;

            psi.Verb = "runas";
            psi.CreateNoWindow = true;

            PluginQueueItem pqi = new PluginQueueItem(p, parameterString, inputName);
            pqi.StartInfo = psi;
            pqi.OriginalOutputName = outputName;

            //Add PQI to file queue
            ListViewItem lvi = new ListViewItem(new string[] { pqi.Name, pqi.File, "0" });

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
                //string lastLine = p.StandardOutput.ReadLine();
                //parseMessage(lastLine);

                /*while (!p.HasExited)
                {
                    string outputLine = p.StandardOutput.ReadLine();
                    int progress = -1;

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

                try
                {
                    File.Move(Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + pqi.OriginalOutputName, Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + pqi.OriginalOutputName + "a");
                    File.Move(Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + pqi.OriginalOutputName + "a", Properties.Settings.Default.CurrentWorkingFolder + Path.DirectorySeparatorChar + pqi.OriginalOutputName);
                }
                catch (Exception ue)
                {
                    MessageBox.Show("Plugin Error: Unknown plugin error: " + ue.Message, "Plugin Error", MessageBoxButtons.OK);
                }

                e.Result = "done";
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
                    MessageBox.Show("Plugin Error: " + w32e.Message, "Plugin Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
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

        private void parseMessage(string outputLine, out int progress)
        {
            progress = -1;
            //OUTPUT
            if (outputLine != null && outputLine.Length > 1)
            {
                if (outputLine[0] == 'p')
                {
                    string percentage = outputLine.Split(' ').ElementAt(1);

                    progress = Int32.Parse(percentage);
                    //runPluginProgressBar.Value = Int32.Parse(percentage);
                }
                else if (outputLine[0] == 's')
                {
                    string message = outputLine.Split(new char[] { ' ' }, 2).Last();
                    //labelStatus.Text = message;
                }
            }
            Console.WriteLine("o: " + outputLine);
        }

        #endregion

        //TS - This will become a plugin so keep the code here for now just so we can see the Form.
        //ExportDataConstruct(OmSource.SourceCategory.File);

        private void gotoDefaultFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Properties.Settings.Default.CurrentWorkingFolder = Properties.Settings.Default.DefaultWorkingFolder;

            LoadWorkingFolder(true);
        }

        private void setDefaultWorkingFolderToCurrentWorkingFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Properties.Settings.Default.DefaultWorkingFolder = Properties.Settings.Default.CurrentWorkingFolder;
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
            foreach (ListViewItem lvi in queueListViewItems2.SelectedItems)
            {
                if (lvi.SubItems[2].Text.Equals("Complete"))
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
    }
}
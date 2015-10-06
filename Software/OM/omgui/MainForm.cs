// TODO

// TODO: Use queue (what about errors)
// TODO: Merged view of files/queue/outputs (remove queue and outputs)
// TODO: Consider a preview for .WAV (wavs) and .CSV (grid, graph) files
// TODO: Time-span (from selection), at least for .WAV export


/*
Make .WAV:
 * Form compatible with multiple source files
 * - Interpolated sample rate (100Hz, 50Hz, 25Hz)
 * - No auto-calibrate
 * (note: derived summaries can be affected by the sample rate)

Make .SVM.CSV
 * Epoch (60 seconds)
 * Filter (off, 0.5-20Hz)
 * Mode: abs(sqrt(svm-1)) vs max(0,sqrt(svm-1))

Make .WTV.CSV
 * Epochs (number of 0.5 minute periods)

Make .CUT.CSV
 * Epochs (number of 1 minute periods)
 * Model (dominant/"right" hand, non-dominant/"left" hand, weight)

 */

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using OmApiNet;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.IO;
using System.Net;
using System.Threading;

using AndreasJohansson.Win32.Shell;
using System.Collections.Specialized;

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
        public int resetIfUnresponsive = 3;

        //PluginQueue
        Queue<PluginQueueItem> pluginQueueItems = new Queue<PluginQueueItem>();

        protected override void WndProc(ref Message msg)
        {
            base.WndProc(ref msg);
            if (msg.Msg == MainForm.queryCancelAutoPlayID && MainForm.queryCancelAutoPlayID != 0)
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

        private string configDumpFile = null;
        private string downloadDumpFile = null;
        private bool noUpdateCheck = false;
        private string startupPath = null;
        public MainForm(int uac, string myConfigDumpFile, string myDownloadDumpFile, bool noUpdateCheck, string startupPath, int resetIfUnresponsive)
        {
            this.configDumpFile = myConfigDumpFile;
            downloadDumpFile = myDownloadDumpFile;
            this.startupPath = startupPath;
            this.resetIfUnresponsive = resetIfUnresponsive;

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

        // /*OmDevice*/ public delegate void OmDeviceDownloadCompleteCallback(ushort id, OmApi.OM_DOWNLOAD_STATUS status, string filename);
        public void DownloadCompleteCallback(ushort id, OmApi.OM_DOWNLOAD_STATUS status, string filename)
        {
            string timeNow, type;

            timeNow = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
            type = "DOWNLOAD-OK";

            string logLine = "" + timeNow + "," + type + "," + Path.GetFileName(filename);

            if (downloadDumpFile != null)
            {
                for (; ; )  // [dump]
                {
                    string errorMessage = "Problem while appending to download log file (" + downloadDumpFile + ") - check the folder exists, you have write permission, and the file is not locked open by another process.";

                    try
                    {
                        StreamWriter sw = File.AppendText(downloadDumpFile);
                        sw.WriteLine(logLine);
                        sw.Close();
                        break;
                    }
                    catch (Exception ex)
                    {
                        errorMessage = errorMessage + "\r\n\r\nDetails: " + ex.Message + "";
                        Console.WriteLine("Warning: " + errorMessage);
                    }

                    DialogResult ddr = MessageBox.Show(null, errorMessage, "Warning", MessageBoxButtons.RetryCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                    if (ddr != System.Windows.Forms.DialogResult.Retry) { break; }
                }
            }
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
                
                bool hasData = false;

                if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_CANCELLED) { download = "Cancelled"; }
                else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_COMPLETE) { download = "Complete"; }
                else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_ERROR) { download = string.Format("Error (0x{0:X})", device.DownloadValue); }
                else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS) { download = "" + device.DownloadValue + "%"; hasData = true; }
                //else if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_NONE) { download = "-"; }

                if (device.StartTime >= device.StopTime) { recording = "Stopped"; }
                else if (device.StartTime == DateTime.MinValue && device.StopTime == DateTime.MaxValue) { recording = "Always"; }
                else
                {
                    recording = string.Format("Interval {0:dd/MM/yy HH:mm:ss}-{1:dd/MM/yy HH:mm:ss}", device.StartTime, device.StopTime);
                }

                if (!hasData) { hasData = device.HasData; }
                if (hasData)
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

Console.WriteLine("START: deviceManager_DeviceUpdate()...");

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
Console.WriteLine("END: deviceManager_DeviceUpdate()...");


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
                listViewDevices.Remove(device.DeviceId);
                devicesListView.Items.Remove(item);

                // Force dataViewer to reset
                listViewDevices_SelectedIndexChanged(null, null);
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
Console.WriteLine("toolStripButtonDownload_Click() STARTED...");
            dataViewer.CancelPreview();
    
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
                    string filename = OmGui.Properties.Settings.Default.FilenameTemplate;
                    if (filename == null || filename.Length == 0) { filename = "{DeviceId}_{SessionId}"; }

                    string folder = GetPath(OmGui.Properties.Settings.Default.CurrentWorkingFolder);
                    System.IO.Directory.CreateDirectory(folder);

                    Dictionary<string, string> metadataMap = new Dictionary<string, string>();
                    if (true)
                    {
                        try
                        {
                            //metadataMap = MetaDataTools.MetadataFromReader(device.Filename);
                            metadataMap = MetaDataTools.MetadataFromFile(device.Filename);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("ERROR: " + ex.Message);
                        }
                    }

                    string deviceIdString = string.Format("{0:00000}", device.DeviceId);
                    metadataMap.TryGetValue("DeviceId", out deviceIdString);

                    string sessionIdString = string.Format("{0:0000000000}", device.SessionId);
                    metadataMap.TryGetValue("SessionId", out sessionIdString);

                    filename = filename.Replace("{DeviceId}", deviceIdString);
                    filename = filename.Replace("{SessionId}", sessionIdString);
                    string[] keys = new string[] { "StudyCentre", "StudyCode", "StudyInvestigator",  "StudyExerciseType", "StudyOperator", "StudyNotes", "SubjectSite", "SubjectCode", "SubjectSex", "SubjectHeight", "SubjectWidth", "SubjectHandedness" };
                    foreach (string key in keys)
                    {
                        filename = filename.Replace("{" + key + "}", metadataMap.ContainsKey(key) ? metadataMap[key] : "");
                    }

                    // Remove forbidden filename characters
                    StringBuilder sb = new StringBuilder();
                    for (int j = 0; j < filename.Length; j++)
                    {
                        char c = filename[j];
                        //if (c < 32 || c >= 127 || @"""<>:/\|?* ".IndexOf(c) >= 0) { c = '_'; }                                                            // blacklist
                        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_') { ; } else { c = '_'; }     // whitelist
                        sb.Append(c);
                    }
                    filename = sb.ToString();
                    if (filename.Length == 0) { filename = "-"; }

                    string prefix = folder + filename;
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
                        device.downloadComplete = DownloadCompleteCallback;
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
Console.WriteLine("toolStripButtonDownload_Click() ENDED...");
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
                int numRecording = 0;       // Devices not stopped/after-interval
                int numDownloading = 0;     // Devices background downloading
                int numDataAndStoppedOrNoDataAndConfigured = 0;  // Devices with some and stopped, or no data but set to record

                DateTime now = DateTime.Now;

                foreach (ListViewItem item in devicesListView.SelectedItems)
                {
                    OmDevice device = (OmDevice)item.Tag;
                    bool hasData;

                    numTotal++;

                    //DOWNLOADING
                    //If it is downloading then set downloading.
                    if (device.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS)
                    {
                        numDownloading++;
                        hasData = true;
                    }
                    else 
                    {
                        hasData = device.HasData;
                    }

                    if (hasData)
                    {
                        //HASDATA
                        //If it has data then some have data.
                        numData++;
                    }

                    //ALL RECORDING
                    if (device.StartTime < device.StopTime && device.StopTime >= now)
                    {
                        numRecording++;
                    }

                    //SETUP FOR RECORD
                    //if (device.StartTime > DateTime.Now && (device.StartTime != DateTime.MaxValue))
                    //{
                    //    numFutureRecording++;
                    //}

                    if ((hasData && (device.StartTime >= device.StopTime || device.StopTime < now)) || (!hasData && device.StartTime < device.StopTime))
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
            if (selected.Length == 1 && selected[0] is OmDevice && ((OmDevice)selected[0]).Connected && !((OmDevice)selected[0]).IsDownloading)
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
            stopBackgroundWorker.RunWorkerCompleted += (s, ea) =>
            {
                int[] selected = new int[devicesListView.SelectedIndices.Count];
                devicesListView.SelectedIndices.CopyTo(selected, 0);
                devicesListView.SelectedIndices.Clear();
                devicesListViewUpdateEnabled();
                foreach (int i in selected)
                {
                    devicesListView.SelectedIndices.Add(i);
                }
            };

            ShowProgressWithBackground("Stopping", "Stopping devices...", stopBackgroundWorker);
            
            devicesListViewUpdateEnabled();

            Cursor.Current = Cursors.Default;
        }


        bool CheckFirmware(OmDevice[] devices)
        {
            // Read bootload information from file
            string bootloadExecutable = null;
            string latestVersion = "";
            IDictionary<string, string> blacklist = new Dictionary<string, string>(); // Black-list of version numbers

            // Find bootload information file
            string bootloadInformation = @"firmware\bootload.ini";
            // Search current directory first
            if (!File.Exists(bootloadInformation))
            {
                // Search under executable folder
                bootloadInformation = Path.GetDirectoryName(Application.ExecutablePath) + Path.DirectorySeparatorChar + bootloadInformation;
            }
            if (!File.Exists(bootloadInformation))
            {
                Trace.WriteLine("Firmware information file not found " + bootloadInformation + "");
                return false;
            }

            // Read bootload information
            string[] lines = new string[0];
            try
            {
                lines = File.ReadAllLines(bootloadInformation);
            }
            catch (Exception e)
            {
                Trace.WriteLine("Problem reading firmware information file " + bootloadInformation + " (" + e.Message + ").");
                return false;
            }

            // Rough .INI parser
            string section = "";
            foreach (string rawLine in lines)
            {
                string line = rawLine.Trim();
                if (line.Length == 0 || line.StartsWith(";") || line.StartsWith("#")) { continue; }                             // Empty lines and comments
                if (line.StartsWith("[")  && line.EndsWith("]")) { section = line.Substring(1, line.Length - 2); continue; }    // Section
                string[] parts = line.Split(new char[] {'=', ':'}, 2);
                if (parts.Length <= 0) { continue; }
                string name = parts[0].Trim();
                string value = (parts.Length > 1) ? parts[1].Trim() : null;

                if (section == "upgrade")
                {
                    blacklist.Add(name, value);
                }
                else if (section == "bootload")
                {
                    if (name == "executable") { bootloadExecutable = value; }       // @"firmware\CWA17_44.cmd"
                    else if (name == "version") { latestVersion = value; }          // @"CWA17_44"
                    
                }
            }

            // Check firmware version
            foreach (OmDevice device in devices)
            {
                string currentFirmware = "CWA17_" + device.FirmwareVersion + "";
                if (blacklist.ContainsKey(currentFirmware))
                {
                    string reason = blacklist[currentFirmware];
                    DialogResult drUpdate = MessageBox.Show(this, "Device " + device.DeviceId + " is running firmware version " + currentFirmware + ".\r\n\r\n" + reason + "\r\n\r\nUpdate the device firmware to " + latestVersion + " now?", "Firmware Update Receommended", MessageBoxButtons.YesNo, MessageBoxIcon.Information);
                    Console.WriteLine("FIRMWARE: Device " + device.DeviceId + " is running firmware version " + currentFirmware + " - update to version " + latestVersion + " (Reason: " + reason + ").");
                    if (drUpdate == System.Windows.Forms.DialogResult.Yes)
                    {
                        DialogResult okUpdate = MessageBox.Show(this, "Important:\r\n\r\n* Do not disconnect device " + device.DeviceId + ".\r\n\r\n* Do not connect any new devices.\r\n\r\n* Check no devices are currently flashing red.\r\n\r\nContinue?", "Firmware Update Warnings", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning);

                        if (okUpdate == System.Windows.Forms.DialogResult.OK)
                        {
                            if (device.IsDownloading)
                            {
                                MessageBox.Show(this, "Device is busy downloading data.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                return true;
                            }

                            Cursor.Current = Cursors.WaitCursor;
                            dataViewer.CancelPreview();

                            Process updateProcess = new Process();
                            BackgroundWorker updateBackgroundWorker = new BackgroundWorker();
                            updateBackgroundWorker.WorkerReportsProgress = true;
                            updateBackgroundWorker.WorkerSupportsCancellation = false;
                            updateBackgroundWorker.DoWork += (s, ea) =>
                            {
                                updateBackgroundWorker.ReportProgress(-1, "Running bootloader...");

                                // Convert the file
                                ProcessStartInfo processInformation = new ProcessStartInfo();

                                processInformation.FileName = bootloadExecutable;

                                List<string> args = new List<string>();
                                //args.Add("\"" + param + "\"");

                                // Construct arguments
                                processInformation.Arguments = string.Join(" ", args.ToArray());
                                processInformation.UseShellExecute = false;

                                // ???
                                bool redirect = true;

                                processInformation.RedirectStandardError = redirect;
                                processInformation.RedirectStandardOutput = true;
                                processInformation.CreateNoWindow = (processInformation.RedirectStandardError || processInformation.RedirectStandardOutput) ? true : false;
                                updateProcess.EnableRaisingEvents = true;
                                updateProcess.StartInfo = processInformation;

                                try
                                {
                                    updateProcess.Start();
                                }
                                catch (Exception ex)
                                {
                                    updateBackgroundWorker.ReportProgress(100, "Problem running bootloader (" + ex.Message + ").");
                                    MessageBox.Show(null, "Problem running bootloader (" + ex.Message + ").", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    return;
                                }


                                updateBackgroundWorker.ReportProgress(-1, "Resetting device...");
                                if (device.Reset() != 0)
                                {
                                    updateBackgroundWorker.ReportProgress(100, "Problem resetting device.");
                                    updateProcess.StandardInput.Write((char)27);
                                    try { updateProcess.Kill(); } catch { ; }
                                    try { updateProcess.WaitForExit(); } catch { ; }
                                    updateProcess = null;
                                    MessageBox.Show(null, "Problem resetting device.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    return;
                                }


                                if (redirect)
                                {
                                    for (; ; )
                                    {
                                        //if (updateProcess.StandardError.EndOfStream) { break; }
                                        string line = updateProcess.StandardError.ReadLine();
                                        if (line == null) { break; }
                                        Trace.WriteLine("BOOTLOADER: " + line);
                                        updateBackgroundWorker.ReportProgress(-1, "BOOTLOADER: " + line);
                                    }
                                }
                                updateBackgroundWorker.ReportProgress(-1, "Waiting for bootloader to complete...");
                                updateProcess.WaitForExit();

                                int exitCode = updateProcess.ExitCode;
                                updateProcess = null;
                                if (exitCode != 0)
                                {
                                    updateBackgroundWorker.ReportProgress(100, "Problem running bootloader (exit code " + exitCode + ")");
                                    MessageBox.Show(null, "Problem running bootloader (exit code " + exitCode + ").", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    return;
                                }

                                updateBackgroundWorker.ReportProgress(100, "Done - wait for device to restart...");
                                Thread.Sleep(2000);
                            };
                            updateBackgroundWorker.RunWorkerCompleted += (s, ea) =>
                            {
                                //devicesListViewUpdateEnabled();
                                if (updateProcess != null)
                                {
                                    try { updateProcess.Kill(); } catch { ; }
                                    try { updateProcess.WaitForExit(); } catch { ; }
                                    updateProcess = null;
                                }
                            };


                            ShowProgressWithBackground("Updating", "Updating device...", updateBackgroundWorker);

                            Cursor.Current = Cursors.Default;
                        }

                        // Let's not do anything else (the user can just press 'clear' or 'record' again)
                        return true;
                    }
                }
                else
                {
                    Console.WriteLine("FIRMWARE: Device " + device.DeviceId + " is running firmware version " + currentFirmware + " (not required to update to version " + latestVersion + ").");
                }
            }
            return false;
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

            // Check none are downloading
            if (!EnsureNoSelectedDownloading())
            {
                return;
            }

            // Check and prompt for firmware updates
            if (CheckFirmware(devices))
            {
                // Don't do anything else now (user can press button again)
                return;
            }

            DateRangeForm rangeForm = new DateRangeForm("Recording Settings", devices);
            DialogResult dr = rangeForm.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                Cursor.Current = Cursors.WaitCursor;

                //DateTime start = DateTime.MinValue;
                //DateTime stop = DateTime.MaxValue;

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

                       try
                       {
                           if (device is OmDevice && !((OmDevice)device).IsDownloading)
                           {
                               string devicePath = null;
                               string dataFile = ((OmDevice)device).Filename;
                               if (dataFile != null && dataFile.Length > 0)
                               {
                                    devicePath = Path.GetDirectoryName(dataFile);
                               }

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
                               if (error == null)
                               {
                                   //Cursor.Current = Cursors.WaitCursor;
                                   //foreach (ListViewItem i in devicesListView.SelectedItems)
                                   //{
                                   //    OmSource device = (OmSource)i.Tag;
                                   if (device is OmDevice && ((OmDevice)device).Connected)
                                   {
                                       if (!((OmDevice)device).SyncTime())
                                           error = "Time sync. failed";
                                   }
                               }

                               if (device is OmDevice && ((OmDevice)device).Connected)
                               {
                                   ((OmDevice)device).SetDebug(rangeForm.Flash ? 3 : 0);
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
                                       DateTime start = rangeForm.StartDate;
                                       DateTime stop = rangeForm.EndDate;

                                       if (!((OmDevice)device).SetInterval(start, stop))
                                           error = "Set interval failed";
                                   }
                               }


                               // Check unpacked
                               if (rangeForm.Unpacked)
                               {
                                   recordBackgroundWorker.ReportProgress((100 * (5 * i + 4) / (devices.Length * 5)), message + "(unpacked setting)");
                                   if (devicePath != null && devicePath.Length > 0)
                                   {
                                       // Wait to allow drive to re-mount
                                       System.Threading.Thread.Sleep(800);

                                       bool success = false;
                                       for (int retries = 0; retries < 15 * 4; retries++)
                                       {
                                           try
                                           {
                                               if (Directory.Exists(devicePath))
                                               {
                                                   string configFile = Path.Combine(devicePath, "SETTINGS.INI");
                                                   string config = "DATAMODE=20\r\n";
                                                   File.WriteAllText(configFile, config, Encoding.ASCII);
                                                   success = true;
                                               }
                                               if (success) { break; }
                                               System.Threading.Thread.Sleep(250);
                                           }
                                           catch (Exception ex)
                                           {
                                               Console.Error.WriteLine("ERROR: Problem writing unpacked configuration: " + ex.Message);
                                           }
                                       }
                                       if (!success)
                                       {
                                           error = "Failed to write unpacked configuration file.";
                                       }
                                   }
                                   else
                                   {
                                       error = "Failed to find drive to write configuration file.";
                                   }

                               }
                           }
                           else
                           {
                               error = "Device is downloading";
                           }
                        }
                        catch (Exception ex)
                        {
                            error = "Exception: " + ex.Message;
                        }
                        
                        if (error != null) 
                        { 
                            fails.Add(device.DeviceId.ToString(), error); 
                        }

                        // Logging
                        {
                            string timeNow, type, start, stop;

                            timeNow = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");

                            if (error == null)
                            {
                                type = "AX3-CONFIG-OK";
                            }
                            else
                            {
                                type = "AX3-CONFIG-ERROR";
                            }

                            if (rangeForm.Always)
                            {
                                start = "0";
                                stop = "-1";
                            }
                            else
                            {
                                start = rangeForm.StartDate.ToString("yyyy-MM-dd HH:mm:ss");
                                stop = rangeForm.EndDate.ToString("yyyy-MM-dd HH:mm:ss");
                            }

                            string logLine = "" + timeNow + "," + type + "," + device.DeviceId + "," + rangeForm.SessionID + "," + start + "," + stop + "," + (int)rangeForm.SamplingFrequency + "," + rangeForm.Range + "," + ((rangeForm.metaData != null && rangeForm.metaData.Length > 0) ? ("\"" + rangeForm.metaData.Replace("\"", "\"\"") + "\"") : "");

                            if (configDumpFile != null)
                            {
                                for (; ; )  // [dump]
                                {
                                    string errorMessage = "Problem while appending to config log file (" + configDumpFile + ") - check the folder exists, you have write permission, and the file is not locked open by another process.";

                                    try
                                    {
                                        StreamWriter sw = File.AppendText(configDumpFile);
                                        sw.WriteLine(logLine);
                                        sw.Close();
                                        break;
                                    }
                                    catch (Exception ex)
                                    {
                                        errorMessage = errorMessage + "\r\n\r\nDetails: " + ex.Message + "";
                                        Console.WriteLine("Warning: " + errorMessage);
                                    }

                                    DialogResult ddr = MessageBox.Show(null, errorMessage, "Warning", MessageBoxButtons.RetryCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                                    if (ddr != System.Windows.Forms.DialogResult.Retry) { break; }
                                }
                            }
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
                recordBackgroundWorker.RunWorkerCompleted += (s, ea) =>
                {
                    devicesListViewUpdateEnabled();
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
                List<OmDevice> devices = new List<OmDevice>();
                foreach (ListViewItem i in devicesListView.SelectedItems)
                {
                    OmSource device = (OmSource)i.Tag;
                    if (device is OmDevice && !((OmDevice)device).IsDownloading)
                    {
                        devices.Add((OmDevice)device);
                    }
                }

                // Check and prompt for firmware updates
                if (CheckFirmware(devices.ToArray()))
                {
                    // Don't do anything else now (user can press button again)
                    return;
                }


                DialogResult dr = MessageBox.Show(this, (wipe ? "Wipe" : "Clear") + " " + devicesListView.SelectedItems.Count + " device(s)?", Text, MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                if (dr == System.Windows.Forms.DialogResult.OK)
                {
                    //Cursor.Current = Cursors.WaitCursor;

                    dataViewer.CancelPreview();
                    dataViewer.Reader = null;
                    dataViewer.Close();

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
                    clearBackgroundWorker.RunWorkerCompleted += (s, ea) =>
                    {
                        devicesListViewUpdateEnabled();
                    };

                    ShowProgressWithBackground((wipe ? "Wiping" : "Clearing"), (wipe ? "Wiping" : "Clearing") + " devices...", clearBackgroundWorker);

                    //Cursor.Current = Cursors.Default;
                }
            }

            // Force dataViewer to reset
            listViewDevices_SelectedIndexChanged(null, null);
        }

        private void ShowProgressWithBackground(string title, string message, BackgroundWorker backgroundWorker)
        {
            BlockBackgroundTasks();

            ProgressBox progressBox = new ProgressBox(title, message, backgroundWorker);
            progressBox.ShowDialog(this);

            // Re-enable refreshes
            EnableBackgroundTasks();

        }


        private void ExportDataConstruct()
        {
            float blockStart = -1;
            float blockCount = -1;
            string blockDescription = null;

            // Selection
            if (dataViewer != null)
            {
                if (dataViewer.HasSelection)
                {
                    blockStart = dataViewer.SelectionBeginBlock + dataViewer.OffsetBlocks;
                    blockCount = dataViewer.SelectionEndBlock - dataViewer.SelectionBeginBlock;
                    blockDescription = dataViewer.SelectionDescription;
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
                ExportData(files[0], blockStart, blockCount, blockDescription);
            else
                ExportData(files, devicesSelected);
        }

        private void ExportData(string fileName, float blockStart, float blockCount, string blockDescription)
        {
            string folder = GetPath(Properties.Settings.Default.CurrentWorkingFolder);
            ExportForm exportForm = new ExportForm(fileName, folder, blockStart, blockCount, blockDescription);
            DialogResult result = exportForm.ShowDialog(this);
        }

        //Exporting many.
        private void ExportData(List<string> files, int numDevicesSelected)
        {
            string folder = GetPath(Properties.Settings.Default.CurrentWorkingFolder);

            ExportForm exportForm;

            foreach (string fileName in files)
            {
                exportForm = new ExportForm(fileName, folder, -1, -1, null);
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
        string defaultTitleText = "Open Movement " + " [V" + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString() + "]";

        private void ChangeWorkingFolder(string newPath)
        {
            Properties.Settings.Default.CurrentWorkingFolder = newPath;
            LoadWorkingFolder();

            //Add to recent folders.
            if (!Properties.Settings.Default.RecentFolders.Contains(newPath))
            {
                if (Properties.Settings.Default.RecentFolders.Count == 5)
                {
                    Properties.Settings.Default.RecentFolders.RemoveAt(4);
                }

                Properties.Settings.Default.RecentFolders.Insert(0, newPath);
            }
            else
            {
                //If we have it we should move it up the list so it is the most recent.
                Properties.Settings.Default.RecentFolders.Remove(newPath);
                Properties.Settings.Default.RecentFolders.Insert(0, newPath);
            }

            UpdateRecentFoldersInGUI();
        }

        private void workingFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            fbd.ShowNewFolderButton = true;
            fbd.SelectedPath = Properties.Settings.Default.CurrentWorkingFolder;
            fbd.Description = "Choose or Create New Working Folder";

            DialogResult dr = fbd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                string newPath = fbd.SelectedPath;
                ChangeWorkingFolder(newPath);
            }

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

        // Toolstrip handler for recentFoldersToolStripMenuItem
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

                toolStripDirectoryChooser.Text = folder;

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
            //fileSystemWatcher.Path = OmGui.Properties.Settings.Default.CurrentWorkingFolder;

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
                if (!Path.GetExtension(f).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase) && !Path.GetExtension(f).Equals(".part", StringComparison.CurrentCultureIgnoreCase) && !f.StartsWith(".") && (File.GetAttributes(f) & FileAttributes.Hidden) == 0)
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
            
            if (!Directory.Exists(Properties.Settings.Default.CurrentWorkingFolder))
            {
                MessageBox.Show("Could not find last working folder: " + Properties.Settings.Default.CurrentWorkingFolder + "\r\n\r\nDefaulting to: " + GetPath("{MyDocuments}"), "Error - Missing Working Folder", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                Properties.Settings.Default.CurrentWorkingFolder = GetPath("{MyDocuments}");
            }

            if (startupPath != null)
            {
                ChangeWorkingFolder(startupPath);
            }
            else
            {
                LoadWorkingFolder();
            }

            //Recent Folders
            if (Properties.Settings.Default.RecentFolders.Count == 0)
            {
                Properties.Settings.Default.RecentFolders.Add(Properties.Settings.Default.CurrentWorkingFolder);
            }

            UpdateRecentFoldersInGUI();

            //Setup Column Sorting
            setupColumnSorter();


            DoUpdateCheck();
        }


        private void DoUpdateCheck()
        {
            // Update check
            if (noUpdateCheck)
            {
                Console.WriteLine("UPDATE: Update checking suppressed.");
            }
            else
            {
                const string updateUrl = "https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Downloads/AX3/omgui.ini"; // "https://openmovement.googlecode.com/svn/downloads/AX3/omgui.ini";
                Console.WriteLine("UPDATE: Using address: " + updateUrl);

                BackgroundWorker updateWorker = new BackgroundWorker();
                //updateWorker.WorkerReportsProgress = true;
                //updateWorker.WorkerSupportsCancellation = true;
                updateWorker.DoWork += (s, ea) =>
                {
                    try
                    {
                        WebRequest webRequest = WebRequest.Create(updateUrl);
                        HttpWebResponse response = webRequest.GetResponse() as HttpWebResponse;

                        if (response.StatusCode == HttpStatusCode.OK)
                        {
                            // Read all lines
                            List<string> lineList = new List<string>();
                            Stream stream = response.GetResponseStream();
                            StreamReader reader = new StreamReader(stream);
                            string newLine;
                            while ((newLine = reader.ReadLine()) != null)
                            {
                                lineList.Add(newLine);
                            }
                            Console.ReadLine();
                            reader.Close();
                            stream.Close();
                            response.Close();
                            string[] lines = lineList.ToArray();

                            // Read bootload information from file
                            string installVersion = null;
                            string installUrl = null;
                            string installDescription = null;
                            IDictionary<string, string> upgrades = new Dictionary<string, string>(); // Version-specific information

                            // Rough .INI parser
                            string section = "";
                            foreach (string rawLine in lines)
                            {
                                string line = rawLine.Trim();
                                if (line.Length == 0 || line.StartsWith(";") || line.StartsWith("#")) { continue; }                             // Empty lines and comments
                                if (line.StartsWith("[") && line.EndsWith("]")) { section = line.Substring(1, line.Length - 2); continue; }    // Section
                                string[] parts = line.Split(new char[] { '=', ':' }, 2);
                                if (parts.Length <= 0) { continue; }
                                string name = parts[0].Trim();
                                string value = (parts.Length > 1) ? parts[1].Trim() : null;

                                if (section == "upgrade")
                                {
                                    upgrades.Add(name, value);
                                }
                                else if (section == "install")
                                {
                                    if (name == "version") { installVersion = value; }
                                    else if (name == "url") { installUrl = value; }
                                    else if (name == "description") { installDescription = value; }
                                }
                            }

                            // Current version
                            Version currentVersion = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
                            int[] currentVersionParts = new int[4];
                            currentVersionParts[0] = currentVersion.Major;
                            currentVersionParts[1] = currentVersion.MajorRevision;
                            currentVersionParts[2] = currentVersion.Minor;
                            currentVersionParts[3] = currentVersion.MinorRevision;

                            // Install version
                            int[] installVersionParts = new int[4];
                            if (installVersion != null)
                            {
                                string[] parts = installVersion.Split(new char[] { '.' });
                                for (int i = 0; i < installVersionParts.Length; i++)
                                {
                                    if (parts.Length > i) { int.TryParse(parts[i], out installVersionParts[i]); }
                                }
                            }

                            // Compare if newer
                            bool outdated = false;
                            for (int i = 0; i < installVersionParts.Length; i++)
                            {
                                if (installVersionParts[i] > currentVersionParts[i]) { outdated = true; break; }
                                if (installVersionParts[i] < currentVersionParts[i]) { break; }
                            }

                            // Messages
                            string currentVersionString = currentVersionParts[0] + "." + currentVersionParts[1] + "." + currentVersionParts[2] + "." + currentVersionParts[3];
                            string installVersionString = installVersionParts[0] + "." + installVersionParts[1] + "." + installVersionParts[2] + "." + installVersionParts[3];
                            string upgradeMessage = null;
                            if (upgrades.ContainsKey(currentVersionString)) { upgradeMessage = upgrades[currentVersionString]; }

                            // Prompt
                            if (!outdated)
                            {
                                Console.WriteLine("UPDATE: Current version " + currentVersionString + " is up to date (>= " + installVersionString + ")");
                            }
                            else
                            {
                                Console.WriteLine("UPDATE: Current version " + currentVersionString + " is needs updating (< " + installVersionString + ")");

                                string promptMessage = "";
                                promptMessage += "There is an update available.\r\n";
                                promptMessage += "\r\n";
                                promptMessage += "The update is V" + installVersionString + " (you are currently running V" + currentVersionString + "):\r\n";
                                if (installDescription != null)
                                {
                                    promptMessage += "" + installDescription.Replace("|", "\r\n") + "\r\n";
                                }
                                if (upgradeMessage != null)
                                {
                                    promptMessage += "" + upgradeMessage.Replace("|", "\r\n") + "\r\n";
                                }
                                if (installUrl != null)
                                {
                                    promptMessage += "\r\n";
                                    promptMessage += "Update address:\r\n";
                                    promptMessage += "" + installUrl + "\r\n";
                                    promptMessage += "\r\n";
                                    promptMessage += "Download update now?";
                                }

                                DialogResult dr = MessageBox.Show(null, promptMessage, "Update Available", (installUrl == null) ? MessageBoxButtons.OK : MessageBoxButtons.YesNo, MessageBoxIcon.Information, MessageBoxDefaultButton.Button1);
                                if (dr == System.Windows.Forms.DialogResult.Yes)
                                {
                                    if (installUrl != null && (installUrl.StartsWith("http://") || installUrl.StartsWith("https://")))
                                    {
                                        Console.WriteLine("UPDATE: User downloading update.");
                                        System.Diagnostics.Process.Start(installUrl);
                                    }
                                    else
                                    {
                                        Console.WriteLine("UPDATE: Error, URL prefix was invalid.");
                                    }
                                }
                                else
                                {
                                    Console.WriteLine("UPDATE: User not downloading update.");
                                }
                            }

                        }
                        else
                        {
                            Console.WriteLine("UPDATE: Unexpected response status code: " + response.StatusCode + " - " + response.StatusDescription + "");
                        }
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("UPDATE: Problem while checking: " + e.Message + "");
                    }
                };
                updateWorker.RunWorkerCompleted += (s, ea) =>
                {
                    Console.WriteLine("UPDATE: Check finished.");
                };

                Console.WriteLine("UPDATE: Check starting...");
                updateWorker.RunWorkerAsync();
            }
        }

        private void AddProfilePluginsToToolStrip()
        {
            for(int i = toolStripFiles.Items.Count; i > 4; i--)
            {
                //toolStripFiles.Items.RemoveAt(toolStripFiles.Items.Count-1);
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
                        string startDateTimeString = startDateTime.ToString("dd/MM/yyyy/_HH:mm:ss.000");
                        string endDateTimeString = endDateTime.ToString("dd/MM/yyyy/_HH:mm:ss.000");

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
                for (int i = 0; i < toolStripFiles.Items.Count; i++)
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
            for (int i = 0; i < toolStripFiles.Items.Count; i++)
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

        string omGUIEXEPath = Directory.GetCurrentDirectory();

        private void RunProcess2(string parameterString, Plugin p, string outputName, string[] inputNames)
        {

            ProcessStartInfo psi = new ProcessStartInfo();

            psi.FileName = p.FilePath + Path.DirectorySeparatorChar + p.RunFilePath;

            //TODO - Idea to try and do the outfile as a filepath rather than filename.
            //Find arguments and replace the output file with the full file path.


            Console.WriteLine("current user working folder: " + Properties.Settings.Default.CurrentWorkingFolder);

            if (!Properties.Settings.Default.CurrentWorkingFolder[Properties.Settings.Default.CurrentWorkingFolder.Length - 1].Equals('\\'))
                parameterString = parameterString.Replace(outputName, "\"" + Properties.Settings.Default.CurrentWorkingFolder + "\\" + outputName + "\"");
            else
                parameterString = parameterString.Replace(outputName, "\"" + Properties.Settings.Default.CurrentWorkingFolder + outputName + "\"");
            
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

            Directory.SetCurrentDirectory(p.FilePath);

            pluginQueueWorker.RunWorkerAsync(pqi);
        }

        List<ListViewItem> queueListViewItems = new List<ListViewItem>();
        //PLUGIN PROCESS RUNNING

        void pluginQueueWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            Directory.SetCurrentDirectory(omGUIEXEPath);

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

                    Console.WriteLine("Plugin Error: " + w32e.Message);
                    MessageBox.Show("Plugin Error: " + "Plugin EXE not present or not compatible.", "Plugin Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
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
            if (outputLine != null && outputLine.Length > 2)
            {
                if ((outputLine[0] == 'p' || outputLine[0] == 'P') && (outputLine[1] == ' ' || outputLine[1] == ':'))
                {
                    string percentage = outputLine.Substring(2);
                    Int32.TryParse(percentage, out progress);
                    //runPluginProgressBar.Value = Int32.Parse(percentage);
                }
                //else if (outputLine[0] == 's' || outputLine[0] == 'S')
                //{
                //    statusMessage = outputLine.Split(new char[] { ' ' }, 2).Last();
                //    //labelStatus.Text = message;
                //}
                else if ((outputLine[0] == 'e' || outputLine[0] == 'E') && (outputLine[1] == ' ' || outputLine[1] == ':'))
                {
                    errorMessage = outputLine.Substring(2);
                }


                Console.WriteLine("o: " + outputLine);
            }
        }

        #endregion

        //TS - This will become a plugin so keep the code here for now just so we can see the Form.
        //ExportDataConstruct(OmSource.SourceCategory.File);

        private void pluginsToolStripButton_Click(object sender, EventArgs e)
        {
            RunPluginsProcess(filesListView.SelectedItems);
        }

        private void openCurrentWorkingFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // "explorer.exe /select, <filename>"
            System.Diagnostics.Process.Start(GetPath(OmGui.Properties.Settings.Default.CurrentWorkingFolder));
            //Process.Start(Properties.Settings.Default.CurrentWorkingFolder);
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
            if (!Path.GetExtension(e.FullPath).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase) && !Path.GetExtension(e.FullPath).Equals(".part", StringComparison.CurrentCultureIgnoreCase))
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
            if (!Path.GetExtension(e.FullPath).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase) && !Path.GetExtension(e.FullPath).Equals(".part", StringComparison.CurrentCultureIgnoreCase))
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
            if (!Path.GetExtension(e.OldFullPath).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase) && !Path.GetExtension(e.OldFullPath).Equals(".part", StringComparison.CurrentCultureIgnoreCase))
            {
                outputListRemoveItem(e.OldFullPath, false);
            }
            if (!Path.GetExtension(e.FullPath).Equals(".cwa", StringComparison.CurrentCultureIgnoreCase) && !Path.GetExtension(e.FullPath).Equals(".part", StringComparison.CurrentCultureIgnoreCase))
            {
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
                toolStripButtonShowFiles.Text = "Show *.CWA Only";
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
            Console.WriteLine("Blocking background tasks...");
            if (backgroundWorkerUpdate.IsBusy)
            {
                Console.WriteLine("...waiting for current background task...");
                Cursor.Current = Cursors.WaitCursor;

                // Spin while waiting for background task (not great, but tasks should be short-lived)
                while (backgroundWorkerUpdate.IsBusy)
                {
                    Application.DoEvents();
                    Thread.Sleep(50);
                }
                Cursor.Current = Cursors.Default;
                Console.WriteLine("...done.");
            }
        }

        private void EnableBackgroundTasks()
        {
            Console.WriteLine("Enabling background tasks...");
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

            if (backgroundWorkerUpdate.IsBusy)
            {
                Console.WriteLine("Update skipped as busy -- #" + refreshCounter + (doIdentifyTask ? " (identify " + identifyTicks + ")" : ""));
            }
            else
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
// HACK: Put this back (fix problem - perhaps only if log window open?)
/*
            if (InvokeRequired)
            {
                this.Invoke((MethodInvoker)delegate
                {
                    BackgroundTaskStatus(active);
                });
                return;
            }
            toolStripBackgroundTask.Visible = active;
*/
        }

        private void backgroundWorkerUpdate_DoWork(object sender, DoWorkEventArgs e)
        {
            // If identify task
            if (e.Argument == null)
            {
                identifyTicks--;

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

Console.WriteLine("backgroundWorkerUpdate - Identify task: " + state);
                lock (identifyDevices)
                {
                    foreach (OmDevice device in identifyDevices)
                    {
                        device.SetLed(state);
                    }
                }

            }
            else
            {
                OmDevice device = (OmDevice)e.Argument;
                bool changed = device.Update(resetIfUnresponsive);
                if (changed)
                {
Console.WriteLine("backgroundWorkerUpdate - changed " + device.DeviceId);
                    UpdateDeviceId(device.DeviceId);
                }
            }
            BackgroundTaskStatus(false);
        }


        // ---------- EXPORT ----------

        private void exportToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ExportDataConstruct();
        }

        string[] GetSelectedFilesForConvert(string newExtension)
        {
            // Check no devices selected
            if (devicesListView.SelectedItems.Count > 0)
            {
                DialogResult dr = MessageBox.Show(this, "Cannot perform this action on files until they have been downloaded.\nDownload the files or deselect the device(s).", "Device(s) selected", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                return null;
            }

            // Get list of files selected
            List<string> files = new List<string>();
            foreach (ListViewItem item in filesListView.SelectedItems)
            {
                OmReader reader = (OmReader)item.Tag;
                files.Add(reader.Filename);
            }

            // Check at least one file selected
            if (files.Count <= 0)
            {
                DialogResult dr = MessageBox.Show(this, "No files selected.", "No files selected", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                return null;
            }

            // Build a list of messages for the overwrite prompt
            StringBuilder sb = new StringBuilder();
            foreach (string file in files)
            {
                string resultFile = Path.ChangeExtension(file, newExtension);
                if (File.Exists(resultFile))
                {
                    DateTime sourceTime = DateTime.MinValue;
                    try { sourceTime = File.GetLastWriteTime(file); }
                    catch { ; }
                    DateTime destinationTime = DateTime.MinValue;
                    try { destinationTime = File.GetLastWriteTime(resultFile); }
                    catch { ; }

                    if (sourceTime <= destinationTime)
                    {
                        sb.Append("File already exists: " + resultFile + "\r\n");
                    }
                    else
                    {
                        sb.Append("Caution, newer file exists: " + resultFile + "\r\n");
                    }
                }
            }

            // Overwrite prompt
            if (sb.Length > 0)
            {
                DialogResult dr = MessageBoxEx.Show(this, "Overwrite the following files:\r\n\r\n" + sb.ToString() + "\r\nAre you sure you want to overwrite?", "Overwrite existing files", MessageBoxExButtons.YesNoCancel, MessageBoxExIcon.Question, MessageBoxExDefaultButton.Button3);
                if (dr != DialogResult.Yes)
                {
                    return null;
                }
            }

            return files.ToArray();
        }

        string[] CheckWavConversion(string[] inputFiles, bool regenerateWav, bool promptBeforeConvert = false)
        {
            StringBuilder sb;

            if (inputFiles == null) { return null; }

            // We need .WAV files, check if any are missing
            sb = new StringBuilder();
            List<string> wavSourceFiles = new List<string>();
            List<string> wavOutputFiles = new List<string>();
            foreach (string file in inputFiles)
            {
                string wavFile = Path.ChangeExtension(file, ".wav");
                wavOutputFiles.Add(file);
                if (regenerateWav)
                {
                    wavSourceFiles.Add(file);
                    sb.Append(".WAV regeneration requested: " + wavFile + "\r\n");
                }
                else if (File.Exists(wavFile))
                {
                    DateTime sourceTime = DateTime.MinValue;
                    try { sourceTime = File.GetLastWriteTime(file); } catch { ; }
                    DateTime destinationTime = DateTime.MinValue;
                    try { destinationTime = File.GetLastWriteTime(wavFile); } catch { ; }

                    if (destinationTime < sourceTime)
                    {
                        wavSourceFiles.Add(file);
                        sb.Append(".WAV conversion older than source file: " + wavFile + "\r\n");
                    }
                }
                else
                {
                    wavSourceFiles.Add(file);
                    sb.Append(".WAV conversion required: " + wavFile + "\r\n");
                }
            }

            // Overwrite prompt
            if (sb.Length > 0)
            {
                DialogResult dr = DialogResult.OK;
                if (promptBeforeConvert)
                {
                    dr = MessageBox.Show(this, "One or more files needs resampling to .WAV format first:\r\n\r\n" + sb.ToString() + "\r\n", "Resample to WAV first?", MessageBoxButtons.OKCancel, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1);
                }
                if (dr == DialogResult.OK)
                {
                    if (!DoWavConvert(wavSourceFiles.ToArray(), ".wav", -1, true, true))
                    {
//MessageBox.Show(this, ".WAV convert not successful.", "Conversion Unsuccessful", MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1);
                        return null;
                    }

                    // Confirm converted files exist
                    sb = new StringBuilder();
                    foreach (string wavFile in wavOutputFiles)
                    {
                        if (!File.Exists(wavFile)) 
                        {
                            sb.Append(wavFile + "\r\n");
                        }
                    }
                    if (sb.Length > 0)
                    {
                        MessageBox.Show(this, "One or more requied .WAV files is missing:\r\n\r\n" + sb.ToString() + "\r\n", "Files missing after conversion", MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1);
                        return null;
                    }

                }
                else
                {
                    return null;
                }
            }

            return inputFiles;
        }

        //string[] UtilityCheckOverwrites(string newExtension)
        //{
        //}

        private static string OMCONVERT_EXE = @"Plugins\OmConvertPlugin\omconvert.exe";

        private bool DoWavConvert(string[] files, string ext, int rate, bool calibrate, bool autoClose)
        {
            if (files == null) { return false; }
            List<string> outputList = new List<string>();

            foreach (string file in files)
            {
                List<string> args = new List<string>();
                string input = file;
                string final = Path.ChangeExtension(file, ".wav");
                string output = final + ".part";

                args.Add("\"" + input + "\"");
                if (rate > 0) { args.Add("-resample"); args.Add("" + rate); }
                args.Add("-calibrate"); args.Add(calibrate ? "1" : "0");
                args.Add("-out"); args.Add("\"" + output + "\"");
                ProcessingForm processingForm = new ProcessingForm(OMCONVERT_EXE, args, output, final);
                DialogResult dr = processingForm.ShowDialog();
                if (dr == DialogResult.Cancel) { return false; }
                outputList.Add(Path.GetFileName(final));
            }

            if (!autoClose)
            {
                MessageBoxEx.Show(this, "Output " + outputList.Count + "/" + files.Length + ":\r\n\r\n" + string.Join("\r\n", outputList.ToArray()) + "\r\n\r\n", "Complete", MessageBoxExButtons.OK, MessageBoxExIcon.Information, MessageBoxExDefaultButton.Button1);
            }
            return true;
        }


        private void wavToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Make .WAV:
            //  * Form compatible with multiple source files
            //  * - Interpolated sample rate (100Hz, 50Hz, 25Hz)
            //  * - No auto-calibrate
            //  * (note: derived summaries can be affected by the sample rate)

            string[] files = GetSelectedFilesForConvert(".resampled.wav");
            if (files == null) { return; }
            ExportWavForm optionsForm = new ExportWavForm();
            DialogResult dr = optionsForm.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.OK) { return; }
            int rate = optionsForm.Rate;
            bool autoCalibrate = optionsForm.AutoCalibrate;
            DoWavConvert(files, ".resampled.wav", rate, autoCalibrate, false);
        }



        private void svmToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Make .SVM.CSV
            //  * Epoch (60 seconds)
            //  * Filter (off, 0.5-20Hz)
            //  * Mode: abs(sum(svm-1)) vs sum(max(0,svm-1))
            bool regenerateWav = (Control.ModifierKeys & Keys.Shift) != 0;
            string[] inputFiles = GetSelectedFilesForConvert(".svm.csv");
            if (inputFiles == null) { return; }
            ExportSvmForm optionsForm = new ExportSvmForm();
            DialogResult dr = optionsForm.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.OK) { return; }
            string[] files = CheckWavConversion(inputFiles, regenerateWav);
            if (files == null) { return; }
            List<string> outputList = new List<string>();
            foreach (string file in files)
            {
                List<string> args = new List<string>();
                string input = Path.ChangeExtension(file, ".wav");
                string final = Path.ChangeExtension(file, ".svm.csv");
                string output = final + ".part";

                args.Add("\"" + input + "\"");
                args.Add("-svm-epoch"); args.Add("" + optionsForm.Epoch);
                args.Add("-svm-filter"); args.Add("" + optionsForm.Filter);
                args.Add("-svm-mode"); args.Add("" + optionsForm.Mode);
                args.Add("-svm-file"); args.Add("\"" + output + "\"");
                ProcessingForm processingForm = new ProcessingForm(OMCONVERT_EXE, args, output, final);
                dr = processingForm.ShowDialog();
                if (dr == System.Windows.Forms.DialogResult.Cancel) { break; }
                outputList.Add(Path.GetFileName(final));
            }
            MessageBoxEx.Show(this, "Output " + outputList.Count + "/" + files.Length + ":\r\n\r\n" + string.Join("\r\n", outputList.ToArray()) + "\r\n\r\n", "Complete", MessageBoxExButtons.OK, MessageBoxExIcon.Information, MessageBoxExDefaultButton.Button1);
        }

        private void cutPointsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Make .CUT.CSV
            //  * Epochs (number of 1 minute periods)
            //  * Model (dominant/"right" hand, non-dominant/"left" hand, weight)
            bool regenerateWav = (Control.ModifierKeys & Keys.Shift) != 0;
            string[] inputFiles = GetSelectedFilesForConvert(".cut.csv");
            if (inputFiles == null) { return; }
            ExportPaeeForm optionsForm = new ExportPaeeForm();
            DialogResult dr = optionsForm.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.OK) { return; }
            string[] files = CheckWavConversion(inputFiles, regenerateWav);
            if (files == null) { return; }
            List<string> outputList = new List<string>();
            foreach (string file in files)
            {
                List<string> args = new List<string>();
                string input = Path.ChangeExtension(file, ".wav");
                string final = Path.ChangeExtension(file, ".cut.csv");
                string output = final + ".part";

                args.Add("\"" + input + "\"");
                args.Add("-paee-epoch"); args.Add("" + optionsForm.Epoch);
                args.Add("-paee-model"); args.Add("" + optionsForm.Model);
                args.Add("-paee-file"); args.Add("\"" + output + "\"");
                ProcessingForm processingForm = new ProcessingForm(OMCONVERT_EXE, args, output, final);
                dr = processingForm.ShowDialog();
                if (dr == System.Windows.Forms.DialogResult.Cancel) { break; }
                outputList.Add(Path.GetFileName(final));
            }
            MessageBoxEx.Show(this, "Output " + outputList.Count + "/" + files.Length + ":\r\n\r\n" + string.Join("\r\n", outputList.ToArray()) + "\r\n\r\n", "Complete", MessageBoxExButtons.OK, MessageBoxExIcon.Information, MessageBoxExDefaultButton.Button1);
        }

        private void wearTimeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Make .WTV.CSV
            //  * Epochs (number of 0.5 minute periods)
            bool regenerateWav = (Control.ModifierKeys & Keys.Shift) != 0;
            string[] inputFiles = GetSelectedFilesForConvert(".wtv.csv");
            if (inputFiles == null) { return; }
            ExportWtvForm optionsForm = new ExportWtvForm();
            DialogResult dr = optionsForm.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.OK) { return; }
            string[] files = CheckWavConversion(inputFiles, regenerateWav);
            if (files == null) { return; }
            List<string> outputList = new List<string>();
            foreach (string file in files)
            {
                List<string> args = new List<string>();
                string input = Path.ChangeExtension(file, ".wav");
                string final = Path.ChangeExtension(file, ".wtv.csv");
                string output = final + ".part";

                args.Add("\"" + input + "\"");
                args.Add("-wtv-epoch"); args.Add("" + optionsForm.Epoch);
                args.Add("-wtv-file"); args.Add("\"" + output + "\"");
                ProcessingForm processingForm = new ProcessingForm(OMCONVERT_EXE, args, output, final);
                dr = processingForm.ShowDialog();
                if (dr == System.Windows.Forms.DialogResult.Cancel) { break; }
                outputList.Add(Path.GetFileName(final));
            }
            MessageBoxEx.Show(this, "Output " + outputList.Count + "/" + files.Length + ":\r\n\r\n" + string.Join("\r\n", outputList.ToArray()) + "\r\n\r\n", "Complete", MessageBoxExButtons.OK, MessageBoxExIcon.Information, MessageBoxExDefaultButton.Button1);
        }


        private void dataViewer_Load(object sender, EventArgs e)
        {

        }

        private void toolStripButtonRefresh_Click(object sender, EventArgs e)
        {
            LoadWorkingFolder();
        }

        private void toolStripButtonCSV_Click_1(object sender, EventArgs e)
        {
            // Make .CSV (resampled)
            bool regenerateWav = (Control.ModifierKeys & Keys.Shift) != 0;
            string[] inputFiles = GetSelectedFilesForConvert(".resampled.csv");
            if (inputFiles == null) { return; }
            ExportCsvForm optionsForm = new ExportCsvForm();
            DialogResult dr = optionsForm.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.OK) { return; }
            string[] files = CheckWavConversion(inputFiles, regenerateWav);
            if (files == null) { return; }
            List<string> outputList = new List<string>();
            foreach (string file in files)
            {
                List<string> args = new List<string>();
                string input = Path.ChangeExtension(file, ".wav");
                string final = Path.ChangeExtension(file, ".resampled.csv");
                string output = final + ".part";

                args.Add("\"" + input + "\"");
                args.Add("-csv-file"); args.Add("\"" + output + "\"");
                ProcessingForm processingForm = new ProcessingForm(OMCONVERT_EXE, args, output, final);
                dr = processingForm.ShowDialog();
                if (dr == System.Windows.Forms.DialogResult.Cancel) { break; }
                outputList.Add(Path.GetFileName(final));
            }
            MessageBoxEx.Show(this, "Output " + outputList.Count + "/" + files.Length + ":\r\n\r\n" + string.Join("\r\n", outputList.ToArray()) + "\r\n\r\n", "Complete", MessageBoxExButtons.OK, MessageBoxExIcon.Information, MessageBoxExDefaultButton.Button1);
        }

        private void toolStripButtonSleep_Click(object sender, EventArgs e)
        {
            // Make .SLEEP.CSV
            //  * Epochs (number of 0.5 minute periods)
            bool regenerateWav = (Control.ModifierKeys & Keys.Shift) != 0;
            string[] inputFiles = GetSelectedFilesForConvert(".sleep.csv");
            if (inputFiles == null) { return; }
            ExportSleepForm optionsForm = new ExportSleepForm();
            DialogResult dr = optionsForm.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.OK) { return; }
            string[] files = CheckWavConversion(inputFiles, regenerateWav);
            if (files == null) { return; }
            List<string> outputList = new List<string>();
            foreach (string file in files)
            {
                List<string> args = new List<string>();
                string input = Path.ChangeExtension(file, ".wav");
                string final = Path.ChangeExtension(file, ".sleep.csv");
                string output = final + ".part";

                args.Add("\"" + input + "\"");
                args.Add("-sleep-file"); args.Add("\"" + output + "\"");
                ProcessingForm processingForm = new ProcessingForm(OMCONVERT_EXE, args, output, final);
                dr = processingForm.ShowDialog();
                if (dr == System.Windows.Forms.DialogResult.Cancel) { break; }
                outputList.Add(Path.GetFileName(final));
            }
            MessageBoxEx.Show(this, "Output " + outputList.Count + "/" + files.Length + ":\r\n\r\n" + string.Join("\r\n", outputList.ToArray()) + "\r\n\r\n", "Complete", MessageBoxExButtons.OK, MessageBoxExIcon.Information, MessageBoxExDefaultButton.Button1);
        }  


    }
}
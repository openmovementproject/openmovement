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

namespace OmGui
{
    public partial class MainForm : Form
    {
        Om om;
        DeviceManager deviceManager;

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern uint RegisterWindowMessage(string lpString);

        private uint queryCancelAutoPlayID;

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
            /*
            this.MouseWheel += (o, e) =>
            {
                Control control = this.GetChildAtPoint(p);
                Control lastControl = control;
                while (control != null)
                {
                    if (control == dataViewer)
                    {
                        Console.WriteLine(e.Delta);
                        //                        dataViewer.DataViewer_MouseWheel(o, e);
                    }
                    lastControl = control;
                    p.X -= control.Location.X;
                    p.Y -= control.Location.Y;
                    control = control.GetChildAtPoint(p);
                }
            };
            */


            try
            {
                string folder = GetPath(OmGui.Properties.Settings.Default.DownloadPath);
                if (!System.IO.Directory.Exists(folder))
                {
                    System.IO.Directory.CreateDirectory(folder);
                }
                string[] filePaths = System.IO.Directory.GetFiles(folder, fileSystemWatcher.Filter, System.IO.SearchOption.TopDirectoryOnly);
                foreach (string f in filePaths)
                {
                    DataFileAdd(f);
                }
                fileSystemWatcher.Path = folder;
            }
            catch (Exception) { ; }
        }


        IDictionary<ushort, ListViewItem> listViewDevices = new Dictionary<ushort, ListViewItem>();
        IDictionary<string, ListViewItem> listViewFiles = new Dictionary<string, ListViewItem>();

        public void UpdateListViewItem(System.Windows.Forms.ListViewItem item)
        {
            OmSource source = (OmSource)item.Tag;
            OmDevice device = source as OmDevice;
            OmReader reader = source as OmReader;

            int img = 8;
            string download = "-";
            string recording = "-";
            string battery = "-";

            if (item.SubItems.Count < 5)
            {
                item.SubItems.Clear();
                item.SubItems.Add("");                      // Device id
                item.SubItems.Add("");                      // Session id
                item.SubItems.Add("");                      // Battery level
                item.SubItems.Add("");                      // Download
                item.SubItems.Add("");                      // Recording
            }

            OmSource.SourceCategory category = source.Category;
            item.Group = ((DeviceListView)item.ListView).Groups[category.ToString()];
            if (category == OmSource.SourceCategory.Removed)
            {
                item.ForeColor = System.Drawing.SystemColors.GrayText;
                item.Font = new System.Drawing.Font(System.Drawing.SystemFonts.DefaultFont, System.Drawing.FontStyle.Italic);
            }
            else if (category == OmSource.SourceCategory.File)
            {
                item.ForeColor = System.Drawing.SystemColors.GrayText;
            }
            else
            {
                item.ForeColor = System.Drawing.SystemColors.WindowText;
                item.Font = System.Drawing.SystemFonts.DefaultFont;
            }


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


            item.ImageIndex = img;
            item.SubItems[0].Text = string.Format("{0:00000}", source.DeviceId);
            item.SubItems[1].Text = (source.SessionId == uint.MinValue) ? "-" : source.SessionId.ToString();
            item.SubItems[2].Text = battery;
            item.SubItems[3].Text = download;
            item.SubItems[4].Text = recording;
        }


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
                listView.Items.Add(item);
                UpdateListViewItem(item);
            }
            else
            {
                item = listViewDevices[deviceId];
                UpdateDevice(item);
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

        private void MainForm_Load(object sender, EventArgs e)
        {
        }

        private void View_CheckChanged(object sender, EventArgs e)
        {
            toolStripMain.Visible = toolbarToolStripMenuItem.Checked;
            statusStripMain.Visible = statusBarToolStripMenuItem.Checked;
            splitContainerLog.Panel2Collapsed = !logToolStripMenuItem.Checked;
            splitContainerPreview.Panel1Collapsed = !previewToolStripMenuItem.Checked;
            splitContainerDevices.Panel2Collapsed = !propertiesToolStripMenuItem.Checked;
        }

        private void UpdateDevice(ListViewItem item)
        {
            OmSource device = (OmSource)item.Tag;
            UpdateListViewItem(item);
            if (device is OmDevice && !((OmDevice)device).Connected)
            {
                listViewDevices.Remove(device.DeviceId);
                listView.Items.Remove(item);
            }
            UpdateEnabled();
        }

        int refreshIndex = 0;
        private void refreshTimer_Tick(object sender, EventArgs e)
        {
            if (refreshIndex >= listView.Items.Count) { refreshIndex = 0; }
            if (refreshIndex < listView.Items.Count)
            {
                ListViewItem item = listView.Items[refreshIndex];
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
            foreach (ListViewItem i in listView.SelectedItems)
            {
                OmDevice device = i.Tag as OmDevice;
                if (device != null && !device.IsDownloading)
                {
                    bool ok = true;
                    string folder = GetPath(OmGui.Properties.Settings.Default.DownloadPath);
                    System.IO.Directory.CreateDirectory(folder);
                    string prefix = folder + string.Format("{0:00000}_{1:0000000000}", device.DeviceId, device.SessionId);
                    string finalFilename = prefix + ".cwa";
                    string downloadFilename = finalFilename;

                    downloadFilename += ".part";
                    if (System.IO.File.Exists(downloadFilename))
                    {
                        DialogResult dr = MessageBox.Show(this, "Download file already exists:\r\n\r\n    " + downloadFilename + "\r\n\r\nOverwrite existing file?", "Overwrite File?", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                        if (dr != DialogResult.OK) { ok = false; }
                        else { System.IO.File.Delete(downloadFilename); }
                    }

                    if (ok && System.IO.File.Exists(finalFilename))
                    {
                        DialogResult dr = MessageBox.Show(this, "File already exists:\r\n\r\n    " + downloadFilename + "\r\n\r\nOverwrite existing file?", "Overwrite File?", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                        if (dr != DialogResult.OK) { ok = false; }
                        else { System.IO.File.Delete(finalFilename);  }
                    }
                    if (ok)
                    {
                        device.BeginDownloading(downloadFilename, finalFilename);
                    }
                }
            }
        }

        public OmSource[] UpdateEnabled()
        {
            bool downloadable = false, downloading = false;
            bool all = true;

            List<OmSource> selected = new List<OmSource>();
            foreach (ListViewItem i in listView.SelectedItems)
            {
                OmSource device = (OmSource)i.Tag;
                if (!(device is OmDevice) || !((OmDevice)device).Connected)
                {
                    all = false;
                    if (device is OmReader)
                    {
                        selected.Add(device);
                    }
                }
                else
                {
                    if (device is OmDevice)
                    {
                        if (((OmDevice)device).DownloadStatus != OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS) { downloadable = true; }
                        if (((OmDevice)device).DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS) { downloading = true; }
                    }
                    selected.Add(device);
                }

            }

            bool any = (selected.Count > 0);
            //propertyGridDevice.Enabled = any && all;
            toolStripButtonDownload.Enabled = any && all && downloadable;
            toolStripButtonCancelDownload.Enabled = any && all && downloading;
            toolStripButtonClear.Enabled = any && all && !downloading;
            toolStripButtonSyncTime.Enabled = any && all;
            toolStripButtonStop.Enabled = any && all && !downloading;
            toolStripButtonRecord.Enabled = any && all && !downloading;
            toolStripButtonInterval.Enabled = any && all && !downloading;
            toolStripButtonSessionId.Enabled = any && all && !downloading;
            toolStripMenuItemConvert.Enabled = any;

            return selected.ToArray();
        }


        private void listViewDevices_SelectedIndexChanged(object sender, EventArgs e)
        {
            OmSource[] selected = UpdateEnabled();
            propertyGridDevice.SelectedObjects = selected;

            if (selected.Length == 1 && selected[0] is OmReader)
            {
                dataViewer.Open(((OmReader)selected[0]).Filename);
            }
            else if (selected.Length == 1 && selected[0] is OmDevice && ((OmDevice)selected[0]).Connected)
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
            PropertyEditor propertyEditor = new PropertyEditor("Options", OmGui.Properties.Settings.Default, true);
            DialogResult dr = propertyEditor.ShowDialog(this);
            if (dr == DialogResult.OK)
            {
                OmGui.Properties.Settings.Default.Save();
            }
            else
            {
                OmGui.Properties.Settings.Default.Reload();
            }
        }


        public void DataFileRemove(string file)
        {
            if (listViewFiles.ContainsKey(file))
            {
                listView.Items.Remove(listViewFiles[file]);
                listViewFiles.Remove(file);
            }
        }

        public void DataFileAdd(string file)
        {
            OmSource device = OmReader.Open(file);
            if (device != null)
            {
                ListViewItem listViewItem = new ListViewItem();
                listViewItem.Tag = device;
                listView.Items.Add(listViewItem);
                listViewFiles[file] = listViewItem;
                UpdateDevice(listViewItem);
            }
        }


        private void fileSystemWatcher_Renamed(object sender, System.IO.RenamedEventArgs e)
        {
            DataFileRemove(e.OldFullPath);
            if (e.FullPath.EndsWith(fileSystemWatcher.Filter.Substring(1), StringComparison.InvariantCultureIgnoreCase))
            {
                DataFileAdd(e.FullPath);
            }
        }

        private void fileSystemWatcher_Created(object sender, System.IO.FileSystemEventArgs e)
        {
            DataFileAdd(e.FullPath);
        }

        private void fileSystemWatcher_Deleted(object sender, System.IO.FileSystemEventArgs e)
        {
            DataFileRemove(e.FullPath);
        }


        private void toolStripButtonSyncTime_Click(object sender, EventArgs e)
        {
            List<string> fails = new List<string>();
            Cursor.Current = Cursors.WaitCursor;
            foreach (ListViewItem i in listView.SelectedItems)
            {
                OmSource device = (OmSource)i.Tag;
                if (device is OmDevice && ((OmDevice)device).Connected)
                {
                    if (!((OmDevice)device).SyncTime())
                        fails.Add(device.DeviceId.ToString());
                }
            }
            Cursor.Current = Cursors.Default;
            if (fails.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
        }

        private void selectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listView.Items)
            {
                item.Selected = true;
            }
        }

        private void toolStripButtonCancelDownload_Click(object sender, EventArgs e)
        {
            Cursor.Current = Cursors.WaitCursor;
            foreach (ListViewItem i in listView.SelectedItems)
            {
                OmSource device = (OmSource)i.Tag;
                if (device is OmDevice && ((OmDevice)device).IsDownloading)
                {
                    ((OmDevice)device).CancelDownload();
                }
            }
            Cursor.Current = Cursors.Default;
        }

        public bool EnsureNoSelectedDownloading()
        {
            int downloading = 0, total = 0;
            foreach (ListViewItem i in listView.SelectedItems)
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
            if (EnsureNoSelectedDownloading())
            {
                List<string> fails = new List<string>();
                dataViewer.CancelPreview();
                Cursor.Current = Cursors.WaitCursor;
                foreach (ListViewItem i in listView.SelectedItems)
                {
                    OmSource device = (OmSource)i.Tag;
                    if (device is OmDevice && !((OmDevice)device).IsDownloading)
                    {
                        if (!((OmDevice)device).NeverRecord())
                            fails.Add(device.DeviceId.ToString());
                    }
                }
                Cursor.Current = Cursors.Default;
                if (fails.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
            }
        }

        private void toolStripButtonRecord_Click(object sender, EventArgs e)
        {
            if (EnsureNoSelectedDownloading())
            {
                List<string> fails = new List<string>();
                dataViewer.CancelPreview();
                Cursor.Current = Cursors.WaitCursor;
                foreach (ListViewItem i in listView.SelectedItems)
                {
                    OmSource device = (OmSource)i.Tag;
                    if (device is OmDevice && !((OmDevice)device).IsDownloading)
                    {
                        if (!((OmDevice)device).AlwaysRecord())
                            fails.Add(device.DeviceId.ToString());
                    }
                }
                Cursor.Current = Cursors.Default;
                if (fails.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
            }
        }

        private void toolStripButtonInterval_Click(object sender, EventArgs e)
        {
            if (EnsureNoSelectedDownloading())
            {
                List<string> fails = new List<string>();

                DateTime start = DateTime.MinValue;
                DateTime stop = DateTime.MaxValue;

                DateRangeForm rangeForm = new DateRangeForm("Session Range", "Session Range");
                DialogResult dr = rangeForm.ShowDialog();
                start = rangeForm.FromDate;
                stop = rangeForm.UntilDate;

                if (dr == System.Windows.Forms.DialogResult.OK)
                {
                    dataViewer.CancelPreview();
                    Cursor.Current = Cursors.WaitCursor;
                    foreach (ListViewItem i in listView.SelectedItems)
                    {
                        OmSource device = (OmSource)i.Tag;
                        if (device is OmDevice && !((OmDevice)device).IsDownloading)
                        {
                            if (!((OmDevice)device).SetInterval(start, stop))
                                fails.Add(device.DeviceId.ToString());
                        }
                    }
                    Cursor.Current = Cursors.Default;
                }

                if (fails.Count > 0) { MessageBox.Show(this, "Failed operation on " + fails.Count + " device(s):\r\n" + string.Join("; ", fails.ToArray()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
            }
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
                    foreach (ListViewItem i in listView.SelectedItems)
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
            if (EnsureNoSelectedDownloading())
            {
                DialogResult dr = MessageBox.Show(this, "Clear " + listView.SelectedItems.Count + " device(s)?", Text, MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
                if (dr == System.Windows.Forms.DialogResult.OK)
                {
                    //Cursor.Current = Cursors.WaitCursor;

                    //dataViewer.CancelPreview();
                    //dataViewer.Reader = null;
                    dataViewer.Close();

                    List<OmDevice> devices = new List<OmDevice>();
                    foreach (ListViewItem i in listView.SelectedItems)
                    {
                        OmSource device = (OmSource)i.Tag;
                        if (device is OmDevice && !((OmDevice)device).IsDownloading)
                        {
                            devices.Add((OmDevice)device);
                        }
                    }

                    BackgroundWorker clearBackgroundWorker = new BackgroundWorker();
                    clearBackgroundWorker.WorkerReportsProgress = true;
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
            System.Diagnostics.Process.Start(GetPath(OmGui.Properties.Settings.Default.DownloadPath));
        }

        private void propertiesToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void toolStripMenuItemConvert_Click(object sender, EventArgs e)
        {
            int numFiles = 0;
            int numDevices = 0;
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
            foreach (ListViewItem i in listView.SelectedItems)
            {
                OmSource device = (OmSource)i.Tag;
                if (!(device is OmDevice) || !((OmDevice)device).Connected)
                {
                    if (device is OmReader)
                    {
                        files.Add(((OmReader)device).Filename);
                        numFiles++;
                    }
                }
                else
                {
                    if (device is OmDevice)
                    {
                        files.Add(((OmDevice)device).Filename);
                        numDevices++;
                    }
                }
            }
            string[] filesArray = files.ToArray();

            if (filesArray.Length < 1) { return; }

            if (filesArray.Length > 1)
            {
                MessageBox.Show(this, "Multiple file export not supported.", "Sorry", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                return;
            }

            string folder = GetPath(OmGui.Properties.Settings.Default.DownloadPath);
            ExportForm exportForm = new ExportForm(filesArray[0], folder, blockStart, blockCount);
            DialogResult result = exportForm.ShowDialog(this);
        }





    }
}

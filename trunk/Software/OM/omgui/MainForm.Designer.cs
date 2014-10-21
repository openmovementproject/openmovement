namespace OmGui
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.ListViewGroup listViewGroup1 = new System.Windows.Forms.ListViewGroup("Devices", System.Windows.Forms.HorizontalAlignment.Left);
            System.Windows.Forms.ListViewGroup listViewGroup2 = new System.Windows.Forms.ListViewGroup("New Data", System.Windows.Forms.HorizontalAlignment.Left);
            System.Windows.Forms.ListViewGroup listViewGroup3 = new System.Windows.Forms.ListViewGroup("Downloading", System.Windows.Forms.HorizontalAlignment.Left);
            System.Windows.Forms.ListViewGroup listViewGroup4 = new System.Windows.Forms.ListViewGroup("Downloaded", System.Windows.Forms.HorizontalAlignment.Left);
            System.Windows.Forms.ListViewGroup listViewGroup5 = new System.Windows.Forms.ListViewGroup("Charging", System.Windows.Forms.HorizontalAlignment.Left);
            System.Windows.Forms.ListViewGroup listViewGroup6 = new System.Windows.Forms.ListViewGroup("Standby", System.Windows.Forms.HorizontalAlignment.Left);
            System.Windows.Forms.ListViewGroup listViewGroup7 = new System.Windows.Forms.ListViewGroup("Outbox", System.Windows.Forms.HorizontalAlignment.Left);
            System.Windows.Forms.ListViewGroup listViewGroup8 = new System.Windows.Forms.ListViewGroup("Removed", System.Windows.Forms.HorizontalAlignment.Left);
            System.Windows.Forms.ListViewGroup listViewGroup9 = new System.Windows.Forms.ListViewGroup("Files", System.Windows.Forms.HorizontalAlignment.Left);
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.refreshTimer = new System.Windows.Forms.Timer(this.components);
            this.fileSystemWatcher = new System.IO.FileSystemWatcher();
            this.toolStripContainer2 = new System.Windows.Forms.ToolStripContainer();
            this.BottomToolStripPanel = new System.Windows.Forms.ToolStripPanel();
            this.statusStripMain = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabelMain = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripBackgroundTask = new System.Windows.Forms.ToolStripStatusLabel();
            this.TopToolStripPanel = new System.Windows.Forms.ToolStripPanel();
            this.toolStripMain = new System.Windows.Forms.ToolStrip();
            this.toolStripButtonDownload = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonCancelDownload = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonClear = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator6 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonInterval = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonStopRecording = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator8 = new System.Windows.Forms.ToolStripSeparator();
            this.devicesToolStripButtonIdentify = new System.Windows.Forms.ToolStripButton();
            this.menuStripMain = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.workingFolderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.recentFoldersToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.exportToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.cutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copyToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            this.selectAllToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolbarToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusBarToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.previewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.propertiesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.logToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openCurrentWorkingFolderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.optionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.RightToolStripPanel = new System.Windows.Forms.ToolStripPanel();
            this.LeftToolStripPanel = new System.Windows.Forms.ToolStripPanel();
            this.ContentPanel = new System.Windows.Forms.ToolStripContentPanel();
            this.splitContainerLog = new System.Windows.Forms.SplitContainer();
            this.splitContainerPreview = new System.Windows.Forms.SplitContainer();
            this.splitContainerDevices = new System.Windows.Forms.SplitContainer();
            this.devicesListView = new OmGui.DeviceListView();
            this.propertyGridDevice = new System.Windows.Forms.PropertyGrid();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.dataViewer = new OmGui.DataViewer();
            this.tabControlFiles = new System.Windows.Forms.TabControl();
            this.tabPageFiles = new System.Windows.Forms.TabPage();
            this.filesListView = new System.Windows.Forms.ListView();
            this.nameFilesColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.locationFilesColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.sizeFilesColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.dateModifiedColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.toolStripFiles = new System.Windows.Forms.ToolStrip();
            this.toolStripButtonShowFiles = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.pluginsToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.DeleteFilesToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.tabPageQueue = new System.Windows.Forms.TabPage();
            this.toolStripQueue = new System.Windows.Forms.ToolStrip();
            this.toolStripQueueButtonCancel = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonClearCompleted = new System.Windows.Forms.ToolStripButton();
            this.queueListViewItems2 = new System.Windows.Forms.ListView();
            this.colName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colFileName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colProgress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPageOutput = new System.Windows.Forms.TabPage();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.outputListView = new System.Windows.Forms.ListView();
            this.columnHeaderFileName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderFilePath = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderSize = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderDateModified = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
            this.fileSystemWatcherOutput = new System.IO.FileSystemWatcher();
            this.backgroundWorkerUpdate = new System.ComponentModel.BackgroundWorker();
            ((System.ComponentModel.ISupportInitialize)(this.fileSystemWatcher)).BeginInit();
            this.toolStripContainer2.SuspendLayout();
            this.statusStripMain.SuspendLayout();
            this.toolStripMain.SuspendLayout();
            this.menuStripMain.SuspendLayout();
            this.splitContainerLog.Panel1.SuspendLayout();
            this.splitContainerLog.Panel2.SuspendLayout();
            this.splitContainerLog.SuspendLayout();
            this.splitContainerPreview.Panel1.SuspendLayout();
            this.splitContainerPreview.Panel2.SuspendLayout();
            this.splitContainerPreview.SuspendLayout();
            this.splitContainerDevices.Panel1.SuspendLayout();
            this.splitContainerDevices.Panel2.SuspendLayout();
            this.splitContainerDevices.SuspendLayout();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.tabControlFiles.SuspendLayout();
            this.tabPageFiles.SuspendLayout();
            this.toolStripFiles.SuspendLayout();
            this.tabPageQueue.SuspendLayout();
            this.toolStripQueue.SuspendLayout();
            this.tabPageOutput.SuspendLayout();
            this.toolStripContainer1.BottomToolStripPanel.SuspendLayout();
            this.toolStripContainer1.ContentPanel.SuspendLayout();
            this.toolStripContainer1.TopToolStripPanel.SuspendLayout();
            this.toolStripContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fileSystemWatcherOutput)).BeginInit();
            this.SuspendLayout();
            // 
            // refreshTimer
            // 
            this.refreshTimer.Tick += new System.EventHandler(this.refreshTimer_Tick);
            // 
            // fileSystemWatcher
            // 
            this.fileSystemWatcher.EnableRaisingEvents = true;
            this.fileSystemWatcher.Filter = "*.cwa";
            this.fileSystemWatcher.SynchronizingObject = this;
            this.fileSystemWatcher.Changed += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_Changed);
            this.fileSystemWatcher.Created += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_Created);
            this.fileSystemWatcher.Deleted += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_Deleted);
            this.fileSystemWatcher.Renamed += new System.IO.RenamedEventHandler(this.fileSystemWatcher_Renamed);
            // 
            // toolStripContainer2
            // 
            // 
            // toolStripContainer2.ContentPanel
            // 
            this.toolStripContainer2.ContentPanel.AutoScroll = true;
            this.toolStripContainer2.ContentPanel.Size = new System.Drawing.Size(906, 610);
            this.toolStripContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.toolStripContainer2.Location = new System.Drawing.Point(0, 0);
            this.toolStripContainer2.Name = "toolStripContainer2";
            this.toolStripContainer2.Size = new System.Drawing.Size(906, 635);
            this.toolStripContainer2.TabIndex = 1;
            this.toolStripContainer2.Text = "toolStripContainer2";
            // 
            // BottomToolStripPanel
            // 
            this.BottomToolStripPanel.Location = new System.Drawing.Point(0, 0);
            this.BottomToolStripPanel.Name = "BottomToolStripPanel";
            this.BottomToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.BottomToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.BottomToolStripPanel.Size = new System.Drawing.Size(0, 0);
            // 
            // statusStripMain
            // 
            this.statusStripMain.Dock = System.Windows.Forms.DockStyle.None;
            this.statusStripMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabelMain,
            this.toolStripBackgroundTask});
            this.statusStripMain.Location = new System.Drawing.Point(0, 0);
            this.statusStripMain.Name = "statusStripMain";
            this.statusStripMain.Size = new System.Drawing.Size(906, 22);
            this.statusStripMain.TabIndex = 0;
            // 
            // toolStripStatusLabelMain
            // 
            this.toolStripStatusLabelMain.Name = "toolStripStatusLabelMain";
            this.toolStripStatusLabelMain.Size = new System.Drawing.Size(891, 17);
            this.toolStripStatusLabelMain.Spring = true;
            this.toolStripStatusLabelMain.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // toolStripBackgroundTask
            // 
            this.toolStripBackgroundTask.Image = global::OmGui.Properties.Resources.loading;
            this.toolStripBackgroundTask.Name = "toolStripBackgroundTask";
            this.toolStripBackgroundTask.Size = new System.Drawing.Size(16, 17);
            this.toolStripBackgroundTask.Visible = false;
            // 
            // TopToolStripPanel
            // 
            this.TopToolStripPanel.Location = new System.Drawing.Point(0, 0);
            this.TopToolStripPanel.Name = "TopToolStripPanel";
            this.TopToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.TopToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.TopToolStripPanel.Size = new System.Drawing.Size(0, 0);
            // 
            // toolStripMain
            // 
            this.toolStripMain.Dock = System.Windows.Forms.DockStyle.None;
            this.toolStripMain.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStripMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonDownload,
            this.toolStripButtonCancelDownload,
            this.toolStripButtonClear,
            this.toolStripSeparator6,
            this.toolStripButtonInterval,
            this.toolStripButtonStopRecording,
            this.toolStripSeparator8,
            this.devicesToolStripButtonIdentify});
            this.toolStripMain.Location = new System.Drawing.Point(0, 28);
            this.toolStripMain.Name = "toolStripMain";
            this.toolStripMain.RenderMode = System.Windows.Forms.ToolStripRenderMode.System;
            this.toolStripMain.Size = new System.Drawing.Size(906, 27);
            this.toolStripMain.Stretch = true;
            this.toolStripMain.TabIndex = 1;
            this.toolStripMain.ItemClicked += new System.Windows.Forms.ToolStripItemClickedEventHandler(this.toolStripMain_ItemClicked);
            // 
            // toolStripButtonDownload
            // 
            this.toolStripButtonDownload.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonDownload.Image")));
            this.toolStripButtonDownload.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonDownload.Name = "toolStripButtonDownload";
            this.toolStripButtonDownload.Size = new System.Drawing.Size(98, 24);
            this.toolStripButtonDownload.Text = "Download";
            this.toolStripButtonDownload.Click += new System.EventHandler(this.toolStripButtonDownload_Click);
            // 
            // toolStripButtonCancelDownload
            // 
            this.toolStripButtonCancelDownload.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCancelDownload.Image")));
            this.toolStripButtonCancelDownload.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCancelDownload.Name = "toolStripButtonCancelDownload";
            this.toolStripButtonCancelDownload.Size = new System.Drawing.Size(73, 24);
            this.toolStripButtonCancelDownload.Text = "Cancel";
            this.toolStripButtonCancelDownload.ToolTipText = "Cancel Download";
            this.toolStripButtonCancelDownload.Click += new System.EventHandler(this.toolStripButtonCancelDownload_Click);
            // 
            // toolStripButtonClear
            // 
            this.toolStripButtonClear.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonClear.Image")));
            this.toolStripButtonClear.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonClear.Name = "toolStripButtonClear";
            this.toolStripButtonClear.Size = new System.Drawing.Size(63, 24);
            this.toolStripButtonClear.Text = "Clear";
            this.toolStripButtonClear.ToolTipText = "Clear Device";
            this.toolStripButtonClear.Click += new System.EventHandler(this.toolStripButtonClear_Click);
            // 
            // toolStripSeparator6
            // 
            this.toolStripSeparator6.Name = "toolStripSeparator6";
            this.toolStripSeparator6.Size = new System.Drawing.Size(6, 27);
            // 
            // toolStripButtonInterval
            // 
            this.toolStripButtonInterval.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonInterval.Image")));
            this.toolStripButtonInterval.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonInterval.Name = "toolStripButtonInterval";
            this.toolStripButtonInterval.Size = new System.Drawing.Size(85, 24);
            this.toolStripButtonInterval.Text = "Record...";
            this.toolStripButtonInterval.ToolTipText = "Record Interval";
            this.toolStripButtonInterval.Click += new System.EventHandler(this.toolStripButtonRecord_Click);
            // 
            // toolStripButtonStopRecording
            // 
            this.toolStripButtonStopRecording.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonStopRecording.Image")));
            this.toolStripButtonStopRecording.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonStopRecording.Name = "toolStripButtonStopRecording";
            this.toolStripButtonStopRecording.Size = new System.Drawing.Size(132, 24);
            this.toolStripButtonStopRecording.Text = "Stop Recording";
            this.toolStripButtonStopRecording.ToolTipText = "Stop Recording";
            this.toolStripButtonStopRecording.Click += new System.EventHandler(this.toolStripButtonStop_Click);
            // 
            // toolStripSeparator8
            // 
            this.toolStripSeparator8.Name = "toolStripSeparator8";
            this.toolStripSeparator8.Size = new System.Drawing.Size(6, 27);
            // 
            // devicesToolStripButtonIdentify
            // 
            this.devicesToolStripButtonIdentify.Image = ((System.Drawing.Image)(resources.GetObject("devicesToolStripButtonIdentify.Image")));
            this.devicesToolStripButtonIdentify.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.devicesToolStripButtonIdentify.Name = "devicesToolStripButtonIdentify";
            this.devicesToolStripButtonIdentify.Size = new System.Drawing.Size(79, 24);
            this.devicesToolStripButtonIdentify.Text = "Identify";
            this.devicesToolStripButtonIdentify.Click += new System.EventHandler(this.devicesToolStripButtonIdentify_Click);
            // 
            // menuStripMain
            // 
            this.menuStripMain.Dock = System.Windows.Forms.DockStyle.None;
            this.menuStripMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.editToolStripMenuItem,
            this.viewToolStripMenuItem,
            this.toolsToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStripMain.Location = new System.Drawing.Point(0, 0);
            this.menuStripMain.Name = "menuStripMain";
            this.menuStripMain.Size = new System.Drawing.Size(906, 28);
            this.menuStripMain.TabIndex = 0;
            this.menuStripMain.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.workingFolderToolStripMenuItem,
            this.recentFoldersToolStripMenuItem,
            this.toolStripSeparator2,
            this.exportToolStripMenuItem,
            this.toolStripSeparator5,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(44, 24);
            this.fileToolStripMenuItem.Text = "&File";
            // 
            // workingFolderToolStripMenuItem
            // 
            this.workingFolderToolStripMenuItem.Name = "workingFolderToolStripMenuItem";
            this.workingFolderToolStripMenuItem.Size = new System.Drawing.Size(236, 24);
            this.workingFolderToolStripMenuItem.Text = "Switch Working Folder...";
            this.workingFolderToolStripMenuItem.Click += new System.EventHandler(this.workingFolderToolStripMenuItem_Click);
            // 
            // recentFoldersToolStripMenuItem
            // 
            this.recentFoldersToolStripMenuItem.Name = "recentFoldersToolStripMenuItem";
            this.recentFoldersToolStripMenuItem.Size = new System.Drawing.Size(236, 24);
            this.recentFoldersToolStripMenuItem.Text = "Recent Folders";
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(233, 6);
            // 
            // exportToolStripMenuItem
            // 
            this.exportToolStripMenuItem.Name = "exportToolStripMenuItem";
            this.exportToolStripMenuItem.Size = new System.Drawing.Size(236, 24);
            this.exportToolStripMenuItem.Text = "&Export CSV...";
            this.exportToolStripMenuItem.Click += new System.EventHandler(this.exportToolStripMenuItem_Click);
            // 
            // toolStripSeparator5
            // 
            this.toolStripSeparator5.Name = "toolStripSeparator5";
            this.toolStripSeparator5.Size = new System.Drawing.Size(233, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(236, 24);
            this.exitToolStripMenuItem.Text = "E&xit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cutToolStripMenuItem,
            this.copyToolStripMenuItem,
            this.pasteToolStripMenuItem,
            this.toolStripSeparator4,
            this.selectAllToolStripMenuItem});
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(47, 24);
            this.editToolStripMenuItem.Text = "&Edit";
            // 
            // cutToolStripMenuItem
            // 
            this.cutToolStripMenuItem.Enabled = false;
            this.cutToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("cutToolStripMenuItem.Image")));
            this.cutToolStripMenuItem.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.cutToolStripMenuItem.Name = "cutToolStripMenuItem";
            this.cutToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.X)));
            this.cutToolStripMenuItem.Size = new System.Drawing.Size(192, 24);
            this.cutToolStripMenuItem.Text = "Cu&t";
            // 
            // copyToolStripMenuItem
            // 
            this.copyToolStripMenuItem.Enabled = false;
            this.copyToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("copyToolStripMenuItem.Image")));
            this.copyToolStripMenuItem.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.copyToolStripMenuItem.Name = "copyToolStripMenuItem";
            this.copyToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.C)));
            this.copyToolStripMenuItem.Size = new System.Drawing.Size(192, 24);
            this.copyToolStripMenuItem.Text = "&Copy";
            // 
            // pasteToolStripMenuItem
            // 
            this.pasteToolStripMenuItem.Enabled = false;
            this.pasteToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("pasteToolStripMenuItem.Image")));
            this.pasteToolStripMenuItem.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.pasteToolStripMenuItem.Name = "pasteToolStripMenuItem";
            this.pasteToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.V)));
            this.pasteToolStripMenuItem.Size = new System.Drawing.Size(192, 24);
            this.pasteToolStripMenuItem.Text = "&Paste";
            // 
            // toolStripSeparator4
            // 
            this.toolStripSeparator4.Name = "toolStripSeparator4";
            this.toolStripSeparator4.Size = new System.Drawing.Size(189, 6);
            // 
            // selectAllToolStripMenuItem
            // 
            this.selectAllToolStripMenuItem.Name = "selectAllToolStripMenuItem";
            this.selectAllToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.A)));
            this.selectAllToolStripMenuItem.Size = new System.Drawing.Size(192, 24);
            this.selectAllToolStripMenuItem.Text = "Select &All";
            this.selectAllToolStripMenuItem.Click += new System.EventHandler(this.selectAllToolStripMenuItem_Click);
            // 
            // viewToolStripMenuItem
            // 
            this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolbarToolStripMenuItem,
            this.statusBarToolStripMenuItem,
            this.previewToolStripMenuItem,
            this.propertiesToolStripMenuItem,
            this.logToolStripMenuItem});
            this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
            this.viewToolStripMenuItem.Size = new System.Drawing.Size(53, 24);
            this.viewToolStripMenuItem.Text = "&View";
            // 
            // toolbarToolStripMenuItem
            // 
            this.toolbarToolStripMenuItem.Checked = true;
            this.toolbarToolStripMenuItem.CheckOnClick = true;
            this.toolbarToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.toolbarToolStripMenuItem.Name = "toolbarToolStripMenuItem";
            this.toolbarToolStripMenuItem.Size = new System.Drawing.Size(145, 24);
            this.toolbarToolStripMenuItem.Text = "&Toolbar";
            this.toolbarToolStripMenuItem.CheckedChanged += new System.EventHandler(this.View_CheckChanged);
            // 
            // statusBarToolStripMenuItem
            // 
            this.statusBarToolStripMenuItem.Checked = true;
            this.statusBarToolStripMenuItem.CheckOnClick = true;
            this.statusBarToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.statusBarToolStripMenuItem.Name = "statusBarToolStripMenuItem";
            this.statusBarToolStripMenuItem.Size = new System.Drawing.Size(145, 24);
            this.statusBarToolStripMenuItem.Text = "&Status Bar";
            this.statusBarToolStripMenuItem.CheckedChanged += new System.EventHandler(this.View_CheckChanged);
            // 
            // previewToolStripMenuItem
            // 
            this.previewToolStripMenuItem.Checked = true;
            this.previewToolStripMenuItem.CheckOnClick = true;
            this.previewToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.previewToolStripMenuItem.Name = "previewToolStripMenuItem";
            this.previewToolStripMenuItem.Size = new System.Drawing.Size(145, 24);
            this.previewToolStripMenuItem.Text = "Pre&view";
            this.previewToolStripMenuItem.CheckedChanged += new System.EventHandler(this.View_CheckChanged);
            // 
            // propertiesToolStripMenuItem
            // 
            this.propertiesToolStripMenuItem.CheckOnClick = true;
            this.propertiesToolStripMenuItem.Enabled = false;
            this.propertiesToolStripMenuItem.Name = "propertiesToolStripMenuItem";
            this.propertiesToolStripMenuItem.Size = new System.Drawing.Size(145, 24);
            this.propertiesToolStripMenuItem.Text = "&Properties";
            this.propertiesToolStripMenuItem.CheckedChanged += new System.EventHandler(this.View_CheckChanged);
            // 
            // logToolStripMenuItem
            // 
            this.logToolStripMenuItem.CheckOnClick = true;
            this.logToolStripMenuItem.Name = "logToolStripMenuItem";
            this.logToolStripMenuItem.Size = new System.Drawing.Size(145, 24);
            this.logToolStripMenuItem.Text = "&Log";
            this.logToolStripMenuItem.CheckedChanged += new System.EventHandler(this.View_CheckChanged);
            // 
            // toolsToolStripMenuItem
            // 
            this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openCurrentWorkingFolderToolStripMenuItem,
            this.toolStripSeparator1,
            this.optionsToolStripMenuItem});
            this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
            this.toolsToolStripMenuItem.Size = new System.Drawing.Size(57, 24);
            this.toolsToolStripMenuItem.Text = "&Tools";
            // 
            // openCurrentWorkingFolderToolStripMenuItem
            // 
            this.openCurrentWorkingFolderToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("openCurrentWorkingFolderToolStripMenuItem.Image")));
            this.openCurrentWorkingFolderToolStripMenuItem.Name = "openCurrentWorkingFolderToolStripMenuItem";
            this.openCurrentWorkingFolderToolStripMenuItem.Size = new System.Drawing.Size(272, 24);
            this.openCurrentWorkingFolderToolStripMenuItem.Text = "Open &Current Working Folder";
            this.openCurrentWorkingFolderToolStripMenuItem.Click += new System.EventHandler(this.openCurrentWorkingFolderToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(269, 6);
            // 
            // optionsToolStripMenuItem
            // 
            this.optionsToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("optionsToolStripMenuItem.Image")));
            this.optionsToolStripMenuItem.Name = "optionsToolStripMenuItem";
            this.optionsToolStripMenuItem.Size = new System.Drawing.Size(272, 24);
            this.optionsToolStripMenuItem.Text = "&Options";
            this.optionsToolStripMenuItem.Click += new System.EventHandler(this.optionsToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(53, 24);
            this.helpToolStripMenuItem.Text = "&Help";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(128, 24);
            this.aboutToolStripMenuItem.Text = "&About...";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // RightToolStripPanel
            // 
            this.RightToolStripPanel.Location = new System.Drawing.Point(0, 0);
            this.RightToolStripPanel.Name = "RightToolStripPanel";
            this.RightToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.RightToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.RightToolStripPanel.Size = new System.Drawing.Size(0, 0);
            // 
            // LeftToolStripPanel
            // 
            this.LeftToolStripPanel.Location = new System.Drawing.Point(0, 0);
            this.LeftToolStripPanel.Name = "LeftToolStripPanel";
            this.LeftToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.LeftToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.LeftToolStripPanel.Size = new System.Drawing.Size(0, 0);
            // 
            // ContentPanel
            // 
            this.ContentPanel.Margin = new System.Windows.Forms.Padding(2);
            this.ContentPanel.Size = new System.Drawing.Size(906, 582);
            // 
            // splitContainerLog
            // 
            this.splitContainerLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerLog.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainerLog.Location = new System.Drawing.Point(0, 0);
            this.splitContainerLog.Margin = new System.Windows.Forms.Padding(2);
            this.splitContainerLog.Name = "splitContainerLog";
            this.splitContainerLog.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainerLog.Panel1
            // 
            this.splitContainerLog.Panel1.Controls.Add(this.splitContainerPreview);
            // 
            // splitContainerLog.Panel2
            // 
            this.splitContainerLog.Panel2.Controls.Add(this.textBoxLog);
            this.splitContainerLog.Size = new System.Drawing.Size(906, 558);
            this.splitContainerLog.SplitterDistance = 530;
            this.splitContainerLog.SplitterWidth = 3;
            this.splitContainerLog.TabIndex = 0;
            // 
            // splitContainerPreview
            // 
            this.splitContainerPreview.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerPreview.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainerPreview.Location = new System.Drawing.Point(0, 0);
            this.splitContainerPreview.Margin = new System.Windows.Forms.Padding(2);
            this.splitContainerPreview.Name = "splitContainerPreview";
            this.splitContainerPreview.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainerPreview.Panel1
            // 
            this.splitContainerPreview.Panel1.Controls.Add(this.splitContainerDevices);
            // 
            // splitContainerPreview.Panel2
            // 
            this.splitContainerPreview.Panel2.Controls.Add(this.splitContainer1);
            this.splitContainerPreview.Size = new System.Drawing.Size(906, 530);
            this.splitContainerPreview.SplitterDistance = 236;
            this.splitContainerPreview.SplitterWidth = 3;
            this.splitContainerPreview.TabIndex = 0;
            // 
            // splitContainerDevices
            // 
            this.splitContainerDevices.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerDevices.IsSplitterFixed = true;
            this.splitContainerDevices.Location = new System.Drawing.Point(0, 0);
            this.splitContainerDevices.Margin = new System.Windows.Forms.Padding(2);
            this.splitContainerDevices.Name = "splitContainerDevices";
            // 
            // splitContainerDevices.Panel1
            // 
            this.splitContainerDevices.Panel1.Controls.Add(this.devicesListView);
            // 
            // splitContainerDevices.Panel2
            // 
            this.splitContainerDevices.Panel2.Controls.Add(this.propertyGridDevice);
            this.splitContainerDevices.Size = new System.Drawing.Size(906, 236);
            this.splitContainerDevices.SplitterDistance = 642;
            this.splitContainerDevices.SplitterWidth = 3;
            this.splitContainerDevices.TabIndex = 1;
            // 
            // devicesListView
            // 
            this.devicesListView.AllowColumnReorder = true;
            this.devicesListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.devicesListView.FullRowSelect = true;
            this.devicesListView.GridLines = true;
            listViewGroup1.Header = "Devices";
            listViewGroup1.Name = "Other";
            listViewGroup2.Header = "New Data";
            listViewGroup2.Name = "NewData";
            listViewGroup3.Header = "Downloading";
            listViewGroup3.Name = "Downloading";
            listViewGroup4.Header = "Downloaded";
            listViewGroup4.Name = "Downloaded";
            listViewGroup5.Header = "Charging";
            listViewGroup5.Name = "Charging";
            listViewGroup6.Header = "Standby";
            listViewGroup6.Name = "Standby";
            listViewGroup7.Header = "Outbox";
            listViewGroup7.Name = "Outbox";
            listViewGroup8.Header = "Removed";
            listViewGroup8.Name = "Removed";
            listViewGroup9.Header = "Files";
            listViewGroup9.Name = "File";
            this.devicesListView.Groups.AddRange(new System.Windows.Forms.ListViewGroup[] {
            listViewGroup1,
            listViewGroup2,
            listViewGroup3,
            listViewGroup4,
            listViewGroup5,
            listViewGroup6,
            listViewGroup7,
            listViewGroup8,
            listViewGroup9});
            this.devicesListView.HideSelection = false;
            this.devicesListView.LabelWrap = false;
            this.devicesListView.Location = new System.Drawing.Point(0, 0);
            this.devicesListView.Name = "devicesListView";
            this.devicesListView.Size = new System.Drawing.Size(642, 236);
            this.devicesListView.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.devicesListView.TabIndex = 0;
            this.devicesListView.UseCompatibleStateImageBehavior = false;
            this.devicesListView.View = System.Windows.Forms.View.Details;
            this.devicesListView.SelectedIndexChanged += new System.EventHandler(this.listViewDevices_SelectedIndexChanged);
            // 
            // propertyGridDevice
            // 
            this.propertyGridDevice.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propertyGridDevice.Location = new System.Drawing.Point(0, 0);
            this.propertyGridDevice.Margin = new System.Windows.Forms.Padding(2);
            this.propertyGridDevice.Name = "propertyGridDevice";
            this.propertyGridDevice.Size = new System.Drawing.Size(261, 236);
            this.propertyGridDevice.TabIndex = 0;
            this.propertyGridDevice.ToolbarVisible = false;
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.dataViewer);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.tabControlFiles);
            this.splitContainer1.Size = new System.Drawing.Size(906, 291);
            this.splitContainer1.SplitterDistance = 111;
            this.splitContainer1.TabIndex = 2;
            // 
            // dataViewer
            // 
            this.dataViewer.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataViewer.Location = new System.Drawing.Point(1, 2);
            this.dataViewer.Margin = new System.Windows.Forms.Padding(2);
            this.dataViewer.Mode = OmGui.DataViewer.ModeType.Zoom;
            this.dataViewer.Name = "dataViewer";
            this.dataViewer.Reader = null;
            this.dataViewer.Size = new System.Drawing.Size(904, 107);
            this.dataViewer.TabIndex = 3;
            // 
            // tabControlFiles
            // 
            this.tabControlFiles.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControlFiles.Controls.Add(this.tabPageFiles);
            this.tabControlFiles.Controls.Add(this.tabPageQueue);
            this.tabControlFiles.Controls.Add(this.tabPageOutput);
            this.tabControlFiles.Location = new System.Drawing.Point(0, 2);
            this.tabControlFiles.Margin = new System.Windows.Forms.Padding(2);
            this.tabControlFiles.Name = "tabControlFiles";
            this.tabControlFiles.SelectedIndex = 0;
            this.tabControlFiles.Size = new System.Drawing.Size(906, 365);
            this.tabControlFiles.TabIndex = 2;
            this.tabControlFiles.SelectedIndexChanged += new System.EventHandler(this.tabControlFiles_SelectedIndexChanged);
            // 
            // tabPageFiles
            // 
            this.tabPageFiles.Controls.Add(this.filesListView);
            this.tabPageFiles.Controls.Add(this.toolStripFiles);
            this.tabPageFiles.Location = new System.Drawing.Point(4, 25);
            this.tabPageFiles.Margin = new System.Windows.Forms.Padding(2);
            this.tabPageFiles.Name = "tabPageFiles";
            this.tabPageFiles.Padding = new System.Windows.Forms.Padding(2);
            this.tabPageFiles.Size = new System.Drawing.Size(898, 336);
            this.tabPageFiles.TabIndex = 0;
            this.tabPageFiles.Text = "Files";
            this.tabPageFiles.UseVisualStyleBackColor = true;
            // 
            // filesListView
            // 
            this.filesListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.filesListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.nameFilesColumnHeader,
            this.locationFilesColumnHeader,
            this.sizeFilesColumnHeader,
            this.dateModifiedColumnHeader});
            this.filesListView.Location = new System.Drawing.Point(2, 30);
            this.filesListView.Name = "filesListView";
            this.filesListView.Size = new System.Drawing.Size(894, 119);
            this.filesListView.TabIndex = 1;
            this.filesListView.UseCompatibleStateImageBehavior = false;
            this.filesListView.View = System.Windows.Forms.View.Details;
            this.filesListView.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.filesListView_ColumnClick);
            this.filesListView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.filesListView_ItemDrag);
            this.filesListView.SelectedIndexChanged += new System.EventHandler(this.filesListView_SelectedIndexChanged);
            this.filesListView.MouseClick += new System.Windows.Forms.MouseEventHandler(this.filesListView_MouseClick);
            // 
            // nameFilesColumnHeader
            // 
            this.nameFilesColumnHeader.Text = "Name";
            this.nameFilesColumnHeader.Width = 101;
            // 
            // locationFilesColumnHeader
            // 
            this.locationFilesColumnHeader.Text = "File Location";
            this.locationFilesColumnHeader.Width = 125;
            // 
            // sizeFilesColumnHeader
            // 
            this.sizeFilesColumnHeader.Text = "Filesize (MB)";
            this.sizeFilesColumnHeader.Width = 78;
            // 
            // dateModifiedColumnHeader
            // 
            this.dateModifiedColumnHeader.Text = "Date Modified";
            this.dateModifiedColumnHeader.Width = 172;
            // 
            // toolStripFiles
            // 
            this.toolStripFiles.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonShowFiles,
            this.toolStripSeparator3,
            this.pluginsToolStripButton,
            this.DeleteFilesToolStripButton});
            this.toolStripFiles.Location = new System.Drawing.Point(2, 2);
            this.toolStripFiles.Name = "toolStripFiles";
            this.toolStripFiles.Size = new System.Drawing.Size(894, 27);
            this.toolStripFiles.TabIndex = 1;
            this.toolStripFiles.Text = "toolStripFiles";
            // 
            // toolStripButtonShowFiles
            // 
            this.toolStripButtonShowFiles.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.toolStripButtonShowFiles.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonShowFiles.Image")));
            this.toolStripButtonShowFiles.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonShowFiles.Name = "toolStripButtonShowFiles";
            this.toolStripButtonShowFiles.Size = new System.Drawing.Size(104, 24);
            this.toolStripButtonShowFiles.Text = "Show All Files";
            this.toolStripButtonShowFiles.Click += new System.EventHandler(this.toolStripButtonShowFiles_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(6, 27);
            // 
            // pluginsToolStripButton
            // 
            this.pluginsToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("pluginsToolStripButton.Image")));
            this.pluginsToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.pluginsToolStripButton.Name = "pluginsToolStripButton";
            this.pluginsToolStripButton.Size = new System.Drawing.Size(85, 24);
            this.pluginsToolStripButton.Text = "&Plugins...";
            this.pluginsToolStripButton.Click += new System.EventHandler(this.pluginsToolStripButton_Click);
            // 
            // DeleteFilesToolStripButton
            // 
            this.DeleteFilesToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("DeleteFilesToolStripButton.Image")));
            this.DeleteFilesToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.DeleteFilesToolStripButton.Name = "DeleteFilesToolStripButton";
            this.DeleteFilesToolStripButton.Size = new System.Drawing.Size(73, 24);
            this.DeleteFilesToolStripButton.Text = "Delete";
            this.DeleteFilesToolStripButton.Click += new System.EventHandler(this.DeleteFilesToolStripButton_Click_1);
            // 
            // tabPageQueue
            // 
            this.tabPageQueue.Controls.Add(this.toolStripQueue);
            this.tabPageQueue.Controls.Add(this.queueListViewItems2);
            this.tabPageQueue.Location = new System.Drawing.Point(4, 25);
            this.tabPageQueue.Margin = new System.Windows.Forms.Padding(2);
            this.tabPageQueue.Name = "tabPageQueue";
            this.tabPageQueue.Padding = new System.Windows.Forms.Padding(2);
            this.tabPageQueue.Size = new System.Drawing.Size(898, 336);
            this.tabPageQueue.TabIndex = 1;
            this.tabPageQueue.Text = "Queue";
            this.tabPageQueue.UseVisualStyleBackColor = true;
            // 
            // toolStripQueue
            // 
            this.toolStripQueue.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripQueueButtonCancel,
            this.toolStripButtonClearCompleted});
            this.toolStripQueue.Location = new System.Drawing.Point(2, 2);
            this.toolStripQueue.Name = "toolStripQueue";
            this.toolStripQueue.Size = new System.Drawing.Size(894, 27);
            this.toolStripQueue.TabIndex = 1;
            this.toolStripQueue.Text = "Clear All";
            // 
            // toolStripQueueButtonCancel
            // 
            this.toolStripQueueButtonCancel.Enabled = false;
            this.toolStripQueueButtonCancel.Image = global::OmGui.Properties.Resources.DeleteHS;
            this.toolStripQueueButtonCancel.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripQueueButtonCancel.Name = "toolStripQueueButtonCancel";
            this.toolStripQueueButtonCancel.Size = new System.Drawing.Size(73, 24);
            this.toolStripQueueButtonCancel.Text = "Cancel";
            this.toolStripQueueButtonCancel.Click += new System.EventHandler(this.toolStripQueueButtonCancel_Click);
            // 
            // toolStripButtonClearCompleted
            // 
            this.toolStripButtonClearCompleted.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonClearCompleted.Image")));
            this.toolStripButtonClearCompleted.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonClearCompleted.Name = "toolStripButtonClearCompleted";
            this.toolStripButtonClearCompleted.Size = new System.Drawing.Size(121, 24);
            this.toolStripButtonClearCompleted.Text = "Clear Finished";
            this.toolStripButtonClearCompleted.Click += new System.EventHandler(this.toolStripButtonClearCompleted_Click);
            // 
            // queueListViewItems2
            // 
            this.queueListViewItems2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.queueListViewItems2.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colName,
            this.colFileName,
            this.colProgress});
            this.queueListViewItems2.FullRowSelect = true;
            this.queueListViewItems2.Location = new System.Drawing.Point(2, 28);
            this.queueListViewItems2.Name = "queueListViewItems2";
            this.queueListViewItems2.Size = new System.Drawing.Size(897, 121);
            this.queueListViewItems2.TabIndex = 0;
            this.queueListViewItems2.UseCompatibleStateImageBehavior = false;
            this.queueListViewItems2.View = System.Windows.Forms.View.Details;
            this.queueListViewItems2.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.queueListViewItems2_ColumnClick);
            this.queueListViewItems2.SelectedIndexChanged += new System.EventHandler(this.queueListViewItems2_SelectedIndexChanged);
            // 
            // colName
            // 
            this.colName.Text = "Plugin Name";
            this.colName.Width = 91;
            // 
            // colFileName
            // 
            this.colFileName.Text = "Filenames (.cwa)";
            this.colFileName.Width = 105;
            // 
            // colProgress
            // 
            this.colProgress.Text = "Progress (%)";
            this.colProgress.Width = 82;
            // 
            // tabPageOutput
            // 
            this.tabPageOutput.Controls.Add(this.toolStrip1);
            this.tabPageOutput.Controls.Add(this.outputListView);
            this.tabPageOutput.Location = new System.Drawing.Point(4, 25);
            this.tabPageOutput.Name = "tabPageOutput";
            this.tabPageOutput.Size = new System.Drawing.Size(898, 336);
            this.tabPageOutput.TabIndex = 2;
            this.tabPageOutput.Text = "Output";
            this.tabPageOutput.UseVisualStyleBackColor = true;
            // 
            // toolStrip1
            // 
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(898, 25);
            this.toolStrip1.TabIndex = 1;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // outputListView
            // 
            this.outputListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.outputListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderFileName,
            this.columnHeaderFilePath,
            this.columnHeaderSize,
            this.columnHeaderDateModified});
            this.outputListView.FullRowSelect = true;
            this.outputListView.Location = new System.Drawing.Point(0, 28);
            this.outputListView.Name = "outputListView";
            this.outputListView.Size = new System.Drawing.Size(898, 121);
            this.outputListView.TabIndex = 0;
            this.outputListView.UseCompatibleStateImageBehavior = false;
            this.outputListView.View = System.Windows.Forms.View.Details;
            this.outputListView.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.outputListView_ColumnClick);
            this.outputListView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.outputListView_ItemDrag);
            this.outputListView.DoubleClick += new System.EventHandler(this.outputListView_DoubleClick);
            this.outputListView.MouseClick += new System.Windows.Forms.MouseEventHandler(this.outputListView_MouseClick);
            // 
            // columnHeaderFileName
            // 
            this.columnHeaderFileName.Text = "File Name";
            this.columnHeaderFileName.Width = 98;
            // 
            // columnHeaderFilePath
            // 
            this.columnHeaderFilePath.Text = "File Location";
            this.columnHeaderFilePath.Width = 120;
            // 
            // columnHeaderSize
            // 
            this.columnHeaderSize.Text = "File Size (MB)";
            this.columnHeaderSize.Width = 86;
            // 
            // columnHeaderDateModified
            // 
            this.columnHeaderDateModified.Text = "Date Modified";
            this.columnHeaderDateModified.Width = 122;
            // 
            // textBoxLog
            // 
            this.textBoxLog.BackColor = System.Drawing.SystemColors.Window;
            this.textBoxLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBoxLog.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxLog.Location = new System.Drawing.Point(0, 0);
            this.textBoxLog.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ReadOnly = true;
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxLog.Size = new System.Drawing.Size(906, 25);
            this.textBoxLog.TabIndex = 0;
            // 
            // toolStripContainer1
            // 
            // 
            // toolStripContainer1.BottomToolStripPanel
            // 
            this.toolStripContainer1.BottomToolStripPanel.Controls.Add(this.statusStripMain);
            // 
            // toolStripContainer1.ContentPanel
            // 
            this.toolStripContainer1.ContentPanel.Controls.Add(this.splitContainerLog);
            this.toolStripContainer1.ContentPanel.Margin = new System.Windows.Forms.Padding(2);
            this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(906, 558);
            this.toolStripContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.toolStripContainer1.Location = new System.Drawing.Point(0, 0);
            this.toolStripContainer1.Margin = new System.Windows.Forms.Padding(2);
            this.toolStripContainer1.Name = "toolStripContainer1";
            this.toolStripContainer1.Size = new System.Drawing.Size(906, 635);
            this.toolStripContainer1.TabIndex = 0;
            this.toolStripContainer1.Text = "toolStripContainer1";
            // 
            // toolStripContainer1.TopToolStripPanel
            // 
            this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.menuStripMain);
            this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStripMain);
            // 
            // fileSystemWatcherOutput
            // 
            this.fileSystemWatcherOutput.EnableRaisingEvents = true;
            this.fileSystemWatcherOutput.IncludeSubdirectories = true;
            this.fileSystemWatcherOutput.SynchronizingObject = this;
            this.fileSystemWatcherOutput.Created += new System.IO.FileSystemEventHandler(this.fileSystemWatcherOutput_Created);
            this.fileSystemWatcherOutput.Deleted += new System.IO.FileSystemEventHandler(this.fileSystemWatcherOutput_Deleted);
            this.fileSystemWatcherOutput.Renamed += new System.IO.RenamedEventHandler(this.fileSystemWatcherOutput_Renamed);
            // 
            // backgroundWorkerUpdate
            // 
            this.backgroundWorkerUpdate.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorkerUpdate_DoWork);
            // 
            // MainForm
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(906, 635);
            this.Controls.Add(this.toolStripContainer1);
            this.Controls.Add(this.toolStripContainer2);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStripMain;
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "MainForm";
            this.Text = " ";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
            this.Load += new System.EventHandler(this.MainForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.fileSystemWatcher)).EndInit();
            this.toolStripContainer2.ResumeLayout(false);
            this.toolStripContainer2.PerformLayout();
            this.statusStripMain.ResumeLayout(false);
            this.statusStripMain.PerformLayout();
            this.toolStripMain.ResumeLayout(false);
            this.toolStripMain.PerformLayout();
            this.menuStripMain.ResumeLayout(false);
            this.menuStripMain.PerformLayout();
            this.splitContainerLog.Panel1.ResumeLayout(false);
            this.splitContainerLog.Panel2.ResumeLayout(false);
            this.splitContainerLog.Panel2.PerformLayout();
            this.splitContainerLog.ResumeLayout(false);
            this.splitContainerPreview.Panel1.ResumeLayout(false);
            this.splitContainerPreview.Panel2.ResumeLayout(false);
            this.splitContainerPreview.ResumeLayout(false);
            this.splitContainerDevices.Panel1.ResumeLayout(false);
            this.splitContainerDevices.Panel2.ResumeLayout(false);
            this.splitContainerDevices.ResumeLayout(false);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.tabControlFiles.ResumeLayout(false);
            this.tabPageFiles.ResumeLayout(false);
            this.tabPageFiles.PerformLayout();
            this.toolStripFiles.ResumeLayout(false);
            this.toolStripFiles.PerformLayout();
            this.tabPageQueue.ResumeLayout(false);
            this.tabPageQueue.PerformLayout();
            this.toolStripQueue.ResumeLayout(false);
            this.toolStripQueue.PerformLayout();
            this.tabPageOutput.ResumeLayout(false);
            this.tabPageOutput.PerformLayout();
            this.toolStripContainer1.BottomToolStripPanel.ResumeLayout(false);
            this.toolStripContainer1.BottomToolStripPanel.PerformLayout();
            this.toolStripContainer1.ContentPanel.ResumeLayout(false);
            this.toolStripContainer1.TopToolStripPanel.ResumeLayout(false);
            this.toolStripContainer1.TopToolStripPanel.PerformLayout();
            this.toolStripContainer1.ResumeLayout(false);
            this.toolStripContainer1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fileSystemWatcherOutput)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Timer refreshTimer;
        private System.IO.FileSystemWatcher fileSystemWatcher;
        private System.Windows.Forms.ToolStripContainer toolStripContainer1;
        private System.Windows.Forms.StatusStrip statusStripMain;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabelMain;
        private System.Windows.Forms.SplitContainer splitContainerLog;
        private System.Windows.Forms.SplitContainer splitContainerPreview;
        private System.Windows.Forms.SplitContainer splitContainerDevices;
        private System.Windows.Forms.PropertyGrid propertyGridDevice;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.ToolStrip toolStripFiles;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.MenuStrip menuStripMain;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem workingFolderToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem cutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copyToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
        private System.Windows.Forms.ToolStripMenuItem selectAllToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolbarToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem statusBarToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem previewToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem propertiesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem logToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openCurrentWorkingFolderToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem optionsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStrip toolStripMain;
        private System.Windows.Forms.ToolStripButton toolStripButtonDownload;
        private System.Windows.Forms.ToolStripButton toolStripButtonCancelDownload;
        private System.Windows.Forms.ToolStripButton toolStripButtonClear;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator6;
        private System.Windows.Forms.ToolStripButton toolStripButtonInterval;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator8;
        private System.Windows.Forms.ToolStripContainer toolStripContainer2;
        private System.Windows.Forms.ToolStripButton pluginsToolStripButton;
        private System.Windows.Forms.ToolStripButton DeleteFilesToolStripButton;
        private System.Windows.Forms.ToolStripButton devicesToolStripButtonIdentify;
        private System.Windows.Forms.ToolStripButton toolStripButtonStopRecording;
        private System.Windows.Forms.ListView filesListView;
        private System.Windows.Forms.ColumnHeader nameFilesColumnHeader;
        private System.Windows.Forms.ColumnHeader locationFilesColumnHeader;
        private System.Windows.Forms.ColumnHeader sizeFilesColumnHeader;
        private System.Windows.Forms.ColumnHeader dateModifiedColumnHeader;
        private DeviceListView devicesListView;
        private System.Windows.Forms.TabControl tabControlFiles;
        private System.Windows.Forms.TabPage tabPageFiles;
        private System.Windows.Forms.TabPage tabPageQueue;
        private System.Windows.Forms.ListView queueListViewItems2;
        private System.Windows.Forms.ColumnHeader colName;
        private System.Windows.Forms.ColumnHeader colFileName;
        private System.Windows.Forms.ColumnHeader colProgress;
        private System.Windows.Forms.ToolStrip toolStripQueue;
        private System.Windows.Forms.ToolStripPanel BottomToolStripPanel;
        private System.Windows.Forms.ToolStripPanel TopToolStripPanel;
        private System.Windows.Forms.ToolStripPanel RightToolStripPanel;
        private System.Windows.Forms.ToolStripPanel LeftToolStripPanel;
        private System.Windows.Forms.ToolStripContentPanel ContentPanel;
        private System.Windows.Forms.ToolStripButton toolStripQueueButtonCancel;
        private System.Windows.Forms.TabPage tabPageOutput;
        private System.IO.FileSystemWatcher fileSystemWatcherOutput;
        private System.Windows.Forms.ListView outputListView;
        private System.Windows.Forms.ColumnHeader columnHeaderFileName;
        private System.Windows.Forms.ColumnHeader columnHeaderFilePath;
        private System.Windows.Forms.ColumnHeader columnHeaderSize;
        private System.Windows.Forms.ColumnHeader columnHeaderDateModified;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton toolStripButtonShowFiles;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripButton toolStripButtonClearCompleted;
        private DataViewer dataViewer;
        private System.Windows.Forms.ToolStripMenuItem recentFoldersToolStripMenuItem;
        private System.ComponentModel.BackgroundWorker backgroundWorkerUpdate;
        private System.Windows.Forms.ToolStripStatusLabel toolStripBackgroundTask;
        private System.Windows.Forms.ToolStripMenuItem exportToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator5;
    }
}


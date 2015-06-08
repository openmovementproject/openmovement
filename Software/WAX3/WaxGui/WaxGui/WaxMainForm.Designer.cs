namespace WaxGui
{
    partial class WaxMainForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(WaxMainForm));
            this.notifyIcon = new System.Windows.Forms.NotifyIcon(this.components);
            this.toolStrip = new System.Windows.Forms.ToolStrip();
            this.toolStripLabelPort = new System.Windows.Forms.ToolStripLabel();
            this.toolStripComboBoxPort = new System.Windows.Forms.ToolStripComboBox();
            this.checkBoxBroadcast = new System.Windows.Forms.ToolStripButton();
            this.checkBoxSetMode = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabelFile = new System.Windows.Forms.ToolStripLabel();
            this.toolStripTextBoxLogFile = new System.Windows.Forms.ToolStripTextBox();
            this.toolStripButtonChooseFile = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonConnect = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonSlow = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonAlt = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripTextBoxDebug = new System.Windows.Forms.ToolStripTextBox();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.timer = new System.Windows.Forms.Timer(this.components);
            this.historyPanel = new WaxGui.HistoryPanel();
            this.toolStrip.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // notifyIcon
            // 
            this.notifyIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("notifyIcon.Icon")));
            this.notifyIcon.Text = "WAX Receiver";
            this.notifyIcon.Click += new System.EventHandler(this.notifyIcon_Click);
            // 
            // toolStrip
            // 
            this.toolStrip.CanOverflow = false;
            this.toolStrip.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabelPort,
            this.toolStripComboBoxPort,
            this.checkBoxBroadcast,
            this.checkBoxSetMode,
            this.toolStripSeparator1,
            this.toolStripLabelFile,
            this.toolStripTextBoxLogFile,
            this.toolStripButtonChooseFile,
            this.toolStripSeparator2,
            this.toolStripButtonConnect,
            this.toolStripButtonSlow,
            this.toolStripButtonAlt,
            this.toolStripSeparator3,
            this.toolStripTextBoxDebug});
            this.toolStrip.Location = new System.Drawing.Point(0, 0);
            this.toolStrip.Name = "toolStrip";
            this.toolStrip.RenderMode = System.Windows.Forms.ToolStripRenderMode.System;
            this.toolStrip.Size = new System.Drawing.Size(1272, 27);
            this.toolStrip.Stretch = true;
            this.toolStrip.TabIndex = 1;
            this.toolStrip.Text = "toolStrip";
            // 
            // toolStripLabelPort
            // 
            this.toolStripLabelPort.Name = "toolStripLabelPort";
            this.toolStripLabelPort.Size = new System.Drawing.Size(32, 24);
            this.toolStripLabelPort.Text = "Port:";
            // 
            // toolStripComboBoxPort
            // 
            this.toolStripComboBoxPort.Name = "toolStripComboBoxPort";
            this.toolStripComboBoxPort.Size = new System.Drawing.Size(108, 27);
            // 
            // checkBoxBroadcast
            // 
            this.checkBoxBroadcast.CheckOnClick = true;
            this.checkBoxBroadcast.Image = ((System.Drawing.Image)(resources.GetObject("checkBoxBroadcast.Image")));
            this.checkBoxBroadcast.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.checkBoxBroadcast.Name = "checkBoxBroadcast";
            this.checkBoxBroadcast.Size = new System.Drawing.Size(79, 24);
            this.checkBoxBroadcast.Text = "Broadcast";
            // 
            // checkBoxSetMode
            // 
            this.checkBoxSetMode.Checked = true;
            this.checkBoxSetMode.CheckOnClick = true;
            this.checkBoxSetMode.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxSetMode.Image = ((System.Drawing.Image)(resources.GetObject("checkBoxSetMode.Image")));
            this.checkBoxSetMode.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.checkBoxSetMode.Name = "checkBoxSetMode";
            this.checkBoxSetMode.Size = new System.Drawing.Size(77, 24);
            this.checkBoxSetMode.Text = "Set Mode";
            this.checkBoxSetMode.Click += new System.EventHandler(this.checkBoxSetMode_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 27);
            // 
            // toolStripLabelFile
            // 
            this.toolStripLabelFile.Name = "toolStripLabelFile";
            this.toolStripLabelFile.Size = new System.Drawing.Size(49, 24);
            this.toolStripLabelFile.Text = "Log file:";
            // 
            // toolStripTextBoxLogFile
            // 
            this.toolStripTextBoxLogFile.Name = "toolStripTextBoxLogFile";
            this.toolStripTextBoxLogFile.Size = new System.Drawing.Size(445, 27);
            this.toolStripTextBoxLogFile.Leave += new System.EventHandler(this.toolStripTextBoxLogFile_Leave);
            this.toolStripTextBoxLogFile.TextChanged += new System.EventHandler(this.toolStripTextBoxLogFile_TextChanged);
            // 
            // toolStripButtonChooseFile
            // 
            this.toolStripButtonChooseFile.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.toolStripButtonChooseFile.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonChooseFile.Image")));
            this.toolStripButtonChooseFile.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonChooseFile.Name = "toolStripButtonChooseFile";
            this.toolStripButtonChooseFile.Size = new System.Drawing.Size(23, 24);
            this.toolStripButtonChooseFile.Text = "...";
            this.toolStripButtonChooseFile.Click += new System.EventHandler(this.toolStripButtonChooseFile_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 27);
            // 
            // toolStripButtonConnect
            // 
            this.toolStripButtonConnect.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonConnect.Image")));
            this.toolStripButtonConnect.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonConnect.Name = "toolStripButtonConnect";
            this.toolStripButtonConnect.Size = new System.Drawing.Size(118, 24);
            this.toolStripButtonConnect.Text = "&Connect and Log";
            this.toolStripButtonConnect.Click += new System.EventHandler(this.toolStripButtonConnect_Click);
            // 
            // toolStripButtonSlow
            // 
            this.toolStripButtonSlow.CheckOnClick = true;
            this.toolStripButtonSlow.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.toolStripButtonSlow.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonSlow.Image")));
            this.toolStripButtonSlow.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonSlow.Name = "toolStripButtonSlow";
            this.toolStripButtonSlow.Size = new System.Drawing.Size(36, 24);
            this.toolStripButtonSlow.Text = "Slow";
            this.toolStripButtonSlow.Click += new System.EventHandler(this.toolStripButtonSlow_Click);
            // 
            // toolStripButtonAlt
            // 
            this.toolStripButtonAlt.CheckOnClick = true;
            this.toolStripButtonAlt.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonAlt.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonAlt.Image")));
            this.toolStripButtonAlt.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonAlt.Name = "toolStripButtonAlt";
            this.toolStripButtonAlt.Size = new System.Drawing.Size(23, 24);
            this.toolStripButtonAlt.Text = "Alt.";
            this.toolStripButtonAlt.Click += new System.EventHandler(this.toolStripButtonAlt_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(6, 27);
            // 
            // toolStripTextBoxDebug
            // 
            this.toolStripTextBoxDebug.AutoSize = false;
            this.toolStripTextBoxDebug.Name = "toolStripTextBoxDebug";
            this.toolStripTextBoxDebug.ReadOnly = true;
            this.toolStripTextBoxDebug.Size = new System.Drawing.Size(356, 27);
            this.toolStripTextBoxDebug.Leave += new System.EventHandler(this.toolStripTextBoxLogFile_Leave);
            this.toolStripTextBoxDebug.TextChanged += new System.EventHandler(this.toolStripTextBoxLogFile_TextChanged);
            // 
            // saveFileDialog
            // 
            this.saveFileDialog.Filter = "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*";
            this.saveFileDialog.OverwritePrompt = false;
            // 
            // statusStrip
            // 
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel});
            this.statusStrip.Location = new System.Drawing.Point(0, 332);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Padding = new System.Windows.Forms.Padding(1, 0, 12, 0);
            this.statusStrip.Size = new System.Drawing.Size(1272, 22);
            this.statusStrip.TabIndex = 2;
            this.statusStrip.Text = "statusStrip1";
            // 
            // toolStripStatusLabel
            // 
            this.toolStripStatusLabel.Name = "toolStripStatusLabel";
            this.toolStripStatusLabel.Size = new System.Drawing.Size(1259, 17);
            this.toolStripStatusLabel.Spring = true;
            this.toolStripStatusLabel.Text = "-";
            this.toolStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // timer
            // 
            this.timer.Enabled = true;
            this.timer.Interval = 200;
            this.timer.Tick += new System.EventHandler(this.timer_Tick);
            // 
            // historyPanel
            // 
            this.historyPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.historyPanel.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.historyPanel.BackColor = System.Drawing.Color.Black;
            this.historyPanel.Location = new System.Drawing.Point(0, 32);
            this.historyPanel.Margin = new System.Windows.Forms.Padding(4);
            this.historyPanel.Name = "historyPanel";
            this.historyPanel.Size = new System.Drawing.Size(1272, 293);
            this.historyPanel.TabIndex = 0;
            this.historyPanel.WaxManager = null;
            // 
            // WaxMainForm
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(1272, 354);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.toolStrip);
            this.Controls.Add(this.historyPanel);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "WaxMainForm";
            this.Text = "WAX Receiver";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.WaxMainForm_FormClosed);
            this.Load += new System.EventHandler(this.WaxMainForm_Load);
            this.Resize += new System.EventHandler(this.WaxMainForm_Resize);
            this.toolStrip.ResumeLayout(false);
            this.toolStrip.PerformLayout();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private HistoryPanel historyPanel;
        private System.Windows.Forms.NotifyIcon notifyIcon;
        private System.Windows.Forms.ToolStrip toolStrip;
        private System.Windows.Forms.ToolStripButton toolStripButtonConnect;
        private System.Windows.Forms.ToolStripLabel toolStripLabelPort;
        private System.Windows.Forms.ToolStripComboBox toolStripComboBoxPort;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripLabel toolStripLabelFile;
        private System.Windows.Forms.ToolStripTextBox toolStripTextBoxLogFile;
        private System.Windows.Forms.ToolStripButton toolStripButtonChooseFile;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
        private System.Windows.Forms.Timer timer;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripTextBox toolStripTextBoxDebug;
        private System.Windows.Forms.ToolStripButton checkBoxBroadcast;
        private System.Windows.Forms.ToolStripButton checkBoxSetMode;
        private System.Windows.Forms.ToolStripButton toolStripButtonSlow;
        private System.Windows.Forms.ToolStripButton toolStripButtonAlt;
    }
}


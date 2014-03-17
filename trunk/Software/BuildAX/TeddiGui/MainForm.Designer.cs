namespace TeddiGui
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.toolStripMain = new System.Windows.Forms.ToolStrip();
            this.toolStripLabelPort = new System.Windows.Forms.ToolStripLabel();
            this.toolStripComboBoxPort = new System.Windows.Forms.ToolStripComboBox();
            this.toolStripButtonConnect = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabelVisualizer = new System.Windows.Forms.ToolStripLabel();
            this.toolStripButtonGraph = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonMap = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonOpen = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonSave = new System.Windows.Forms.ToolStripButton();
            this.splitContainerLog = new System.Windows.Forms.SplitContainer();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.toolStripMain.SuspendLayout();
            this.splitContainerLog.Panel2.SuspendLayout();
            this.splitContainerLog.SuspendLayout();
            this.SuspendLayout();
            // 
            // toolStripMain
            // 
            this.toolStripMain.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStripMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabelPort,
            this.toolStripComboBoxPort,
            this.toolStripButtonConnect,
            this.toolStripSeparator1,
            this.toolStripLabelVisualizer,
            this.toolStripButtonGraph,
            this.toolStripButtonMap,
            this.toolStripSeparator2,
            this.toolStripButtonOpen,
            this.toolStripButtonSave});
            this.toolStripMain.Location = new System.Drawing.Point(0, 0);
            this.toolStripMain.Name = "toolStripMain";
            this.toolStripMain.RenderMode = System.Windows.Forms.ToolStripRenderMode.System;
            this.toolStripMain.Size = new System.Drawing.Size(1024, 28);
            this.toolStripMain.TabIndex = 0;
            this.toolStripMain.Text = "toolStripMain";
            // 
            // toolStripLabelPort
            // 
            this.toolStripLabelPort.Name = "toolStripLabelPort";
            this.toolStripLabelPort.Size = new System.Drawing.Size(39, 25);
            this.toolStripLabelPort.Text = "Port:";
            // 
            // toolStripComboBoxPort
            // 
            this.toolStripComboBoxPort.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.toolStripComboBoxPort.Name = "toolStripComboBoxPort";
            this.toolStripComboBoxPort.Size = new System.Drawing.Size(160, 28);
            // 
            // toolStripButtonConnect
            // 
            this.toolStripButtonConnect.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonConnect.Image")));
            this.toolStripButtonConnect.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonConnect.Name = "toolStripButtonConnect";
            this.toolStripButtonConnect.Size = new System.Drawing.Size(83, 25);
            this.toolStripButtonConnect.Text = "Connect";
            this.toolStripButtonConnect.Click += new System.EventHandler(this.toolStripButtonConnect_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 28);
            // 
            // toolStripLabelVisualizer
            // 
            this.toolStripLabelVisualizer.Name = "toolStripLabelVisualizer";
            this.toolStripLabelVisualizer.Size = new System.Drawing.Size(75, 25);
            this.toolStripLabelVisualizer.Text = "Visualizer:";
            // 
            // toolStripButtonGraph
            // 
            this.toolStripButtonGraph.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonGraph.Image")));
            this.toolStripButtonGraph.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonGraph.Name = "toolStripButtonGraph";
            this.toolStripButtonGraph.Size = new System.Drawing.Size(69, 25);
            this.toolStripButtonGraph.Text = "Graph";
            this.toolStripButtonGraph.Click += new System.EventHandler(this.toolStripButtonGraph_Click);
            // 
            // toolStripButtonMap
            // 
            this.toolStripButtonMap.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonMap.Image")));
            this.toolStripButtonMap.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonMap.Name = "toolStripButtonMap";
            this.toolStripButtonMap.Size = new System.Drawing.Size(59, 25);
            this.toolStripButtonMap.Text = "Map";
            this.toolStripButtonMap.Click += new System.EventHandler(this.toolStripButtonMap_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 28);
            // 
            // toolStripButtonOpen
            // 
            this.toolStripButtonOpen.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonOpen.Image")));
            this.toolStripButtonOpen.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonOpen.Name = "toolStripButtonOpen";
            this.toolStripButtonOpen.Size = new System.Drawing.Size(74, 25);
            this.toolStripButtonOpen.Text = "&Open...";
            this.toolStripButtonOpen.Click += new System.EventHandler(this.toolStripButtonOpen_Click);
            // 
            // toolStripButtonSave
            // 
            this.toolStripButtonSave.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonSave.Image")));
            this.toolStripButtonSave.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonSave.Name = "toolStripButtonSave";
            this.toolStripButtonSave.Size = new System.Drawing.Size(69, 25);
            this.toolStripButtonSave.Text = "&Save...";
            this.toolStripButtonSave.Click += new System.EventHandler(this.toolStripButtonSave_Click);
            // 
            // splitContainerLog
            // 
            this.splitContainerLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerLog.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainerLog.Location = new System.Drawing.Point(0, 28);
            this.splitContainerLog.Margin = new System.Windows.Forms.Padding(4);
            this.splitContainerLog.Name = "splitContainerLog";
            this.splitContainerLog.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainerLog.Panel2
            // 
            this.splitContainerLog.Panel2.Controls.Add(this.textBoxLog);
            this.splitContainerLog.Size = new System.Drawing.Size(1024, 798);
            this.splitContainerLog.SplitterDistance = 761;
            this.splitContainerLog.SplitterWidth = 5;
            this.splitContainerLog.TabIndex = 1;
            // 
            // textBoxLog
            // 
            this.textBoxLog.BackColor = System.Drawing.SystemColors.Window;
            this.textBoxLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBoxLog.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxLog.Location = new System.Drawing.Point(0, 0);
            this.textBoxLog.Margin = new System.Windows.Forms.Padding(4);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ReadOnly = true;
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxLog.Size = new System.Drawing.Size(1024, 32);
            this.textBoxLog.TabIndex = 0;
            // 
            // saveFileDialog
            // 
            this.saveFileDialog.Filter = "CSV Files (*.csv)|*.csv|All Files|*.*";
            // 
            // openFileDialog
            // 
            this.openFileDialog.Filter = "CSV Files (*.csv)|*.csv|All Files|*.*";
            // 
            // MainForm
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(1024, 826);
            this.Controls.Add(this.splitContainerLog);
            this.Controls.Add(this.toolStripMain);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "MainForm";
            this.Text = "BuildAX Monitor";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.toolStripMain.ResumeLayout(false);
            this.toolStripMain.PerformLayout();
            this.splitContainerLog.Panel2.ResumeLayout(false);
            this.splitContainerLog.Panel2.PerformLayout();
            this.splitContainerLog.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ToolStrip toolStripMain;
        private System.Windows.Forms.ToolStripLabel toolStripLabelPort;
        private System.Windows.Forms.ToolStripComboBox toolStripComboBoxPort;
        private System.Windows.Forms.ToolStripButton toolStripButtonConnect;
        private System.Windows.Forms.SplitContainer splitContainerLog;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripLabel toolStripLabelVisualizer;
        private System.Windows.Forms.ToolStripButton toolStripButtonGraph;
        private System.Windows.Forms.ToolStripButton toolStripButtonMap;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripButton toolStripButtonOpen;
        private System.Windows.Forms.ToolStripButton toolStripButtonSave;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
    }
}


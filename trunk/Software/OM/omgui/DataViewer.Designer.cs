namespace OmGui
{
    partial class DataViewer
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DataViewer));
            this.groupBoxOptions = new System.Windows.Forms.GroupBox();
            this.toolStripData = new System.Windows.Forms.ToolStrip();
            this.toolStripButtonZoom = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonSelection = new System.Windows.Forms.ToolStripButton();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.checkBoxX = new System.Windows.Forms.CheckBox();
            this.checkBoxY = new System.Windows.Forms.CheckBox();
            this.checkBoxZ = new System.Windows.Forms.CheckBox();
            this.checkBoxAccel = new System.Windows.Forms.CheckBox();
            this.checkBoxLight = new System.Windows.Forms.CheckBox();
            this.checkBoxTemp = new System.Windows.Forms.CheckBox();
            this.checkBoxBatt = new System.Windows.Forms.CheckBox();
            this.graphPanel = new OmGui.GraphPanel();
            this.timerAnimate = new System.Windows.Forms.Timer(this.components);
            this.groupBoxOptions.SuspendLayout();
            this.toolStripData.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBoxOptions
            // 
            this.groupBoxOptions.Controls.Add(this.toolStripData);
            this.groupBoxOptions.Controls.Add(this.tableLayoutPanel1);
            this.groupBoxOptions.Dock = System.Windows.Forms.DockStyle.Right;
            this.groupBoxOptions.Location = new System.Drawing.Point(858, 0);
            this.groupBoxOptions.Name = "groupBoxOptions";
            this.groupBoxOptions.Size = new System.Drawing.Size(106, 236);
            this.groupBoxOptions.TabIndex = 0;
            this.groupBoxOptions.TabStop = false;
            this.groupBoxOptions.Text = "Options";
            // 
            // toolStripData
            // 
            this.toolStripData.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStripData.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonZoom,
            this.toolStripButtonSelection});
            this.toolStripData.Location = new System.Drawing.Point(3, 18);
            this.toolStripData.Name = "toolStripData";
            this.toolStripData.RenderMode = System.Windows.Forms.ToolStripRenderMode.System;
            this.toolStripData.Size = new System.Drawing.Size(100, 25);
            this.toolStripData.TabIndex = 1;
            this.toolStripData.Text = "toolStripData";
            // 
            // toolStripButtonZoom
            // 
            this.toolStripButtonZoom.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonZoom.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonZoom.Image")));
            this.toolStripButtonZoom.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonZoom.Name = "toolStripButtonZoom";
            this.toolStripButtonZoom.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonZoom.Text = "Zoom";
            this.toolStripButtonZoom.Click += new System.EventHandler(this.toolStripButtonZoom_Click);
            // 
            // toolStripButtonSelection
            // 
            this.toolStripButtonSelection.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonSelection.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonSelection.Image")));
            this.toolStripButtonSelection.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonSelection.Name = "toolStripButtonSelection";
            this.toolStripButtonSelection.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonSelection.Text = "Selection";
            this.toolStripButtonSelection.Click += new System.EventHandler(this.toolStripButtonSelection_Click);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.checkBoxX, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxY, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxZ, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxAccel, 0, 3);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxLight, 0, 4);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxTemp, 0, 5);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxBatt, 0, 6);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(6, 50);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 7;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 14.28571F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 14.28571F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 14.28571F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 14.28571F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 14.28571F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 14.28571F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 14.28571F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(94, 178);
            this.tableLayoutPanel1.TabIndex = 0;
            // 
            // checkBoxX
            // 
            this.checkBoxX.AutoSize = true;
            this.checkBoxX.Checked = true;
            this.checkBoxX.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxX.Location = new System.Drawing.Point(3, 3);
            this.checkBoxX.Name = "checkBoxX";
            this.checkBoxX.Size = new System.Drawing.Size(69, 19);
            this.checkBoxX.TabIndex = 0;
            this.checkBoxX.Text = "X-Axis";
            this.checkBoxX.UseVisualStyleBackColor = true;
            this.checkBoxX.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBoxY
            // 
            this.checkBoxY.AutoSize = true;
            this.checkBoxY.Checked = true;
            this.checkBoxY.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxY.Location = new System.Drawing.Point(3, 28);
            this.checkBoxY.Name = "checkBoxY";
            this.checkBoxY.Size = new System.Drawing.Size(69, 19);
            this.checkBoxY.TabIndex = 0;
            this.checkBoxY.Text = "Y-Axis";
            this.checkBoxY.UseVisualStyleBackColor = true;
            this.checkBoxY.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBoxZ
            // 
            this.checkBoxZ.AutoSize = true;
            this.checkBoxZ.Checked = true;
            this.checkBoxZ.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxZ.Location = new System.Drawing.Point(3, 53);
            this.checkBoxZ.Name = "checkBoxZ";
            this.checkBoxZ.Size = new System.Drawing.Size(69, 19);
            this.checkBoxZ.TabIndex = 0;
            this.checkBoxZ.Text = "Z-Axis";
            this.checkBoxZ.UseVisualStyleBackColor = true;
            this.checkBoxZ.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBoxAccel
            // 
            this.checkBoxAccel.AutoSize = true;
            this.checkBoxAccel.Enabled = false;
            this.checkBoxAccel.Location = new System.Drawing.Point(3, 78);
            this.checkBoxAccel.Name = "checkBoxAccel";
            this.checkBoxAccel.Size = new System.Drawing.Size(68, 19);
            this.checkBoxAccel.TabIndex = 0;
            this.checkBoxAccel.Text = "Accel.";
            this.checkBoxAccel.UseVisualStyleBackColor = true;
            this.checkBoxAccel.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBoxLight
            // 
            this.checkBoxLight.AutoSize = true;
            this.checkBoxLight.Location = new System.Drawing.Point(3, 103);
            this.checkBoxLight.Name = "checkBoxLight";
            this.checkBoxLight.Size = new System.Drawing.Size(61, 19);
            this.checkBoxLight.TabIndex = 0;
            this.checkBoxLight.Text = "Light";
            this.checkBoxLight.UseVisualStyleBackColor = true;
            this.checkBoxLight.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBoxTemp
            // 
            this.checkBoxTemp.AutoSize = true;
            this.checkBoxTemp.Location = new System.Drawing.Point(3, 128);
            this.checkBoxTemp.Name = "checkBoxTemp";
            this.checkBoxTemp.Size = new System.Drawing.Size(70, 19);
            this.checkBoxTemp.TabIndex = 0;
            this.checkBoxTemp.Text = "Temp.";
            this.checkBoxTemp.UseVisualStyleBackColor = true;
            this.checkBoxTemp.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBoxBatt
            // 
            this.checkBoxBatt.AutoSize = true;
            this.checkBoxBatt.Location = new System.Drawing.Point(3, 153);
            this.checkBoxBatt.Name = "checkBoxBatt";
            this.checkBoxBatt.Size = new System.Drawing.Size(59, 21);
            this.checkBoxBatt.TabIndex = 0;
            this.checkBoxBatt.Text = "Batt.";
            this.checkBoxBatt.UseVisualStyleBackColor = true;
            this.checkBoxBatt.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // graphPanel
            // 
            this.graphPanel.Cursor = System.Windows.Forms.Cursors.IBeam;
            this.graphPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.graphPanel.Image = null;
            this.graphPanel.Location = new System.Drawing.Point(0, 0);
            this.graphPanel.Name = "graphPanel";
            this.graphPanel.Size = new System.Drawing.Size(858, 236);
            this.graphPanel.TabIndex = 1;
            this.graphPanel.SizeChanged += new System.EventHandler(this.graphPanel_SizeChanged);
            this.graphPanel.Click += new System.EventHandler(this.graphPanel_Click);
            this.graphPanel.Paint += new System.Windows.Forms.PaintEventHandler(this.graphPanel_Paint);
            this.graphPanel.DoubleClick += new System.EventHandler(this.graphPanel_DoubleClick);
            this.graphPanel.MouseDown += new System.Windows.Forms.MouseEventHandler(this.graphPanel_MouseDown);
            this.graphPanel.MouseLeave += new System.EventHandler(this.graphPanel_MouseLeave);
            this.graphPanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.graphPanel_MouseMove);
            this.graphPanel.MouseUp += new System.Windows.Forms.MouseEventHandler(this.graphPanel_MouseUp);
            // 
            // timerAnimate
            // 
            this.timerAnimate.Interval = 50;
            this.timerAnimate.Tick += new System.EventHandler(this.timerAnimate_Tick);
            // 
            // DataViewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.graphPanel);
            this.Controls.Add(this.groupBoxOptions);
            this.Name = "DataViewer";
            this.Size = new System.Drawing.Size(964, 236);
            this.groupBoxOptions.ResumeLayout(false);
            this.groupBoxOptions.PerformLayout();
            this.toolStripData.ResumeLayout(false);
            this.toolStripData.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBoxOptions;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.CheckBox checkBoxX;
        private System.Windows.Forms.CheckBox checkBoxY;
        private System.Windows.Forms.CheckBox checkBoxZ;
        private System.Windows.Forms.CheckBox checkBoxAccel;
        private System.Windows.Forms.CheckBox checkBoxLight;
        private System.Windows.Forms.CheckBox checkBoxTemp;
        private GraphPanel graphPanel;
        private System.Windows.Forms.ToolStrip toolStripData;
        private System.Windows.Forms.ToolStripButton toolStripButtonZoom;
        private System.Windows.Forms.ToolStripButton toolStripButtonSelection;
        private System.Windows.Forms.Timer timerAnimate;
        private System.Windows.Forms.CheckBox checkBoxBatt;
    }
}

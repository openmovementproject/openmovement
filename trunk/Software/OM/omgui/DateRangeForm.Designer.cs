namespace OmGui
{
    partial class DateRangeForm
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
            this.panel1 = new System.Windows.Forms.Panel();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOk = new System.Windows.Forms.Button();
            this.alwaysCheckBox = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.sessionIdTextBox = new System.Windows.Forms.TextBox();
            this.datePicker = new System.Windows.Forms.DateTimePicker();
            this.labelFrom = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.syncToPCCheckBox = new System.Windows.Forms.CheckBox();
            this.timePicker = new System.Windows.Forms.DateTimePicker();
            this.label5 = new System.Windows.Forms.Label();
            this.dayPicker = new System.Windows.Forms.NumericUpDown();
            this.hoursPicker = new System.Windows.Forms.NumericUpDown();
            this.minutesPicker = new System.Windows.Forms.NumericUpDown();
            this.buttonSetupMetadata = new System.Windows.Forms.Button();
            this.label6 = new System.Windows.Forms.Label();
            this.comboBoxSamplingFreq = new System.Windows.Forms.ComboBox();
            this.comboBoxRange = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.label8 = new System.Windows.Forms.Label();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dayPicker)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.hoursPicker)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.minutesPicker)).BeginInit();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.panel1.Controls.Add(this.buttonCancel);
            this.panel1.Controls.Add(this.buttonOk);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 257);
            this.panel1.Margin = new System.Windows.Forms.Padding(2);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(490, 47);
            this.panel1.TabIndex = 1;
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(396, 8);
            this.buttonCancel.Margin = new System.Windows.Forms.Padding(2);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(85, 30);
            this.buttonCancel.TabIndex = 6;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // buttonOk
            // 
            this.buttonOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOk.Location = new System.Drawing.Point(307, 8);
            this.buttonOk.Margin = new System.Windows.Forms.Padding(2);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.Size = new System.Drawing.Size(85, 30);
            this.buttonOk.TabIndex = 5;
            this.buttonOk.Text = "OK";
            this.buttonOk.UseVisualStyleBackColor = true;
            this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
            // 
            // alwaysCheckBox
            // 
            this.alwaysCheckBox.AutoSize = true;
            this.alwaysCheckBox.Location = new System.Drawing.Point(429, 71);
            this.alwaysCheckBox.Name = "alwaysCheckBox";
            this.alwaysCheckBox.Size = new System.Drawing.Size(59, 17);
            this.alwaysCheckBox.TabIndex = 4;
            this.alwaysCheckBox.Text = "Always";
            this.alwaysCheckBox.UseVisualStyleBackColor = true;
            this.alwaysCheckBox.CheckedChanged += new System.EventHandler(this.alwaysCheckBox_CheckedChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 183);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(58, 13);
            this.label1.TabIndex = 5;
            this.label1.Text = "Session ID";
            // 
            // sessionIdTextBox
            // 
            this.sessionIdTextBox.Location = new System.Drawing.Point(95, 180);
            this.sessionIdTextBox.Name = "sessionIdTextBox";
            this.sessionIdTextBox.Size = new System.Drawing.Size(82, 20);
            this.sessionIdTextBox.TabIndex = 6;
            this.sessionIdTextBox.Text = "0";
            // 
            // datePicker
            // 
            this.datePicker.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.datePicker.Checked = false;
            this.datePicker.CustomFormat = "dd/MM/yyyy";
            this.datePicker.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.datePicker.Location = new System.Drawing.Point(46, 24);
            this.datePicker.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.datePicker.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.datePicker.Name = "datePicker";
            this.datePicker.Size = new System.Drawing.Size(116, 20);
            this.datePicker.TabIndex = 9;
            this.datePicker.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            // 
            // labelFrom
            // 
            this.labelFrom.AutoSize = true;
            this.labelFrom.Location = new System.Drawing.Point(11, 24);
            this.labelFrom.Name = "labelFrom";
            this.labelFrom.Size = new System.Drawing.Size(30, 13);
            this.labelFrom.TabIndex = 8;
            this.labelFrom.Text = "Date";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 72);
            this.label2.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(31, 13);
            this.label2.TabIndex = 10;
            this.label2.Text = "Days";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(280, 72);
            this.label3.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(44, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "Minutes";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(142, 72);
            this.label4.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(35, 13);
            this.label4.TabIndex = 12;
            this.label4.Text = "Hours";
            // 
            // syncToPCCheckBox
            // 
            this.syncToPCCheckBox.AutoSize = true;
            this.syncToPCCheckBox.Checked = true;
            this.syncToPCCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.syncToPCCheckBox.Location = new System.Drawing.Point(342, 24);
            this.syncToPCCheckBox.Margin = new System.Windows.Forms.Padding(2);
            this.syncToPCCheckBox.Name = "syncToPCCheckBox";
            this.syncToPCCheckBox.Size = new System.Drawing.Size(105, 17);
            this.syncToPCCheckBox.TabIndex = 16;
            this.syncToPCCheckBox.Text = "Sync to PC Time";
            this.syncToPCCheckBox.UseVisualStyleBackColor = true;
            // 
            // timePicker
            // 
            this.timePicker.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.timePicker.Checked = false;
            this.timePicker.CustomFormat = "HH:mm:ss";
            this.timePicker.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.timePicker.Location = new System.Drawing.Point(207, 24);
            this.timePicker.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.timePicker.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.timePicker.Name = "timePicker";
            this.timePicker.Size = new System.Drawing.Size(116, 20);
            this.timePicker.TabIndex = 17;
            this.timePicker.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(172, 24);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(30, 13);
            this.label5.TabIndex = 18;
            this.label5.Text = "Time";
            // 
            // dayPicker
            // 
            this.dayPicker.Location = new System.Drawing.Point(48, 69);
            this.dayPicker.Margin = new System.Windows.Forms.Padding(2);
            this.dayPicker.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.dayPicker.Name = "dayPicker";
            this.dayPicker.Size = new System.Drawing.Size(90, 20);
            this.dayPicker.TabIndex = 20;
            // 
            // hoursPicker
            // 
            this.hoursPicker.Location = new System.Drawing.Point(182, 69);
            this.hoursPicker.Margin = new System.Windows.Forms.Padding(2);
            this.hoursPicker.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.hoursPicker.Name = "hoursPicker";
            this.hoursPicker.Size = new System.Drawing.Size(90, 20);
            this.hoursPicker.TabIndex = 21;
            // 
            // minutesPicker
            // 
            this.minutesPicker.Location = new System.Drawing.Point(327, 70);
            this.minutesPicker.Margin = new System.Windows.Forms.Padding(2);
            this.minutesPicker.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.minutesPicker.Name = "minutesPicker";
            this.minutesPicker.Size = new System.Drawing.Size(90, 20);
            this.minutesPicker.TabIndex = 22;
            // 
            // buttonSetupMetadata
            // 
            this.buttonSetupMetadata.Location = new System.Drawing.Point(14, 216);
            this.buttonSetupMetadata.Margin = new System.Windows.Forms.Padding(2);
            this.buttonSetupMetadata.Name = "buttonSetupMetadata";
            this.buttonSetupMetadata.Size = new System.Drawing.Size(124, 25);
            this.buttonSetupMetadata.TabIndex = 23;
            this.buttonSetupMetadata.Text = "&Setup Metadata...";
            this.buttonSetupMetadata.UseVisualStyleBackColor = true;
            this.buttonSetupMetadata.Click += new System.EventHandler(this.buttonSetupMetadata_Click);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(12, 148);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(77, 13);
            this.label6.TabIndex = 24;
            this.label6.Text = "Sampling Freq.";
            // 
            // comboBoxSamplingFreq
            // 
            this.comboBoxSamplingFreq.FormattingEnabled = true;
            this.comboBoxSamplingFreq.Items.AddRange(new object[] {
            "3200",
            "800",
            "400",
            "200",
            "100",
            "50",
            "25",
            "12.5",
            "6.25"});
            this.comboBoxSamplingFreq.Location = new System.Drawing.Point(95, 145);
            this.comboBoxSamplingFreq.Name = "comboBoxSamplingFreq";
            this.comboBoxSamplingFreq.Size = new System.Drawing.Size(82, 21);
            this.comboBoxSamplingFreq.TabIndex = 25;
            this.comboBoxSamplingFreq.Text = "100";
            // 
            // comboBoxRange
            // 
            this.comboBoxRange.FormattingEnabled = true;
            this.comboBoxRange.Items.AddRange(new object[] {
            "2",
            "4",
            "8",
            "16"});
            this.comboBoxRange.Location = new System.Drawing.Point(240, 145);
            this.comboBoxRange.Name = "comboBoxRange";
            this.comboBoxRange.Size = new System.Drawing.Size(55, 21);
            this.comboBoxRange.TabIndex = 27;
            this.comboBoxRange.Text = "4";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(195, 148);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(39, 13);
            this.label7.TabIndex = 26;
            this.label7.Text = "Range";
            // 
            // comboBox1
            // 
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Items.AddRange(new object[] {
            "GMT - 9",
            "GMT - 8",
            "GMT - 7",
            "GMT - 6",
            "GMT - 5",
            "GMT - 4",
            "GMT - 3",
            "GMT - 2",
            "GMT - 1",
            "GMT + 0",
            "GMT + 1",
            "GMT + 2",
            "GMT + 3",
            "GMT + 4",
            "GMT 5",
            "GMT 6",
            "GMT 7",
            "GMT 8",
            "GMT 9",
            "GMT 10",
            "GMT 11",
            "GMT 12"});
            this.comboBox1.Location = new System.Drawing.Point(95, 108);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(82, 21);
            this.comboBox1.TabIndex = 29;
            this.comboBox1.Text = "GMT +0";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(12, 111);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(53, 13);
            this.label8.TabIndex = 28;
            this.label8.Text = "Timezone";
            // 
            // DateRangeForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.ClientSize = new System.Drawing.Size(490, 304);
            this.Controls.Add(this.comboBox1);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.comboBoxRange);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.comboBoxSamplingFreq);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.buttonSetupMetadata);
            this.Controls.Add(this.minutesPicker);
            this.Controls.Add(this.hoursPicker);
            this.Controls.Add(this.dayPicker);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.timePicker);
            this.Controls.Add(this.syncToPCCheckBox);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.datePicker);
            this.Controls.Add(this.labelFrom);
            this.Controls.Add(this.sessionIdTextBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.alwaysCheckBox);
            this.Controls.Add(this.panel1);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "DateRangeForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Date Range";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.DateRangeForm_FormClosing);
            this.Load += new System.EventHandler(this.DateRangeForm_Load);
            this.panel1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dayPicker)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.hoursPicker)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.minutesPicker)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOk;
        private System.Windows.Forms.CheckBox alwaysCheckBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox sessionIdTextBox;
        private System.Windows.Forms.DateTimePicker datePicker;
        private System.Windows.Forms.Label labelFrom;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.CheckBox syncToPCCheckBox;
        private System.Windows.Forms.DateTimePicker timePicker;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown dayPicker;
        private System.Windows.Forms.NumericUpDown hoursPicker;
        private System.Windows.Forms.NumericUpDown minutesPicker;
        private System.Windows.Forms.Button buttonSetupMetadata;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox comboBoxSamplingFreq;
        private System.Windows.Forms.ComboBox comboBoxRange;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.Label label8;
    }
}
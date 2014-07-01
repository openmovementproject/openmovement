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
            this.checkBoxFlash = new System.Windows.Forms.CheckBox();
            this.richTextBoxWarning = new System.Windows.Forms.RichTextBox();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOk = new System.Windows.Forms.Button();
            this.datePickerStart = new System.Windows.Forms.DateTimePicker();
            this.labelFrom = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.daysPicker = new System.Windows.Forms.NumericUpDown();
            this.hoursPicker = new System.Windows.Forms.NumericUpDown();
            this.minutesPicker = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.comboBoxSamplingFreq = new System.Windows.Forms.ComboBox();
            this.comboBoxRange = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.numericUpDownSessionID = new System.Windows.Forms.NumericUpDown();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.labelRateRangeSetting = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.timePickerStart = new System.Windows.Forms.DateTimePicker();
            this.timePickerEnd = new System.Windows.Forms.DateTimePicker();
            this.label25 = new System.Windows.Forms.Label();
            this.label14 = new System.Windows.Forms.Label();
            this.datePickerEnd = new System.Windows.Forms.DateTimePicker();
            this.label15 = new System.Windows.Forms.Label();
            this.radioButtonDuration = new System.Windows.Forms.RadioButton();
            this.radioButtonImmediately = new System.Windows.Forms.RadioButton();
            this.label16 = new System.Windows.Forms.Label();
            this.delayDaysPicker = new System.Windows.Forms.NumericUpDown();
            this.label18 = new System.Windows.Forms.Label();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.textBoxWeight = new System.Windows.Forms.TextBox();
            this.textBoxHeight = new System.Windows.Forms.TextBox();
            this.comboBoxSite = new System.Windows.Forms.ComboBox();
            this.label26 = new System.Windows.Forms.Label();
            this.comboBoxSubjectHandedness = new System.Windows.Forms.ComboBox();
            this.comboBoxSubjectSex = new System.Windows.Forms.ComboBox();
            this.textBoxSubjectNotes = new System.Windows.Forms.TextBox();
            this.label19 = new System.Windows.Forms.Label();
            this.label20 = new System.Windows.Forms.Label();
            this.label21 = new System.Windows.Forms.Label();
            this.label22 = new System.Windows.Forms.Label();
            this.label23 = new System.Windows.Forms.Label();
            this.label24 = new System.Windows.Forms.Label();
            this.textBoxSubjectCode = new System.Windows.Forms.TextBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.textBoxStudyNotes = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.textBoxStudyCode = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.textBoxStudyOperator = new System.Windows.Forms.TextBox();
            this.label13 = new System.Windows.Forms.Label();
            this.textBoxStudyExerciseType = new System.Windows.Forms.TextBox();
            this.textBoxStudyCentre = new System.Windows.Forms.TextBox();
            this.textBoxStudyInvestigator = new System.Windows.Forms.TextBox();
            this.buttonDefault = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.daysPicker)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.hoursPicker)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.minutesPicker)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSessionID)).BeginInit();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.delayDaysPicker)).BeginInit();
            this.tableLayoutPanel1.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.panel1.Controls.Add(this.checkBoxFlash);
            this.panel1.Controls.Add(this.richTextBoxWarning);
            this.panel1.Controls.Add(this.buttonCancel);
            this.panel1.Controls.Add(this.buttonOk);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 579);
            this.panel1.Margin = new System.Windows.Forms.Padding(2);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(529, 82);
            this.panel1.TabIndex = 5;
            // 
            // checkBoxFlash
            // 
            this.checkBoxFlash.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.checkBoxFlash.AutoSize = true;
            this.checkBoxFlash.Location = new System.Drawing.Point(346, 6);
            this.checkBoxFlash.Name = "checkBoxFlash";
            this.checkBoxFlash.Size = new System.Drawing.Size(130, 17);
            this.checkBoxFlash.TabIndex = 7;
            this.checkBoxFlash.Text = "Flash during recording";
            this.checkBoxFlash.UseVisualStyleBackColor = true;
            // 
            // richTextBoxWarning
            // 
            this.richTextBoxWarning.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxWarning.BackColor = System.Drawing.SystemColors.Info;
            this.richTextBoxWarning.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.richTextBoxWarning.Location = new System.Drawing.Point(4, 5);
            this.richTextBoxWarning.Margin = new System.Windows.Forms.Padding(2);
            this.richTextBoxWarning.Name = "richTextBoxWarning";
            this.richTextBoxWarning.ReadOnly = true;
            this.richTextBoxWarning.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
            this.richTextBoxWarning.Size = new System.Drawing.Size(340, 71);
            this.richTextBoxWarning.TabIndex = 53;
            this.richTextBoxWarning.Text = "";
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(435, 43);
            this.buttonCancel.Margin = new System.Windows.Forms.Padding(2);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(85, 30);
            this.buttonCancel.TabIndex = 55;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOk
            // 
            this.buttonOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOk.Location = new System.Drawing.Point(346, 43);
            this.buttonOk.Margin = new System.Windows.Forms.Padding(2);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.Size = new System.Drawing.Size(85, 30);
            this.buttonOk.TabIndex = 54;
            this.buttonOk.Text = "OK";
            this.buttonOk.UseVisualStyleBackColor = true;
            this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
            // 
            // datePickerStart
            // 
            this.datePickerStart.Checked = false;
            this.datePickerStart.CustomFormat = "dd-MMM-yyyy";
            this.datePickerStart.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.datePickerStart.Location = new System.Drawing.Point(92, 80);
            this.datePickerStart.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.datePickerStart.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.datePickerStart.Name = "datePickerStart";
            this.datePickerStart.Size = new System.Drawing.Size(120, 20);
            this.datePickerStart.TabIndex = 11;
            this.datePickerStart.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            this.datePickerStart.ValueChanged += new System.EventHandler(this.startDuration_ValueChanged);
            // 
            // labelFrom
            // 
            this.labelFrom.AutoSize = true;
            this.labelFrom.Location = new System.Drawing.Point(34, 80);
            this.labelFrom.Name = "labelFrom";
            this.labelFrom.Size = new System.Drawing.Size(58, 13);
            this.labelFrom.TabIndex = 10;
            this.labelFrom.Text = "Start Date:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(34, 110);
            this.label2.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(50, 13);
            this.label2.TabIndex = 14;
            this.label2.Text = "Duration:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(243, 110);
            this.label3.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(33, 13);
            this.label3.TabIndex = 18;
            this.label3.Text = "hours";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(146, 110);
            this.label4.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(29, 13);
            this.label4.TabIndex = 16;
            this.label4.Text = "days";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(222, 80);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(58, 13);
            this.label5.TabIndex = 12;
            this.label5.Text = "Start Time:";
            // 
            // daysPicker
            // 
            this.daysPicker.Location = new System.Drawing.Point(92, 108);
            this.daysPicker.Margin = new System.Windows.Forms.Padding(2);
            this.daysPicker.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.daysPicker.Name = "daysPicker";
            this.daysPicker.Size = new System.Drawing.Size(47, 20);
            this.daysPicker.TabIndex = 15;
            this.daysPicker.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.daysPicker.ValueChanged += new System.EventHandler(this.startDuration_ValueChanged);
            // 
            // hoursPicker
            // 
            this.hoursPicker.Location = new System.Drawing.Point(191, 108);
            this.hoursPicker.Margin = new System.Windows.Forms.Padding(2);
            this.hoursPicker.Maximum = new decimal(new int[] {
            24,
            0,
            0,
            0});
            this.hoursPicker.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this.hoursPicker.Name = "hoursPicker";
            this.hoursPicker.Size = new System.Drawing.Size(47, 20);
            this.hoursPicker.TabIndex = 17;
            this.hoursPicker.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.hoursPicker.ValueChanged += new System.EventHandler(this.startDuration_ValueChanged);
            // 
            // minutesPicker
            // 
            this.minutesPicker.Location = new System.Drawing.Point(299, 108);
            this.minutesPicker.Margin = new System.Windows.Forms.Padding(2);
            this.minutesPicker.Maximum = new decimal(new int[] {
            60,
            0,
            0,
            0});
            this.minutesPicker.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this.minutesPicker.Name = "minutesPicker";
            this.minutesPicker.Size = new System.Drawing.Size(47, 20);
            this.minutesPicker.TabIndex = 19;
            this.minutesPicker.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.minutesPicker.ValueChanged += new System.EventHandler(this.startDuration_ValueChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(6, 29);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(99, 13);
            this.label6.TabIndex = 3;
            this.label6.Text = "Sampling Freq. (Hz)";
            // 
            // comboBoxSamplingFreq
            // 
            this.comboBoxSamplingFreq.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxSamplingFreq.FormattingEnabled = true;
            this.comboBoxSamplingFreq.Items.AddRange(new object[] {
            "3200",
            "1600",
            "800",
            "400",
            "200",
            "100",
            "50",
            "25",
            "12.5"});
            this.comboBoxSamplingFreq.Location = new System.Drawing.Point(111, 26);
            this.comboBoxSamplingFreq.Name = "comboBoxSamplingFreq";
            this.comboBoxSamplingFreq.Size = new System.Drawing.Size(82, 21);
            this.comboBoxSamplingFreq.TabIndex = 4;
            this.comboBoxSamplingFreq.SelectedIndexChanged += new System.EventHandler(this.comboBoxSamplingFreq_SelectedIndexChanged);
            // 
            // comboBoxRange
            // 
            this.comboBoxRange.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxRange.FormattingEnabled = true;
            this.comboBoxRange.Items.AddRange(new object[] {
            "2",
            "4",
            "8",
            "16"});
            this.comboBoxRange.Location = new System.Drawing.Point(275, 26);
            this.comboBoxRange.Name = "comboBoxRange";
            this.comboBoxRange.Size = new System.Drawing.Size(55, 21);
            this.comboBoxRange.TabIndex = 6;
            this.comboBoxRange.SelectedIndexChanged += new System.EventHandler(this.comboBoxRange_SelectedIndexChanged);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(206, 29);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(63, 13);
            this.label7.TabIndex = 5;
            this.label7.Text = "Range (±g) ";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(8, 21);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(110, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Recording Session ID";
            // 
            // numericUpDownSessionID
            // 
            this.numericUpDownSessionID.Location = new System.Drawing.Point(124, 20);
            this.numericUpDownSessionID.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.numericUpDownSessionID.Name = "numericUpDownSessionID";
            this.numericUpDownSessionID.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownSessionID.TabIndex = 1;
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Controls.Add(this.buttonDefault);
            this.groupBox2.Controls.Add(this.labelRateRangeSetting);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.comboBoxSamplingFreq);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.comboBoxRange);
            this.groupBox2.Location = new System.Drawing.Point(11, 46);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(504, 56);
            this.groupBox2.TabIndex = 2;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Sampling";
            // 
            // labelRateRangeSetting
            // 
            this.labelRateRangeSetting.AutoSize = true;
            this.labelRateRangeSetting.Location = new System.Drawing.Point(345, 29);
            this.labelRateRangeSetting.Name = "labelRateRangeSetting";
            this.labelRateRangeSetting.Size = new System.Drawing.Size(10, 13);
            this.labelRateRangeSetting.TabIndex = 7;
            this.labelRateRangeSetting.Text = "-";
            // 
            // groupBox3
            // 
            this.groupBox3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox3.Controls.Add(this.timePickerStart);
            this.groupBox3.Controls.Add(this.timePickerEnd);
            this.groupBox3.Controls.Add(this.label25);
            this.groupBox3.Controls.Add(this.label14);
            this.groupBox3.Controls.Add(this.datePickerEnd);
            this.groupBox3.Controls.Add(this.label15);
            this.groupBox3.Controls.Add(this.radioButtonDuration);
            this.groupBox3.Controls.Add(this.radioButtonImmediately);
            this.groupBox3.Controls.Add(this.labelFrom);
            this.groupBox3.Controls.Add(this.datePickerStart);
            this.groupBox3.Controls.Add(this.label5);
            this.groupBox3.Controls.Add(this.minutesPicker);
            this.groupBox3.Controls.Add(this.label16);
            this.groupBox3.Controls.Add(this.label2);
            this.groupBox3.Controls.Add(this.delayDaysPicker);
            this.groupBox3.Controls.Add(this.hoursPicker);
            this.groupBox3.Controls.Add(this.daysPicker);
            this.groupBox3.Controls.Add(this.label3);
            this.groupBox3.Controls.Add(this.label18);
            this.groupBox3.Controls.Add(this.label4);
            this.groupBox3.Location = new System.Drawing.Point(11, 110);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(504, 171);
            this.groupBox3.TabIndex = 7;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Recording Time";
            // 
            // timePickerStart
            // 
            this.timePickerStart.Checked = false;
            this.timePickerStart.CustomFormat = "HH:mm";
            this.timePickerStart.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.timePickerStart.Location = new System.Drawing.Point(282, 80);
            this.timePickerStart.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.timePickerStart.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.timePickerStart.Name = "timePickerStart";
            this.timePickerStart.Size = new System.Drawing.Size(114, 20);
            this.timePickerStart.TabIndex = 13;
            this.timePickerStart.Value = new System.DateTime(2013, 3, 13, 12, 0, 0, 0);
            this.timePickerStart.ValueChanged += new System.EventHandler(this.startDuration_ValueChanged);
            // 
            // timePickerEnd
            // 
            this.timePickerEnd.Checked = false;
            this.timePickerEnd.CustomFormat = "HH:mm";
            this.timePickerEnd.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.timePickerEnd.Location = new System.Drawing.Point(282, 143);
            this.timePickerEnd.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.timePickerEnd.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.timePickerEnd.Name = "timePickerEnd";
            this.timePickerEnd.Size = new System.Drawing.Size(114, 20);
            this.timePickerEnd.TabIndex = 24;
            this.timePickerEnd.Value = new System.DateTime(2011, 4, 17, 23, 0, 0, 0);
            this.timePickerEnd.ValueChanged += new System.EventHandler(this.end_ValueChanged);
            // 
            // label25
            // 
            this.label25.AutoSize = true;
            this.label25.Location = new System.Drawing.Point(352, 110);
            this.label25.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label25.Name = "label25";
            this.label25.Size = new System.Drawing.Size(43, 13);
            this.label25.TabIndex = 20;
            this.label25.Text = "minutes";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(34, 143);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(55, 13);
            this.label14.TabIndex = 21;
            this.label14.Text = "End Date:";
            // 
            // datePickerEnd
            // 
            this.datePickerEnd.Checked = false;
            this.datePickerEnd.CustomFormat = "dd-MMM-yyyy";
            this.datePickerEnd.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.datePickerEnd.Location = new System.Drawing.Point(92, 143);
            this.datePickerEnd.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.datePickerEnd.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.datePickerEnd.Name = "datePickerEnd";
            this.datePickerEnd.Size = new System.Drawing.Size(120, 20);
            this.datePickerEnd.TabIndex = 22;
            this.datePickerEnd.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            this.datePickerEnd.ValueChanged += new System.EventHandler(this.end_ValueChanged);
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(226, 143);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(55, 13);
            this.label15.TabIndex = 23;
            this.label15.Text = "End Time:";
            // 
            // radioButtonDuration
            // 
            this.radioButtonDuration.AutoSize = true;
            this.radioButtonDuration.Location = new System.Drawing.Point(20, 52);
            this.radioButtonDuration.Name = "radioButtonDuration";
            this.radioButtonDuration.Size = new System.Drawing.Size(60, 17);
            this.radioButtonDuration.TabIndex = 9;
            this.radioButtonDuration.TabStop = true;
            this.radioButtonDuration.Text = "Interval";
            this.radioButtonDuration.UseVisualStyleBackColor = true;
            this.radioButtonDuration.CheckedChanged += new System.EventHandler(this.radioButtonDuration_CheckedChanged);
            // 
            // radioButtonImmediately
            // 
            this.radioButtonImmediately.AutoSize = true;
            this.radioButtonImmediately.Location = new System.Drawing.Point(20, 28);
            this.radioButtonImmediately.Name = "radioButtonImmediately";
            this.radioButtonImmediately.Size = new System.Drawing.Size(152, 17);
            this.radioButtonImmediately.TabIndex = 8;
            this.radioButtonImmediately.TabStop = true;
            this.radioButtonImmediately.Text = "Immediately on Disconnect";
            this.radioButtonImmediately.UseVisualStyleBackColor = true;
            this.radioButtonImmediately.CheckedChanged += new System.EventHandler(this.radioButtonImmediately_CheckedChanged);
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(307, 54);
            this.label16.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(37, 13);
            this.label16.TabIndex = 14;
            this.label16.Text = "Delay:";
            // 
            // delayDaysPicker
            // 
            this.delayDaysPicker.Location = new System.Drawing.Point(348, 52);
            this.delayDaysPicker.Margin = new System.Windows.Forms.Padding(2);
            this.delayDaysPicker.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.delayDaysPicker.Name = "delayDaysPicker";
            this.delayDaysPicker.Size = new System.Drawing.Size(47, 20);
            this.delayDaysPicker.TabIndex = 15;
            this.delayDaysPicker.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.delayDaysPicker.ValueChanged += new System.EventHandler(this.delayDaysPicker_ValueChanged);
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(399, 54);
            this.label18.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(29, 13);
            this.label18.TabIndex = 16;
            this.label18.Text = "days";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.Controls.Add(this.groupBox4, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.groupBox1, 0, 0);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(11, 288);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 1;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(504, 286);
            this.tableLayoutPanel1.TabIndex = 4;
            // 
            // groupBox4
            // 
            this.groupBox4.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox4.Controls.Add(this.textBoxWeight);
            this.groupBox4.Controls.Add(this.textBoxHeight);
            this.groupBox4.Controls.Add(this.comboBoxSite);
            this.groupBox4.Controls.Add(this.label26);
            this.groupBox4.Controls.Add(this.comboBoxSubjectHandedness);
            this.groupBox4.Controls.Add(this.comboBoxSubjectSex);
            this.groupBox4.Controls.Add(this.textBoxSubjectNotes);
            this.groupBox4.Controls.Add(this.label19);
            this.groupBox4.Controls.Add(this.label20);
            this.groupBox4.Controls.Add(this.label21);
            this.groupBox4.Controls.Add(this.label22);
            this.groupBox4.Controls.Add(this.label23);
            this.groupBox4.Controls.Add(this.label24);
            this.groupBox4.Controls.Add(this.textBoxSubjectCode);
            this.groupBox4.Location = new System.Drawing.Point(254, 2);
            this.groupBox4.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox4.Size = new System.Drawing.Size(248, 282);
            this.groupBox4.TabIndex = 38;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Subject";
            // 
            // textBoxWeight
            // 
            this.textBoxWeight.Location = new System.Drawing.Point(99, 92);
            this.textBoxWeight.Name = "textBoxWeight";
            this.textBoxWeight.Size = new System.Drawing.Size(100, 20);
            this.textBoxWeight.TabIndex = 46;
            // 
            // textBoxHeight
            // 
            this.textBoxHeight.Location = new System.Drawing.Point(99, 69);
            this.textBoxHeight.Name = "textBoxHeight";
            this.textBoxHeight.Size = new System.Drawing.Size(100, 20);
            this.textBoxHeight.TabIndex = 44;
            // 
            // comboBoxSite
            // 
            this.comboBoxSite.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxSite.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxSite.FormattingEnabled = true;
            this.comboBoxSite.Items.AddRange(new object[] {
            "",
            "left wrist",
            "right wrist",
            "waist",
            "left ankle",
            "right ankle",
            "left thigh",
            "right thigh",
            "left hip",
            "right hip",
            "left upper-arm",
            "right upper-arm",
            "chest",
            "sacrum",
            "neck",
            "head"});
            this.comboBoxSite.Location = new System.Drawing.Point(100, 140);
            this.comboBoxSite.Margin = new System.Windows.Forms.Padding(2);
            this.comboBoxSite.Name = "comboBoxSite";
            this.comboBoxSite.Size = new System.Drawing.Size(139, 21);
            this.comboBoxSite.TabIndex = 50;
            // 
            // label26
            // 
            this.label26.AutoSize = true;
            this.label26.Location = new System.Drawing.Point(4, 141);
            this.label26.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label26.Name = "label26";
            this.label26.Size = new System.Drawing.Size(25, 13);
            this.label26.TabIndex = 49;
            this.label26.Text = "Site";
            // 
            // comboBoxSubjectHandedness
            // 
            this.comboBoxSubjectHandedness.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxSubjectHandedness.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxSubjectHandedness.FormattingEnabled = true;
            this.comboBoxSubjectHandedness.Items.AddRange(new object[] {
            "",
            "left",
            "right"});
            this.comboBoxSubjectHandedness.Location = new System.Drawing.Point(99, 115);
            this.comboBoxSubjectHandedness.Margin = new System.Windows.Forms.Padding(2);
            this.comboBoxSubjectHandedness.Name = "comboBoxSubjectHandedness";
            this.comboBoxSubjectHandedness.Size = new System.Drawing.Size(140, 21);
            this.comboBoxSubjectHandedness.TabIndex = 48;
            // 
            // comboBoxSubjectSex
            // 
            this.comboBoxSubjectSex.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxSubjectSex.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxSubjectSex.FormattingEnabled = true;
            this.comboBoxSubjectSex.Items.AddRange(new object[] {
            "",
            "male",
            "female"});
            this.comboBoxSubjectSex.Location = new System.Drawing.Point(98, 46);
            this.comboBoxSubjectSex.Margin = new System.Windows.Forms.Padding(2);
            this.comboBoxSubjectSex.Name = "comboBoxSubjectSex";
            this.comboBoxSubjectSex.Size = new System.Drawing.Size(140, 21);
            this.comboBoxSubjectSex.TabIndex = 42;
            // 
            // textBoxSubjectNotes
            // 
            this.textBoxSubjectNotes.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxSubjectNotes.Location = new System.Drawing.Point(99, 166);
            this.textBoxSubjectNotes.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxSubjectNotes.MaxLength = 32;
            this.textBoxSubjectNotes.Multiline = true;
            this.textBoxSubjectNotes.Name = "textBoxSubjectNotes";
            this.textBoxSubjectNotes.Size = new System.Drawing.Size(140, 108);
            this.textBoxSubjectNotes.TabIndex = 52;
            // 
            // label19
            // 
            this.label19.AutoSize = true;
            this.label19.Location = new System.Drawing.Point(4, 169);
            this.label19.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(35, 13);
            this.label19.TabIndex = 51;
            this.label19.Text = "Notes";
            // 
            // label20
            // 
            this.label20.AutoSize = true;
            this.label20.Location = new System.Drawing.Point(4, 49);
            this.label20.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(25, 13);
            this.label20.TabIndex = 41;
            this.label20.Text = "Sex";
            // 
            // label21
            // 
            this.label21.AutoSize = true;
            this.label21.Location = new System.Drawing.Point(4, 24);
            this.label21.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label21.Name = "label21";
            this.label21.Size = new System.Drawing.Size(32, 13);
            this.label21.TabIndex = 39;
            this.label21.Text = "Code";
            // 
            // label22
            // 
            this.label22.AutoSize = true;
            this.label22.Location = new System.Drawing.Point(4, 70);
            this.label22.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label22.Name = "label22";
            this.label22.Size = new System.Drawing.Size(38, 13);
            this.label22.TabIndex = 43;
            this.label22.Text = "Height";
            // 
            // label23
            // 
            this.label23.AutoSize = true;
            this.label23.Location = new System.Drawing.Point(4, 94);
            this.label23.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label23.Name = "label23";
            this.label23.Size = new System.Drawing.Size(41, 13);
            this.label23.TabIndex = 45;
            this.label23.Text = "Weight";
            // 
            // label24
            // 
            this.label24.AutoSize = true;
            this.label24.Location = new System.Drawing.Point(4, 117);
            this.label24.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label24.Name = "label24";
            this.label24.Size = new System.Drawing.Size(67, 13);
            this.label24.TabIndex = 47;
            this.label24.Text = "Handedness";
            // 
            // textBoxSubjectCode
            // 
            this.textBoxSubjectCode.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxSubjectCode.Location = new System.Drawing.Point(99, 24);
            this.textBoxSubjectCode.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxSubjectCode.MaxLength = 32;
            this.textBoxSubjectCode.Name = "textBoxSubjectCode";
            this.textBoxSubjectCode.Size = new System.Drawing.Size(140, 20);
            this.textBoxSubjectCode.TabIndex = 40;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.textBoxStudyNotes);
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.label9);
            this.groupBox1.Controls.Add(this.textBoxStudyCode);
            this.groupBox1.Controls.Add(this.label10);
            this.groupBox1.Controls.Add(this.label11);
            this.groupBox1.Controls.Add(this.label12);
            this.groupBox1.Controls.Add(this.textBoxStudyOperator);
            this.groupBox1.Controls.Add(this.label13);
            this.groupBox1.Controls.Add(this.textBoxStudyExerciseType);
            this.groupBox1.Controls.Add(this.textBoxStudyCentre);
            this.groupBox1.Controls.Add(this.textBoxStudyInvestigator);
            this.groupBox1.Location = new System.Drawing.Point(2, 2);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox1.Size = new System.Drawing.Size(248, 282);
            this.groupBox1.TabIndex = 25;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Study";
            // 
            // textBoxStudyNotes
            // 
            this.textBoxStudyNotes.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxStudyNotes.Location = new System.Drawing.Point(99, 139);
            this.textBoxStudyNotes.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyNotes.MaxLength = 32;
            this.textBoxStudyNotes.Multiline = true;
            this.textBoxStudyNotes.Name = "textBoxStudyNotes";
            this.textBoxStudyNotes.Size = new System.Drawing.Size(140, 135);
            this.textBoxStudyNotes.TabIndex = 37;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(4, 141);
            this.label8.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(35, 13);
            this.label8.TabIndex = 36;
            this.label8.Text = "Notes";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(4, 49);
            this.label9.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(62, 13);
            this.label9.TabIndex = 28;
            this.label9.Text = "Study Code";
            // 
            // textBoxStudyCode
            // 
            this.textBoxStudyCode.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxStudyCode.Location = new System.Drawing.Point(99, 46);
            this.textBoxStudyCode.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyCode.MaxLength = 32;
            this.textBoxStudyCode.Name = "textBoxStudyCode";
            this.textBoxStudyCode.Size = new System.Drawing.Size(140, 20);
            this.textBoxStudyCode.TabIndex = 29;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(4, 24);
            this.label10.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(68, 13);
            this.label10.TabIndex = 26;
            this.label10.Text = "Study Centre";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(4, 70);
            this.label11.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(92, 13);
            this.label11.TabIndex = 30;
            this.label11.Text = "Study Investigator";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(4, 94);
            this.label12.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(74, 13);
            this.label12.TabIndex = 32;
            this.label12.Text = "Exercise Type";
            // 
            // textBoxStudyOperator
            // 
            this.textBoxStudyOperator.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxStudyOperator.Location = new System.Drawing.Point(99, 116);
            this.textBoxStudyOperator.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyOperator.MaxLength = 32;
            this.textBoxStudyOperator.Name = "textBoxStudyOperator";
            this.textBoxStudyOperator.Size = new System.Drawing.Size(140, 20);
            this.textBoxStudyOperator.TabIndex = 35;
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(4, 117);
            this.label13.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(48, 13);
            this.label13.TabIndex = 34;
            this.label13.Text = "Operator";
            // 
            // textBoxStudyExerciseType
            // 
            this.textBoxStudyExerciseType.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxStudyExerciseType.Location = new System.Drawing.Point(99, 92);
            this.textBoxStudyExerciseType.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyExerciseType.MaxLength = 32;
            this.textBoxStudyExerciseType.Name = "textBoxStudyExerciseType";
            this.textBoxStudyExerciseType.Size = new System.Drawing.Size(140, 20);
            this.textBoxStudyExerciseType.TabIndex = 33;
            // 
            // textBoxStudyCentre
            // 
            this.textBoxStudyCentre.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxStudyCentre.Location = new System.Drawing.Point(99, 24);
            this.textBoxStudyCentre.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyCentre.MaxLength = 32;
            this.textBoxStudyCentre.Name = "textBoxStudyCentre";
            this.textBoxStudyCentre.Size = new System.Drawing.Size(140, 20);
            this.textBoxStudyCentre.TabIndex = 27;
            // 
            // textBoxStudyInvestigator
            // 
            this.textBoxStudyInvestigator.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxStudyInvestigator.Location = new System.Drawing.Point(99, 67);
            this.textBoxStudyInvestigator.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyInvestigator.MaxLength = 32;
            this.textBoxStudyInvestigator.Name = "textBoxStudyInvestigator";
            this.textBoxStudyInvestigator.Size = new System.Drawing.Size(140, 20);
            this.textBoxStudyInvestigator.TabIndex = 31;
            // 
            // buttonDefault
            // 
            this.buttonDefault.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonDefault.Location = new System.Drawing.Point(433, 22);
            this.buttonDefault.Name = "buttonDefault";
            this.buttonDefault.Size = new System.Drawing.Size(65, 27);
            this.buttonDefault.TabIndex = 8;
            this.buttonDefault.Text = "Defaults";
            this.buttonDefault.UseVisualStyleBackColor = true;
            this.buttonDefault.Visible = false;
            this.buttonDefault.Click += new System.EventHandler(this.buttonDefault_Click);
            // 
            // DateRangeForm
            // 
            this.AcceptButton = this.buttonOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(529, 661);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.numericUpDownSessionID);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.panel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
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
            this.Shown += new System.EventHandler(this.DateRangeForm_Shown);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.daysPicker)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.hoursPicker)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.minutesPicker)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSessionID)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.delayDaysPicker)).EndInit();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOk;
        private System.Windows.Forms.DateTimePicker datePickerStart;
        private System.Windows.Forms.Label labelFrom;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown daysPicker;
        private System.Windows.Forms.NumericUpDown hoursPicker;
        private System.Windows.Forms.NumericUpDown minutesPicker;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox comboBoxSamplingFreq;
        private System.Windows.Forms.ComboBox comboBoxRange;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown numericUpDownSessionID;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.DateTimePicker datePickerEnd;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.RadioButton radioButtonDuration;
        private System.Windows.Forms.RadioButton radioButtonImmediately;
        private System.Windows.Forms.Label label25;
        private System.Windows.Forms.RichTextBox richTextBoxWarning;
        private System.Windows.Forms.DateTimePicker timePickerEnd;
        private System.Windows.Forms.DateTimePicker timePickerStart;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.ComboBox comboBoxSite;
        private System.Windows.Forms.Label label26;
        private System.Windows.Forms.ComboBox comboBoxSubjectHandedness;
        private System.Windows.Forms.ComboBox comboBoxSubjectSex;
        private System.Windows.Forms.TextBox textBoxSubjectNotes;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.Label label21;
        private System.Windows.Forms.Label label22;
        private System.Windows.Forms.Label label23;
        private System.Windows.Forms.Label label24;
        private System.Windows.Forms.TextBox textBoxSubjectCode;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox textBoxStudyNotes;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox textBoxStudyCode;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TextBox textBoxStudyOperator;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.TextBox textBoxStudyExerciseType;
        private System.Windows.Forms.TextBox textBoxStudyCentre;
        private System.Windows.Forms.TextBox textBoxStudyInvestigator;
        private System.Windows.Forms.TextBox textBoxWeight;
        private System.Windows.Forms.TextBox textBoxHeight;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.NumericUpDown delayDaysPicker;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.CheckBox checkBoxFlash;
        private System.Windows.Forms.Label labelRateRangeSetting;
        private System.Windows.Forms.Button buttonDefault;
    }
}
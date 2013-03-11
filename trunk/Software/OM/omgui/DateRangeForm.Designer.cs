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
            this.datePicker = new System.Windows.Forms.DateTimePicker();
            this.labelFrom = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.timePicker = new System.Windows.Forms.DateTimePicker();
            this.label5 = new System.Windows.Forms.Label();
            this.dayPicker = new System.Windows.Forms.NumericUpDown();
            this.hoursPicker = new System.Windows.Forms.NumericUpDown();
            this.minutesPicker = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.comboBoxSamplingFreq = new System.Windows.Forms.ComboBox();
            this.comboBoxRange = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
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
            this.numericUpDownSessionID = new System.Windows.Forms.NumericUpDown();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.label14 = new System.Windows.Forms.Label();
            this.dateTimePicker1 = new System.Windows.Forms.DateTimePicker();
            this.dateTimePicker2 = new System.Windows.Forms.DateTimePicker();
            this.label15 = new System.Windows.Forms.Label();
            this.radioButton2 = new System.Windows.Forms.RadioButton();
            this.radioButton1 = new System.Windows.Forms.RadioButton();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.comboBoxSubjectTimezone = new System.Windows.Forms.ComboBox();
            this.label16 = new System.Windows.Forms.Label();
            this.numericUpDownSubjectWeight = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownSubjectHeight = new System.Windows.Forms.NumericUpDown();
            this.comboBoxSubjectHandedness = new System.Windows.Forms.ComboBox();
            this.label17 = new System.Windows.Forms.Label();
            this.label18 = new System.Windows.Forms.Label();
            this.comboBoxSubjectSex = new System.Windows.Forms.ComboBox();
            this.textBoxSubjectNotes = new System.Windows.Forms.TextBox();
            this.label19 = new System.Windows.Forms.Label();
            this.label20 = new System.Windows.Forms.Label();
            this.label21 = new System.Windows.Forms.Label();
            this.label22 = new System.Windows.Forms.Label();
            this.label23 = new System.Windows.Forms.Label();
            this.label24 = new System.Windows.Forms.Label();
            this.textBoxSubjectCode = new System.Windows.Forms.TextBox();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dayPicker)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.hoursPicker)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.minutesPicker)).BeginInit();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSessionID)).BeginInit();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSubjectWeight)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSubjectHeight)).BeginInit();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.panel1.Controls.Add(this.buttonCancel);
            this.panel1.Controls.Add(this.buttonOk);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 536);
            this.panel1.Margin = new System.Windows.Forms.Padding(2);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(607, 47);
            this.panel1.TabIndex = 1;
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(513, 8);
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
            this.buttonOk.Location = new System.Drawing.Point(424, 8);
            this.buttonOk.Margin = new System.Windows.Forms.Padding(2);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.Size = new System.Drawing.Size(85, 30);
            this.buttonOk.TabIndex = 5;
            this.buttonOk.Text = "OK";
            this.buttonOk.UseVisualStyleBackColor = true;
            this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
            // 
            // datePicker
            // 
            this.datePicker.Checked = false;
            this.datePicker.CustomFormat = "dd/MM/yyyy";
            this.datePicker.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.datePicker.Location = new System.Drawing.Point(69, 76);
            this.datePicker.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.datePicker.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.datePicker.Name = "datePicker";
            this.datePicker.Size = new System.Drawing.Size(120, 20);
            this.datePicker.TabIndex = 9;
            this.datePicker.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            // 
            // labelFrom
            // 
            this.labelFrom.AutoSize = true;
            this.labelFrom.Location = new System.Drawing.Point(34, 76);
            this.labelFrom.Name = "labelFrom";
            this.labelFrom.Size = new System.Drawing.Size(30, 13);
            this.labelFrom.TabIndex = 8;
            this.labelFrom.Text = "Date";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(34, 108);
            this.label2.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(31, 13);
            this.label2.TabIndex = 10;
            this.label2.Text = "Days";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(302, 108);
            this.label3.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(44, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "Minutes";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(164, 108);
            this.label4.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(35, 13);
            this.label4.TabIndex = 12;
            this.label4.Text = "Hours";
            // 
            // timePicker
            // 
            this.timePicker.Checked = false;
            this.timePicker.CustomFormat = "HH:mm";
            this.timePicker.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.timePicker.Location = new System.Drawing.Point(230, 76);
            this.timePicker.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.timePicker.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.timePicker.Name = "timePicker";
            this.timePicker.Size = new System.Drawing.Size(117, 20);
            this.timePicker.TabIndex = 17;
            this.timePicker.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(195, 76);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(30, 13);
            this.label5.TabIndex = 18;
            this.label5.Text = "Time";
            // 
            // dayPicker
            // 
            this.dayPicker.Location = new System.Drawing.Point(70, 105);
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
            this.hoursPicker.Location = new System.Drawing.Point(204, 105);
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
            this.minutesPicker.Location = new System.Drawing.Point(349, 106);
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
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(6, 29);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(99, 13);
            this.label6.TabIndex = 24;
            this.label6.Text = "Sampling Freq. (Hz)";
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
            this.comboBoxSamplingFreq.Location = new System.Drawing.Point(111, 26);
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
            this.comboBoxRange.Location = new System.Drawing.Point(275, 26);
            this.comboBoxRange.Name = "comboBoxRange";
            this.comboBoxRange.Size = new System.Drawing.Size(55, 21);
            this.comboBoxRange.TabIndex = 27;
            this.comboBoxRange.Text = "8";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(206, 29);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(63, 13);
            this.label7.TabIndex = 26;
            this.label7.Text = "Range (±g) ";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(8, 10);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(110, 13);
            this.label1.TabIndex = 28;
            this.label1.Text = "Recording Session ID";
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
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
            this.groupBox1.Location = new System.Drawing.Point(11, 267);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox1.Size = new System.Drawing.Size(282, 218);
            this.groupBox1.TabIndex = 30;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Study";
            // 
            // textBoxStudyNotes
            // 
            this.textBoxStudyNotes.Location = new System.Drawing.Point(99, 139);
            this.textBoxStudyNotes.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyNotes.MaxLength = 200;
            this.textBoxStudyNotes.Multiline = true;
            this.textBoxStudyNotes.Name = "textBoxStudyNotes";
            this.textBoxStudyNotes.Size = new System.Drawing.Size(180, 78);
            this.textBoxStudyNotes.TabIndex = 11;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(4, 141);
            this.label8.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(35, 13);
            this.label8.TabIndex = 10;
            this.label8.Text = "Notes";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(4, 49);
            this.label9.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(62, 13);
            this.label9.TabIndex = 8;
            this.label9.Text = "Study Code";
            // 
            // textBoxStudyCode
            // 
            this.textBoxStudyCode.Location = new System.Drawing.Point(99, 46);
            this.textBoxStudyCode.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyCode.MaxLength = 128;
            this.textBoxStudyCode.Name = "textBoxStudyCode";
            this.textBoxStudyCode.Size = new System.Drawing.Size(180, 20);
            this.textBoxStudyCode.TabIndex = 9;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(4, 24);
            this.label10.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(68, 13);
            this.label10.TabIndex = 0;
            this.label10.Text = "Study Centre";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(4, 70);
            this.label11.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(92, 13);
            this.label11.TabIndex = 1;
            this.label11.Text = "Study Investigator";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(4, 94);
            this.label12.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(74, 13);
            this.label12.TabIndex = 2;
            this.label12.Text = "Exercise Type";
            // 
            // textBoxStudyOperator
            // 
            this.textBoxStudyOperator.Location = new System.Drawing.Point(99, 116);
            this.textBoxStudyOperator.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyOperator.MaxLength = 128;
            this.textBoxStudyOperator.Name = "textBoxStudyOperator";
            this.textBoxStudyOperator.Size = new System.Drawing.Size(180, 20);
            this.textBoxStudyOperator.TabIndex = 7;
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(4, 117);
            this.label13.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(48, 13);
            this.label13.TabIndex = 3;
            this.label13.Text = "Operator";
            // 
            // textBoxStudyExerciseType
            // 
            this.textBoxStudyExerciseType.Location = new System.Drawing.Point(99, 92);
            this.textBoxStudyExerciseType.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyExerciseType.MaxLength = 128;
            this.textBoxStudyExerciseType.Name = "textBoxStudyExerciseType";
            this.textBoxStudyExerciseType.Size = new System.Drawing.Size(180, 20);
            this.textBoxStudyExerciseType.TabIndex = 6;
            // 
            // textBoxStudyCentre
            // 
            this.textBoxStudyCentre.Location = new System.Drawing.Point(99, 24);
            this.textBoxStudyCentre.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyCentre.MaxLength = 128;
            this.textBoxStudyCentre.Name = "textBoxStudyCentre";
            this.textBoxStudyCentre.Size = new System.Drawing.Size(180, 20);
            this.textBoxStudyCentre.TabIndex = 4;
            // 
            // textBoxStudyInvestigator
            // 
            this.textBoxStudyInvestigator.Location = new System.Drawing.Point(99, 67);
            this.textBoxStudyInvestigator.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxStudyInvestigator.MaxLength = 128;
            this.textBoxStudyInvestigator.Name = "textBoxStudyInvestigator";
            this.textBoxStudyInvestigator.Size = new System.Drawing.Size(180, 20);
            this.textBoxStudyInvestigator.TabIndex = 5;
            // 
            // numericUpDownSessionID
            // 
            this.numericUpDownSessionID.Location = new System.Drawing.Point(124, 8);
            this.numericUpDownSessionID.Name = "numericUpDownSessionID";
            this.numericUpDownSessionID.Size = new System.Drawing.Size(120, 20);
            this.numericUpDownSessionID.TabIndex = 31;
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.comboBoxSamplingFreq);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.comboBoxRange);
            this.groupBox2.Location = new System.Drawing.Point(11, 34);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(582, 56);
            this.groupBox2.TabIndex = 32;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Sampling";
            // 
            // groupBox3
            // 
            this.groupBox3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox3.Controls.Add(this.label14);
            this.groupBox3.Controls.Add(this.dateTimePicker1);
            this.groupBox3.Controls.Add(this.dateTimePicker2);
            this.groupBox3.Controls.Add(this.label15);
            this.groupBox3.Controls.Add(this.radioButton2);
            this.groupBox3.Controls.Add(this.radioButton1);
            this.groupBox3.Controls.Add(this.labelFrom);
            this.groupBox3.Controls.Add(this.datePicker);
            this.groupBox3.Controls.Add(this.timePicker);
            this.groupBox3.Controls.Add(this.label5);
            this.groupBox3.Controls.Add(this.minutesPicker);
            this.groupBox3.Controls.Add(this.label2);
            this.groupBox3.Controls.Add(this.hoursPicker);
            this.groupBox3.Controls.Add(this.dayPicker);
            this.groupBox3.Controls.Add(this.label3);
            this.groupBox3.Controls.Add(this.label4);
            this.groupBox3.Location = new System.Drawing.Point(11, 96);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(582, 166);
            this.groupBox3.TabIndex = 33;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Recording Time";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(34, 140);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(30, 13);
            this.label14.TabIndex = 23;
            this.label14.Text = "Date";
            // 
            // dateTimePicker1
            // 
            this.dateTimePicker1.Checked = false;
            this.dateTimePicker1.CustomFormat = "dd/MM/yyyy";
            this.dateTimePicker1.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dateTimePicker1.Location = new System.Drawing.Point(69, 140);
            this.dateTimePicker1.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.dateTimePicker1.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.dateTimePicker1.Name = "dateTimePicker1";
            this.dateTimePicker1.Size = new System.Drawing.Size(120, 20);
            this.dateTimePicker1.TabIndex = 24;
            this.dateTimePicker1.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            // 
            // dateTimePicker2
            // 
            this.dateTimePicker2.Checked = false;
            this.dateTimePicker2.CustomFormat = "HH:mm";
            this.dateTimePicker2.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dateTimePicker2.Location = new System.Drawing.Point(230, 140);
            this.dateTimePicker2.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.dateTimePicker2.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.dateTimePicker2.Name = "dateTimePicker2";
            this.dateTimePicker2.Size = new System.Drawing.Size(117, 20);
            this.dateTimePicker2.TabIndex = 25;
            this.dateTimePicker2.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(195, 140);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(30, 13);
            this.label15.TabIndex = 26;
            this.label15.Text = "Time";
            // 
            // radioButton2
            // 
            this.radioButton2.AutoSize = true;
            this.radioButton2.Location = new System.Drawing.Point(20, 56);
            this.radioButton2.Name = "radioButton2";
            this.radioButton2.Size = new System.Drawing.Size(85, 17);
            this.radioButton2.TabIndex = 1;
            this.radioButton2.TabStop = true;
            this.radioButton2.Text = "radioButton2";
            this.radioButton2.UseVisualStyleBackColor = true;
            // 
            // radioButton1
            // 
            this.radioButton1.AutoSize = true;
            this.radioButton1.Location = new System.Drawing.Point(20, 32);
            this.radioButton1.Name = "radioButton1";
            this.radioButton1.Size = new System.Drawing.Size(85, 17);
            this.radioButton1.TabIndex = 0;
            this.radioButton1.TabStop = true;
            this.radioButton1.Text = "radioButton1";
            this.radioButton1.UseVisualStyleBackColor = true;
            // 
            // groupBox4
            // 
            this.groupBox4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox4.Controls.Add(this.comboBoxSubjectTimezone);
            this.groupBox4.Controls.Add(this.label16);
            this.groupBox4.Controls.Add(this.numericUpDownSubjectWeight);
            this.groupBox4.Controls.Add(this.numericUpDownSubjectHeight);
            this.groupBox4.Controls.Add(this.comboBoxSubjectHandedness);
            this.groupBox4.Controls.Add(this.label17);
            this.groupBox4.Controls.Add(this.label18);
            this.groupBox4.Controls.Add(this.comboBoxSubjectSex);
            this.groupBox4.Controls.Add(this.textBoxSubjectNotes);
            this.groupBox4.Controls.Add(this.label19);
            this.groupBox4.Controls.Add(this.label20);
            this.groupBox4.Controls.Add(this.label21);
            this.groupBox4.Controls.Add(this.label22);
            this.groupBox4.Controls.Add(this.label23);
            this.groupBox4.Controls.Add(this.label24);
            this.groupBox4.Controls.Add(this.textBoxSubjectCode);
            this.groupBox4.Location = new System.Drawing.Point(311, 267);
            this.groupBox4.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox4.Size = new System.Drawing.Size(282, 258);
            this.groupBox4.TabIndex = 34;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Subject";
            // 
            // comboBoxSubjectTimezone
            // 
            this.comboBoxSubjectTimezone.FormattingEnabled = true;
            this.comboBoxSubjectTimezone.Items.AddRange(new object[] {
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
            this.comboBoxSubjectTimezone.Location = new System.Drawing.Point(99, 229);
            this.comboBoxSubjectTimezone.Name = "comboBoxSubjectTimezone";
            this.comboBoxSubjectTimezone.Size = new System.Drawing.Size(82, 21);
            this.comboBoxSubjectTimezone.TabIndex = 31;
            this.comboBoxSubjectTimezone.Text = "GMT +0";
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(4, 232);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(53, 13);
            this.label16.TabIndex = 30;
            this.label16.Text = "Timezone";
            // 
            // numericUpDownSubjectWeight
            // 
            this.numericUpDownSubjectWeight.Location = new System.Drawing.Point(99, 95);
            this.numericUpDownSubjectWeight.Margin = new System.Windows.Forms.Padding(2);
            this.numericUpDownSubjectWeight.Name = "numericUpDownSubjectWeight";
            this.numericUpDownSubjectWeight.Size = new System.Drawing.Size(90, 20);
            this.numericUpDownSubjectWeight.TabIndex = 17;
            // 
            // numericUpDownSubjectHeight
            // 
            this.numericUpDownSubjectHeight.Location = new System.Drawing.Point(99, 72);
            this.numericUpDownSubjectHeight.Margin = new System.Windows.Forms.Padding(2);
            this.numericUpDownSubjectHeight.Name = "numericUpDownSubjectHeight";
            this.numericUpDownSubjectHeight.Size = new System.Drawing.Size(90, 20);
            this.numericUpDownSubjectHeight.TabIndex = 16;
            // 
            // comboBoxSubjectHandedness
            // 
            this.comboBoxSubjectHandedness.FormattingEnabled = true;
            this.comboBoxSubjectHandedness.Items.AddRange(new object[] {
            "left",
            "right"});
            this.comboBoxSubjectHandedness.Location = new System.Drawing.Point(99, 115);
            this.comboBoxSubjectHandedness.Margin = new System.Windows.Forms.Padding(2);
            this.comboBoxSubjectHandedness.Name = "comboBoxSubjectHandedness";
            this.comboBoxSubjectHandedness.Size = new System.Drawing.Size(95, 21);
            this.comboBoxSubjectHandedness.TabIndex = 15;
            // 
            // label17
            // 
            this.label17.AutoSize = true;
            this.label17.Location = new System.Drawing.Point(195, 94);
            this.label17.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(19, 13);
            this.label17.TabIndex = 14;
            this.label17.Text = "kg";
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(195, 72);
            this.label18.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(21, 13);
            this.label18.TabIndex = 13;
            this.label18.Text = "cm";
            // 
            // comboBoxSubjectSex
            // 
            this.comboBoxSubjectSex.FormattingEnabled = true;
            this.comboBoxSubjectSex.Items.AddRange(new object[] {
            "male",
            "female"});
            this.comboBoxSubjectSex.Location = new System.Drawing.Point(99, 46);
            this.comboBoxSubjectSex.Margin = new System.Windows.Forms.Padding(2);
            this.comboBoxSubjectSex.Name = "comboBoxSubjectSex";
            this.comboBoxSubjectSex.Size = new System.Drawing.Size(95, 21);
            this.comboBoxSubjectSex.TabIndex = 12;
            // 
            // textBoxSubjectNotes
            // 
            this.textBoxSubjectNotes.Location = new System.Drawing.Point(99, 140);
            this.textBoxSubjectNotes.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxSubjectNotes.MaxLength = 200;
            this.textBoxSubjectNotes.Multiline = true;
            this.textBoxSubjectNotes.Name = "textBoxSubjectNotes";
            this.textBoxSubjectNotes.Size = new System.Drawing.Size(180, 78);
            this.textBoxSubjectNotes.TabIndex = 11;
            // 
            // label19
            // 
            this.label19.AutoSize = true;
            this.label19.Location = new System.Drawing.Point(8, 141);
            this.label19.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(35, 13);
            this.label19.TabIndex = 10;
            this.label19.Text = "Notes";
            // 
            // label20
            // 
            this.label20.AutoSize = true;
            this.label20.Location = new System.Drawing.Point(4, 49);
            this.label20.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(25, 13);
            this.label20.TabIndex = 8;
            this.label20.Text = "Sex";
            // 
            // label21
            // 
            this.label21.AutoSize = true;
            this.label21.Location = new System.Drawing.Point(4, 24);
            this.label21.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label21.Name = "label21";
            this.label21.Size = new System.Drawing.Size(32, 13);
            this.label21.TabIndex = 0;
            this.label21.Text = "Code";
            // 
            // label22
            // 
            this.label22.AutoSize = true;
            this.label22.Location = new System.Drawing.Point(4, 70);
            this.label22.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label22.Name = "label22";
            this.label22.Size = new System.Drawing.Size(38, 13);
            this.label22.TabIndex = 1;
            this.label22.Text = "Height";
            // 
            // label23
            // 
            this.label23.AutoSize = true;
            this.label23.Location = new System.Drawing.Point(4, 94);
            this.label23.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label23.Name = "label23";
            this.label23.Size = new System.Drawing.Size(41, 13);
            this.label23.TabIndex = 2;
            this.label23.Text = "Weight";
            // 
            // label24
            // 
            this.label24.AutoSize = true;
            this.label24.Location = new System.Drawing.Point(4, 117);
            this.label24.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label24.Name = "label24";
            this.label24.Size = new System.Drawing.Size(67, 13);
            this.label24.TabIndex = 3;
            this.label24.Text = "Handedness";
            // 
            // textBoxSubjectCode
            // 
            this.textBoxSubjectCode.Location = new System.Drawing.Point(99, 24);
            this.textBoxSubjectCode.Margin = new System.Windows.Forms.Padding(2);
            this.textBoxSubjectCode.MaxLength = 128;
            this.textBoxSubjectCode.Name = "textBoxSubjectCode";
            this.textBoxSubjectCode.Size = new System.Drawing.Size(180, 20);
            this.textBoxSubjectCode.TabIndex = 4;
            // 
            // DateRangeForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.ClientSize = new System.Drawing.Size(607, 583);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.numericUpDownSessionID);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.panel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
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
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSessionID)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSubjectWeight)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSubjectHeight)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOk;
        private System.Windows.Forms.DateTimePicker datePicker;
        private System.Windows.Forms.Label labelFrom;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.DateTimePicker timePicker;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown dayPicker;
        private System.Windows.Forms.NumericUpDown hoursPicker;
        private System.Windows.Forms.NumericUpDown minutesPicker;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox comboBoxSamplingFreq;
        private System.Windows.Forms.ComboBox comboBoxRange;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label1;
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
        private System.Windows.Forms.NumericUpDown numericUpDownSessionID;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.DateTimePicker dateTimePicker1;
        private System.Windows.Forms.DateTimePicker dateTimePicker2;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.RadioButton radioButton2;
        private System.Windows.Forms.RadioButton radioButton1;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.ComboBox comboBoxSubjectTimezone;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.NumericUpDown numericUpDownSubjectWeight;
        private System.Windows.Forms.NumericUpDown numericUpDownSubjectHeight;
        private System.Windows.Forms.ComboBox comboBoxSubjectHandedness;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.ComboBox comboBoxSubjectSex;
        private System.Windows.Forms.TextBox textBoxSubjectNotes;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.Label label21;
        private System.Windows.Forms.Label label22;
        private System.Windows.Forms.Label label23;
        private System.Windows.Forms.Label label24;
        private System.Windows.Forms.TextBox textBoxSubjectCode;
    }
}
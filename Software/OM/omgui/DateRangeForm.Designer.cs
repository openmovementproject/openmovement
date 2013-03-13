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
            this.datePickerStart = new System.Windows.Forms.DateTimePicker();
            this.labelFrom = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
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
            this.label25 = new System.Windows.Forms.Label();
            this.label14 = new System.Windows.Forms.Label();
            this.datePickerEnd = new System.Windows.Forms.DateTimePicker();
            this.label15 = new System.Windows.Forms.Label();
            this.radioButtonDuration = new System.Windows.Forms.RadioButton();
            this.radioButtonImmediately = new System.Windows.Forms.RadioButton();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.checkBoxWeight = new System.Windows.Forms.CheckBox();
            this.checkBoxHeight = new System.Windows.Forms.CheckBox();
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
            this.comboBoxSite = new System.Windows.Forms.ComboBox();
            this.label26 = new System.Windows.Forms.Label();
            this.comboBoxSide = new System.Windows.Forms.ComboBox();
            this.label27 = new System.Windows.Forms.Label();
            this.richTextBoxWarning = new System.Windows.Forms.RichTextBox();
            this.timePickerEnd = new System.Windows.Forms.DateTimePicker();
            this.timePickerStart = new System.Windows.Forms.DateTimePicker();
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
            this.panel1.Controls.Add(this.richTextBoxWarning);
            this.panel1.Controls.Add(this.buttonCancel);
            this.panel1.Controls.Add(this.buttonOk);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 737);
            this.panel1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(809, 101);
            this.panel1.TabIndex = 1;
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(684, 53);
            this.buttonCancel.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(113, 37);
            this.buttonCancel.TabIndex = 6;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOk
            // 
            this.buttonOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOk.Location = new System.Drawing.Point(565, 53);
            this.buttonOk.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.Size = new System.Drawing.Size(113, 37);
            this.buttonOk.TabIndex = 5;
            this.buttonOk.Text = "OK";
            this.buttonOk.UseVisualStyleBackColor = true;
            this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
            // 
            // datePickerStart
            // 
            this.datePickerStart.Checked = false;
            this.datePickerStart.CustomFormat = "dd/MM/yyyy";
            this.datePickerStart.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.datePickerStart.Location = new System.Drawing.Point(123, 98);
            this.datePickerStart.Margin = new System.Windows.Forms.Padding(4);
            this.datePickerStart.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.datePickerStart.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.datePickerStart.Name = "datePickerStart";
            this.datePickerStart.Size = new System.Drawing.Size(159, 22);
            this.datePickerStart.TabIndex = 9;
            this.datePickerStart.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            this.datePickerStart.ValueChanged += new System.EventHandler(this.datePickerStart_ValueChanged);
            // 
            // labelFrom
            // 
            this.labelFrom.AutoSize = true;
            this.labelFrom.Location = new System.Drawing.Point(45, 98);
            this.labelFrom.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelFrom.Name = "labelFrom";
            this.labelFrom.Size = new System.Drawing.Size(76, 17);
            this.labelFrom.TabIndex = 8;
            this.labelFrom.Text = "Start Date:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(45, 136);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(66, 17);
            this.label2.TabIndex = 10;
            this.label2.Text = "Duration:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(430, 137);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(44, 17);
            this.label3.TabIndex = 11;
            this.label3.Text = "hours";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(252, 137);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(38, 17);
            this.label4.TabIndex = 12;
            this.label4.Text = "days";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(291, 98);
            this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(77, 17);
            this.label5.TabIndex = 18;
            this.label5.Text = "Start Time:";
            // 
            // dayPicker
            // 
            this.dayPicker.Location = new System.Drawing.Point(126, 133);
            this.dayPicker.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.dayPicker.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.dayPicker.Name = "dayPicker";
            this.dayPicker.Size = new System.Drawing.Size(120, 22);
            this.dayPicker.TabIndex = 20;
            this.dayPicker.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.dayPicker.ValueChanged += new System.EventHandler(this.dayPicker_ValueChanged);
            // 
            // hoursPicker
            // 
            this.hoursPicker.Location = new System.Drawing.Point(304, 134);
            this.hoursPicker.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.hoursPicker.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.hoursPicker.Name = "hoursPicker";
            this.hoursPicker.Size = new System.Drawing.Size(120, 22);
            this.hoursPicker.TabIndex = 21;
            this.hoursPicker.ValueChanged += new System.EventHandler(this.hoursPicker_ValueChanged);
            // 
            // minutesPicker
            // 
            this.minutesPicker.Location = new System.Drawing.Point(488, 135);
            this.minutesPicker.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.minutesPicker.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.minutesPicker.Name = "minutesPicker";
            this.minutesPicker.Size = new System.Drawing.Size(120, 22);
            this.minutesPicker.TabIndex = 22;
            this.minutesPicker.ValueChanged += new System.EventHandler(this.minutesPicker_ValueChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(8, 36);
            this.label6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(134, 17);
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
            this.comboBoxSamplingFreq.Location = new System.Drawing.Point(148, 32);
            this.comboBoxSamplingFreq.Margin = new System.Windows.Forms.Padding(4);
            this.comboBoxSamplingFreq.Name = "comboBoxSamplingFreq";
            this.comboBoxSamplingFreq.Size = new System.Drawing.Size(108, 24);
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
            this.comboBoxRange.Location = new System.Drawing.Point(367, 32);
            this.comboBoxRange.Margin = new System.Windows.Forms.Padding(4);
            this.comboBoxRange.Name = "comboBoxRange";
            this.comboBoxRange.Size = new System.Drawing.Size(72, 24);
            this.comboBoxRange.TabIndex = 27;
            this.comboBoxRange.Text = "8";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(275, 36);
            this.label7.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(84, 17);
            this.label7.TabIndex = 26;
            this.label7.Text = "Range (±g) ";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(11, 26);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(144, 17);
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
            this.groupBox1.Location = new System.Drawing.Point(15, 352);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.groupBox1.Size = new System.Drawing.Size(376, 275);
            this.groupBox1.TabIndex = 30;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Study";
            // 
            // textBoxStudyNotes
            // 
            this.textBoxStudyNotes.Location = new System.Drawing.Point(132, 171);
            this.textBoxStudyNotes.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxStudyNotes.MaxLength = 32;
            this.textBoxStudyNotes.Multiline = true;
            this.textBoxStudyNotes.Name = "textBoxStudyNotes";
            this.textBoxStudyNotes.Size = new System.Drawing.Size(239, 95);
            this.textBoxStudyNotes.TabIndex = 11;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(5, 174);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(45, 17);
            this.label8.TabIndex = 10;
            this.label8.Text = "Notes";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(5, 60);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(81, 17);
            this.label9.TabIndex = 8;
            this.label9.Text = "Study Code";
            // 
            // textBoxStudyCode
            // 
            this.textBoxStudyCode.Location = new System.Drawing.Point(132, 57);
            this.textBoxStudyCode.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxStudyCode.MaxLength = 32;
            this.textBoxStudyCode.Name = "textBoxStudyCode";
            this.textBoxStudyCode.Size = new System.Drawing.Size(239, 22);
            this.textBoxStudyCode.TabIndex = 9;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(5, 30);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(90, 17);
            this.label10.TabIndex = 0;
            this.label10.Text = "Study Centre";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(5, 86);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(121, 17);
            this.label11.TabIndex = 1;
            this.label11.Text = "Study Investigator";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(5, 116);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(97, 17);
            this.label12.TabIndex = 2;
            this.label12.Text = "Exercise Type";
            // 
            // textBoxStudyOperator
            // 
            this.textBoxStudyOperator.Location = new System.Drawing.Point(132, 143);
            this.textBoxStudyOperator.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxStudyOperator.MaxLength = 32;
            this.textBoxStudyOperator.Name = "textBoxStudyOperator";
            this.textBoxStudyOperator.Size = new System.Drawing.Size(239, 22);
            this.textBoxStudyOperator.TabIndex = 7;
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(5, 144);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(65, 17);
            this.label13.TabIndex = 3;
            this.label13.Text = "Operator";
            // 
            // textBoxStudyExerciseType
            // 
            this.textBoxStudyExerciseType.Location = new System.Drawing.Point(132, 113);
            this.textBoxStudyExerciseType.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxStudyExerciseType.MaxLength = 32;
            this.textBoxStudyExerciseType.Name = "textBoxStudyExerciseType";
            this.textBoxStudyExerciseType.Size = new System.Drawing.Size(239, 22);
            this.textBoxStudyExerciseType.TabIndex = 6;
            // 
            // textBoxStudyCentre
            // 
            this.textBoxStudyCentre.Location = new System.Drawing.Point(132, 30);
            this.textBoxStudyCentre.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxStudyCentre.MaxLength = 32;
            this.textBoxStudyCentre.Name = "textBoxStudyCentre";
            this.textBoxStudyCentre.Size = new System.Drawing.Size(239, 22);
            this.textBoxStudyCentre.TabIndex = 4;
            // 
            // textBoxStudyInvestigator
            // 
            this.textBoxStudyInvestigator.Location = new System.Drawing.Point(132, 82);
            this.textBoxStudyInvestigator.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxStudyInvestigator.MaxLength = 32;
            this.textBoxStudyInvestigator.Name = "textBoxStudyInvestigator";
            this.textBoxStudyInvestigator.Size = new System.Drawing.Size(239, 22);
            this.textBoxStudyInvestigator.TabIndex = 5;
            // 
            // numericUpDownSessionID
            // 
            this.numericUpDownSessionID.Location = new System.Drawing.Point(165, 24);
            this.numericUpDownSessionID.Margin = new System.Windows.Forms.Padding(4);
            this.numericUpDownSessionID.Name = "numericUpDownSessionID";
            this.numericUpDownSessionID.Size = new System.Drawing.Size(160, 22);
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
            this.groupBox2.Location = new System.Drawing.Point(15, 56);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(4);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(4);
            this.groupBox2.Size = new System.Drawing.Size(776, 69);
            this.groupBox2.TabIndex = 32;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Sampling";
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
            this.groupBox3.Controls.Add(this.label2);
            this.groupBox3.Controls.Add(this.hoursPicker);
            this.groupBox3.Controls.Add(this.dayPicker);
            this.groupBox3.Controls.Add(this.label3);
            this.groupBox3.Controls.Add(this.label4);
            this.groupBox3.Location = new System.Drawing.Point(15, 135);
            this.groupBox3.Margin = new System.Windows.Forms.Padding(4);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Padding = new System.Windows.Forms.Padding(4);
            this.groupBox3.Size = new System.Drawing.Size(776, 211);
            this.groupBox3.TabIndex = 33;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Recording Time";
            // 
            // label25
            // 
            this.label25.AutoSize = true;
            this.label25.Location = new System.Drawing.Point(614, 137);
            this.label25.Name = "label25";
            this.label25.Size = new System.Drawing.Size(57, 17);
            this.label25.TabIndex = 27;
            this.label25.Text = "minutes";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(45, 176);
            this.label14.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(71, 17);
            this.label14.TabIndex = 23;
            this.label14.Text = "End Date:";
            // 
            // datePickerEnd
            // 
            this.datePickerEnd.Checked = false;
            this.datePickerEnd.CustomFormat = "dd/MM/yyyy";
            this.datePickerEnd.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.datePickerEnd.Location = new System.Drawing.Point(123, 176);
            this.datePickerEnd.Margin = new System.Windows.Forms.Padding(4);
            this.datePickerEnd.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.datePickerEnd.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.datePickerEnd.Name = "datePickerEnd";
            this.datePickerEnd.Size = new System.Drawing.Size(159, 22);
            this.datePickerEnd.TabIndex = 24;
            this.datePickerEnd.Value = new System.DateTime(2013, 2, 12, 0, 0, 0, 0);
            this.datePickerEnd.ValueChanged += new System.EventHandler(this.datePickerEnd_ValueChanged);
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(291, 176);
            this.label15.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(72, 17);
            this.label15.TabIndex = 26;
            this.label15.Text = "End Time:";
            // 
            // radioButtonDuration
            // 
            this.radioButtonDuration.AutoSize = true;
            this.radioButtonDuration.Location = new System.Drawing.Point(27, 64);
            this.radioButtonDuration.Margin = new System.Windows.Forms.Padding(4);
            this.radioButtonDuration.Name = "radioButtonDuration";
            this.radioButtonDuration.Size = new System.Drawing.Size(71, 21);
            this.radioButtonDuration.TabIndex = 1;
            this.radioButtonDuration.TabStop = true;
            this.radioButtonDuration.Text = "Set-up";
            this.radioButtonDuration.UseVisualStyleBackColor = true;
            // 
            // radioButtonImmediately
            // 
            this.radioButtonImmediately.AutoSize = true;
            this.radioButtonImmediately.Location = new System.Drawing.Point(27, 34);
            this.radioButtonImmediately.Margin = new System.Windows.Forms.Padding(4);
            this.radioButtonImmediately.Name = "radioButtonImmediately";
            this.radioButtonImmediately.Size = new System.Drawing.Size(197, 21);
            this.radioButtonImmediately.TabIndex = 0;
            this.radioButtonImmediately.TabStop = true;
            this.radioButtonImmediately.Text = "Immediately on Disconnect";
            this.radioButtonImmediately.UseVisualStyleBackColor = true;
            this.radioButtonImmediately.CheckedChanged += new System.EventHandler(this.radioButtonImmediately_CheckedChanged);
            // 
            // groupBox4
            // 
            this.groupBox4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox4.Controls.Add(this.comboBoxSide);
            this.groupBox4.Controls.Add(this.label27);
            this.groupBox4.Controls.Add(this.comboBoxSite);
            this.groupBox4.Controls.Add(this.label26);
            this.groupBox4.Controls.Add(this.checkBoxWeight);
            this.groupBox4.Controls.Add(this.checkBoxHeight);
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
            this.groupBox4.Location = new System.Drawing.Point(415, 352);
            this.groupBox4.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.groupBox4.Size = new System.Drawing.Size(376, 376);
            this.groupBox4.TabIndex = 34;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Subject";
            // 
            // checkBoxWeight
            // 
            this.checkBoxWeight.AutoSize = true;
            this.checkBoxWeight.Location = new System.Drawing.Point(132, 119);
            this.checkBoxWeight.Name = "checkBoxWeight";
            this.checkBoxWeight.Size = new System.Drawing.Size(18, 17);
            this.checkBoxWeight.TabIndex = 33;
            this.checkBoxWeight.UseVisualStyleBackColor = true;
            this.checkBoxWeight.CheckedChanged += new System.EventHandler(this.checkBoxWeight_CheckedChanged);
            // 
            // checkBoxHeight
            // 
            this.checkBoxHeight.AutoSize = true;
            this.checkBoxHeight.Location = new System.Drawing.Point(132, 91);
            this.checkBoxHeight.Name = "checkBoxHeight";
            this.checkBoxHeight.Size = new System.Drawing.Size(18, 17);
            this.checkBoxHeight.TabIndex = 32;
            this.checkBoxHeight.UseVisualStyleBackColor = true;
            this.checkBoxHeight.CheckedChanged += new System.EventHandler(this.checkBoxHeight_CheckedChanged);
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
            this.comboBoxSubjectTimezone.Location = new System.Drawing.Point(132, 345);
            this.comboBoxSubjectTimezone.Margin = new System.Windows.Forms.Padding(4);
            this.comboBoxSubjectTimezone.Name = "comboBoxSubjectTimezone";
            this.comboBoxSubjectTimezone.Size = new System.Drawing.Size(108, 24);
            this.comboBoxSubjectTimezone.TabIndex = 31;
            this.comboBoxSubjectTimezone.Text = "GMT +0";
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(5, 349);
            this.label16.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(70, 17);
            this.label16.TabIndex = 30;
            this.label16.Text = "Timezone";
            // 
            // numericUpDownSubjectWeight
            // 
            this.numericUpDownSubjectWeight.Location = new System.Drawing.Point(161, 116);
            this.numericUpDownSubjectWeight.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.numericUpDownSubjectWeight.Name = "numericUpDownSubjectWeight";
            this.numericUpDownSubjectWeight.Size = new System.Drawing.Size(120, 22);
            this.numericUpDownSubjectWeight.TabIndex = 17;
            // 
            // numericUpDownSubjectHeight
            // 
            this.numericUpDownSubjectHeight.Location = new System.Drawing.Point(161, 88);
            this.numericUpDownSubjectHeight.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.numericUpDownSubjectHeight.Name = "numericUpDownSubjectHeight";
            this.numericUpDownSubjectHeight.Size = new System.Drawing.Size(120, 22);
            this.numericUpDownSubjectHeight.TabIndex = 16;
            // 
            // comboBoxSubjectHandedness
            // 
            this.comboBoxSubjectHandedness.FormattingEnabled = true;
            this.comboBoxSubjectHandedness.Items.AddRange(new object[] {
            "",
            "left",
            "right"});
            this.comboBoxSubjectHandedness.Location = new System.Drawing.Point(132, 142);
            this.comboBoxSubjectHandedness.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.comboBoxSubjectHandedness.Name = "comboBoxSubjectHandedness";
            this.comboBoxSubjectHandedness.Size = new System.Drawing.Size(125, 24);
            this.comboBoxSubjectHandedness.TabIndex = 15;
            // 
            // label17
            // 
            this.label17.AutoSize = true;
            this.label17.Location = new System.Drawing.Point(289, 115);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(23, 17);
            this.label17.TabIndex = 14;
            this.label17.Text = "kg";
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(289, 88);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(26, 17);
            this.label18.TabIndex = 13;
            this.label18.Text = "cm";
            // 
            // comboBoxSubjectSex
            // 
            this.comboBoxSubjectSex.FormattingEnabled = true;
            this.comboBoxSubjectSex.Items.AddRange(new object[] {
            "",
            "male",
            "female"});
            this.comboBoxSubjectSex.Location = new System.Drawing.Point(132, 57);
            this.comboBoxSubjectSex.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.comboBoxSubjectSex.Name = "comboBoxSubjectSex";
            this.comboBoxSubjectSex.Size = new System.Drawing.Size(125, 24);
            this.comboBoxSubjectSex.TabIndex = 12;
            // 
            // textBoxSubjectNotes
            // 
            this.textBoxSubjectNotes.Location = new System.Drawing.Point(132, 235);
            this.textBoxSubjectNotes.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxSubjectNotes.MaxLength = 32;
            this.textBoxSubjectNotes.Multiline = true;
            this.textBoxSubjectNotes.Name = "textBoxSubjectNotes";
            this.textBoxSubjectNotes.Size = new System.Drawing.Size(239, 95);
            this.textBoxSubjectNotes.TabIndex = 11;
            // 
            // label19
            // 
            this.label19.AutoSize = true;
            this.label19.Location = new System.Drawing.Point(11, 237);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(45, 17);
            this.label19.TabIndex = 10;
            this.label19.Text = "Notes";
            // 
            // label20
            // 
            this.label20.AutoSize = true;
            this.label20.Location = new System.Drawing.Point(5, 60);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(31, 17);
            this.label20.TabIndex = 8;
            this.label20.Text = "Sex";
            // 
            // label21
            // 
            this.label21.AutoSize = true;
            this.label21.Location = new System.Drawing.Point(5, 30);
            this.label21.Name = "label21";
            this.label21.Size = new System.Drawing.Size(41, 17);
            this.label21.TabIndex = 0;
            this.label21.Text = "Code";
            // 
            // label22
            // 
            this.label22.AutoSize = true;
            this.label22.Location = new System.Drawing.Point(5, 86);
            this.label22.Name = "label22";
            this.label22.Size = new System.Drawing.Size(49, 17);
            this.label22.TabIndex = 1;
            this.label22.Text = "Height";
            // 
            // label23
            // 
            this.label23.AutoSize = true;
            this.label23.Location = new System.Drawing.Point(5, 116);
            this.label23.Name = "label23";
            this.label23.Size = new System.Drawing.Size(52, 17);
            this.label23.TabIndex = 2;
            this.label23.Text = "Weight";
            // 
            // label24
            // 
            this.label24.AutoSize = true;
            this.label24.Location = new System.Drawing.Point(5, 144);
            this.label24.Name = "label24";
            this.label24.Size = new System.Drawing.Size(88, 17);
            this.label24.TabIndex = 3;
            this.label24.Text = "Handedness";
            // 
            // textBoxSubjectCode
            // 
            this.textBoxSubjectCode.Location = new System.Drawing.Point(132, 30);
            this.textBoxSubjectCode.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxSubjectCode.MaxLength = 32;
            this.textBoxSubjectCode.Name = "textBoxSubjectCode";
            this.textBoxSubjectCode.Size = new System.Drawing.Size(239, 22);
            this.textBoxSubjectCode.TabIndex = 4;
            // 
            // comboBoxSite
            // 
            this.comboBoxSite.FormattingEnabled = true;
            this.comboBoxSite.Items.AddRange(new object[] {
            "",
            "wrist",
            "waist",
            "ankle",
            "thigh",
            "hip",
            "upper-arm",
            "chest",
            "sacrum",
            "neck",
            "head"});
            this.comboBoxSite.Location = new System.Drawing.Point(133, 172);
            this.comboBoxSite.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.comboBoxSite.Name = "comboBoxSite";
            this.comboBoxSite.Size = new System.Drawing.Size(125, 24);
            this.comboBoxSite.TabIndex = 35;
            // 
            // label26
            // 
            this.label26.AutoSize = true;
            this.label26.Location = new System.Drawing.Point(6, 174);
            this.label26.Name = "label26";
            this.label26.Size = new System.Drawing.Size(32, 17);
            this.label26.TabIndex = 34;
            this.label26.Text = "Site";
            // 
            // comboBoxSide
            // 
            this.comboBoxSide.FormattingEnabled = true;
            this.comboBoxSide.Items.AddRange(new object[] {
            "",
            "left",
            "right"});
            this.comboBoxSide.Location = new System.Drawing.Point(133, 203);
            this.comboBoxSide.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.comboBoxSide.Name = "comboBoxSide";
            this.comboBoxSide.Size = new System.Drawing.Size(125, 24);
            this.comboBoxSide.TabIndex = 37;
            // 
            // label27
            // 
            this.label27.AutoSize = true;
            this.label27.Location = new System.Drawing.Point(6, 205);
            this.label27.Name = "label27";
            this.label27.Size = new System.Drawing.Size(36, 17);
            this.label27.TabIndex = 36;
            this.label27.Text = "Side";
            // 
            // richTextBoxWarning
            // 
            this.richTextBoxWarning.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.richTextBoxWarning.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.richTextBoxWarning.Location = new System.Drawing.Point(5, 6);
            this.richTextBoxWarning.Name = "richTextBoxWarning";
            this.richTextBoxWarning.ReadOnly = true;
            this.richTextBoxWarning.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
            this.richTextBoxWarning.Size = new System.Drawing.Size(556, 87);
            this.richTextBoxWarning.TabIndex = 8;
            this.richTextBoxWarning.Text = "";
            // 
            // timePickerEnd
            // 
            this.timePickerEnd.Checked = false;
            this.timePickerEnd.CustomFormat = "HH:mm";
            this.timePickerEnd.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.timePickerEnd.Location = new System.Drawing.Point(371, 176);
            this.timePickerEnd.Margin = new System.Windows.Forms.Padding(4);
            this.timePickerEnd.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.timePickerEnd.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.timePickerEnd.Name = "timePickerEnd";
            this.timePickerEnd.Size = new System.Drawing.Size(155, 22);
            this.timePickerEnd.TabIndex = 28;
            this.timePickerEnd.Value = new System.DateTime(2013, 3, 13, 12, 0, 0, 0);
            this.timePickerEnd.ValueChanged += new System.EventHandler(this.timePickerEnd_ValueChanged);
            // 
            // timePickerStart
            // 
            this.timePickerStart.Checked = false;
            this.timePickerStart.CustomFormat = "HH:mm";
            this.timePickerStart.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.timePickerStart.Location = new System.Drawing.Point(376, 98);
            this.timePickerStart.Margin = new System.Windows.Forms.Padding(4);
            this.timePickerStart.MaxDate = new System.DateTime(2063, 12, 31, 0, 0, 0, 0);
            this.timePickerStart.MinDate = new System.DateTime(2000, 3, 1, 0, 0, 0, 0);
            this.timePickerStart.Name = "timePickerStart";
            this.timePickerStart.Size = new System.Drawing.Size(155, 22);
            this.timePickerStart.TabIndex = 29;
            this.timePickerStart.Value = new System.DateTime(2013, 3, 13, 12, 0, 0, 0);
            this.timePickerStart.ValueChanged += new System.EventHandler(this.timePickerStart_ValueChanged);
            // 
            // DateRangeForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.ClientSize = new System.Drawing.Size(809, 838);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.numericUpDownSessionID);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.panel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "DateRangeForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Date Range";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.DateRangeForm_FormClosing);
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
        private System.Windows.Forms.DateTimePicker datePickerStart;
        private System.Windows.Forms.Label labelFrom;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
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
        private System.Windows.Forms.DateTimePicker datePickerEnd;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.RadioButton radioButtonDuration;
        private System.Windows.Forms.RadioButton radioButtonImmediately;
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
        private System.Windows.Forms.CheckBox checkBoxWeight;
        private System.Windows.Forms.CheckBox checkBoxHeight;
        private System.Windows.Forms.Label label25;
        private System.Windows.Forms.ComboBox comboBoxSide;
        private System.Windows.Forms.Label label27;
        private System.Windows.Forms.ComboBox comboBoxSite;
        private System.Windows.Forms.Label label26;
        private System.Windows.Forms.RichTextBox richTextBoxWarning;
        private System.Windows.Forms.DateTimePicker timePickerEnd;
        private System.Windows.Forms.DateTimePicker timePickerStart;
    }
}
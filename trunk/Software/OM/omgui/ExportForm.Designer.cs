namespace OmGui
{
    partial class ExportForm
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
            this.buttonConvert = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.labelFileSource = new System.Windows.Forms.Label();
            this.labelFileOutput = new System.Windows.Forms.Label();
            this.textBoxSourceFile = new System.Windows.Forms.TextBox();
            this.textBoxOutputFile = new System.Windows.Forms.TextBox();
            this.buttonBrowse = new System.Windows.Forms.Button();
            this.groupBoxStream = new System.Windows.Forms.GroupBox();
            this.radioButtonStreamGyro = new System.Windows.Forms.RadioButton();
            this.radioButtonStreamAccel = new System.Windows.Forms.RadioButton();
            this.groupBoxValues = new System.Windows.Forms.GroupBox();
            this.radioButtonValuesInt = new System.Windows.Forms.RadioButton();
            this.radioButtonValuesFloat = new System.Windows.Forms.RadioButton();
            this.groupBoxTimestamps = new System.Windows.Forms.GroupBox();
            this.radioButtonTimeTimestamp = new System.Windows.Forms.RadioButton();
            this.radioButtonTimeMatlab = new System.Windows.Forms.RadioButton();
            this.radioButtonTimeExcel = new System.Windows.Forms.RadioButton();
            this.radioButtonTimeDays = new System.Windows.Forms.RadioButton();
            this.radioButtonTimeSerial = new System.Windows.Forms.RadioButton();
            this.radioButtonTimeSecs = new System.Windows.Forms.RadioButton();
            this.radioButtonTimeSequence = new System.Windows.Forms.RadioButton();
            this.radioButtonTimeNone = new System.Windows.Forms.RadioButton();
            this.groupBoxSubSample = new System.Windows.Forms.GroupBox();
            this.textBoxSampleStep = new System.Windows.Forms.TextBox();
            this.textBoxSampleLength = new System.Windows.Forms.TextBox();
            this.textBoxSampleStart = new System.Windows.Forms.TextBox();
            this.labelSampleInterval = new System.Windows.Forms.Label();
            this.labelSampleCount = new System.Windows.Forms.Label();
            this.labelSampleFirst = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.textBoxBlockCount = new System.Windows.Forms.TextBox();
            this.textBoxBlockStart = new System.Windows.Forms.TextBox();
            this.labelBlockCount = new System.Windows.Forms.Label();
            this.labelBlockStart = new System.Windows.Forms.Label();
            this.groupBoxStream.SuspendLayout();
            this.groupBoxValues.SuspendLayout();
            this.groupBoxTimestamps.SuspendLayout();
            this.groupBoxSubSample.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonConvert
            // 
            this.buttonConvert.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonConvert.Location = new System.Drawing.Point(266, 397);
            this.buttonConvert.Name = "buttonConvert";
            this.buttonConvert.Size = new System.Drawing.Size(75, 23);
            this.buttonConvert.TabIndex = 0;
            this.buttonConvert.Text = "C&onvert";
            this.buttonConvert.UseVisualStyleBackColor = true;
            this.buttonConvert.Click += new System.EventHandler(this.buttonConvert_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(348, 397);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 1;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // saveFileDialog
            // 
            this.saveFileDialog.DefaultExt = "csv";
            this.saveFileDialog.Filter = "CSV Files (*.csv)|*.csv|All files (*.*)|*.*";
            this.saveFileDialog.Title = "CSV File";
            // 
            // labelFileSource
            // 
            this.labelFileSource.AutoSize = true;
            this.labelFileSource.Location = new System.Drawing.Point(12, 15);
            this.labelFileSource.Name = "labelFileSource";
            this.labelFileSource.Size = new System.Drawing.Size(63, 13);
            this.labelFileSource.TabIndex = 5;
            this.labelFileSource.Text = "Source File:";
            // 
            // labelFileOutput
            // 
            this.labelFileOutput.AutoSize = true;
            this.labelFileOutput.Location = new System.Drawing.Point(12, 45);
            this.labelFileOutput.Name = "labelFileOutput";
            this.labelFileOutput.Size = new System.Drawing.Size(61, 13);
            this.labelFileOutput.TabIndex = 6;
            this.labelFileOutput.Text = "Output File:";
            // 
            // textBoxSourceFile
            // 
            this.textBoxSourceFile.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxSourceFile.Location = new System.Drawing.Point(91, 12);
            this.textBoxSourceFile.Name = "textBoxSourceFile";
            this.textBoxSourceFile.ReadOnly = true;
            this.textBoxSourceFile.Size = new System.Drawing.Size(332, 20);
            this.textBoxSourceFile.TabIndex = 7;
            // 
            // textBoxOutputFile
            // 
            this.textBoxOutputFile.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxOutputFile.Location = new System.Drawing.Point(91, 42);
            this.textBoxOutputFile.Name = "textBoxOutputFile";
            this.textBoxOutputFile.Size = new System.Drawing.Size(247, 20);
            this.textBoxOutputFile.TabIndex = 8;
            // 
            // buttonBrowse
            // 
            this.buttonBrowse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonBrowse.Location = new System.Drawing.Point(351, 42);
            this.buttonBrowse.Name = "buttonBrowse";
            this.buttonBrowse.Size = new System.Drawing.Size(71, 23);
            this.buttonBrowse.TabIndex = 9;
            this.buttonBrowse.Text = "Browse...";
            this.buttonBrowse.UseVisualStyleBackColor = true;
            this.buttonBrowse.Click += new System.EventHandler(this.buttonBrowse_Click);
            // 
            // groupBoxStream
            // 
            this.groupBoxStream.Controls.Add(this.radioButtonStreamGyro);
            this.groupBoxStream.Controls.Add(this.radioButtonStreamAccel);
            this.groupBoxStream.Location = new System.Drawing.Point(15, 75);
            this.groupBoxStream.Name = "groupBoxStream";
            this.groupBoxStream.Size = new System.Drawing.Size(197, 67);
            this.groupBoxStream.TabIndex = 10;
            this.groupBoxStream.TabStop = false;
            this.groupBoxStream.Text = "&Stream";
            // 
            // radioButtonStreamGyro
            // 
            this.radioButtonStreamGyro.AutoSize = true;
            this.radioButtonStreamGyro.Location = new System.Drawing.Point(6, 42);
            this.radioButtonStreamGyro.Name = "radioButtonStreamGyro";
            this.radioButtonStreamGyro.Size = new System.Drawing.Size(76, 17);
            this.radioButtonStreamGyro.TabIndex = 0;
            this.radioButtonStreamGyro.Text = "Gyroscope";
            this.radioButtonStreamGyro.UseVisualStyleBackColor = true;
            // 
            // radioButtonStreamAccel
            // 
            this.radioButtonStreamAccel.AutoSize = true;
            this.radioButtonStreamAccel.Checked = true;
            this.radioButtonStreamAccel.Location = new System.Drawing.Point(6, 19);
            this.radioButtonStreamAccel.Name = "radioButtonStreamAccel";
            this.radioButtonStreamAccel.Size = new System.Drawing.Size(93, 17);
            this.radioButtonStreamAccel.TabIndex = 0;
            this.radioButtonStreamAccel.TabStop = true;
            this.radioButtonStreamAccel.Text = "Accelerometer";
            this.radioButtonStreamAccel.UseVisualStyleBackColor = true;
            // 
            // groupBoxValues
            // 
            this.groupBoxValues.Controls.Add(this.radioButtonValuesInt);
            this.groupBoxValues.Controls.Add(this.radioButtonValuesFloat);
            this.groupBoxValues.Location = new System.Drawing.Point(15, 148);
            this.groupBoxValues.Name = "groupBoxValues";
            this.groupBoxValues.Size = new System.Drawing.Size(197, 71);
            this.groupBoxValues.TabIndex = 10;
            this.groupBoxValues.TabStop = false;
            this.groupBoxValues.Text = "&Values";
            // 
            // radioButtonValuesInt
            // 
            this.radioButtonValuesInt.AutoSize = true;
            this.radioButtonValuesInt.Location = new System.Drawing.Point(6, 42);
            this.radioButtonValuesInt.Name = "radioButtonValuesInt";
            this.radioButtonValuesInt.Size = new System.Drawing.Size(47, 17);
            this.radioButtonValuesInt.TabIndex = 0;
            this.radioButtonValuesInt.Text = "Raw";
            this.radioButtonValuesInt.UseVisualStyleBackColor = true;
            // 
            // radioButtonValuesFloat
            // 
            this.radioButtonValuesFloat.AutoSize = true;
            this.radioButtonValuesFloat.Checked = true;
            this.radioButtonValuesFloat.Location = new System.Drawing.Point(6, 19);
            this.radioButtonValuesFloat.Name = "radioButtonValuesFloat";
            this.radioButtonValuesFloat.Size = new System.Drawing.Size(33, 17);
            this.radioButtonValuesFloat.TabIndex = 0;
            this.radioButtonValuesFloat.TabStop = true;
            this.radioButtonValuesFloat.Text = "G";
            this.radioButtonValuesFloat.UseVisualStyleBackColor = true;
            // 
            // groupBoxTimestamps
            // 
            this.groupBoxTimestamps.Controls.Add(this.radioButtonTimeTimestamp);
            this.groupBoxTimestamps.Controls.Add(this.radioButtonTimeMatlab);
            this.groupBoxTimestamps.Controls.Add(this.radioButtonTimeExcel);
            this.groupBoxTimestamps.Controls.Add(this.radioButtonTimeDays);
            this.groupBoxTimestamps.Controls.Add(this.radioButtonTimeSerial);
            this.groupBoxTimestamps.Controls.Add(this.radioButtonTimeSecs);
            this.groupBoxTimestamps.Controls.Add(this.radioButtonTimeSequence);
            this.groupBoxTimestamps.Controls.Add(this.radioButtonTimeNone);
            this.groupBoxTimestamps.Location = new System.Drawing.Point(218, 80);
            this.groupBoxTimestamps.Name = "groupBoxTimestamps";
            this.groupBoxTimestamps.Size = new System.Drawing.Size(205, 210);
            this.groupBoxTimestamps.TabIndex = 10;
            this.groupBoxTimestamps.TabStop = false;
            this.groupBoxTimestamps.Text = "&Timestamps";
            // 
            // radioButtonTimeTimestamp
            // 
            this.radioButtonTimeTimestamp.AutoSize = true;
            this.radioButtonTimeTimestamp.Checked = true;
            this.radioButtonTimeTimestamp.Location = new System.Drawing.Point(6, 65);
            this.radioButtonTimeTimestamp.Name = "radioButtonTimeTimestamp";
            this.radioButtonTimeTimestamp.Size = new System.Drawing.Size(145, 17);
            this.radioButtonTimeTimestamp.TabIndex = 0;
            this.radioButtonTimeTimestamp.TabStop = true;
            this.radioButtonTimeTimestamp.Text = "Formatted (Y-M-D h:m:s.f)";
            this.radioButtonTimeTimestamp.UseVisualStyleBackColor = true;
            // 
            // radioButtonTimeMatlab
            // 
            this.radioButtonTimeMatlab.AutoSize = true;
            this.radioButtonTimeMatlab.Location = new System.Drawing.Point(6, 180);
            this.radioButtonTimeMatlab.Name = "radioButtonTimeMatlab";
            this.radioButtonTimeMatlab.Size = new System.Drawing.Size(139, 17);
            this.radioButtonTimeMatlab.TabIndex = 0;
            this.radioButtonTimeMatlab.Text = "Fractional Days (Matlab)";
            this.radioButtonTimeMatlab.UseVisualStyleBackColor = true;
            // 
            // radioButtonTimeExcel
            // 
            this.radioButtonTimeExcel.AutoSize = true;
            this.radioButtonTimeExcel.Location = new System.Drawing.Point(6, 157);
            this.radioButtonTimeExcel.Name = "radioButtonTimeExcel";
            this.radioButtonTimeExcel.Size = new System.Drawing.Size(133, 17);
            this.radioButtonTimeExcel.TabIndex = 0;
            this.radioButtonTimeExcel.Text = "Fractional Days (Excel)";
            this.radioButtonTimeExcel.UseVisualStyleBackColor = true;
            // 
            // radioButtonTimeDays
            // 
            this.radioButtonTimeDays.AutoSize = true;
            this.radioButtonTimeDays.Location = new System.Drawing.Point(6, 134);
            this.radioButtonTimeDays.Name = "radioButtonTimeDays";
            this.radioButtonTimeDays.Size = new System.Drawing.Size(141, 17);
            this.radioButtonTimeDays.TabIndex = 0;
            this.radioButtonTimeDays.Text = "Fractional Days (relative)";
            this.radioButtonTimeDays.UseVisualStyleBackColor = true;
            // 
            // radioButtonTimeSerial
            // 
            this.radioButtonTimeSerial.AutoSize = true;
            this.radioButtonTimeSerial.Location = new System.Drawing.Point(6, 111);
            this.radioButtonTimeSerial.Name = "radioButtonTimeSerial";
            this.radioButtonTimeSerial.Size = new System.Drawing.Size(130, 17);
            this.radioButtonTimeSerial.TabIndex = 0;
            this.radioButtonTimeSerial.Text = "Seconds (Unix epoch)";
            this.radioButtonTimeSerial.UseVisualStyleBackColor = true;
            // 
            // radioButtonTimeSecs
            // 
            this.radioButtonTimeSecs.AutoSize = true;
            this.radioButtonTimeSecs.Location = new System.Drawing.Point(6, 88);
            this.radioButtonTimeSecs.Name = "radioButtonTimeSecs";
            this.radioButtonTimeSecs.Size = new System.Drawing.Size(110, 17);
            this.radioButtonTimeSecs.TabIndex = 0;
            this.radioButtonTimeSecs.Text = "Seconds (relative)";
            this.radioButtonTimeSecs.UseVisualStyleBackColor = true;
            // 
            // radioButtonTimeSequence
            // 
            this.radioButtonTimeSequence.AutoSize = true;
            this.radioButtonTimeSequence.Location = new System.Drawing.Point(6, 42);
            this.radioButtonTimeSequence.Name = "radioButtonTimeSequence";
            this.radioButtonTimeSequence.Size = new System.Drawing.Size(100, 17);
            this.radioButtonTimeSequence.TabIndex = 0;
            this.radioButtonTimeSequence.Text = "Sample Number";
            this.radioButtonTimeSequence.UseVisualStyleBackColor = true;
            // 
            // radioButtonTimeNone
            // 
            this.radioButtonTimeNone.AutoSize = true;
            this.radioButtonTimeNone.Location = new System.Drawing.Point(6, 19);
            this.radioButtonTimeNone.Name = "radioButtonTimeNone";
            this.radioButtonTimeNone.Size = new System.Drawing.Size(51, 17);
            this.radioButtonTimeNone.TabIndex = 0;
            this.radioButtonTimeNone.Text = "None";
            this.radioButtonTimeNone.UseVisualStyleBackColor = true;
            // 
            // groupBoxSubSample
            // 
            this.groupBoxSubSample.Controls.Add(this.textBoxSampleStep);
            this.groupBoxSubSample.Controls.Add(this.textBoxSampleLength);
            this.groupBoxSubSample.Controls.Add(this.textBoxSampleStart);
            this.groupBoxSubSample.Controls.Add(this.labelSampleInterval);
            this.groupBoxSubSample.Controls.Add(this.labelSampleCount);
            this.groupBoxSubSample.Controls.Add(this.labelSampleFirst);
            this.groupBoxSubSample.Location = new System.Drawing.Point(15, 225);
            this.groupBoxSubSample.Name = "groupBoxSubSample";
            this.groupBoxSubSample.Size = new System.Drawing.Size(197, 110);
            this.groupBoxSubSample.TabIndex = 10;
            this.groupBoxSubSample.TabStop = false;
            this.groupBoxSubSample.Text = "Su&b-Sample";
            // 
            // textBoxSampleStep
            // 
            this.textBoxSampleStep.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxSampleStep.Location = new System.Drawing.Point(55, 75);
            this.textBoxSampleStep.Name = "textBoxSampleStep";
            this.textBoxSampleStep.Size = new System.Drawing.Size(136, 20);
            this.textBoxSampleStep.TabIndex = 1;
            // 
            // textBoxSampleLength
            // 
            this.textBoxSampleLength.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxSampleLength.Location = new System.Drawing.Point(55, 49);
            this.textBoxSampleLength.Name = "textBoxSampleLength";
            this.textBoxSampleLength.Size = new System.Drawing.Size(136, 20);
            this.textBoxSampleLength.TabIndex = 1;
            // 
            // textBoxSampleStart
            // 
            this.textBoxSampleStart.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxSampleStart.Location = new System.Drawing.Point(55, 23);
            this.textBoxSampleStart.Name = "textBoxSampleStart";
            this.textBoxSampleStart.Size = new System.Drawing.Size(136, 20);
            this.textBoxSampleStart.TabIndex = 1;
            // 
            // labelSampleInterval
            // 
            this.labelSampleInterval.AutoSize = true;
            this.labelSampleInterval.Location = new System.Drawing.Point(6, 78);
            this.labelSampleInterval.Name = "labelSampleInterval";
            this.labelSampleInterval.Size = new System.Drawing.Size(45, 13);
            this.labelSampleInterval.TabIndex = 0;
            this.labelSampleInterval.Text = "Interval:";
            // 
            // labelSampleCount
            // 
            this.labelSampleCount.AutoSize = true;
            this.labelSampleCount.Location = new System.Drawing.Point(6, 52);
            this.labelSampleCount.Name = "labelSampleCount";
            this.labelSampleCount.Size = new System.Drawing.Size(38, 13);
            this.labelSampleCount.TabIndex = 0;
            this.labelSampleCount.Text = "Count:";
            // 
            // labelSampleFirst
            // 
            this.labelSampleFirst.AutoSize = true;
            this.labelSampleFirst.Location = new System.Drawing.Point(6, 26);
            this.labelSampleFirst.Name = "labelSampleFirst";
            this.labelSampleFirst.Size = new System.Drawing.Size(29, 13);
            this.labelSampleFirst.TabIndex = 0;
            this.labelSampleFirst.Text = "First:";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.textBoxBlockCount);
            this.groupBox1.Controls.Add(this.textBoxBlockStart);
            this.groupBox1.Controls.Add(this.labelBlockCount);
            this.groupBox1.Controls.Add(this.labelBlockStart);
            this.groupBox1.Location = new System.Drawing.Point(15, 341);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(197, 75);
            this.groupBox1.TabIndex = 10;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Specific Blocks";
            // 
            // textBoxBlockCount
            // 
            this.textBoxBlockCount.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxBlockCount.Location = new System.Drawing.Point(55, 49);
            this.textBoxBlockCount.Name = "textBoxBlockCount";
            this.textBoxBlockCount.Size = new System.Drawing.Size(136, 20);
            this.textBoxBlockCount.TabIndex = 1;
            // 
            // textBoxBlockStart
            // 
            this.textBoxBlockStart.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxBlockStart.Location = new System.Drawing.Point(55, 23);
            this.textBoxBlockStart.Name = "textBoxBlockStart";
            this.textBoxBlockStart.Size = new System.Drawing.Size(136, 20);
            this.textBoxBlockStart.TabIndex = 1;
            // 
            // labelBlockCount
            // 
            this.labelBlockCount.AutoSize = true;
            this.labelBlockCount.Location = new System.Drawing.Point(6, 52);
            this.labelBlockCount.Name = "labelBlockCount";
            this.labelBlockCount.Size = new System.Drawing.Size(38, 13);
            this.labelBlockCount.TabIndex = 0;
            this.labelBlockCount.Text = "Count:";
            // 
            // labelBlockStart
            // 
            this.labelBlockStart.AutoSize = true;
            this.labelBlockStart.Location = new System.Drawing.Point(6, 26);
            this.labelBlockStart.Name = "labelBlockStart";
            this.labelBlockStart.Size = new System.Drawing.Size(32, 13);
            this.labelBlockStart.TabIndex = 0;
            this.labelBlockStart.Text = "Start:";
            // 
            // ExportForm
            // 
            this.AcceptButton = this.buttonConvert;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(434, 432);
            this.Controls.Add(this.groupBoxTimestamps);
            this.Controls.Add(this.groupBoxValues);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.groupBoxSubSample);
            this.Controls.Add(this.groupBoxStream);
            this.Controls.Add(this.labelFileSource);
            this.Controls.Add(this.labelFileOutput);
            this.Controls.Add(this.textBoxSourceFile);
            this.Controls.Add(this.textBoxOutputFile);
            this.Controls.Add(this.buttonBrowse);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonConvert);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ExportForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.Text = "Export / Convert";
            this.Load += new System.EventHandler(this.ExportForm_Load);
            this.groupBoxStream.ResumeLayout(false);
            this.groupBoxStream.PerformLayout();
            this.groupBoxValues.ResumeLayout(false);
            this.groupBoxValues.PerformLayout();
            this.groupBoxTimestamps.ResumeLayout(false);
            this.groupBoxTimestamps.PerformLayout();
            this.groupBoxSubSample.ResumeLayout(false);
            this.groupBoxSubSample.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonConvert;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.Label labelFileSource;
        private System.Windows.Forms.Label labelFileOutput;
        private System.Windows.Forms.TextBox textBoxSourceFile;
        private System.Windows.Forms.TextBox textBoxOutputFile;
        private System.Windows.Forms.Button buttonBrowse;
        private System.Windows.Forms.GroupBox groupBoxStream;
        private System.Windows.Forms.RadioButton radioButtonStreamGyro;
        private System.Windows.Forms.RadioButton radioButtonStreamAccel;
        private System.Windows.Forms.GroupBox groupBoxValues;
        private System.Windows.Forms.RadioButton radioButtonValuesInt;
        private System.Windows.Forms.RadioButton radioButtonValuesFloat;
        private System.Windows.Forms.GroupBox groupBoxTimestamps;
        private System.Windows.Forms.RadioButton radioButtonTimeTimestamp;
        private System.Windows.Forms.RadioButton radioButtonTimeMatlab;
        private System.Windows.Forms.RadioButton radioButtonTimeExcel;
        private System.Windows.Forms.RadioButton radioButtonTimeDays;
        private System.Windows.Forms.RadioButton radioButtonTimeSerial;
        private System.Windows.Forms.RadioButton radioButtonTimeSecs;
        private System.Windows.Forms.RadioButton radioButtonTimeSequence;
        private System.Windows.Forms.RadioButton radioButtonTimeNone;
        private System.Windows.Forms.GroupBox groupBoxSubSample;
        private System.Windows.Forms.TextBox textBoxSampleStep;
        private System.Windows.Forms.TextBox textBoxSampleLength;
        private System.Windows.Forms.TextBox textBoxSampleStart;
        private System.Windows.Forms.Label labelSampleInterval;
        private System.Windows.Forms.Label labelSampleCount;
        private System.Windows.Forms.Label labelSampleFirst;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox textBoxBlockCount;
        private System.Windows.Forms.TextBox textBoxBlockStart;
        private System.Windows.Forms.Label labelBlockCount;
        private System.Windows.Forms.Label labelBlockStart;
    }
}
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using OmApiNet;

namespace OmGui
{
    public partial class DateRangeForm : Form
    {
        private DateRangeMetadataForm drmf;

        bool setOK = false;

        public DateRangeForm(string title, string prompt, OmDevice device)
        {
            InitializeComponent();
            Text = title;
            //labelPrompt.Text = prompt;
            FromDate = DateTime.MinValue;
            //UntilDate = DateTime.MaxValue;

            timePicker.Format = DateTimePickerFormat.Time;
            timePicker.ShowUpDown = true;
        }

        public bool Always { get; set; }

        public int SessionID { get; set; }

        public enum SyncTimeType
        {
            None,
            PC = 1,
            Zero = 2
        }

        public SyncTimeType SyncTime { get; set; }

        public DateTime FromDate
        {
            get
            {
                if (!datePicker.Checked) { return DateTime.MinValue; }
                return datePicker.Value; 
            }
            set
            {
                if (value < datePicker.MinDate)
                {
                    datePicker.Checked = false;
                    datePicker.Value = datePicker.MinDate;
                }
                else if (value > datePicker.MaxDate)
                {
                    datePicker.Checked = false;
                    datePicker.Value = datePicker.MaxDate;
                }
                else
                {
                    datePicker.Value = value;
                }

            }
        }

        public DateTime UntilDate
        {
            get
            {
                return DateTime.Now;
            }
        }

        //public DateTime UntilDate
        //{
        //    get 
        //    {
        //        if (!dateTimePickerUntil.Checked) { return DateTime.MaxValue; }
        //        return dateTimePickerUntil.Value; 
        //    }
        //    set
        //    {
        //        if (value < dateTimePickerUntil.MinDate)
        //        {
        //            dateTimePickerUntil.Checked = false;
        //            dateTimePickerUntil.Value = dateTimePickerUntil.MinDate;
        //        }
        //        else if (value > dateTimePickerUntil.MaxDate)
        //        {
        //            dateTimePickerUntil.Checked = false;
        //            dateTimePickerUntil.Value = dateTimePickerUntil.MaxDate;
        //        }
        //        else
        //        {
        //            dateTimePickerUntil.Value = value;
        //        }

        //    }
        //}


        private void buttonOk_Click(object sender, EventArgs e)
        {
            setOK = true;

            if (syncToPCCheckBox.Checked)
                SyncTime = SyncTimeType.PC;
            else
                SyncTime = SyncTimeType.Zero;

            SessionID = Int32.Parse(sessionIdTextBox.Text);
            DialogResult = System.Windows.Forms.DialogResult.OK;

            //TS - TODO - Build UntilDate from the data provided.
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            DialogResult = System.Windows.Forms.DialogResult.Cancel;
            Close();
        }

        private void DateRangeForm_Load(object sender, EventArgs e)
        {
            Always = false;
            SyncTime = SyncTimeType.None;
        }

        private void alwaysCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (alwaysCheckBox.Checked)
            {
                datePicker.Enabled = false;
                timePicker.Enabled = false;
                dayPicker.Enabled = false;
                hoursPicker.Enabled = false;
                minutesPicker.Enabled = false;
            }
            else
            {
                datePicker.Enabled = true;
                timePicker.Enabled = true;
                dayPicker.Enabled = true;
                hoursPicker.Enabled = true;
                minutesPicker.Enabled = true;
            }
        }

        private void buttonSetupMetadata_Click(object sender, EventArgs e)
        {
            //Using the same form so we can store data in it rather than mirroring data here.
            if(drmf == null)
                drmf = new DateRangeMetadataForm();

            drmf.ShowDialog();
        }

        // Roughly estimate battery life (in seconds) based on percentage remaining and sampling frequency
        static double EstimateBatteryLife(int percent, int rate)
        {
            const int percentReserved = 10;
            const double dischargeRate100Hz = 0.15;
            double dischargeRate;
            if (rate <= 100) { dischargeRate = dischargeRate100Hz; }
            else { dischargeRate = dischargeRate100Hz * rate / 100; }
            if (percent < percentReserved) { return 0; }
            return (percent - percentReserved) / dischargeRate * 60 * 60;
        }

        static double EstimateCapacityFromBytesFree(long bytesFree, int rate)
        {
            long clustersFree = (bytesFree / 32768);
            if (clustersFree <= 0) { return 0; }
            long numSamples = (clustersFree * (32768 / 512) - 2) * 120;    // assume 32kB clusters, 120 samples per sector, reserve two sectors for header
            return (numSamples / (1.06 * rate));      // assume actual sampling rate could be up to 6% over
        }

        private void DateRangeForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            //TS - TODO - Do warnings
            DateTime startDate = datePicker.Value;
            DateTime endDate = startDate.Add(new TimeSpan((int)dayPicker.Value, (int)hoursPicker.Value, (int)minutesPicker.Value, 0));


            //Only show warnings if we are closing because of OK
            if (setOK)
            {
                //If more than 25 days give warning
                if (dayPicker.Value > 25)
                {
                    DialogResult dr = MessageBox.Show("The record length is very long and the device may not last this long", "Warning - Record length", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning);

                    if (dr != System.Windows.Forms.DialogResult.OK)
                        e.Cancel = true;
                }
            }
        }
    }
}

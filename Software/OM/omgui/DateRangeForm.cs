// TODO: Save delayDays and timeOfDay (if delayDays > 0), rate and range.
// TODO: Delay before refreshing preview (and fix problem of preview staying after erase).

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Xml;

using OmApiNet;

namespace OmGui
{
    public partial class DateRangeForm : Form
    {
        //bool setOK = false;
        //bool warningsOn = true;

        private Dictionary<string, string> SettingsProfileDictionary { get; set; }
        private string settingsProfileFilePath = Path.Combine(Properties.Settings.Default.CurrentWorkingFolder, "recordSetup.xml");
        //private OmDevice Device { get; set; }
        private OmDevice[] Devices { get; set; }

        private bool hasSyncGyro = false;

        public DateRangeForm(string title, OmDevice[] devices)
        {
            // Gyro enabled
            hasSyncGyro = devices.Length > 0 ? true : false;
            foreach (var device in devices)
            {
                hasSyncGyro &= device.HasSyncGyro;
            }

            Devices = devices;

            //if (devices.Length > 0)
            //    Device = devices[0];

            InitializeComponent();

            Text = title;
            //labelPrompt.Text = prompt;
            //FromDate = DateTime.MinValue;
            //UntilDate = DateTime.MaxValue;

            DateTime now = DateTime.Now;
            StartDate = now.Date;
            timePickerStart.ShowUpDown = true;
            timePickerEnd.ShowUpDown = true;
            Duration = new TimeSpan(0, 0, 0, 0);

            //Set radio buttons enabled
            radioButtonImmediately.Checked = true;

            // Defaults before load
            buttonDefault_Click(null, null);

            //Try and load in record dialog settings.
            XmlDocument doc;
            SettingsProfileDictionary = loadSettingsProfile(out doc);
            if (SettingsProfileDictionary != null)
            {
                resetFieldsToDictionary(SettingsProfileDictionary);
            }
            else
            {
                SettingsProfileDictionary = new Dictionary<string, string>();
            }

            ////Set the height/weight enabled
            //checkBoxHeight.Checked = true;
            //checkBoxWeight.Checked = true;

            //Set default recording times
            if (delayDaysPicker.Value == 0)
            {
                // If no day delay specified, reset the time-of-day to now
                StartDate = StartDate.Date + now.TimeOfDay;
            }
            EndDate = StartDate + Duration;

            // Hack?
            datePickerStart.Visible = false;
            datePickerEnd.Visible = false;
            datePickerStart.Visible = true;
            datePickerEnd.Visible = true;

            // Gyro
            comboBoxGyroRange.SelectedIndex = 0;
            if (hasSyncGyro)
            {
                // No low-power option
                checkBoxLowPower.Visible = false;
                checkBoxLowPower.Checked = false;
                // Always unpacked data
                checkBoxUnpacked.Visible = false;
                checkBoxUnpacked.Checked = false;
                // Gyro range
                label17.Visible = true;
                comboBoxGyroRange.Visible = true;
            }

            updateWarningMessages();

        }

        private Dictionary<string, string> loadSettingsProfile(out XmlDocument xmlDocument)
        {
            xmlDocument = new XmlDocument();

            Console.WriteLine(Properties.Settings.Default.CurrentWorkingFolder + "\\" + "recordSetup.xml");

            //Want to look if the XML exists in this working directory and if it does then we want to load the defaults in.
            if (File.Exists(Properties.Settings.Default.CurrentWorkingFolder + "\\" + "recordSetup.xml"))
            {
                Dictionary<string, string> settingsDictionary = new Dictionary<string, string>();

                StreamReader recordProfile = new StreamReader(Properties.Settings.Default.CurrentWorkingFolder +
                    "\\" + "recordSetup.xml");
                String profileAsString = recordProfile.ReadToEnd();
                xmlDocument.LoadXml(profileAsString);

                XmlNode rootNode = xmlDocument.DocumentElement;

                foreach (XmlNode node in rootNode.ChildNodes)
                {
                    if(!node.Name.Equals("RecordProfile"))
                        settingsDictionary.Add(node.Name, node.InnerText);
                }

                return settingsDictionary;
            }

            return null;
        }

        private void saveDictionaryFromFields(Dictionary<string, string> settingsDictionary)
        {
            settingsDictionary.Clear();

            settingsDictionary.Add("StudyCentre", textBoxStudyCentre.Text);
            settingsDictionary.Add("StudyCode", textBoxStudyCode.Text);
            settingsDictionary.Add("StudyInvestigator", textBoxStudyInvestigator.Text);
            settingsDictionary.Add("StudyExerciseType", textBoxStudyExerciseType.Text);
            settingsDictionary.Add("StudyOperator", textBoxStudyOperator.Text);
            settingsDictionary.Add("StudyNotes", textBoxStudyNotes.Text);
            settingsDictionary.Add("SubjectCode", textBoxSubjectCode.Text);
            settingsDictionary.Add("SubjectSex", comboBoxSubjectSex.SelectedIndex <= 0 ? "" : comboBoxSubjectSex.SelectedItem.ToString());
            settingsDictionary.Add("SubjectHeight", textBoxHeight.Text.ToString());
            settingsDictionary.Add("SubjectWeight", textBoxWeight.Text.ToString());
            settingsDictionary.Add("SubjectHandedness", comboBoxSubjectHandedness.SelectedIndex <= 0 ? "" : comboBoxSubjectHandedness.SelectedItem.ToString());
            //settingsDictionary.Add("SubjectTimezone", comboBoxSubjectTimezone.SelectedIndex.ToString());
            settingsDictionary.Add("SubjectSite", comboBoxSite.SelectedIndex <= 0 ? "" : comboBoxSite.SelectedItem.ToString());
            settingsDictionary.Add("SubjectNotes", textBoxSubjectNotes.Text);

            // New settings
            settingsDictionary.Add("Frequency", comboBoxSamplingFreq.SelectedItem.ToString());
            if (!this.hasSyncGyro)
            {
                settingsDictionary.Add("LowPower", checkBoxLowPower.Checked ? "True" : "False");
                settingsDictionary.Add("Unpacked", checkBoxUnpacked.Checked ? "True" : "False");
            }
            settingsDictionary.Add("Range", comboBoxRange.SelectedItem.ToString());
            if (this.hasSyncGyro)
            {
                int.TryParse(comboBoxGyroRange.SelectedItem == null ? "" : comboBoxGyroRange.SelectedItem.ToString(), out int gyroRange);
                settingsDictionary.Add("GyroRange", gyroRange.ToString());
            }
            settingsDictionary.Add("DelayDays", DayDelay.ToString());
            settingsDictionary.Add("TimeOfDay", StartDate.TimeOfDay.TotalSeconds.ToString());
            settingsDictionary.Add("Duration", Duration.TotalSeconds.ToString());
            settingsDictionary.Add("RecordingTime", radioButtonImmediately.Checked ? "Immediately" : (radioButtonDuration.Checked ? "Duration" : ""));
            settingsDictionary.Add("Flash", checkBoxFlash.Checked ? "True" : "False");
        }
        
        private void resetFieldsToDictionary(Dictionary<string, string> settingsDictionary)
        {
            //Now we have the dictionary we pre-fill the fields.
            foreach (KeyValuePair<string, string> pair in settingsDictionary)
            {
                if (pair.Key.Equals("StudyCentre"))
                {
                    textBoxStudyCentre.Text = pair.Value;
                }
                else if (pair.Key.Equals("StudyCode"))
                {
                    textBoxStudyCode.Text = pair.Value;
                }
                else if (pair.Key.Equals("StudyInvestigator"))
                {
                    textBoxStudyInvestigator.Text = pair.Value;
                }
                else if (pair.Key.Equals("StudyExerciseType"))
                {
                    textBoxStudyExerciseType.Text = pair.Value;
                }
                else if (pair.Key.Equals("StudyOperator"))
                {
                    textBoxStudyOperator.Text = pair.Value;
                }
                else if (pair.Key.Equals("StudyNotes"))
                {
                    textBoxStudyNotes.Text = pair.Value;
                }
                /*else if (pair.Key.Equals("SubjectCode"))
                {
                    textBoxSubjectCode.Text = pair.Value;
                }
                else if (pair.Key.Equals("SubjectSex"))
                {
                    comboBoxSubjectSex.SelectedIndex = Int32.Parse(pair.Value);
                }
                else if (pair.Key.Equals("SubjectHeight"))
                {
                    textBoxHeight.Text = pair.Value;
                }
                else if (pair.Key.Equals("SubjectWeight"))
                {
                    textBoxWeight.Text = pair.Value;
                }
                else if (pair.Key.Equals("SubjectHandedness"))
                {
                    comboBoxSubjectHandedness.SelectedIndex = Int32.Parse(pair.Value);
                }
                //else if (pair.Key.Equals("SubjectTimezone"))
                //{
                //    comboBoxSubjectTimezone.SelectedIndex = Int32.Parse(pair.Value);
                //}
                else if (pair.Key.Equals("SubjectSite"))
                {
                    comboBoxSite.SelectedIndex = Int32.Parse(pair.Value);
                }
                else if (pair.Key.Equals("SubjectNotes"))
                {
                    textBoxSubjectNotes.Text = pair.Value;
                }
                */
                else if (pair.Key.Equals("LowPower") && !this.hasSyncGyro)
                {
                    if (pair.Value.Equals("True")) { checkBoxLowPower.Checked = true; }
                    else if (pair.Value.Equals("False")) { checkBoxLowPower.Checked = false; }
                }
                else if (pair.Key.Equals("Frequency"))
                {
                    comboBoxSamplingFreq.SelectedValue = pair.Value;
                    foreach (object o in comboBoxSamplingFreq.Items)
                    {
                        if (o.ToString().Equals(pair.Value)) { comboBoxSamplingFreq.SelectedItem = o; break; }
                    }
                }
                else if (pair.Key.Equals("Range"))
                {
                    comboBoxRange.SelectedValue = pair.Value;
                    foreach (object o in comboBoxRange.Items)
                    {
                        if (o.ToString().Equals(pair.Value)) { comboBoxRange.SelectedItem = o; break; }
                    }
                }
                else if (pair.Key.Equals("GyroRange") && this.hasSyncGyro)
                {
                    int.TryParse(pair.Value, out int value);
                    if (value == 0)
                    {
                        comboBoxGyroRange.SelectedIndex = 0;
                    }
                    else
                    {
                        comboBoxGyroRange.SelectedValue = "" + value;
                    }
                    foreach (object o in comboBoxGyroRange.Items)
                    {
                        if (o.ToString().Equals(pair.Value)) { comboBoxGyroRange.SelectedItem = o; break; }
                    }
                }
                else if (pair.Key.Equals("DelayDays"))
                {
                    DayDelay = Convert.ToInt32(pair.Value);
                }
                else if (pair.Key.Equals("TimeOfDay"))
                {
                    TimeSpan time = TimeSpan.FromSeconds(Convert.ToDouble(pair.Value));
                    StartDate = StartDate.Date.Add(time);
                }
                else if (pair.Key.Equals("Duration"))
                {
                    Duration = TimeSpan.FromSeconds(Convert.ToDouble(pair.Value));
                }
                else if (pair.Key.Equals("RecordingTime"))
                {
                    if (pair.Value.Equals("Immediately")) { radioButtonImmediately.Checked = true; }
                    else if (pair.Value.Equals("Duration")) { radioButtonDuration.Checked = true; }
                }
                else if (pair.Key.Equals("Flash"))
                {
                    if (pair.Value.Equals("True")) { checkBoxFlash.Checked = true; }
                    else if (pair.Value.Equals("False")) { checkBoxFlash.Checked = false; }
                }
                else if (pair.Key.Equals("Unpacked") && !this.hasSyncGyro)
                {
                    if (pair.Value.Equals("True")) { checkBoxUnpacked.Checked = true; }
                    else if (pair.Value.Equals("False")) { checkBoxFlash.Checked = false; }
                }
            }
        }

        private void saveDictionaryToXML(Dictionary<string, string> settingsDictionary)
        {
            XmlDocument xml = new XmlDocument();
            XmlNode outerNode = xml.CreateElement("RecordProfile");
            xml.AppendChild(outerNode);

            foreach(KeyValuePair<string, string> kvp in settingsDictionary)
            {
                XmlNode node = xml.CreateElement(kvp.Key);
                node.InnerText = kvp.Value;
                outerNode.AppendChild(node);
            }

            try
            {
                xml.Save(settingsProfileFilePath);
            }
            catch (XmlException e)
            {
                Console.WriteLine("Xml Error: Could not save recordProfile.xml - " + e.Message);
            }
            catch (Exception)
            {
                // TODO: Never silently eat errors!
            }
        }

        public bool Always { get; set; }


        private bool settingDelay = false;
        private bool settingDelayFromStartDate = false;
        public int DayDelay
        {
            get
            {
                return (int)delayDaysPicker.Value;
            }
            set
            {
                int delay = value;
                if (delay < delayDaysPicker.Minimum) { delay = (int)delayDaysPicker.Minimum; }
                if (delay > delayDaysPicker.Maximum) { delay = (int)delayDaysPicker.Maximum; }
                delayDaysPicker.Value = delay;
                DateTime now = DateTime.Now;
                if (!settingDelayFromStartDate)
                {
                    StartDate = now.Date + TimeSpan.FromDays(DayDelay) + StartDate.TimeOfDay;
                }
                updateWarningMessages();
            }
        }

        private void delayDaysPicker_ValueChanged(object sender, EventArgs e)
        {
            if (!settingDelay)
            {
                settingDelay = true;
                DayDelay = (int)delayDaysPicker.Value;
                settingDelay = false;
            }
        }

        // Save values
        public DateTime StartDate
        {
            get 
            {
                return datePickerStart.Value.Date + timePickerStart.Value.TimeOfDay; 
            }
            set
            {
                datePickerStart.Value = value.Date; // remove time
                timePickerStart.Value = value.Date + new TimeSpan(value.TimeOfDay.Hours, value.TimeOfDay.Minutes, 0); // remove seconds

                // Delay
                settingDelayFromStartDate = true;
                DayDelay = (datePickerStart.Value.Date - DateTime.Now.Date).Days;
                settingDelayFromStartDate = false;

                updateWarningMessages();
//Console.WriteLine("START ==> " + StartDate);
            }
        }

        public TimeSpan Duration
        {
            get
            {
                return new TimeSpan((int)daysPicker.Value, (int)hoursPicker.Value, (int)minutesPicker.Value, 0);
            }
            set
            {
//Console.WriteLine("START=" + StartDate + ", END=" + EndDate + ", DURATION=" + value.ToString() + "");
                TimeSpan v = value + new TimeSpan(0, 0, 30);
                daysPicker.Value = (int)v.TotalDays > 0 ? (int)v.TotalDays : 0;
                hoursPicker.Value = v.Hours > 0 ? v.Hours : 0;
                minutesPicker.Value = v.Minutes > 0 ? v.Minutes : 0;
                updateWarningMessages();
//Console.WriteLine("DURATION ==> " + Duration);
            }
        }

        public DateTime EndDate 
        {
            get 
            { 
                return datePickerEnd.Value.Date + timePickerEnd.Value.TimeOfDay; 
            }
            set
            {
                datePickerEnd.Value = value.Date;   // remove time
                timePickerEnd.Value = value.Date + new TimeSpan(value.TimeOfDay.Hours, value.TimeOfDay.Minutes, 0); // remove seconds

//Console.WriteLine("SET: " + value.Date + " " + value.TimeOfDay);
//Console.WriteLine("-->: " + datePickerEnd.Value.Date + " " + timePickerEnd.Value.TimeOfDay);
//datePickerEnd.Invalidate();

                updateWarningMessages();

//Console.WriteLine("END ==> " + EndDate);

            }
        }

        public int SessionID { get; set; }



        Dictionary<string, string> metaDataEntries;
        List<MetaDataEntry> metaDataList;
        MetaDataTools mdt = new MetaDataTools();

        public int SamplingFrequency { get; set; }
        private int[] SamplingFrequencies = { 3200, 1600, 800, 400, 200, 100, 50, 25, 12, 6 };

        public int Range { get; set; }
        private int[] SamplingRanges = { 2, 4, 8, 16 };

        public int GyroRange { get; set; }
        private int[] GyroRanges = { 0, 2000, 1000, 500, 250, 125 };

        public bool LowPower { get; set; }

        public string metaData = null;

        private void buttonOk_Click(object sender, EventArgs e)
        {
Cursor.Current = Cursors.WaitCursor;

            metaDataEntries = MetaDataEntry.shorthandMetaData;

            //setOK = true;

            SessionID = (int) numericUpDownSessionID.Value;

            saveDictionaryFromFields(SettingsProfileDictionary);
            saveDictionaryToXML(SettingsProfileDictionary);
            metaDataList = new List<MetaDataEntry>();

            List<string> mdStringList = new List<string>();

            //fill mde
            metaDataList.Add(new MetaDataEntry("_c", SettingsProfileDictionary["StudyCentre"])); mdStringList.Add("_c");
            metaDataList.Add(new MetaDataEntry("_s", SettingsProfileDictionary["StudyCode"])); mdStringList.Add("_s");
            metaDataList.Add(new MetaDataEntry("_i", SettingsProfileDictionary["StudyInvestigator"])); mdStringList.Add("_i");
            metaDataList.Add(new MetaDataEntry("_x", SettingsProfileDictionary["StudyExerciseType"])); mdStringList.Add("_x");
            metaDataList.Add(new MetaDataEntry("_so", SettingsProfileDictionary["StudyOperator"])); mdStringList.Add("_so");
            metaDataList.Add(new MetaDataEntry("_n", SettingsProfileDictionary["StudyNotes"])); mdStringList.Add("_n");

            metaDataList.Add(new MetaDataEntry("_p", SettingsProfileDictionary["SubjectSite"])); mdStringList.Add("_p");
            metaDataList.Add(new MetaDataEntry("_sc", SettingsProfileDictionary["SubjectCode"])); mdStringList.Add("_sc");
            metaDataList.Add(new MetaDataEntry("_se", SettingsProfileDictionary["SubjectSex"])); mdStringList.Add("_se");
            metaDataList.Add(new MetaDataEntry("_h", SettingsProfileDictionary["SubjectHeight"])); mdStringList.Add("_h");
            metaDataList.Add(new MetaDataEntry("_w", SettingsProfileDictionary["SubjectWeight"])); mdStringList.Add("_w");
            metaDataList.Add(new MetaDataEntry("_ha", SettingsProfileDictionary["SubjectHandedness"])); mdStringList.Add("_ha");
            metaDataList.Add(new MetaDataEntry("_sn", SettingsProfileDictionary["SubjectNotes"])); mdStringList.Add("_sn");

            //Create metadata
            metaData =  MetaDataTools.CreateMetaData(metaDataList);

            /*
            foreach (OmDevice device in Devices)
            {
                int i = OmApi.OmSetMetadata(device.DeviceId, metaData, metaData.Length);

                Console.WriteLine("OmSetMetadata Result: " + i);

                // Debug print the metadata
                if (false)
                {
                    StringBuilder sb = new StringBuilder(9600);
                    OmApi.OmGetMetadata(device.DeviceId, sb);
                    Console.WriteLine("sb: " + sb.ToString());

                    Dictionary<string, string> test = (Dictionary<string, string>)MetaDataTools.ParseMetaData(sb.ToString(), mdStringList);

                    foreach (KeyValuePair<string, string> kvp in test)
                    {
                        Console.WriteLine(kvp.Key + " - " + kvp.Value);
                    }
                }
            }
            */

            //mdt.SaveMetaData(metaData);

            SamplingFrequency = (int)float.Parse(comboBoxSamplingFreq.SelectedItem.ToString(), System.Globalization.CultureInfo.InvariantCulture);
            Range = (int)float.Parse(comboBoxRange.SelectedItem.ToString());
            int.TryParse(comboBoxGyroRange.SelectedItem == null ? "" : comboBoxGyroRange.SelectedItem.ToString(), out int gyroRange);
            GyroRange = gyroRange;
            LowPower = checkBoxLowPower.Checked;

            DialogResult = System.Windows.Forms.DialogResult.OK;
        }

        public bool Flash
        {
            get
            {
                return checkBoxFlash.Checked;
            }
        }

        public bool Unpacked
        {
            get
            {
                return checkBoxUnpacked.Checked;
            }
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

        static double EstimateCapacityFromBytesFree(long bytesFree, int rate, bool unpacked)
        {
            long clustersFree = (bytesFree / 32768);
            if (clustersFree <= 0) { return 0; }
            int samplesPerSector = unpacked ? 80 : 120;
            long numSamples = (clustersFree * (32768 / 512) - 2) * samplesPerSector;    // assume 32kB clusters, 120 samples per sector, reserve two sectors for header
            return (numSamples / (1.06 * rate));      // assume actual sampling rate could be up to 6% over
        }

        private void DateRangeForm_FormClosing(object sender, FormClosingEventArgs e)
        {
        }

        #region Height/Weight Checkbox Logic
        //checkbox enabling logic for height/weight
        //private void checkBoxHeight_CheckedChanged(object sender, EventArgs e)
        //{
        //    CheckBox cbHeight = (CheckBox)sender;

        //    if (cbHeight.Checked)
        //        numericUpDownSubjectHeight.Enabled = true;
        //    else
        //        numericUpDownSubjectHeight.Enabled = false;
        //}

        //private void checkBoxWeight_CheckedChanged(object sender, EventArgs e)
        //{
        //    CheckBox cbWeight = (CheckBox)sender;

        //    if (cbWeight.Checked)
        //        numericUpDownSubjectWeight.Enabled = true;
        //    else
        //        numericUpDownSubjectWeight.Enabled = false;
        //}
        #endregion

        #region RecordingTime Radio Button Logic
        private void radioButtonImmediately_CheckedChanged(object sender, EventArgs e)
        {
            RadioButton rb = (RadioButton)sender;

            if (rb.Checked)
            {
                datePickerStart.Enabled = false;
                timePickerStart.Enabled = false;
                datePickerEnd.Enabled = false;
                timePickerEnd.Enabled = false;

                delayDaysPicker.Enabled = false;
                daysPicker.Enabled = false;
                hoursPicker.Enabled = false;
                minutesPicker.Enabled = false;

                //Set always
                Always = true;
            }
            else
            {
                datePickerStart.Enabled = true;
                timePickerStart.Enabled = true;
                datePickerEnd.Enabled = true;
                timePickerEnd.Enabled = true;

                delayDaysPicker.Enabled = true;
                daysPicker.Enabled = true;
                hoursPicker.Enabled = true;
                minutesPicker.Enabled = true;

                Always = false;
            }

            updateWarningMessages();
        }
        #endregion


        #region UpdateWarningMessage Events & Change Duration/End Date Events


        bool startDurationChanging;
        // Changing the start or duration sets the end based on the start and duration
        private void startDuration_ValueChanged(object sender, EventArgs e)
        {
            // Ignore if this change was expected
            if (startDurationChanging) { return; }

            startDurationChanging = true;

            // Minutes < 0
            if (minutesPicker.Value < 0)
            {
                if (hoursPicker.Value == 0 && daysPicker.Value == 0)
                {
                    minutesPicker.Value = 0;
                }
                else
                {
                    hoursPicker.Value = hoursPicker.Value - 1;
                    minutesPicker.Value = 59;
                }
            }

            // Minutes >= 60
            if (minutesPicker.Value >= 60)
            {
                minutesPicker.Value = 0;
                hoursPicker.Value = hoursPicker.Value + 1;
            }

            // Hours < 0
            if (hoursPicker.Value < 0)
            {
                if (daysPicker.Value == 0)
                {
                    hoursPicker.Value = 0;
                }
                else
                {
                    daysPicker.Value = daysPicker.Value - 1;
                    hoursPicker.Value = 23;
                }
            }

            // Hours >= 24
            if (hoursPicker.Value >= 24)
            {
                hoursPicker.Value = 0;
                daysPicker.Value = daysPicker.Value + 1;
            }

            // Clamp start seconds to zero (seems to change otherwise?)
            StartDate = StartDate;

            startDurationChanging = false;


            // Calculate the new end date/time
            EndDate = StartDate + Duration;
        }


        bool endChanging;
        // Changing the end sets the duration based on the end and start
        private void end_ValueChanged(object sender, EventArgs e)
        {
            // Ignore if this change was expected
            if (endChanging) { return; }

            // Calculate the duration
            endChanging = true;
            Duration = EndDate - StartDate;
            endChanging = false;
        }


        #endregion

        #region Warning Message Logic
        bool[] warningMessagesFlags = { false, false, false, false, false, false, false, false, false, false, false };
        string[] warningMessages = {"Selected device(s) not fully charged", // 0
                                    "Selected device(s) not fully cleared", // 1
                                    "Selected device(s) capacity could limit duration", // 2
                                    "Selected device(s) current battery charge could limit duration", // 3
                                    "Delayed start time is more than 14 days in the future", // 4
                                    "End time is in the past", // 5
                                    "Start time is in the past", // 6
                                    "Chosen sampling frequency is not officially supported (use at own risk)", // 7
                                    "Chosen start and end times do not make an interval (end <= start)", // 8
                                    "Low power accelerometer produces noisier data (and does not significantly extend duration)", // 9
                                    "A gyro-enabled device is being configured for accelerometer data only (no gyro data).", // 10
        };
        private void updateWarningMessages()
        {
            //Make date start and end from dates and times;
            DateTime startDate = StartDate;
            DateTime endDate = EndDate;

            for (int i = 0; i < warningMessagesFlags.Length; i++)
            {
                warningMessagesFlags[i] = false;
            }

            foreach (OmDevice device in Devices)
            {
                //Not fully charged.
                if (device.BatteryLevel < 90)
                    warningMessagesFlags[0] = true;

                //Not fully cleared
                if (device.HasData)
                    warningMessagesFlags[1] = true;

                //Duration could be limited by device capacity
                //TS - TODO

                //Duration could be limited by battery (on rate)
                double estimateBatteryInSeconds = comboBoxSamplingFreq.SelectedItem == null ? 0 : EstimateBatteryLife(device.BatteryLevel, (int)float.Parse(comboBoxSamplingFreq.SelectedItem.ToString(), System.Globalization.CultureInfo.InvariantCulture));
                TimeSpan ts = endDate - startDate;
                if (ts.TotalSeconds > estimateBatteryInSeconds)
                    warningMessagesFlags[3] = true;
            }


            //Delayed start time is more than 14 days in the future
            if (radioButtonDuration.Checked && startDate > (DateTime.Now.Add(new TimeSpan(14, 0, 0, 0))))
                warningMessagesFlags[4] = true;

            //End time is in the past.
            DateTime d = DateTime.Now;
            if (radioButtonDuration.Checked && (endDate < d) && endDate != startDate)
                warningMessagesFlags[5] = true;

            //Start date is more than a day in the past
            if ((startDate < DateTime.Now.Subtract(new TimeSpan(1, 0, 0, 0))) && radioButtonDuration.Checked)
                warningMessagesFlags[6] = true;

            //Warning for sampling frequency
            int sampFreq = comboBoxSamplingFreq.SelectedItem != null ? (int)float.Parse(comboBoxSamplingFreq.SelectedItem.ToString(), System.Globalization.CultureInfo.InvariantCulture) : 0;
            if (sampFreq > 200 || sampFreq < 50)
                warningMessagesFlags[7] = true;

            //Start date is after end date
            if (radioButtonDuration.Checked && StartDate >= EndDate)
                warningMessagesFlags[8] = true;

            //Low power mode is being used
            if (checkBoxLowPower.Checked)
            {
                warningMessagesFlags[9] = true;
            }

            //Gyro device in accelerometer-only mode
            int.TryParse(comboBoxGyroRange.SelectedItem == null ? "" : comboBoxGyroRange.SelectedItem.ToString(), out int gyroRange);
            if (hasSyncGyro && gyroRange == 0)
            {
                warningMessagesFlags[10] = true;
            }

            //Warning for non-standard rate/range
            int sampRange = comboBoxRange.SelectedItem != null ? int.Parse(comboBoxRange.SelectedItem.ToString()) : 0;
            if (sampFreq == 100 && sampRange == 8)
            {
                labelRateRangeSetting.Text = "";
            }
            else if (sampFreq > 200 || sampFreq < 50)
            {
                labelRateRangeSetting.Text = "not guaranteed";
            }
            else
            {
                labelRateRangeSetting.Text = "non-standard";
            }

            //Now that we have the flags, we can build the string.
            StringBuilder s = new StringBuilder();
            for (int i = 0; i < warningMessagesFlags.Length; i++)
            {
                if (warningMessagesFlags[i])
                    s.AppendLine("\u2022 " + warningMessages[i]);
            }

            //If we have no warnings then display message
            if (s.ToString().Length == 0)
            {
                richTextBoxWarning.Text = "";
                richTextBoxWarning.Visible = false;
            }
            else
            {
                richTextBoxWarning.Text = "WARNINGS\n" + s.ToString();
                richTextBoxWarning.Visible = true;
            }

        }
        #endregion

        private void comboBoxSamplingFreq_SelectedIndexChanged(object sender, EventArgs e)
        {
            updateWarningMessages();
        }

        private void comboBoxRange_SelectedIndexChanged(object sender, EventArgs e)
        {
            updateWarningMessages();
        }

        private void DateRangeForm_Load(object sender, EventArgs e)
        {

        }

        private void radioButtonDuration_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void buttonDefault_Click(object sender, EventArgs e)
        {
            comboBoxSamplingFreq.SelectedIndex = 5;
            comboBoxRange.SelectedIndex = 2;
            updateWarningMessages();
        }


        bool firstShown = true;
        private void DateRangeForm_Shown(object sender, EventArgs e)
        {
            if (firstShown)
            {
                firstShown = false;

                // HACK: Horrible hack to make the date picker respond properly
                int delay = (int)delayDaysPicker.Value;
                int alt = (delay + 1 < delayDaysPicker.Maximum) ? delay + 1 : delay - 1;
                delayDaysPicker.Value = alt;
                delayDaysPicker.Value = delay;
            }
        }

        private void numericUpDown_Enter(object sender, EventArgs e)
        {
            ((NumericUpDown)sender).Select(0, ((NumericUpDown)sender).Value.ToString().Length);
        }

        private void textBox_Enter(object sender, EventArgs e)
        {
            ((TextBox)sender).SelectAll();
        }

        private void checkBoxUnpacked_CheckedChanged(object sender, EventArgs e)
        {
            updateWarningMessages();
        }

        private void checkBoxLowPower_CheckedChanged(object sender, EventArgs e)
        {
            updateWarningMessages();
        }

        private void panelMain_Paint(object sender, PaintEventArgs e)
        {

        }

    }
}

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
        bool setOK = false;
        bool warningsOn = true;

        private Dictionary<string, string> SettingsProfileDictionary { get; set; }
        private string settingsProfileFilePath = Properties.Settings.Default.CurrentWorkingFolder + "\\" + "recordSetup.xml";
        private OmDevice Device { get; set; }
        private OmDevice[] Devices { get; set; }

        public DateRangeForm(string title, OmDevice[] devices)
        {
            if (devices.Length > 0)
            {
                warningsOn = false;
            }

            Devices = devices;

            InitializeComponent();
            Text = title;
            //labelPrompt.Text = prompt;
            FromDate = DateTime.MinValue;
            //UntilDate = DateTime.MaxValue;

            timePickerStart.ShowUpDown = true;
            timePickerEnd.ShowUpDown = true;

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

            //Set radio buttons enabled
            radioButtonImmediately.Checked = true;

            //Set default recording times
            datePickerEndIgnoreEvent = true;
            datePickerStartIgnoreEvent = true;
            timePickerEndIgnoreEvent = true;
            timePickerStartIgnoreEvent = true;
            dayPickerIgnoreEvent = true;
            minutePickerIgnoreEvent = true;
            hourPickerIgnoreEvent = true;

            datePickerStart.Value = DateTime.Now;
            timePickerStart.Value = DateTime.Now;
            datePickerEnd.Value = DateTime.Now;//.AddDays(1);
            timePickerEnd.Value = DateTime.Now;//.AddDays(1);
            hoursPicker.Value = 0;
            minutesPicker.Value = 0;
            daysPicker.Value = 0;

            datePickerEndIgnoreEvent = false;
            datePickerStartIgnoreEvent = false;
            timePickerEndIgnoreEvent = false;
            timePickerStartIgnoreEvent = false;
            dayPickerIgnoreEvent = false;
            minutePickerIgnoreEvent = false;
            hourPickerIgnoreEvent = false;

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
            settingsDictionary.Add("SubjectSex", comboBoxSubjectSex.SelectedIndex.ToString());
            settingsDictionary.Add("SubjectHeight", textBoxHeight.Text.ToString());
            settingsDictionary.Add("SubjectWidth", textBoxWeight.Text.ToString());
            settingsDictionary.Add("SubjectHandedness", comboBoxSubjectHandedness.SelectedIndex.ToString());
            //settingsDictionary.Add("SubjectTimezone", comboBoxSubjectTimezone.SelectedIndex.ToString());
            settingsDictionary.Add("SubjectSite", comboBoxSite.SelectedIndex.ToString());
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
                else if (pair.Key.Equals("SubjectCode"))
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
            catch (Exception e)
            {
            }
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
                return datePickerStart.Value.Date.Add(timePickerStart.Value.TimeOfDay);
            }
            set
            {
                //if (value < datePickerStart.MinDate)
                //{
                //    datePickerStart.Checked = false;
                //    datePickerStart.Value = datePickerStart.MinDate;
                //}
                //else if (value > datePickerStart.MaxDate)
                //{
                //    datePickerStart.Checked = false;
                //    datePickerStart.Value = datePickerStart.MaxDate;
                //}
                //else
                //{
                //    datePickerStart.Value = value;
                //}
                //value = datePickerStart.Value;
            }
        }

        public DateTime UntilDate
        {
            get { return datePickerEnd.Value.Date.Add(timePickerEnd.Value.TimeOfDay); }
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

        Dictionary<string, string> metaDataEntries;
        List<MetaDataEntry> metaDataList;
        MetaDataTools mdt = new MetaDataTools();

        public int SamplingFrequency { get; set; }
        private int[] SamplingFrequencies = { 3200, 800, 400, 200, 100, 50, 25, 12, 6 };

        public int Range { get; set; }
        private int[] SamplingRanges = { 2, 4, 8, 16 };

        private void buttonOk_Click(object sender, EventArgs e)
        {
            metaDataEntries = MetaDataEntry.shorthandMetaData;

            setOK = true;

            SyncTime = SyncTimeType.PC;

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
            metaDataList.Add(new MetaDataEntry("_w", SettingsProfileDictionary["SubjectWidth"])); mdStringList.Add("_w");
            metaDataList.Add(new MetaDataEntry("_ha", SettingsProfileDictionary["SubjectHandedness"])); mdStringList.Add("_ha");

            //Create metadata
            string md = MetaDataTools.CreateMetaData(metaDataList);

            foreach (OmDevice device in Devices)
            {
                int i = OmApi.OmSetMetadata(device.DeviceId, md, md.Length);

                Console.WriteLine("OmSetMetadata Result: " + i);

                StringBuilder sb = new StringBuilder(9600);
                OmApi.OmGetMetadata(device.DeviceId, sb);

                Console.WriteLine("sb: " + sb.ToString());

                Dictionary<string, string> test = (Dictionary<string, string>) MetaDataTools.ParseMetaData(sb.ToString(), mdStringList);

                foreach(KeyValuePair<string, string> kvp in test)
                {
                    Console.WriteLine(kvp.Key + " - " + kvp.Value);
                }
            }
            

            //mdt.SaveMetaData(md);

            SamplingFrequency = SamplingFrequencies[comboBoxSamplingFreq.SelectedIndex];
            Range = SamplingRanges[comboBoxRange.SelectedIndex];

            DialogResult = System.Windows.Forms.DialogResult.OK;

            //TS - TODO - Build UntilDate from the data provided.
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
            DateTime startDate = datePickerStart.Value;
            DateTime endDate = startDate.Add(new TimeSpan((int)daysPicker.Value, (int)hoursPicker.Value, (int)minutesPicker.Value, 0));
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

                daysPicker.Enabled = true;
                hoursPicker.Enabled = true;
                minutesPicker.Enabled = true;

                Always = false;
            }

            updateWarningMessages();
        }
        #endregion

        #region UpdateWarningMessage Events & Change Duration/End Date Events
        bool datePickerEndIgnoreEvent = false;
        bool timePickerEndIgnoreEvent = false;
        bool datePickerStartIgnoreEvent = false;
        bool timePickerStartIgnoreEvent = false;
        bool dayPickerIgnoreEvent = false;
        bool hourPickerIgnoreEvent = false;
        bool minutePickerIgnoreEvent = false;

        //Start date
        DateTime previousDateStart = DateTime.Now;
        private void datePickerStart_ValueChanged(object sender, EventArgs e)
        {
            if (datePickerStartIgnoreEvent)
                return;

            DateTime t = datePickerEnd.Value.Add(timePickerEnd.Value.TimeOfDay);
            DateTime t2 = datePickerStart.Value.Add(timePickerStart.Value.TimeOfDay);

            TimeSpan difference = t.Subtract(t2);

            if (difference.Days < 1000)
            {
                //TS - If the end date is before the start date, error at them
                if (difference.TotalSeconds < 0)
                {
                    MessageBox.Show("End date/time cannot be before start date/time.", "Date Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    datePickerStart.Value = previousDateStart;
                }
                else
                {
                    dayPickerIgnoreEvent = true;
                    daysPicker.Value = difference.Days;
                    dayPickerIgnoreEvent = false;

                    hourPickerIgnoreEvent = true;
                    hoursPicker.Value = difference.Hours;
                    hourPickerIgnoreEvent = false;

                    minutePickerIgnoreEvent = true;
                    minutesPicker.Value = difference.Minutes;
                    minutePickerIgnoreEvent = false;

                    previousDateStart = datePickerStart.Value;
                }
            }

            /*TimeSpan ts = datePickerEnd.Value.Subtract(datePickerStart.Value);

            if (ts.Days < 0)
            {
                MessageBox.Show("The End Date cannot be older than the Start Date", "Invalid End Date", MessageBoxButtons.OK, MessageBoxIcon.Error);
                datePickerEnd.Value = datePickerStart.Value;
            }
            else
            {
                if(ts.Days > (int) dayPicker.Maximum)
                {
                    dayPickerIgnoreEvent = true;
                    dayPicker.Value = dayPicker.Maximum;
                    dayPickerIgnoreEvent = false;
                }
                else
                {
                    dayPickerIgnoreEvent = true;
                    dayPicker.Value = ts.Days;
                    dayPickerIgnoreEvent = false;
                }
            }*/

            updateWarningMessages();
        }

        DateTime previousTimeStart = DateTime.Now;
        private void timePickerStart_ValueChanged(object sender, EventArgs e)
        {
            if (timePickerStartIgnoreEvent)
                return;

            DateTime t = datePickerEnd.Value.Add(timePickerEnd.Value.TimeOfDay);
            DateTime t2 = datePickerStart.Value.Add(timePickerStart.Value.TimeOfDay);

            TimeSpan difference = t.Subtract(t2);

            //TS - If the end date is before the start date, error at them
            if (difference.TotalSeconds < 0)
            {
                MessageBox.Show("End date/time cannot be before start date/time.", "Date Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                timePickerStart.Value = previousTimeStart;
            }
            else
            {
                dayPickerIgnoreEvent = true;
                daysPicker.Value = difference.Days;
                dayPickerIgnoreEvent = false;

                hourPickerIgnoreEvent = true;
                hoursPicker.Value = difference.Hours;
                hourPickerIgnoreEvent = false;

                minutePickerIgnoreEvent = true;
                minutesPicker.Value = difference.Minutes;
                minutePickerIgnoreEvent = false;

                previousTimeStart = timePickerStart.Value;
            }

            /*
            //TimeSpan ts = timePickerEnd.Value.TimeOfDay.Subtract(timePickerStart.Value.TimeOfDay);

            //If dates same then can still be in past...
            if (datePickerEnd.Value.Equals(datePickerStart.Value))
            {
                //Dates same, see if hours are less.
                if (ts.Hours < 0)
                {
                    MessageBox.Show("The End Date cannot be older than the Start Date", "Invalid End Date", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    timePickerEnd.Value = timePickerStart.Value;
                }
                else if (ts.Hours == 0)
                {
                    //If hours are equal then see if minutes are less
                    if (ts.Minutes < 0)
                    {
                        MessageBox.Show("The End Date cannot be older than the Start Date", "Invalid End Date", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        timePickerEnd.Value = timePickerStart.Value;
                    }
                }

                hourPickerIgnoreEvent = true;
                hoursPicker.Value = ts.Hours;
                hourPickerIgnoreEvent = false;

                minutePickerIgnoreEvent = true;
                minutesPicker.Value = ts.Minutes;
                minutePickerIgnoreEvent = false;
            }
            //Everything is okay so update hours and minutes...
            else
            {
                hourPickerIgnoreEvent = true;
                hoursPicker.Value = ts.Hours;
                hourPickerIgnoreEvent = false;

                minutePickerIgnoreEvent = true;
                minutesPicker.Value = ts.Minutes;
                minutePickerIgnoreEvent = false;
            }*/

            updateWarningMessages();
        }

        //End Date
        DateTime previousDateEnd = DateTime.Now;
        private void datePickerEnd_ValueChanged(object sender, EventArgs e)
        {
            if (datePickerEndIgnoreEvent)
                return;

            DateTime t = datePickerEnd.Value.Add(timePickerEnd.Value.TimeOfDay);
            DateTime t2 = datePickerStart.Value.Add(timePickerStart.Value.TimeOfDay);

            TimeSpan difference = t.Subtract(t2);

            //TS - If the end date is before the start date, error at them
            if (difference.TotalSeconds < 0)
            {
                MessageBox.Show("End date/time cannot be before start date/time.", "Date Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                datePickerEnd.Value = previousDateEnd;
            }
            else
            {
                dayPickerIgnoreEvent = true;
                daysPicker.Value = difference.Days;
                dayPickerIgnoreEvent = false;

                hourPickerIgnoreEvent = true;
                hoursPicker.Value = difference.Hours;
                hourPickerIgnoreEvent = false;

                minutePickerIgnoreEvent = true;
                minutesPicker.Value = difference.Minutes;
                minutePickerIgnoreEvent = false;

                previousDateEnd = datePickerEnd.Value;
            }

            /*TimeSpan ts = datePickerEnd.Value.Subtract(datePickerStart.Value);

            if (ts.Days < 0)
            {
                MessageBox.Show("The End Date cannot be older than the Start Date", "Invalid End Date", MessageBoxButtons.OK, MessageBoxIcon.Error);
                datePickerEnd.Value = datePickerStart.Value;
            }
            else
            {
                dayPickerIgnoreEvent = true;
                dayPicker.Value = ts.Days;
                dayPickerIgnoreEvent = false;
            }*/

            updateWarningMessages();
        }

        DateTime previousTimeEnd = DateTime.Now;
        private void timePickerEnd_ValueChanged(object sender, EventArgs e)
        {
            if (timePickerEndIgnoreEvent)
                return;

            DateTime t = datePickerEnd.Value.Add(timePickerEnd.Value.TimeOfDay);
            DateTime t2 = datePickerStart.Value.Add(timePickerStart.Value.TimeOfDay);

            TimeSpan difference = t.Subtract(t2);

            //TS - If the end date is before the start date, error at them
            if (difference.TotalSeconds < 0)
            {
                MessageBox.Show("End date cannot be before start date.", "Date Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                timePickerEnd.Value = previousTimeEnd;
            }
            else
            {
                dayPickerIgnoreEvent = true;
                daysPicker.Value = difference.Days;
                dayPickerIgnoreEvent = false;

                hourPickerIgnoreEvent = true;
                hoursPicker.Value = difference.Hours;
                hourPickerIgnoreEvent = false;

                minutePickerIgnoreEvent = true;
                minutesPicker.Value = difference.Minutes;
                minutePickerIgnoreEvent = false;

                previousTimeEnd = timePickerEnd.Value;
            }
            /*TimeSpan ts = timePickerEnd.Value.TimeOfDay.Subtract(timePickerStart.Value.TimeOfDay);

            //If dates same then can still be in past...
            if (datePickerEnd.Value.Equals(datePickerStart.Value))
            {
                //Dates same, see if hours are less.
                if (ts.Hours < 0)
                {
                    MessageBox.Show("The End Date cannot be older than the Start Date", "Invalid End Date", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    timePickerEnd.Value = timePickerStart.Value;
                }
                else if (ts.Hours == 0)
                {
                    //If hours are equal then see if minutes are less
                    if (ts.Minutes < 0)
                    {
                        MessageBox.Show("The End Date cannot be older than the Start Date", "Invalid End Date", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        timePickerEnd.Value = timePickerStart.Value;
                    }
                }
            }
            //Everything is okay so update hours and minutes...
            else
            {
                hourPickerIgnoreEvent = true;
                hoursPicker.Value = ts.Hours;
                hourPickerIgnoreEvent = false;

                minutePickerIgnoreEvent = true;
                minutesPicker.Value = ts.Minutes;
                minutePickerIgnoreEvent = false;
            }*/

            updateWarningMessages();
        }

        int hoursPickerLastValue = 0;
        private void hoursPicker_ValueChanged(object sender, EventArgs e)
        {
            if (hourPickerIgnoreEvent)
                return;

            int change = (int)hoursPicker.Value - hoursPickerLastValue;
            hoursPickerLastValue = (int)hoursPicker.Value;

            DateTime d = timePickerEnd.Value.Add(new TimeSpan(change, 0, 0));

            timePickerEndIgnoreEvent = true;
            timePickerEnd.Value = d;
            timePickerEndIgnoreEvent = false;

            updateWarningMessages();
        }

        int minutesPickerLastValue = 0;
        private void minutesPicker_ValueChanged(object sender, EventArgs e)
        {
            if (minutePickerIgnoreEvent)
                return;

            int change = (int)minutesPicker.Value - minutesPickerLastValue;
            minutesPickerLastValue = (int)minutesPicker.Value;

            DateTime d = timePickerEnd.Value.Add(new TimeSpan(0, change, 0));

            timePickerEndIgnoreEvent = true;
            timePickerEnd.Value = d;
            timePickerEndIgnoreEvent = false;

            updateWarningMessages();
        }

        int dayPickerLastValue = 0;
        private void dayPicker_ValueChanged(object sender, EventArgs e)
        {
            if (dayPickerIgnoreEvent)
                return;

            int change = (int) daysPicker.Value - dayPickerLastValue;
            dayPickerLastValue = (int)daysPicker.Value;

            DateTime d = datePickerEnd.Value.Add(new TimeSpan(change, 0, 0, 0));

            datePickerEndIgnoreEvent = true;
            datePickerEnd.Value = d;
            datePickerEndIgnoreEvent = false;

            updateWarningMessages();
        }
        #endregion

        #region Warning Message Logic
        bool[] warningMessagesFlags = { false, false, false, false, false, false, false, false };
        string[] warningMessages = {"Device not fully charged",
                                    "Device not fully cleared",
                                    "Duration could be limited by device capacity",
                                    "Duration could be limited by remaining battery",
                                    "Delayed start time is more than 14 days in the future",
                                    "End time is in the past",
                                    "Start time is in the past",
                                    "Current Sampling Frequency not guaranteed on this firmware"};
        private void updateWarningMessages()
        {
            if (Device != null && warningsOn)
            {
                //Make date start and end from dates and times;
                DateTime startDate = datePickerStart.Value.Date + timePickerStart.Value.TimeOfDay;
                DateTime endDate = datePickerEnd.Value.Date + timePickerEnd.Value.TimeOfDay;

                for (int i = 0; i < warningMessagesFlags.Length; i++)
                {
                    warningMessagesFlags[i] = false;
                }

                //Not fully charged.
                if (Device.BatteryLevel < 95)
                    warningMessagesFlags[0] = true;

                //Not fully cleared
                if (Device.HasData)
                    warningMessagesFlags[1] = true;

                //Duration could be limited by device capacity
                //TS - TODO

                //Duration could be limited by battery (on rate)
                double estimateBatteryInSeconds = EstimateBatteryLife(Device.BatteryLevel, Int32.Parse(comboBoxSamplingFreq.Text));
                TimeSpan ts = endDate - startDate;
                if (ts.TotalSeconds > estimateBatteryInSeconds)
                    warningMessagesFlags[3] = true;

                //Delayed start time is more than 14 days in the future
                if (startDate > (DateTime.Now.Add(new TimeSpan(14, 0, 0, 0))))
                    warningMessagesFlags[4] = true;

                //End time is in the past.
                DateTime d = DateTime.Now;
                if (radioButtonDuration.Checked && (endDate < d))
                    warningMessagesFlags[5] = true;

                //Start date is more than a day in the past
                if ((startDate < DateTime.Now.Subtract(new TimeSpan(1, 0, 0, 0))) && radioButtonDuration.Checked)
                    warningMessagesFlags[6] = true;

                //Warning for sampling frequency
                int sampFreq = int.Parse(comboBoxSamplingFreq.Text);
                if (sampFreq > 200 || sampFreq < 50)
                    warningMessagesFlags[7] = true;

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
                    richTextBoxWarning.Text = "WARNINGS\nNo warnings";
                    richTextBoxWarning.Visible = false;
                }
                else
                {
                    richTextBoxWarning.Text = "WARNINGS\n" + s.ToString();
                    richTextBoxWarning.Visible = true;
                }
            }
        }
        #endregion

        private void comboBoxSamplingFreq_SelectedIndexChanged(object sender, EventArgs e)
        {
            updateWarningMessages();
        }
    }
}

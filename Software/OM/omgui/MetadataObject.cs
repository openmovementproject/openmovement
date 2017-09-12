using OmApiNet;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace OmGui
{
    class MetadataObject
    {
        [CategoryAttribute("Recording"), DisplayName("Device ID"), DescriptionAttribute("Recording Device ID"), DefaultValueAttribute(""), ReadOnly(true)]
        public string DeviceId             { get; protected set; }
        [CategoryAttribute("Recording"), DisplayName("Session ID"), DescriptionAttribute("Recording Session ID"), DefaultValueAttribute(""), ReadOnly(true)]
        public string SessionId            { get; protected set; }

        [CategoryAttribute("Recording"), DisplayName("Sampling Rate"), DescriptionAttribute("Sampling Rate (Hz)"), DefaultValueAttribute(""), ReadOnly(true)]
        public string SamplingRate { get; protected set; }
        [CategoryAttribute("Recording"), DisplayName("Sampling Range"), DescriptionAttribute("Sampling Range (+/- g)"), DefaultValueAttribute(""), ReadOnly(true)]
        public string SamplingRange { get; protected set; }

        //[CategoryAttribute("Recording"), DisplayName("Start Time"), DescriptionAttribute("Recording Start Time"), DefaultValueAttribute(""), ReadOnly(true)]
        //public string StartTime            { get; protected set; }
        [CategoryAttribute("Recording"), DisplayName("End Time"), DescriptionAttribute("Recording End Time"), DefaultValueAttribute(""), ReadOnly(true)]
        public string EndTime              { get; protected set; }

        [CategoryAttribute("Study"), DisplayName("Centre"), DescriptionAttribute("Study Centre"), DefaultValueAttribute(""), ReadOnly(true)]
        public string StudyCentre          { get; protected set; }
        [CategoryAttribute("Study"), DisplayName("Code"), DescriptionAttribute("Study Code"), DefaultValueAttribute(""), ReadOnly(true)]
        public string StudyCode            { get; protected set; }
        [CategoryAttribute("Study"), DisplayName("Investigator"), DescriptionAttribute("Study Investigator"), DefaultValueAttribute(""), ReadOnly(true)]
        public string StudyInvestigator    { get; protected set; }
        [CategoryAttribute("Study"), DisplayName("Exercise Type"), DescriptionAttribute("Study Exercise Type"), DefaultValueAttribute(""), ReadOnly(true)]
        public string StudyExerciseType    { get; protected set; }
        [CategoryAttribute("Study"), DisplayName("Operator"), DescriptionAttribute("Study Operator"), DefaultValueAttribute(""), ReadOnly(true)]
        public string StudyOperator        { get; protected set; }
        [CategoryAttribute("Study"), DisplayName("Notes"), DescriptionAttribute("Study Notes"), DefaultValueAttribute(""), ReadOnly(true)]
        public string StudyNotes           { get; protected set; }

        [CategoryAttribute("Subject"), DisplayName("Site"), DescriptionAttribute("Subject Site"), DefaultValueAttribute(""), ReadOnly(true)] // (1=left wrist, 2=right wrist, 3=waist, 4=left ankle, 5=right ankle, 6=left thigh, 7=right thigh, 8=left hip, 9=right hip, 10=left upper-arm, 12=right upper-arm, 13=chest, 14=sacrum, 15=neck, 16=head)
        public string SubjectSite          { get; protected set; }
        [CategoryAttribute("Subject"), DisplayName("Code"), DescriptionAttribute("Subject Code"), DefaultValueAttribute(""), ReadOnly(true)]
        public string SubjectCode          { get; protected set; }
        [CategoryAttribute("Subject"), DisplayName("Sex"), DescriptionAttribute("Subject Sex"), DefaultValueAttribute(""), ReadOnly(true)]  // (1=male, 2=female)
        public string SubjectSex           { get; protected set; }
        [CategoryAttribute("Subject"), DisplayName("Height"), DescriptionAttribute("Subject Height"), DefaultValueAttribute(""), ReadOnly(true)]
        public string SubjectHeight        { get; protected set; }
        [CategoryAttribute("Subject"), DisplayName("Weight"), DescriptionAttribute("Subject Weight"), DefaultValueAttribute(""), ReadOnly(true)]
        public string SubjectWeight        { get; protected set; }
        [CategoryAttribute("Subject"), DisplayName("Handedness"), DescriptionAttribute("Subject Handedness"), DefaultValueAttribute(""), ReadOnly(true)]    // (1=left, 2=right)
        public string SubjectHandedness    { get; protected set; }
        [CategoryAttribute("Subject"), DisplayName("Notes"), DescriptionAttribute("Subject Notes"), DefaultValueAttribute(""), ReadOnly(true)]
        public string SubjectNotes         { get; protected set; }

        protected MetadataObject()
        {

        }

        public static MetadataObject FromMetadata(Dictionary<string, string> metadata)
        {
            MetadataObject metadataObject = new MetadataObject();
            string s;

            s = null; metadata.TryGetValue("DeviceId",          out s); metadataObject.DeviceId = s;
            s = null; metadata.TryGetValue("SessionId",         out s); metadataObject.SessionId = s;

            s = null; metadata.TryGetValue("SamplingRate",      out s); metadataObject.SamplingRate = s;
            s = null; metadata.TryGetValue("SamplingRange",     out s); metadataObject.SamplingRange = s;

            //            s = null; metadata.TryGetValue("StartTime",         out s); metadataObject.StartTime = s;
            s = null; metadata.TryGetValue("EndTime",           out s); metadataObject.EndTime = s;

            s = null; metadata.TryGetValue("StudyCentre",       out s); metadataObject.StudyCentre = s;
            s = null; metadata.TryGetValue("StudyCode",         out s); metadataObject.StudyCode = s;
            s = null; metadata.TryGetValue("StudyInvestigator", out s); metadataObject.StudyInvestigator = s;
            s = null; metadata.TryGetValue("StudyExerciseType", out s); metadataObject.StudyExerciseType = s;
            s = null; metadata.TryGetValue("StudyOperator",     out s); metadataObject.StudyOperator = s;
            s = null; metadata.TryGetValue("StudyNotes",        out s); metadataObject.StudyNotes = s;

            s = null; metadata.TryGetValue("SubjectSite",       out s); metadataObject.SubjectSite = s;
            s = null; metadata.TryGetValue("SubjectCode",       out s); metadataObject.SubjectCode = s;
            s = null; metadata.TryGetValue("SubjectSex",        out s); metadataObject.SubjectSex = s;
            s = null; metadata.TryGetValue("SubjectHeight",     out s); metadataObject.SubjectHeight = s;
            s = null; metadata.TryGetValue("SubjectWeight",     out s); metadataObject.SubjectWeight = s;
            s = null; metadata.TryGetValue("SubjectHandedness", out s); metadataObject.SubjectHandedness = s;
            s = null; metadata.TryGetValue("SubjectNotes",      out s); metadataObject.SubjectNotes = s;

            // Fix up numeric references
            if (metadataObject.SubjectSex != null)
            {
                string[] values = { "", "male", "female" };
                int i = -1;
                int.TryParse(metadataObject.SubjectSex, out i);
                if (i >= 0 && i < values.Length)
                {
                    metadataObject.SubjectSex = values[i];
                }
            }
            if (metadataObject.SubjectHandedness != null)
            {
                string[] values = { "", "left", "right" };
                int i = -1;
                int.TryParse(metadataObject.SubjectHandedness, out i);
                if (i >= 0 && i < values.Length)
                {
                    metadataObject.SubjectHandedness = values[i];
                }
            }
            if (metadataObject.SubjectSite != null)
            {
                string[] values = { "", "left wrist", "right wrist", "waist", "left ankle", "right ankle", "left thigh", "right thigh", "left hip", "right hip", "left upper-arm", "right upper-arm", "chest", "sacrum", "neck", "head" };
                int i = -1;
                int.TryParse(metadataObject.SubjectSite, out i);
                if (i >= 0 && i < values.Length)
                {
                    metadataObject.SubjectSite = values[i];
                }
            }

            return metadataObject;
        }

        public static MetadataObject FromFile(string filename)
        {
            try
            {
                Dictionary<string, string> metadata = MetaDataTools.MetadataFromFile(filename);
                MetadataObject metadataObject = FromMetadata(metadata);
                return metadataObject;
            }
            catch (Exception)
            {
                Console.Error.WriteLine("ERROR: Problem reading metadata directly from file.");
                return null;
            }
        }

        public static MetadataObject FromReader(string filename)
        {
            try
            {
                Dictionary<string, string> metadata = MetaDataTools.MetadataFromReader(filename);
                MetadataObject metadataObject = FromMetadata(metadata);
                return metadataObject;
            }
            catch (Exception)
            {
                Console.Error.WriteLine("ERROR: Problem reading metadata from file (via Reader).");
                return null;
            }
        }

    }
}

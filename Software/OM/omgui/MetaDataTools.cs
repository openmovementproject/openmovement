using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;

namespace OmGui
{
    class MetaDataTools
    {
        protected IList<MetaDataEntry> metaDataEntries = new List<MetaDataEntry>();

        public const int ANNOTATION_OFFSET = 64;
        public const int ANNOTATION_SEGMENT_COUNT = 14;
        public const int ANNOTATION_SEGMENT_LENGTH = 32;
        public const int ANNOTATION_TOTAL_LENGTH = (ANNOTATION_SEGMENT_COUNT * ANNOTATION_SEGMENT_LENGTH);
        public const char ANNOTATION_PADDING = ' ';
        private string oldRawAnnotation = null;

        public static readonly ICollection<string> inputElements = new List<string> { "_c", "_s", "_i", "_x", "_v", "_p", "_so", "_n" };
        public static readonly ICollection<string> outputElements = new List<string> { "_c", "_s", "_i", "_x", "_v", "_p", "_so", "_n", "_ro", "_b", "_e", "_co" };
        
        public static string UrlEncode(string s)
        {
            if (s == null) { return null; }

            byte[] input = (new System.Text.UTF8Encoding()).GetBytes(s);

            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < input.Length; i++)
            {
                byte b = input[i];
                if (b == (byte)' ') { sb.Append('+'); }
                else if ((b >= (byte)'A' && b <= (byte)'Z') || (b >= (byte)'a' && b <= (byte)'z') || (b >= (byte)'0' && b <= (byte)'9') || b == (byte)'~' || b == (byte)'_' || b == (byte)'.' || b == (byte)'-') { sb.Append((char)b); }
                else { sb.Append(string.Format("%{0:X2}", b)); }
            }
            return sb.ToString();
        }

        public static bool IsHex(byte v) { return (v >= '0' && v <= '9') || (v >= 'a' && v <= 'f') || (v >= 'A' && v <= 'F'); }

        public static string UrlDecode(string s)
        {
            if (s == null) { return null; }
            List<byte> buffer = new List<byte>();
            for (int i = 0; i < s.Length; i++)
            {
                byte c = (byte)s[i];
                if (c == '+') { buffer.Add((byte)' '); }
                else if (c == '%' && i + 2 < s.Length && IsHex((byte)s[i + 1]) && IsHex((byte)s[i + 2]))
                {
                    byte c1 = (byte)s[i + 1];
                    if (c1 >= (byte)'0' && c1 <= (byte)'9') { c1 -= (byte)'0'; }
                    else if (c1 >= (byte)'a' && c1 <= (byte)'f') { c1 -= (byte)'a'; c1 += 10; }
                    else if (c1 >= (byte)'A' && c1 <= (byte)'F') { c1 -= (byte)'A'; c1 += 10; }

                    byte c2 = (byte)s[i + 2];
                    if (c2 >= '0' && c2 <= (byte)'9') { c2 -= (byte)'0'; }
                    else if (c2 >= (byte)'a' && c2 <= (byte)'f') { c2 -= (byte)'a'; c2 += 10; }
                    else if (c2 >= (byte)'A' && c2 <= (byte)'F') { c2 -= (byte)'A'; c2 += 10; }

                    buffer.Add((byte)((c1 << 4) | c2));
                    i += 2;
                }
                else buffer.Add(c);
            }
            byte[] bufferArray = buffer.ToArray<byte>();
            return (new System.Text.UTF8Encoding()).GetString(bufferArray);
        }

        public static List<string> mdStringList = new List<string>() { "_c", "_s", "_i", "_x", "_so", "_n", "_p", "_sc", "_se", "_h", "_w", "_ha", "_sn" };

        public static Dictionary<string, string> metaDataMappingDictionary = new Dictionary<string, string>()
        {
            { "_c",   "StudyCentre" },
            { "_s",   "StudyCode" },
            { "_i",   "StudyInvestigator" }, 
            { "_x",   "StudyExerciseType" },
            { "_so",  "StudyOperator" },
            { "_n",   "StudyNotes" },
            { "_p",   "SubjectSite" },
            { "_sc",  "SubjectCode" },
            { "_se",  "SubjectSex" },
            { "_h",   "SubjectHeight" },
            { "_w",   "SubjectWeight" },
            { "_ha",  "SubjectHandedness" },
            { "_sn",  "SubjectNotes" },
        };
        // "DeviceId"
        // "SessionId"
        // "StartTime" / "StartTimeNumeric"
        // "EndTime" / "EndTimeNumeric"

        public static IDictionary<string, string> ParseMetaData(string source, ICollection<string> basicSet)
        {
            IDictionary<string, string> metaData = new Dictionary<string, string>();

            // Replace invalid characters
            StringBuilder sb = new StringBuilder();
            if (source != null)
            {
                for (int i = 0; i < source.Length; i++)
                {
                    char c = source[i];
                    if (c < 0x20 || c >= 0xff) { c = ' '; }
                    sb.Append(c);
                }
                source = sb.ToString();
            }

            foreach (string key in basicSet)
            {
                metaData[key] = "";
            }

            if (source != null)
            {
                source = source.Trim();
                string[] nvps = source.Split(new char[] { '&' });
                foreach (string nvp in nvps)
                {
                    string[] parts = nvp.Split(new char[] { '=' }, 2);
                    string name, value;

                    if (parts.Length >= 2) { name = parts[0]; value = parts[1]; }
                    else { name = ""; value = parts[0]; }

                    name = UrlDecode(name);
                    value = UrlDecode(value);

                    if (name.Trim().Length > 0 || value.Trim().Length > 0)
                    {
                        // Append any existing value for the same key name
                        if (metaData.ContainsKey(name) && metaData[name].Length > 0)
                        {
                            //value = value + " <=[ " + metaData[name] + "]";
                        }
                        metaData[name] = value;
                    }

                }
            }
            return metaData;
        }

        public string CreateMetaData()
        {
            return CreateMetaData(metaDataEntries);
        }

        public static string CreateMetaData(IList<MetaDataEntry> metaData)
        {
            StringBuilder sb = new StringBuilder();
            if (metaData != null)
            {
                foreach (MetaDataEntry entry in metaData)
                {
                    // Ignore built-in keys if there is no value
                    if (!(entry.key.StartsWith("_") && (entry.Value == null || entry.Value.Trim().Length == 0)))
                    {
                        if (entry.key.Trim().Length > 0 || entry.Value.Trim().Length > 0)
                        {
                            if (sb.Length > 0) { sb.Append("&"); }
                            sb.Append(UrlEncode(entry.key));
                            sb.Append("=");
                            sb.Append(UrlEncode(entry.Value));
                        }
                    }
                }
            }
            return sb.ToString();
        }

        public static string CreateMetaData(IDictionary<string, string> metaData)
        {
            StringBuilder sb = new StringBuilder();
            if (metaData != null)
            {
                foreach (KeyValuePair<string, string> entry in metaData)
                {
                    if (entry.Key.Trim().Length > 0 || entry.Value.Trim().Length > 0)
                    {
                        if (sb.Length > 0) { sb.Append("&"); }
                        sb.Append(UrlEncode(entry.Key));
                        sb.Append("=");
                        sb.Append(UrlEncode(entry.Value));
                    }
                }
            }
            return sb.ToString();
        }

        public const int DATA_SECTOR_SIZE = 512;
        public const int DATA_SECTORS_PER_BLOCK = 256;
        public const int DATA_ERASE_BLOCK_SIZE = DATA_SECTORS_PER_BLOCK * DATA_SECTOR_SIZE;
        public const int DATA_METADATA_OFFSET = 0 * DATA_ERASE_BLOCK_SIZE;
        public const int DATA_SESSIONINFO_OFFSET = 1 * DATA_ERASE_BLOCK_SIZE;
        //public const int DATA_DATA_OFFSET = 2 * DATA_ERASE_BLOCK_SIZE;
        public const int DATA_SESSION_SIZE_SECTORS = 16 * DATA_SECTORS_PER_BLOCK;
        public const int DATA_TOTAL_SESSION_BLOCKS = 256 - 1;
        public const int DATA_CAPACITY = DATA_TOTAL_SESSION_BLOCKS * DATA_ERASE_BLOCK_SIZE;



        public static DateTime ConvertDateTime(uint timestamp)
        {
            // [YYYYYYMM MMDDDDDh hhhhmmmm mmssssss]
            int year = (int)(2000 + ((timestamp >> 26) & 0x3f));
            int month = (int)((timestamp >> 22) & 0x0f);
            int day = (int)((timestamp >> 17) & 0x1f);
            int hours = (int)((timestamp >> 12) & 0x1f);
            int minutes = (int)((timestamp >> 6) & 0x3f);
            int seconds = (int)((timestamp >> 0) & 0x3f);
            DateTime dateTime = DateTime.MinValue;
            if (month >= 1 && month <= 12)
            {
                try { dateTime = new DateTime(year, month, day, hours, minutes, seconds); }
                catch (Exception) { }
            }
            return dateTime;
        }


        public uint deviceId;
        public uint deviceSessionId;
        public byte samplingRateCode;
        public double samplingRate;
        public int samplingRange;
        public DateTime? startDate = null;
        public DateTime? endDate = null;
        public TimeSpan? duration = null;
        public uint fileSize = 0;
        public uint dataOffset = 0;

        public string StartDateString
        {
            get
            {
                if (!startDate.HasValue) { return ""; }
                return String.Format("{0:yyyy-MM-dd HH:mm:ss}", startDate.Value);
            }
        }

        public string StartDateString2
        {
            get
            {
                if (!startDate.HasValue) { return ""; }
                return String.Format("{0:yyyy-MM-dd-HH-mm-ss}", startDate.Value);
            }
        }

        public string EndDateString
        {
            get
            {
                if (!endDate.HasValue) { return ""; }
                return String.Format("{0:yyyy-MM-dd HH:mm:ss}", endDate.Value);
            }
        }

        public string DurationString
        {
            get
            {
                if (!duration.HasValue) { return ""; }
                return TimeSpanString(duration);
            }
        }

        public string NowDateString
        {
            get
            {
                return String.Format("{0:yyyy-MM-dd HH:mm:ss}", DateTime.Now);
            }
        }

        //public string RecoveryOperator
        //{
        //    get
        //    {
        //        return Properties.Settings.Default.Operator;
        //    }
        //}

        public static string TimeSpanString(TimeSpan? timeDifference)
        {
            string difference = "";
            if (!timeDifference.HasValue) { difference = "<error>"; }
            else if (Math.Abs(timeDifference.Value.TotalDays) >= 365.25) { difference = string.Format("{0:0.0} years", timeDifference.Value.TotalDays / 365.25); }
            else if (Math.Abs(timeDifference.Value.TotalDays) >= 1.0) { difference = string.Format("{0:0.0} days", timeDifference.Value.TotalDays); }
            else if (Math.Abs(timeDifference.Value.TotalHours) >= 1.0) { difference = string.Format("{0:0.0} hours", timeDifference.Value.TotalHours); }
            else if (Math.Abs(timeDifference.Value.TotalMinutes) >= 1.0) { difference = string.Format("{0:0.0} mins", timeDifference.Value.TotalMinutes); }
            else if (Math.Abs(timeDifference.Value.TotalSeconds) >= 1.0) { difference = string.Format("{0:0.0} secs", timeDifference.Value.TotalSeconds); }
            else { difference = "<zero>"; }
            return difference;
        }


        public bool SaveMetaData(string data)
        {
            bool success = true;
            Cursor oldCursor = Cursor.Current;
            Cursor.Current = Cursors.WaitCursor;

            if (data == null) { data = ""; }

            if (data.Length > ANNOTATION_TOTAL_LENGTH)
            {
                Console.WriteLine("Annotation length too large: " + data.Length + "");
                data = data.Substring(0, ANNOTATION_TOTAL_LENGTH);
            }

            if (data.Length < ANNOTATION_TOTAL_LENGTH)
            {
                data = data + new string(ANNOTATION_PADDING, ANNOTATION_TOTAL_LENGTH - data.Length);
            }

            for (int i = 0; i < ANNOTATION_SEGMENT_COUNT; i++)
            {
                string command = string.Format("ANNOTATE{0:00}", i);

                string segment = data.Substring(i * ANNOTATION_SEGMENT_LENGTH, ANNOTATION_SEGMENT_LENGTH);

                string oldSegment = null;
                if (oldRawAnnotation != null && oldRawAnnotation.Length >= (i * ANNOTATION_SEGMENT_LENGTH + ANNOTATION_SEGMENT_LENGTH))
                {
                    oldSegment = oldRawAnnotation.Substring(i * ANNOTATION_SEGMENT_LENGTH, ANNOTATION_SEGMENT_LENGTH);
                }

                if (segment != oldSegment)
                {
                    //if (deviceControl.SingleCommand(command + "=" + segment, command + "=") == null)
                    //{
                     //   success = false;
                    //}
                }
            }

            Cursor.Current = oldCursor;
            return success;
        }




        public int GetCapacity(bool samples)
        {
            if (!samples)
            {
                // Return percentage
                return (int)(100 * fileSize / DATA_CAPACITY);
            }
            else
            {
                int freeSessionSectors = (int)((DATA_CAPACITY - fileSize) / DATA_SESSION_SIZE_SECTORS);
                int freeSamples = freeSessionSectors * 120;
                return freeSamples;
            }
        }

        public TimeSpan GetCapacity(float sampleRate)
        {
            int freeSamples = GetCapacity(true);
            float freeTime = freeSamples / sampleRate;
            return new TimeSpan(0, 0, (int)freeTime);
        }

        public string FromFile(string deviceFileName)
        {
            if (deviceFileName == null) { return null; }
            StringBuilder metaDataString = new StringBuilder();
            try
            {
                //FileInfo fi = new FileInfo(sourceFilename);
                //uint length = (uint)fi.Length;

                using (FileStream fs = new FileStream(deviceFileName, FileMode.Open, FileAccess.Read, FileShare.Read))
                {
                    fileSize = (uint)fs.Length;

                    // Read metadata header
                    dataOffset = 0;
                    byte[] metadataBuffer = new byte[DATA_ERASE_BLOCK_SIZE];
                    try
                    {
                        fs.Seek(DATA_METADATA_OFFSET, SeekOrigin.Begin);
                        if (fs.Read(metadataBuffer, 0, 1024) == metadataBuffer.Length)
                        {
                            fs.Seek(DATA_METADATA_OFFSET, SeekOrigin.Begin);
                            if (fs.Read(metadataBuffer, 0, metadataBuffer.Length) != metadataBuffer.Length)
                            {
                                //Trace.WriteLine("PREVIEW: Unexpectedly read too few bytes (MD).");
                            }
                        }
                    }
                    catch (Exception) { ; }
                    dataOffset = BitConverter.ToUInt16(metadataBuffer, 2);
                    if (dataOffset != 0) { dataOffset += 4; }

                    deviceId = BitConverter.ToUInt16(metadataBuffer, 5);
                    deviceSessionId = BitConverter.ToUInt32(metadataBuffer, 7);
                    samplingRateCode = metadataBuffer[36];
                    samplingRate = (3200.0 / (1 << (15 - (samplingRateCode & 0x0f))));
                    samplingRange = (16 >> (samplingRateCode >> 6));

                    for (int i = 0; i < ANNOTATION_TOTAL_LENGTH + DATA_SECTOR_SIZE; i++)
                    {
                        char c = (char)metadataBuffer[ANNOTATION_OFFSET + i];
                        if (c != 0xff && c != ' ')
                        {
                            if (c == '?') { c = '&'; }
                            metaDataString.Append(c);
                        }
                    }

                    // Read sessioninfo header
                    byte[] sessioninfoBuffer = new byte[DATA_ERASE_BLOCK_SIZE];
                    if (metadataBuffer[0] == 0x4D && metadataBuffer[1] == 0x44 && metadataBuffer[2] == 0xFC && metadataBuffer[3] == 0xFF)
                    {
                        dataOffset += DATA_ERASE_BLOCK_SIZE;
                        try
                        {
                            fs.Seek(DATA_SESSIONINFO_OFFSET, SeekOrigin.Begin);
                            if (fs.Read(sessioninfoBuffer, 0, sessioninfoBuffer.Length) != sessioninfoBuffer.Length)
                            {
                                //Trace.WriteLine("Unexpectedly read too few bytes (SI).");
                            }
                        }
                        catch (Exception) { ; }
                    }


                    startDate = null;
                    uint lastSessionOffset = 0;
                    uint lastSessionNumSectors = 0;
                    for (int i = 0; i < DATA_SECTORS_PER_BLOCK; i++)
                    {
                        ushort sessionInfoHeader = BitConverter.ToUInt16(sessioninfoBuffer, i * DATA_SECTOR_SIZE + 0);          // @0 0x4953 = ("SI") Session Information

                        if (sessionInfoHeader == 0x4953)
                        {
                            uint sessionInfoSessionId = BitConverter.ToUInt32(sessioninfoBuffer, i * DATA_SECTOR_SIZE + 4);         // @4 [4] (32-bit unique session identifier, 0 = unknown)
                            if (sessionInfoSessionId == deviceSessionId)
                            {
                                ushort sessionInfoType = BitConverter.ToUInt16(sessioninfoBuffer, i * DATA_SECTOR_SIZE + 10);           // @10 [2] (b0: 0 = session continuation, 1 = new session, b1-b15: reserved)
                                uint sessionInfoSessionOffset = BitConverter.ToUInt32(sessioninfoBuffer, i * DATA_SECTOR_SIZE + 12);    // @12 [4] offset in bytes within file for the first session block
                                uint startTimeValue = BitConverter.ToUInt32(sessioninfoBuffer, i * DATA_SECTOR_SIZE + 16);              // @16 [4] (last reported RTC value, 0 = unknown) [YYYYYYMM MMDDDDDh hhhhmmmm mmssssss]

                                DateTime startTime = ConvertDateTime(startTimeValue);
                                if (!startDate.HasValue) { startDate = startTime; }

                                lastSessionOffset = sessionInfoSessionOffset;
                                lastSessionNumSectors = DATA_SESSION_SIZE_SECTORS;
                            }
                        }
                        else if (sessionInfoHeader == 0x5841)     // AX (old file format)
                        {
                            uint sessionInfoSessionId = BitConverter.ToUInt32(sessioninfoBuffer, i * DATA_SECTOR_SIZE + 6);         // @6 [4] (32-bit unique session identifier, 0 = unknown)
                            if (sessionInfoSessionId == deviceSessionId)
                            {
                                //ushort sessionInfoType = 1;
                                //uint sessionInfoSessionOffset = DATA_SESSIONINFO_OFFSET;
                                uint startTimeValue = BitConverter.ToUInt32(sessioninfoBuffer, i * DATA_SECTOR_SIZE + 14);              // @14 [4] timestamp

                                DateTime startTime = ConvertDateTime(startTimeValue);
                                if (!startDate.HasValue) { startDate = startTime; }

                                lastSessionOffset = (uint)((fs.Length + DATA_ERASE_BLOCK_SIZE - 1) / DATA_ERASE_BLOCK_SIZE);
                                if (lastSessionOffset != 0) { lastSessionOffset = (lastSessionOffset - 1) * DATA_ERASE_BLOCK_SIZE; }
                                lastSessionNumSectors = DATA_SESSION_SIZE_SECTORS;
                            }
                        }
                    }

                    // Find last date/time with binary search
                    byte[] buffer = new byte[DATA_SECTOR_SIZE];
                    endDate = null;
                    uint sectorA = 0;
                    uint sectorB = lastSessionNumSectors - 1;
                    while (sectorA < sectorB)
                    {
                        uint midSector = (sectorA + sectorB) / 2;
                        fs.Seek(lastSessionOffset + midSector * DATA_SECTOR_SIZE, SeekOrigin.Begin);
                        fs.Read(buffer, 0, buffer.Length);

                        ushort sessionInfoHeader = BitConverter.ToUInt16(buffer, 0);
                        uint sessionInfoSessionId = BitConverter.ToUInt32(buffer, 6);         // @6 [4] (32-bit unique session identifier, 0 = unknown)
                        uint timeValue = 0;
                        if (sessionInfoHeader == 0x5841 && sessionInfoSessionId == deviceSessionId)
                        {
                            timeValue = BitConverter.ToUInt32(buffer, 14);   // @14 [4] timestamp
                        }

                        // Binary descent
                        if (timeValue != 0)
                        {
                            if (sectorA == midSector) { break; }
                            sectorA = midSector;
                            endDate = ConvertDateTime(timeValue);
                        }
                        else
                        {
                            if (sectorB == midSector) { break; }
                            sectorB = midSector;
                        }
                    }

                    if (startDate.HasValue && endDate.HasValue)
                    {
                        duration = endDate.Value - startDate.Value;
                    }
                    else
                    {
                        duration = null;
                    }

                }
            }
            catch (IOException ex)
            {
                Console.WriteLine("METADATA: " + ex.Message);
            }
            return metaDataString.ToString();
        }

        public string GetValue(string key)
        {
            string value = null;
            if (metaDataEntries != null)
            {
                foreach (MetaDataEntry entry in metaDataEntries)
                {
                    if (entry.key.Equals(key) && (entry.Value != null && entry.Value.Length > 0))
                    {
                        value = entry.Value;
                    }
                }
            }
            return value;
        }


        public static Dictionary<string, string> MetadataFromReader(string filename)
        {
            // Read meta-data
            ushort? deviceId = null;
            uint? sessionId = null;
            DateTime? startTime = null;
            DateTime? endTime = null;
            string md = "";
            OmApiNet.OmReader reader = null;
            try
            {
                reader = OmApiNet.OmReader.Open(filename);
                deviceId = reader.DeviceId;
                sessionId = reader.SessionId;
                startTime = reader.StartTime;
                endTime = reader.EndTime;
                md = reader.MetaData;
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("ERROR: Problem reading metadata: " + e.Message);
            }
            finally
            {
                if (reader != null)
                {
                    try
                    {
                        reader.Close();
                    }
                    catch (Exception e)
                    {
                        Console.Error.WriteLine("ERROR: Problem closing reader: " + e.Message);
                    }
                    reader = null;
                }
            }

            Dictionary<string, string> metadataMap = new Dictionary<string, string>();

            Dictionary<string, string> parsed = (Dictionary<string, string>)MetaDataTools.ParseMetaData(md, MetaDataTools.mdStringList);
            if (parsed != null)
            {
                foreach (KeyValuePair<string, string> kvp in parsed)
                {
                    metadataMap.Add(MetaDataTools.metaDataMappingDictionary[kvp.Key], kvp.Value);
                }
            }

            if (deviceId.HasValue) { metadataMap.Add("DeviceId", string.Format("{0:00000}", deviceId.Value)); }
            if (sessionId.HasValue) { metadataMap.Add("SessionId", string.Format("{0:0000000000}", sessionId.Value)); }
            if (startTime.HasValue) { metadataMap.Add("StartTime", string.Format("{0:yyyy-MM-dd HH:mm:ss}", startTime.Value)); metadataMap.Add("StartTimeNumeric", string.Format("{0:yyyyMMddHHmmss}", startTime.Value)); }
            if (endTime.HasValue) { metadataMap.Add("EndTime", string.Format("{0:yyyy-MM-dd HH:mm:ss}", endTime.Value)); metadataMap.Add("EndTimeNumeric", string.Format("{0:yyyyMMddHHmmss}", endTime.Value)); }

            return metadataMap;
            // Dictionary<string, string> metadataMap = MetaDataTools.MetadataFromFile(filename);
            // foreach (KeyValuePair<string, string> kvp in metadataMap) { kvp.Key + "=" + kvp.Value; };
        }


        public static Dictionary<string, string> MetadataFromFile(string filename)
        {
            MetaDataTools tools = new MetaDataTools();
            string md = tools.FromFile(filename);

            Dictionary<string, string> metadataMap = new Dictionary<string, string>();

            Dictionary<string, string> parsed = (Dictionary<string, string>)MetaDataTools.ParseMetaData(md, MetaDataTools.mdStringList);
            if (parsed != null)
            {
                foreach (KeyValuePair<string, string> kvp in parsed)
                {
                    metadataMap.Add(MetaDataTools.metaDataMappingDictionary[kvp.Key], kvp.Value);
                }
            }

            metadataMap.Add("DeviceId", string.Format("{0:00000}", tools.deviceId));
            metadataMap.Add("SessionId", string.Format("{0:0000000000}", tools.deviceSessionId));
            if (tools.samplingRate != 0) { metadataMap.Add("SamplingRate", string.Format("{0}", tools.samplingRate)); }
            if (tools.samplingRange != 0) { metadataMap.Add("SamplingRange", string.Format("{0}", tools.samplingRange)); }
            if (tools.startDate.HasValue) { metadataMap.Add("StartTime", string.Format("{0:yyyy-MM-dd HH:mm:ss}", tools.startDate.Value)); metadataMap.Add("StartTimeNumeric", string.Format("{0:yyyyMMddHHmmss}", tools.startDate.Value)); }
            if (tools.endDate.HasValue) { metadataMap.Add("EndTime", string.Format("{0:yyyy-MM-dd HH:mm:ss}", tools.endDate.Value)); metadataMap.Add("EndTimeNumeric", string.Format("{0:yyyyMMddHHmmss}", tools.endDate.Value)); }

            return metadataMap;
        }



    }
}

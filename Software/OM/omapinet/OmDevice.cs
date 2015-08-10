using System;
using System.IO;
using System.Text;

namespace OmApiNet
{
    public class OmDevice : OmSource
    {
        protected Om om;

        public bool validData;
        public int failedCount = 0;
        private bool hasChanged;
        public DateTime lastUpdate = DateTime.MinValue;

        public delegate void OmDeviceDownloadCompleteCallback(ushort id, OmApi.OM_DOWNLOAD_STATUS status, string filename);
        public OmDeviceDownloadCompleteCallback downloadComplete = null;

        public OmDevice(Om om, ushort deviceId)
        {
            this.om = om;
            this.deviceId = deviceId;
            validData = false;

            filename = null;
            try
            {
                StringBuilder filenamesb = new StringBuilder(256);
                if (OmApi.OmGetDataFilename(deviceId, filenamesb) == OmApi.OM_OK)
                {
                    filename = filenamesb.ToString();
                }
            }
            catch (Exception) { Console.Error.WriteLine("ERROR: Problem getting data filename for device " + deviceId + "."); }

        }

        // Properties
        public override SourceCategory Category
        { 
            get 
            {
                SourceCategory category = SourceCategory.Other;

                if (!Connected)
                {
                    category = SourceCategory.Removed;        // Device not attached
                }
                else
                {
                    if (IsDownloading)
                    {
                        category = SourceCategory.Downloading;        // Device attached, downloading
                    }
                    else
                    {
                        if (HasData)
                        {
                            if (HasNewData)
                            {
                                category = SourceCategory.NewData;        // Device attached, not downloading, non-empty data file, archive attribute set
// TODO: Once API supports data detection (non-empty & modified), remove this line
category = SourceCategory.Other;
                            }
                            else
                            {
                                category = SourceCategory.Downloaded;     // Device attached, not downloading, non-empty data file, archive attribute cleared
                            }
                        }
                        else
                        {
                            if (SessionId == 0)
                            {
                                if (BatteryLevel < 100)
                                {
                                    category = SourceCategory.Charging;       // Device attached, not downloading, empty data file, zero session-id, charging
                                }
                                else
                                {
                                    category = SourceCategory.Standby;        // Device attached, not downloading, empty data file, zero session-id, charged
                                }
                            }
                            else
                            {
                                category = SourceCategory.Outbox;         // Device attached, not downloading, empty data file, non-zero session id
                            }
                        }
                    }
                }

                return category; 
            } 
        }

        protected ushort deviceId;
        public override ushort DeviceId { get { return deviceId; } }

        protected uint sessionId = uint.MaxValue;
        public override uint SessionId { get { return sessionId; } }

        protected int firmwareVersion = int.MinValue;
        public int FirmwareVersion { get { return firmwareVersion; } }

        protected int hardwareVersion = int.MinValue;
        public int HardwareVersion { get { return hardwareVersion; } }

        protected OmApi.OM_LED_STATE ledColor = OmApi.OM_LED_STATE.OM_LED_UNKNOWN;
        public OmApi.OM_LED_STATE LedColor { get { return ledColor; } }

        protected int batteryLevel = int.MinValue;
        public int BatteryLevel { get { return batteryLevel; } }

        protected TimeSpan timeDifference = TimeSpan.MinValue;
        public TimeSpan TimeDifference { get { return timeDifference; } }

        protected DateTime startTime = DateTime.MinValue;
        public DateTime StartTime { get { return startTime; } }

        protected DateTime stopTime = DateTime.MinValue;
        public DateTime StopTime { get { return stopTime; } }

        protected OmApi.OM_DOWNLOAD_STATUS downloadStatus = OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_NONE;
        public OmApi.OM_DOWNLOAD_STATUS DownloadStatus { get { return downloadStatus; } }

        protected int downloadValue = 0;
        public int DownloadValue { get { return downloadValue; } }

        protected string filename;
        public string Filename { get { return filename; } }

        //public bool HasData { get { if (Filename == null) { return false; }  return File.Exists(Filename); } }

        public bool HasData
        {
            get
            {
                if (IsDownloading) { return true; }     // Otherwise the below code seems very costly while downloading from a device

                string fName = "";
                StringBuilder filenamesb = new StringBuilder(256);
                if (OmApi.OmGetDataFilename(deviceId, filenamesb) == OmApi.OM_OK)
                {
                    fName = filenamesb.ToString();
                }

                if (!fName.Equals(""))
                {
                    if (!File.Exists(fName)) { return false; }

                    try
                    {
                        FileInfo f = new FileInfo(fName);

                        if (f.Length > 1024)
                            return true;
                    }
                    catch (Exception)
                    {
                        return false;
                    }
                }

                return false;
            }
        }

        // TODO: Remember to clear archive on successful download
        public bool HasNewData 
        {
            get 
            {
                if (IsDownloading) { return true; }     // Otherwise code below is very costly while downloading
                if (Filename == null) { Console.Error.WriteLine("ERROR: Problem getting filename attributes for device " + deviceId + " with unknown data file name."); return false; }
                try
                {
                    if (!File.Exists(Filename)) { Console.Error.WriteLine("ERROR: Problem getting filename attributes for device " + deviceId + " with missing data file: " + Filename); return false; }
                    FileAttributes attributes = File.GetAttributes(Filename);
                    return ((attributes & FileAttributes.Archive) != 0);
                }
                catch (Exception) { Console.Error.WriteLine("ERROR: Problem getting filename attributes for device " + deviceId + ": " + Filename); return false; }
            } 
        }


        public bool IsDownloading
        {
            get { return Connected && DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS; }
        }

        //TS - Added IsRecording instead of using the logic Dan had in MainForm everywhere.
        public enum RecordStatus
        {
            Stopped,
            Always,
            Interval
        }

        public RecordStatus IsRecording
        {
            get
            {
                if (StopTime <= StartTime || StopTime <= DateTime.Now) { return RecordStatus.Stopped; }
                else if (StartTime <= DateTime.MinValue && StopTime >= DateTime.MaxValue) { return RecordStatus.Always; }
                else { return RecordStatus.Interval; }
            }
        }
        //TS - End Of IsRecording.

        private bool connected;
        public bool Connected { get { return connected; } }
        internal void SetConnected(bool value)
        {
            if (connected != value) 
            { 
                connected = value; 
                validData = false;
                ledColor = OmApi.OM_LED_STATE.OM_LED_UNKNOWN;
                downloadStatus = OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_NONE;
                downloadValue = 0;
            } 
        }


        public bool Update(int resetIfUnresponsive)
        {
            bool changed = false;
            DateTime now = DateTime.Now;

            double updateInterval = 30.0;                           // Usually a 30 second update interval
            updateInterval = updateInterval + (failedCount * 10.0);
            updateInterval = Math.Min(updateInterval, 120.0);       // At most, 2 minute interval if not communicating

            if (lastUpdate == DateTime.MinValue || (now - lastUpdate) > TimeSpan.FromSeconds(updateInterval))
            {
                int error = 0;

//Console.WriteLine("backgroundWorkerUpdate - checking battery for " + this.deviceId + "...");
                int newBatteryLevel = OmApi.OmGetBatteryLevel(deviceId);
                lastUpdate = now;
                if (OmApi.OM_FAILED(newBatteryLevel)) { error |= 0x10; }

                // Battery level has changed, or first error reading battery level
                if (newBatteryLevel != batteryLevel || (error != 0 && failedCount == 0))
                {
                    batteryLevel = newBatteryLevel;
                    changed = true;
                }

                if (error == 0 && !validData)
                {
                    //Console.WriteLine("backgroundWorkerUpdate - first check for " + this.deviceId + "...");
                    int res;

                    // TODO: Error checking
                    res = OmApi.OmGetVersion(deviceId, out firmwareVersion, out hardwareVersion);
                    error |= (OmApi.OM_FAILED(res) ? 0x01 : 0);

                    uint time;
                    res = OmApi.OmGetTime(deviceId, out time);
                    error |= (OmApi.OM_FAILED(res) ? 0x02 : 0);
                    timeDifference = (OmApi.OmDateTimeUnpack(time) - now);

                    uint startTimeValue, stopTimeValue;
                    res = OmApi.OmGetDelays(deviceId, out startTimeValue, out stopTimeValue);
                    error |= (OmApi.OM_FAILED(res) ? 0x04 : 0);
                    startTime = OmApi.OmDateTimeUnpack(startTimeValue);
                    stopTime = OmApi.OmDateTimeUnpack(stopTimeValue);

                    res = OmApi.OmGetSessionId(deviceId, out sessionId);
                    error |= (OmApi.OM_FAILED(res) ? 0x08 : 0);

                    changed = true;
                    if (error == 0) { validData = true; }
                }


                if (error != 0)
                {
                    if (error != 0) { Console.WriteLine("ERROR: Problem fetching data for device: " + deviceId + " (code " + error + ")"); }
                    failedCount++;

                    // Every odd failure, try to reset the device
                    if (resetIfUnresponsive > 0 && (failedCount % resetIfUnresponsive) == 0 && !this.IsDownloading)
                    {
                        Console.WriteLine("NOTE: Resetting device " + deviceId + " (failed " + failedCount + " times)...");
                        Reset();
                    }
                }
            }

            changed |= hasChanged; 
            hasChanged = false;

            return changed;
        }

        public bool SetLed(OmApi.OM_LED_STATE state)
        {
            ledColor = state;
            if (OmApi.OM_FAILED(OmApi.OmSetLed(deviceId, (int)ledColor)))
            {
                return false;
            }
            hasChanged = true;
            om.OnChanged(new OmDeviceEventArgs(this));
            return true;
        }

        public volatile int volatileTemp;

        public bool SyncTime()
        {
            DateTime time;
            DateTime previous = DateTime.Now;
            while ((time = DateTime.Now) == previous) { volatileTemp = 0; }  // Spin until the second rollover (up to 1 second)
            // ...now set the time.

            if (OmApi.OM_FAILED(OmApi.OmSetTime(deviceId, OmApi.OmDateTimePack(time))))
            {
                return false;
            }
            timeDifference = TimeSpan.Zero;
            hasChanged = true;
            om.OnChanged(new OmDeviceEventArgs(this));
            return true;
        }

        public void UpdateDownloadStatus(OmApi.OM_DOWNLOAD_STATUS status, int value)
        {
            downloadStatus = status;
            downloadValue = value;
            hasChanged = true;
            om.OnChanged(new OmDeviceEventArgs(this, status));
        }

        string downloadFilename = null;
        string downloadFilenameRename = null;
        public void BeginDownloading(string filename, string renameFilename)
        {
            downloadFilename = filename;
            downloadFilenameRename = renameFilename;
            OmApi.OmBeginDownloading(deviceId, 0, -1, filename);
        }

        public void FinishedDownloading()
        {
            if (downloadFilename != null)
            {
                if (downloadFilenameRename != null && downloadFilename != downloadFilenameRename)
                {
                    File.Move(downloadFilename, downloadFilenameRename);
                }

                if (downloadComplete != null)
                {
                    string fn = ((downloadFilenameRename != null) ? downloadFilenameRename : downloadFilename);
                    downloadComplete(this.deviceId, OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_COMPLETE, fn);
                }

            }
        }

        public void CancelDownload()
        {
            OmApi.OmCancelDownload(deviceId);
        }

        public bool SetInterval(DateTime start, DateTime stop)
        {
            bool failed = false;

            failed |= OmApi.OM_FAILED(OmApi.OmSetDelays(deviceId, OmApi.OmDateTimePack(start), OmApi.OmDateTimePack(stop)));
            if (!failed)
            {
                failed |= OmApi.OM_FAILED(OmApi.OmCommit(deviceId));
            }

//validData = false;
            if (!failed)
            {
                this.startTime = start;
                this.stopTime = stop;
            }

            hasChanged = true;
            om.OnChanged(new OmDeviceEventArgs(this));
            return !failed;
        }

        public bool NeverRecord()
        {
            return SetInterval(DateTime.MaxValue, DateTime.MaxValue);
        }

        public bool AlwaysRecord()
        {
            return SetInterval(DateTime.MinValue, DateTime.MaxValue);
        }

        public bool SetSessionId(uint sessionId, bool commit)
        {
            bool failed = false;

            failed |= OmApi.OM_FAILED(OmApi.OmSetSessionId(deviceId, sessionId));
            if (commit)
            {
                failed |= OmApi.OM_FAILED(OmApi.OmCommit(deviceId));
            }

//validData = false;
            if (!failed)
            {
                this.sessionId = sessionId;
            }

            hasChanged = true;
            om.OnChanged(new OmDeviceEventArgs(this));
            return !failed;
        }

        public bool SetDebug(int debugCode)
        {
            int status = OmApi.OmCommand(deviceId, "\r\nDEBUG " + debugCode + "\r\n", null, 0, "DEBUG=", 2000, IntPtr.Zero, 0);
            return (status >= 0);
        }

        public bool Clear(bool wipe)
        {
            bool failed = false;

            // ???
            //failed |= OmApi.OM_FAILED(OmApi.OmCommit(deviceId));

            failed |= OmApi.OM_FAILED(OmApi.OmSetSessionId(deviceId, 0));                                                      // Clear the session id
            failed |= OmApi.OM_FAILED(OmApi.OmSetMetadata(deviceId, "", 0));                                                   // No metadata
            failed |= OmApi.OM_FAILED(OmApi.OmSetDelays(deviceId, OmApi.OM_DATETIME_INFINITE, OmApi.OM_DATETIME_INFINITE));    // Never log
            failed |= OmApi.OM_FAILED(OmApi.OmSetAccelConfig(deviceId, OmApi.OM_ACCEL_DEFAULT_RATE, OmApi.OM_ACCEL_DEFAULT_RANGE));    // Default configuration
            failed |= OmApi.OM_FAILED(OmApi.OmEraseDataAndCommit(deviceId, wipe ? OmApi.OM_ERASE_LEVEL.OM_ERASE_WIPE : OmApi.OM_ERASE_LEVEL.OM_ERASE_QUICKFORMAT)); // Erase data and commit
            //failed |= OmApi.OM_FAILED(OmApi.OmClearDataAndCommit(deviceId));                                                   // Clear data and commit

//validData = false;
            if (!failed)
            {
                this.sessionId = 0;
                this.startTime = OmApi.OmDateTimeUnpack(OmApi.OM_DATETIME_INFINITE);
                this.stopTime = OmApi.OmDateTimeUnpack(OmApi.OM_DATETIME_INFINITE);
            }

            hasChanged = true;
            om.OnChanged(new OmDeviceEventArgs(this));
            return !failed;
        }


        public int Reset()
        {
            // [DllImport("libomapi.dll")] public static extern int OmCommand(int deviceId, string command, [MarshalAs(UnmanagedType.LPStr)] StringBuilder metadata, int bufferSize, string expected, uint timeoutMs, IntPtr parseParts, int parseMax); // char **parseParts
            return OmApi.OmCommand((int)deviceId, "\r\nreset\r\n", (StringBuilder)null, 0, "RESET", (uint)500, IntPtr.Zero, 0);
        }
    }
}

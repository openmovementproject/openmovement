using System;
using System.IO;
using System.Runtime.InteropServices;
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
		private int deviceWarning = 0;	// 0=none, 1=discharged, 2=damaged?
		public int DeviceWarning { get { return deviceWarning; } }

        public delegate void OmDeviceDownloadCompleteCallback(uint id, OmApi.OM_DOWNLOAD_STATUS status, string filename, string downloadFilename);
        public OmDeviceDownloadCompleteCallback downloadComplete = null;

        public bool HasSyncGyro
        {
            get
            {
                return serialId != null && (serialId.StartsWith("AX6") || serialId.StartsWith("CWA64"));    // "CWA64" is prototype AX6 firmware
            }
        }

        public OmDevice(Om om, uint deviceId)
        {
            this.om = om;
            this.deviceId = deviceId;
            validData = false;

            filename = null;
            port = null;
            serialId = null;
            try
            {
                StringBuilder filenamesb = new StringBuilder(256);
                if (OmApi.OmGetDataFilename((int)deviceId, filenamesb) == OmApi.OM_OK)
                {
                    filename = filenamesb.ToString();
                    path = Path.GetDirectoryName(filename);
                }

                StringBuilder pathsb = new StringBuilder(256);
                if (OmApi.OmGetDevicePath((int)deviceId, pathsb) == OmApi.OM_OK)
                {
                    path = pathsb.ToString();
                }

                StringBuilder portsb = new StringBuilder(256);
                if (OmApi.OmGetDevicePort((int)deviceId, portsb) == OmApi.OM_OK)
                {
                    port = portsb.ToString();
                }

                StringBuilder serialIdsb = new StringBuilder(256);
                if (OmApi.OmGetDeviceSerial((int)deviceId, serialIdsb) == OmApi.OM_OK)
                {
                    serialId = serialIdsb.ToString();
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

        protected uint deviceId;
        public override uint DeviceId { get { return deviceId; } }

        protected uint sessionId = uint.MaxValue;
        public override uint SessionId { get { return sessionId; } }

        protected int firmwareVersion = int.MinValue;
        public int FirmwareVersion { get { return firmwareVersion; } }

        public bool ValidData { get { return validData; } }

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

        protected string path;
        public string DevicePath { get { return path; } }

        protected string port;
        public string Port { get { return port; } }

        protected string serialId;
        public string SerialId { get { return serialId; } } // Full device serial id

        //public bool HasData { get { if (Filename == null) { return false; }  return File.Exists(Filename); } }


        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool GetDiskFreeSpaceEx(string lpDirectoryName, out ulong lpFreeBytesAvailable, out ulong lpTotalNumberOfBytes, out ulong lpTotalNumberOfFreeBytes);

        private static long GetTotalSpace(string path)
        {
            if (path != null && !path.EndsWith("\\")) { path += '\\'; }
            if (!GetDiskFreeSpaceEx(path, out ulong _freeBytesAvailable, out ulong totalNumberOfBytes, out ulong _totalNumberOfFreeBytes))
            {
                string root = Path.GetPathRoot(path);
                if (root != null && root.Length >= 3 && root[1] == ':' && root[2] == '\\')
                {
                    DriveInfo info = new DriveInfo("" + root[0]);
                    return info.TotalSize;
                }
                return -1;
            }
            return (long)totalNumberOfBytes;
        }

        private static long GetFreeSpace(string path)
        {
            if (path != null && !path.EndsWith("\\")) { path += '\\'; }
            if (!GetDiskFreeSpaceEx(path, out ulong freeBytesAvailable, out ulong _totalNumberOfBytes, out ulong _totalNumberOfFreeBytes))
            {
                string root = Path.GetPathRoot(path);
                if (root != null && root.Length >= 3 && root[1] == ':' && root[2] == '\\')
                {
                    DriveInfo info = new DriveInfo("" + root[0]);
                    return info.AvailableFreeSpace;
                }
                return -1;
            }
            return (long)freeBytesAvailable;
        }


        private static long STANDARD_CAPACITY_AX3 = 507904000;
        public long deviceCapacity = -1;
        public long DeviceCapacity
        {
            get
            {
                if (deviceCapacity < 0)
                {
                    deviceCapacity = GetTotalSpace(DevicePath);
                }
                if (deviceCapacity < 0 && serialId.StartsWith("CWA"))
                {
                    deviceCapacity = STANDARD_CAPACITY_AX3;
                }
                return deviceCapacity;
            }
        }


        public bool HasData
        {
            get
            {
                if (IsDownloading) { return true; }     // Otherwise the below code seems very costly while downloading from a device

                string fName = "";
                StringBuilder filenamesb = new StringBuilder(256);
                if (OmApi.OmGetDataFilename((int)deviceId, filenamesb) == OmApi.OM_OK)
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


        public bool Update(int resetIfUnresponsive, bool force = false)
        {
            bool changed = false;
            DateTime now = DateTime.Now;

            double updateInterval = 30.0;                           // Usually a 30 second update interval
            updateInterval = updateInterval + (failedCount * 10.0);
            updateInterval = Math.Min(updateInterval, 120.0);       // At most, 2 minute interval if not communicating

            if (force || lastUpdate == DateTime.MinValue || (now - lastUpdate) > TimeSpan.FromSeconds(updateInterval))
            {
                int error = 0;

//Console.WriteLine("backgroundWorkerUpdate - checking battery for " + this.deviceId + "...");
                int newBatteryLevel = OmApi.OmGetBatteryLevel((int)deviceId);
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
                    res = OmApi.OmGetVersion((int)deviceId, out firmwareVersion, out hardwareVersion);
                    error |= (OmApi.OM_FAILED(res) ? 0x01 : 0);

                    uint time;
                    res = OmApi.OmGetTime((int)deviceId, out time);
                    error |= (OmApi.OM_FAILED(res) ? 0x02 : 0);
					DateTime deviceTime = OmApi.OmDateTimeUnpack(time);
                    timeDifference = (deviceTime - now);
					// DateTime deviceTime = DateTime.Now + omDevice.TimeDifference;

					// Caution the user if the battery was allowed to reset 
					// (the RTC clock became reset)
					DateTime cautionDate = new DateTime(2008, 1, 1, 0, 0, 0);
					if (deviceWarning < 1 && deviceTime < cautionDate) {
						deviceWarning = 1;  // Completely flattening battery may damage it
                        changed = true;
                    }

                    // Warn the user of likely damaged device if RTC reset less than
                    // 15 minutes ago, yet the device reports >= 70% charge
                    DateTime warningDate = new DateTime(2000, 1, 1, 0, 15, 0);
					int warningPercent = 70;
					if (deviceWarning < 2 && deviceTime < warningDate && batteryLevel >= warningPercent) {
						deviceWarning = 2;  // Device battery or RTC may be damaged
                        changed = true;
                    }

                    uint startTimeValue, stopTimeValue;
                    res = OmApi.OmGetDelays((int)deviceId, out startTimeValue, out stopTimeValue);
                    error |= (OmApi.OM_FAILED(res) ? 0x04 : 0);
                    startTime = OmApi.OmDateTimeUnpack(startTimeValue);
                    stopTime = OmApi.OmDateTimeUnpack(stopTimeValue);

                    res = OmApi.OmGetSessionId((int)deviceId, out sessionId);
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
            if (OmApi.OM_FAILED(OmApi.OmSetLed((int)deviceId, (int)ledColor)))
            {
                return false;
            }
            hasChanged = true;
            om.OnChanged(new OmDeviceEventArgs(this));
            return true;
        }

        public static volatile int volatileTemp = 0;	// Junk to prevent code elimination

        public bool SyncTime()
        {
            DateTime time;
            DateTime previous = DateTime.Now;
            while ((time = DateTime.Now).Second == previous.Second) { volatileTemp++; }  // Busy spin until the second rollover (up to 1 second)

            // ...now set the time (always to the nearest second)
            DateTime setTime = new DateTime(time.Year, time.Month, time.Day, time.Hour, time.Minute, time.Second);
            if (OmApi.OM_FAILED(OmApi.OmSetTime((int)deviceId, OmApi.OmDateTimePack(setTime))))
            {
                Console.WriteLine("TIMESYNC: Failed to write time.");
                return false;   // Failed to write time
            }

            // Verify that the clock was set as expected
            if (OmApi.OM_FAILED(OmApi.OmGetTime((int)deviceId, out uint newTime)))
            {
                Console.WriteLine("TIMESYNC: Failed to read time.");
                return false;   // Failed to read time
            }
            DateTime newDateTime = OmApi.OmDateTimeUnpack(newTime);
            timeDifference = newDateTime - setTime;
//            Console.WriteLine("Setting time to: " + setTime + " -- received: " + newDateTime + " (delta " + timeDifference + "ms).");
            if (Math.Abs(timeDifference.TotalMilliseconds) > 3000)
            {
                Console.WriteLine("TIMESYNC: Time was not within range: " + (int)timeDifference.TotalSeconds);
                return false;   // Clock was not set correctly
            }

            hasChanged = true;
            om.OnChanged(new OmDeviceEventArgs(this));

            // Verify that the clock is ticking
            DateTime checkStart = DateTime.Now;
            for (; ; )
            {
                if (OmApi.OM_FAILED(OmApi.OmGetTime((int)deviceId, out uint currentTime)))
                {
                    Console.WriteLine("TIMESYNC: Failed to read time while checking change.");
                    return false;   // Failed to read time
                }
                if (currentTime > newTime && ((DateTime.Now - OmApi.OmDateTimeUnpack(currentTime)).TotalMilliseconds < 5000))
                {
                    break;          // The clock is ticking and within a few seconds of now
                }
                var td = DateTime.Now - checkStart;
                if (td.TotalMilliseconds > 4000)
                {
                    Console.WriteLine("TIMESYNC: Time was not within range while checking change: " + (int)td.TotalSeconds);
                    return false;   // The clock is not ticking
                }
            }

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
            OmApi.OmBeginDownloading((int)deviceId, 0, -1, filename);
        }

        public void FinishedDownloading()
        {
            if (downloadFilename != null)
            {
                string finalFilename = downloadFilename;
                if (downloadFilenameRename != null && downloadFilename != downloadFilenameRename)
                {
                    File.Move(downloadFilename, downloadFilenameRename);
                    finalFilename = downloadFilenameRename;
                }

                if (downloadComplete != null)
                {
                    downloadComplete(this.deviceId, OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_COMPLETE, finalFilename, downloadFilename);
                }

            }
        }

        public void CancelDownload()
        {
            OmApi.OmCancelDownload((int)deviceId);
        }

        public bool SetInterval(DateTime start, DateTime stop)
        {
            bool failed = false;

            failed |= OmApi.OM_FAILED(OmApi.OmSetDelays((int)deviceId, OmApi.OmDateTimePack(start), OmApi.OmDateTimePack(stop)));
            if (!failed)
            {
                failed |= OmApi.OM_FAILED(OmApi.OmCommit((int)deviceId));
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

            failed |= OmApi.OM_FAILED(OmApi.OmSetSessionId((int)deviceId, sessionId));
            if (commit)
            {
                failed |= OmApi.OM_FAILED(OmApi.OmCommit((int)deviceId));
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
            int status = OmApi.OmCommand((int)deviceId, "\r\nDEBUG " + debugCode + "\r\n", null, 0, "DEBUG=", 2000, IntPtr.Zero, 0);
            return (status >= 0);
        }

        public bool Clear(bool wipe)
        {
            bool failed = false;

            // ???
            //failed |= OmApi.OM_FAILED(OmApi.OmCommit(deviceId));

            failed |= OmApi.OM_FAILED(OmApi.OmSetSessionId((int)deviceId, 0));                                                      // Clear the session id
            failed |= OmApi.OM_FAILED(OmApi.OmSetMetadata((int)deviceId, "", 0));                                                   // No metadata
            failed |= OmApi.OM_FAILED(OmApi.OmSetDelays((int)deviceId, OmApi.OM_DATETIME_INFINITE, OmApi.OM_DATETIME_INFINITE));    // Never log
            failed |= OmApi.OM_FAILED(OmApi.OmSetAccelConfig((int)deviceId, OmApi.OM_ACCEL_DEFAULT_RATE, OmApi.OM_ACCEL_DEFAULT_RANGE));    // Default configuration
            failed |= OmApi.OM_FAILED(OmApi.OmEraseDataAndCommit((int)deviceId, wipe ? OmApi.OM_ERASE_LEVEL.OM_ERASE_WIPE : OmApi.OM_ERASE_LEVEL.OM_ERASE_QUICKFORMAT)); // Erase data and commit
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

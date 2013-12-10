using System;
using System.IO;
using System.Text;

namespace OmApiNet
{
    public class OmDevice : OmSource
    {
        protected Om om;

        public bool validData;
        private bool hasChanged;
        public DateTime lastBatteryUpdate = DateTime.MinValue;

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
                string fName = "";
                StringBuilder filenamesb = new StringBuilder(256);
                if (OmApi.OmGetDataFilename(deviceId, filenamesb) == OmApi.OM_OK)
                {
                    fName = filenamesb.ToString();
                }

                if (!fName.Equals(""))
                {
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
                // DateTime.Now >= StopTime
                if (StopTime <= StartTime) { return RecordStatus.Stopped; }
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


        public bool Update()
        {
            bool changed = false;
            DateTime now = DateTime.Now;

            if (!validData)
            {
                bool error = false;
                int res;

                // TODO: Error checking
                res = OmApi.OmGetVersion(deviceId, out firmwareVersion, out hardwareVersion);
                error |= OmApi.OM_FAILED(res);
                 
                uint time;
                res = OmApi.OmGetTime(deviceId, out time);
                error |= OmApi.OM_FAILED(res);
                timeDifference = (OmApi.OmDateTimeUnpack(time) - now);

                uint startTimeValue, stopTimeValue;
                res = OmApi.OmGetDelays(deviceId, out startTimeValue, out stopTimeValue);
                error |= OmApi.OM_FAILED(res);
                startTime = OmApi.OmDateTimeUnpack(startTimeValue);
                stopTime = OmApi.OmDateTimeUnpack(stopTimeValue);

                res = OmApi.OmGetSessionId(deviceId, out sessionId);
                error |= OmApi.OM_FAILED(res);

error = false;      // Ignore error here as retrying won't help up (log to console)
Console.WriteLine("ERROR: Problem fetching data for device: " + deviceId);

                changed = true;
                if (!error) { validData = true; }
            }

            if (lastBatteryUpdate == DateTime.MinValue || (now - lastBatteryUpdate) > TimeSpan.FromSeconds(60.0f))
            {
                int newBatteryLevel = OmApi.OmGetBatteryLevel(deviceId);
                lastBatteryUpdate = now;

                if (newBatteryLevel != batteryLevel)
                {
                    batteryLevel = newBatteryLevel;
                    changed = true;
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

        public bool SyncTime(int now)
        {
            DateTime time;
            if(now == 1)
                time = DateTime.Now;
            else
                time = new DateTime(0);

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

    }
}

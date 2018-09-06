/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

// Om - Class-based .NET Wrapper for the Open Movement API
// Dan Jackson, 2011-2012

using System;
using System.Collections.Generic;

namespace OmApiNet
{
    public class Om : IDisposable
    {
        /** Open Movement API singleton instance */
        private static Om instance;

        /** Get the Open Movement API singleton instance */
        public static Om Instance
        {
            get
            {
                if (instance == null) { instance = new Om(); }
                return instance;
            }
        }


        // Must keep a reference to callbacks to prevent the garbage collector removing the delegate
        private volatile OmApi.OmLogCallback logCallbackDelegate;
        private volatile OmApi.OmDeviceCallback deviceCallbackDelegate;
        private volatile OmApi.OmDownloadCallback downloadCallbackDelegate;

        /** Private constructor (singleton class) */
        private Om()
        {
            // Register our log callback handler first (before startup)
            logCallbackDelegate = new OmApi.OmLogCallback(LogCallback);
            GC.SuppressFinalize(logCallbackDelegate);
            OmApi.OmSetLogCallback(logCallbackDelegate, IntPtr.Zero);

            // Register our device callback handler (before startup, so we get the initial set of devices)
            deviceCallbackDelegate = new OmApi.OmDeviceCallback(DeviceCallback);
            GC.SuppressFinalize(deviceCallbackDelegate);
            OmApi.OmSetDeviceCallback(deviceCallbackDelegate, IntPtr.Zero);

            // Startup the API
            int result = OmApi.OmStartup(OmApi.OM_VERSION);
            OmAssert(result, "OmStartup");

            /*
            // Get the existing device ids
            int total = OmApi.OmGetDeviceIds(null, 0);
            int[] deviceIds = new int[total];
            int count = OmApi.OmGetDeviceIds(deviceIds, deviceIds.Length);
            if (count > deviceIds.Length) { count = deviceIds.Length; }
            for (int i = 0; i < count; i++)
            {
                Console.WriteLine("" + (i + 1) + "/" + count + " = " + deviceIds[i]);
                deviceCallbackDelegate(IntPtr.Zero, deviceIds[i], OmApi.OM_DEVICE_STATUS.OM_DEVICE_CONNECTED);
            }
            */

            downloadCallbackDelegate = new OmApi.OmDownloadCallback(DownloadCallback);
            GC.SuppressFinalize(downloadCallbackDelegate);
            OmApi.OmSetDownloadCallback(downloadCallbackDelegate, IntPtr.Zero);
        }

        /** Destructor */
        ~Om()
        {
            Dispose(false);
        }

        /** Overridden dispose handler */
        public void Dispose()
        {
            Dispose(true);
        }

        /** Disposed flag */
        private bool disposed;

        /** Dispose handler */
        public void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (disposing)
                {
                    // Dispose of any managed resources
                    ;
                }

                // Clean up unmanaged resources
                OmApi.OmShutdown();

                // Allow finalizing
                if (logCallbackDelegate != null) { GC.ReRegisterForFinalize(logCallbackDelegate); }
                if (deviceCallbackDelegate != null) { GC.ReRegisterForFinalize(deviceCallbackDelegate); }

                // Disposing complete
                disposed = true;
            }
        }

        /** Macro to assert an OmApi value was successful, throws an exception otherwise */
        public static void OmAssert(int result, string message = null)
        {
            if (OmApi.OM_FAILED(result))
            {
                throw new OmException(result, message);
            }
            return;
        }

        protected IDictionary<uint, OmDevice> devices = new Dictionary<uint, OmDevice>();

        /** Calls an event handler for any attached devices. Useful for discovering devices attached at startup. */
        public void ForAllAttachedDevices(OmDeviceEventHandler deviceEventHandler)
        {
            lock (devices)
            {
                foreach (OmDevice device in devices.Values)
                {
                    if (device.Connected)
                    {
                        deviceEventHandler(this, new OmDeviceEventArgs(device));
                    }
                }
            }
        }

        public OmDevice GetDevice(uint deviceId)
        {
            OmDevice device = null;
            if (deviceId != 0)
            {
                // Obtain a reference to the device object, or create one if new
                if (devices.ContainsKey(deviceId))
                {
                    device = devices[deviceId];
                }
                else
                {
                    device = new OmDevice(this, deviceId);
                    lock (devices)
                    {
                        devices[deviceId] = device;
                    }
                }
            }
            return device;
        }

        protected void LogCallback(IntPtr reference, string message)
        {
            Console.WriteLine("LOG: " + message);
        }

        protected void DeviceCallback(IntPtr reference, int deviceId, OmApi.OM_DEVICE_STATUS status)
        {
Console.WriteLine("" + status + " - " + (uint)deviceId);
            OmDevice device = GetDevice((uint)deviceId);
            if (device == null) { return; }

            if (status == OmApi.OM_DEVICE_STATUS.OM_DEVICE_CONNECTED)
            {
                device.SetConnected(true);
                if (DeviceAttached != null)
                {
                    DeviceAttached(this, new OmDeviceEventArgs(device));
                }
            }
            else if (status == OmApi.OM_DEVICE_STATUS.OM_DEVICE_REMOVED)
            {
                device.SetConnected(false);
                if (DeviceRemoved != null)
                {
                    DeviceRemoved(this, new OmDeviceEventArgs(device));
                }
            }
        }


        private IDictionary<int, int> cacheProgressValue = new Dictionary<int, int>();

        protected void DownloadCallback(IntPtr reference, int deviceId, OmApi.OM_DOWNLOAD_STATUS status, int value)
        {
            lock (cacheProgressValue)
            {
                // HACK: Only update if the percentage value changes
                if (cacheProgressValue.ContainsKey(deviceId))
                {
                    if (status != OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_PROGRESS)
                    {
                        cacheProgressValue.Remove(deviceId);        // Remove
                    }
                    else
                    {
                        if (cacheProgressValue[deviceId] == value)
                        {
                            return;                                 // Ignore same percentage value
                        }
                        else
                        {
                            cacheProgressValue[deviceId] = value;   // Update
                        }
                    }
                }
            }

            //Console.WriteLine("" + status + " - " + deviceId);
            OmDevice device = GetDevice((uint)deviceId);
            if (device == null) { return; }

            device.UpdateDownloadStatus(status, value);
        }

        public void OnChanged(OmDeviceEventArgs e)
        {
            if (e.DownloadStatus == OmApi.OM_DOWNLOAD_STATUS.OM_DOWNLOAD_COMPLETE)
            {
                e.Device.FinishedDownloading();
            }

            if (DeviceChanged != null)
            {
                DeviceChanged(this, e);
            }
        }


        public delegate void OmDeviceEventHandler(object sender, OmDeviceEventArgs e);
        public event OmDeviceEventHandler DeviceAttached;
        public event OmDeviceEventHandler DeviceRemoved;
        public event OmDeviceEventHandler DeviceChanged;

    }


    public class OmDeviceEventArgs : EventArgs
    {
        public OmDevice Device { get; protected set; }
        public OmApi.OM_DOWNLOAD_STATUS DownloadStatus { get; protected set; }
        public OmDeviceEventArgs(OmDevice device)
        {
            Device = device;
        }
        public OmDeviceEventArgs(OmDevice device, OmApi.OM_DOWNLOAD_STATUS downloadStatus) : this(device)
        {
            DownloadStatus = downloadStatus;
        }
    }

}

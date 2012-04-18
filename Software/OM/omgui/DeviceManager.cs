using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using OmApiNet;

namespace OmGui
{
    public class DeviceEventArgs : EventArgs
    {
        public OmDevice Device { get; protected set; }
        public OmDeviceEventArgs OmEvent { get; protected set; }
        public DeviceEventArgs(OmDevice device, OmDeviceEventArgs omEvent)
        {
            Device = device;
            OmEvent = omEvent;
        }
    }


    public class DeviceManager
    {
        private Om om;

        public delegate void DeviceEventHandler(object sender, DeviceEventArgs e);
        public event DeviceEventHandler DeviceUpdate;

        //private IDictionary<ushort, OmDevice> devices = new Dictionary<ushort, OmDevice>();

        public DeviceManager(Om om)
        {
            this.om = om;

            // Device event handlers
            om.DeviceAttached += om_DeviceAttached;
            om.DeviceRemoved += om_DeviceRemoved;
            om.DeviceChanged += om_DeviceChanged;
        }

        public void FindInitiallyAttached()
        {
            // Initially-attached devices
            om.ForAllAttachedDevices(om_DeviceAttached);
        }

        void OnDeviceUpdate(OmDeviceEventArgs e)
        {
            OmDevice device = e.Device;
            /*
            if (!devices.ContainsKey(e.Device.DeviceId))
            {
                devices[e.Device.DeviceId] = new Device(e.Device);
            }
            Device device = devices[e.Device.DeviceId];
            */
            if (DeviceUpdate != null)
            { 
                DeviceUpdate(this, new DeviceEventArgs(device, e)); 
            }
        }

        void om_DeviceChanged(object sender, OmDeviceEventArgs e)
        {
            //Console.WriteLine("CHANGED: " + e.Device.DeviceId);
            OnDeviceUpdate(e);
        }

        void om_DeviceAttached(object sender, OmDeviceEventArgs e)
        {
            Console.WriteLine("ATTACHED: " + e.Device.DeviceId);
            OnDeviceUpdate(e);
        }

        void om_DeviceRemoved(object sender, OmDeviceEventArgs e)
        {
            Console.WriteLine("REMOVED: " + e.Device.DeviceId);
            OnDeviceUpdate(e);
        }

    }
}

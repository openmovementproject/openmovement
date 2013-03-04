using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TeddiGui
{
    public class Manager
    {
        private IDictionary<int, Device> devices = new Dictionary<int, Device>();
        public IDictionary<int, Device> Devices { get { return devices; } }

        public Manager()
        {
        }

        public void PacketReceived(Packet packet)
        {
            Device device = null;
            if (devices.ContainsKey(packet.DeviceId))
            {
                device = devices[packet.DeviceId];
            }
            else
            {
                Console.WriteLine("NOTICE: New sensor: " + packet.DeviceId + "");
                device = new Device(packet.DeviceId, true);
                devices.Add(packet.DeviceId, device);
            }

            device.ParentAddress = packet.ParentAddress;

            device.AddSamples(packet.Samples);
        }


        public void StatusPacketReceived(StatusPacket statusPacket)
        {
            Console.WriteLine("ROUTER: #" + statusPacket.deviceId + " (" + statusPacket.shortAddress + " = #" + Device.RouterAltId(statusPacket.shortAddress) + "), parent " + statusPacket.parentAddress + " (" + statusPacket.parentAltAddress + " = #" + Device.RouterAltId(statusPacket.parentAltAddress) + ").");
            Console.Write("NEIGHBOURS of #" + Device.RouterAltId(statusPacket.shortAddress) + ": ");
            for (int i = 0; i < statusPacket.neighbours.Length; i++)
            {
                Console.Write("#" + statusPacket.neighbours[i] + "; ");
            }
            Console.WriteLine("");

            Device device = null;
            if (devices.ContainsKey(statusPacket.deviceId))
            {
                device = devices[statusPacket.deviceId];
            }
            else
            {
                Console.WriteLine("NOTICE: New router: " + statusPacket.deviceId + "");
                device = new Device(statusPacket.deviceId, false);
                devices.Add(statusPacket.deviceId, device);
            }
            device.AddStatus(statusPacket);

//throw new NotImplementedException();
        }


    }
}

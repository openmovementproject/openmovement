using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using OmApiNet;

namespace omapinet_examples
{
    public partial class TestForm : Form
    {
        public TestForm()
        {
            InitializeComponent();

            // Redirect Console.Write to GUI log
            (new TextBoxStreamWriter(textBoxLog)).SetConsoleOut();
        }

        private void TestForm_Load(object sender, EventArgs e)
        {

        }

        private void buttonRun_Click(object sender, EventArgs e)
        {
            int ret;

            // Start up the API
            ret = OmApi.OmStartup(OmApi.OM_VERSION);
            Console.WriteLine("OmStartup() = {0}", ret);

            // Get number of devices
            int[] devices = new int[0];
            ret = OmApi.OmGetDeviceIds(devices, 0);
            Console.WriteLine("OmGetDeviceIds() = {0}", ret);

            // Get device IDs
            devices = new int[ret];
            ret = OmApi.OmGetDeviceIds(devices, devices.Length);
            Console.WriteLine("OmGetDeviceIds() = {0}", ret);
            if (ret > devices.Length) { ret = devices.Length; } // ignore any devices added since last call
            for (int i = 0; i < ret; i++)
            {
                Console.WriteLine("Device {0} = #{1}", i + 1, devices[i]);
            }

            // Shutdown the API
            ret = OmApi.OmShutdown();
            Console.WriteLine("OmShutdown() = {0}", ret);
        }
    }
}

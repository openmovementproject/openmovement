using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Management;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace Wax9Gui
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            // Logging
            (new TextBoxStreamWriter(textBoxLog)).SetConsoleOut();
            //Trace.Listeners.Add(new ConsoleTraceListener());
            Console.WriteLine("Started.");

            // Command templates
            List<string> commands = new List<string>();
            commands.Add(@"waxrec.exe %PORT% -log -tee -out log_%LABEL%-%YEAR%-%MONTH%-%DAY%-%HOUR%-%MIN%-%SEC%.csv -init ""\r\nRATE M 1 80\r\nRATE X 1 100\r\nDATAMODE 1\r\nSTREAM\r\n""");
            commands.Add(@"IMU-demo\demo.exe -in %PORT% -init ""\r\nRATE M 1 80\r\nRATE X 1 100\r\nDATAMODE 1\r\nSTREAM\r\n""");
            comboBoxTemplate.Items.AddRange(commands.ToArray());

            comboBoxTemplate.SelectedIndex = 0;

            timerUpdate.Enabled = true;
        }

        private void UpdatePorts()
        {
            string existingPort = null;

            Application.UseWaitCursor = true;
            this.UseWaitCursor = true;
            this.Cursor = Cursors.WaitCursor;
            this.Enabled = false;
            devicesChanged = false;
            for (int i = 0; i < 50; i++) { Application.DoEvents(); }

            // Sort ports by numerical component
            String[] ports = GetPortList();  // System.IO.Ports.SerialPort.GetPortNames();

            // Fix any stray characters on some machines returned by System.IO.Ports.SerialPort.GetPortNames()
            for (int i = 0; i < ports.Length; i++)
            {
                string port = ports[i];
                if (port.StartsWith("COM", StringComparison.InvariantCultureIgnoreCase) && port.Length > 4) // "COM1#"
                {
                    for (int j = 3; j < port.Length; j++)
                    {
                        // If not a numeric character
                        if (!char.IsDigit(port[j]))
                        {
                            // If it's not the last character, leave it as it is
                            if (j < port.Length - 1) { break; }
                            // If it is the last character, trim it off
                            //Console.Out.WriteLine("NOTICE: Trimming invalid port name: " + port);
                            ports[i] = port.Substring(0, port.Length - 1);
                            break;
                        }
                    }
                }
            }

            int[] portValues = new int[ports.Length];
            for (int i = 0; i < portValues.Length; i++)
            {
                string p = ports[i];
                int value = 0;
                for (int j = 0; j < p.Length; j++)
                {
                    if (char.IsDigit(p[j])) { value = value * 10 + (int)(p[j] - '0'); }
                }
                if (value == 0) { value = int.MaxValue; }       // Don't favour non-numeric ports
                if (value == 1) { value = int.MaxValue - 2; }   // Don't favour COM1
                if (value == 2) { value = int.MaxValue - 1; }   // Don't favour COM2
                portValues[i] = value;
            }
            Array.Sort(portValues, ports);

            // Update port list
            if (existingPort != null)
            {
                existingPort = comboBoxPorts.Text;
            }
            comboBoxPorts.Items.Clear();
            comboBoxPorts.Items.AddRange(ports);

            // Use first item if no port specified
            if ((existingPort == null || existingPort.Length == 0) && ports.Length > 0)
            {
                existingPort = ports[0];
                // Prefer the last found device with an address
                for (int i = 0; i < ports.Length; i++)
                {
                    if (ports[i].IndexOf('<') >= 0) { existingPort = ports[i]; }
                }
            }

            // Check port exists
            for (int i = 0; i < ports.Length; i++)
            {
                if (ports[i].Equals(existingPort, StringComparison.OrdinalIgnoreCase))
                {
                    comboBoxPorts.SelectedIndex = i;
                }
            }

            this.Enabled = true;
            this.UseWaitCursor = false;
            Application.UseWaitCursor = false;
            this.Cursor = Cursors.Default;
            //Application.DoEvents();
        }

        private bool devicesChanged = false;

        protected override void WndProc(ref Message m)
        {
            // Definitions from dbt.h and winuser.h
            const int WM_DEVICECHANGE = 0x0219;
            //const int DBT_DEVICEARRIVAL = 0x8000;

            if (m.Msg == WM_DEVICECHANGE)   // && m.WParam.ToInt32() == DBT_DEVICEARRIVAL
            {
                //int devType = Marshal.ReadInt32(m.LParam, 4);
                //if (devType == DBT_DEVTYP_...

                // Without the invoke it would call the DisconnectedContext MDA / RPC_E_WRONG_THREAD
                //UpdatePorts();
                //MethodInvoker updatePorts = new MethodInvoker(UpdatePorts);
                //this.BeginInvoke(updatePorts); //IAsyncResult result = updatePorts.BeginInvoke(null, ""); updatePorts.EndInvoke(result);
                devicesChanged = true;
            }
            base.WndProc(ref m);
        }

        private void MainForm_Load(object sender, EventArgs e)
        {

        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        public string ProcessTemplate(string input)
        {
            DateTime now = DateTime.Now;
            string port = comboBoxPorts.Text;
            string portLabel = "device";
            string fullAddress = "-";
            string address = "-";
            string label = "";

            int portSep = port.IndexOf(' ');
            if (portSep >= 0) 
            {
                portLabel = port.Substring(portSep + 1).Trim(new char[] { ' ' });
                port = port.Substring(0, portSep); 
            }

            label = port;
            if (portLabel.Length > 0)
            {
                int start = portLabel.IndexOf('<');
                int end = portLabel.IndexOf('>');
                if (start >= 0 && end > start)
                {
                    fullAddress = portLabel.Substring(start + 1, end - start - 1);
                    address = fullAddress.Replace(":", "");
                    if (address.Length > 4) { label = address.Substring(address.Length - 4, 4); }
                }
            }

            string s = input;
            // Device/port
            s = s.Replace("%PORT%", @"\\.\" + port);
            s = s.Replace("%FULL_ADDRESS%", fullAddress);
            s = s.Replace("%ADDRESS%", address);
            s = s.Replace("%LABEL%", label);
            // Time
            s = s.Replace("%YEAR%", now.ToString("yyyy"));
            s = s.Replace("%MONTH%", now.ToString("MM"));
            s = s.Replace("%DAY%", now.ToString("dd"));
            s = s.Replace("%HOUR%", now.ToString("HH"));
            s = s.Replace("%MIN%", now.ToString("mm"));
            s = s.Replace("%SEC%", now.ToString("ss"));
            return s;
        }


        public void Update(object sender, EventArgs e)
        {
            textBoxCommand.Text = ProcessTemplate(comboBoxTemplate.Text);
            buttonRun.Enabled = textBoxCommand.Text.Length > 0;
        }

        public void Run(string command)
        {
            string args = "";
            int split = -1;

            if (command.Length > 0 && command[0] == '\"')
            {
                split = command.IndexOf('\"', 1);
            }
            else
            {
                split = command.IndexOf(' ');
            }

            if (split >= 0)
            {
                args = command.Substring(split + 1).TrimStart();
                command = command.Substring(0, split);
            }

            Process process = new Process();
            try
            {
                // Change to executable's directory
                string path = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
                if (path.StartsWith(@"file:\")) { path = path.Substring(6); }
                path = System.IO.Path.GetDirectoryName(System.IO.Path.Combine(path, command));

                Console.WriteLine("Path: " + path);
                Console.WriteLine("Command: " + command);
                Console.WriteLine("Args: " + args);

                System.IO.Directory.SetCurrentDirectory(path);
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.FileName = command;
                process.StartInfo.Arguments = args;
                //process.StartInfo.CreateNoWindow = true;
                if (process.Start())
                {
                    Console.WriteLine("Started: " + command);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void buttonRun_Click(object sender, EventArgs e)
        {
            Run(textBoxCommand.Text);
        }



        // Does the async version of the enumeration in a synchronous way (apparently the usual sync version blocks things weirdly)
        public void EnumerateInstances(string queryString, ObjectReadyEventHandler myObserver)
        {
            AutoResetEvent finished = new AutoResetEvent(false);

            ManagementObjectSearcher searcher = new ManagementObjectSearcher("root\\CIMV2", queryString);
            ManagementOperationObserver observer = new ManagementOperationObserver();
            observer.ObjectReady += myObserver;
            observer.Completed += (sender, obj) => // CompletedEventHandler(object sender, CompletedEventArgs obj)
            {
                finished.Set();
            };
            searcher.Get(observer);      // Asynchronous
            finished.WaitOne();
            return;
        }

        

        public IDictionary<string, string> GetPorts()
        {
            // DeviceID PNPDeviceID
            // COM9     BTHENUM\{00001101-0000-1000-8000-00805F9B34FB}_LOCALMFG&0000\8&4E34AC5&0&000000000000_00000006
            // COM11    BTHENUM\{00001101-0000-1000-8000-00805F9B34FB}_LOCALMFG&0000\8&4E34AC5&0&000000000000_00000011
            // COM10    BTHENUM\{00001101-0000-1000-8000-00805F9B34FB}_LOCALMFG&000F\8&4E34AC5&0&0017EC34AB99_C00000000
            // COM15    BTHENUM\{00001101-0000-1000-8000-00805F9B34FB}_LOCALMFG&0000\8&4E34AC5&0&000000000000_00000012
            // COM16    BTHENUM\{00001101-0000-1000-8000-00805F9B34FB}_LOCALMFG&000F\8&4E34AC5&0&0017EC34A94E_C00000000
            Console.WriteLine("Updating devices...");

            IDictionary <string, string> results = new Dictionary<string, string>();

            string queryString;
            //queryString = "SELECT * FROM WIN32_SerialPort";
            queryString = "SELECT * FROM Win32_PnPEntity WHERE ClassGuid=\"{4d36e978-e325-11ce-bfc1-08002be10318}\"";

            List<object> values = new List<object>();
            EnumerateInstances(queryString, (sender, obj) => // ObjectReadyEventHandler(object sender, ObjectReadyEventArgs obj) 
            {
                try
                {
                    ManagementObject port = (ManagementObject)obj.NewObject;

                    // DeviceId is "BTHENUM\{00001101-0000-1000-8000-00805F9B34FB}_LOCALMFG&0002\7&4BFCAD4&0&C83E990CF8D8_C00000000"
                    // Port number is "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\" + DeviceId + "\Device Parameters\PortName"
                    // Device Name is in HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\BTHENUM\Dev_$MAC\*\FriendlyName

                    /*
                    Console.WriteLine("---");
                    foreach (System.Management.PropertyData Property in port.Properties)
                    {
                        Console.WriteLine(Property.Name + " " + (Property.Value == null ? null : Property.Value.ToString()));
                        if (Property.Value != null && Property.Value is string[])
                        {
                            string[] strings = (string[])Property.Value;
                            foreach (string s in strings)
                            {
                                Console.WriteLine("..." + s);
                            }
                        }
                    }
                    */

                    // Find Port
                    string deviceId = port.Properties["DeviceID"].Value.ToString();
                    string portName = null;
                    try
                    {
                        RegistryKey deviceParameters = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Enum\" + deviceId + @"\Device Parameters");
                        portName = deviceParameters.GetValue("PortName").ToString();
                    }
                    catch (Exception)
                    {
                        Console.WriteLine("ERROR: Exception determining port name for " + deviceId);
                        return;
                    }

                    // Find Name
                    string label = null;
                    if (port.Properties["Name"] != null)
                    {
                        System.Management.PropertyData nameProperty = port.Properties["Name"];
                        label = "\"" + nameProperty.Value.ToString() + "\"";
                    }

                    // Find Bluetooth MAC Address
                    string mac = null;
                    if (deviceId.StartsWith(@"BTHENUM\"))
                    {
                        string[] slashParts = deviceId.Split(new char[] { '\\' });
                        if (slashParts.Length > 2)
                        {
                            string[] ampersandParts = slashParts[2].Split(new char[] { '&' });
                            for (int i = 0; i < ampersandParts.Length; i++)
                            {
                                string temp = ampersandParts[i];
                                int underscore = temp.IndexOf('_');
                                if (underscore >= 0) { temp = temp.Substring(0, underscore); }
                                //if (temp.Length > 12) { temp = temp.Substring(temp.Length - 12); }
                                if (temp.Length == 12) { mac = temp; }
                            }
                        }

                        if (mac == "000000000000") { mac = null; }
                    }

                    // Find Bluetooth device name
                    string deviceName = null;
                    if (mac != null)
                    {
                        // Device Name is in HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\BTHENUM\Dev_$MAC\*\FriendlyName
                        try
                        {
                            RegistryKey devKey = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Enum\BTHENUM\" + "Dev_" + mac);
                            // Find first sub-key
                            string[] subKeys = devKey.GetSubKeyNames();
                            if (subKeys.Length > 0)
                            {
                                // Device Name is in HKEY_LOCAL_MACHINE\  Dev_$MAC\*\FriendlyName
                                RegistryKey devKey2 = devKey.OpenSubKey(subKeys[0]);
                                deviceName = devKey2.GetValue("FriendlyName").ToString();
                            }
                        }
                        catch (Exception)
                        {
                            Console.WriteLine("WARNING: Exception determining Bluetooth device name for " + deviceId);
                        }
                    }

                    if (deviceName != null)
                    {
                        label = label + " [" + deviceName + "]";
                    }

                    //label = label + " Bluetooth";
                    if (mac != null)
                    {
                        string address = "" + mac[0] + mac[1] + ':' + mac[2] + mac[3] + ':' + mac[4] + mac[5] + ':' + mac[6] + mac[7] + ':' + mac[8] + mac[9] + ':' + mac[10] + mac[11];
                        label = label + " <" + address + ">";
                    }

                    Console.WriteLine("Port: " + portName + " - " + label);
                    results.Add(portName, label);


                }
                catch (ManagementException e)
                {
                    Console.WriteLine("Error: " + e.Message);
                }
            });


            Console.WriteLine("...done.");
            return results;
        }

        public string[] GetPortList()
        {
            IDictionary<string, string> portData = GetPorts();  // new Dictionary<string, string>();
            string[] ports = new string[portData.Count];
            int i = 0;
            foreach (KeyValuePair<string, string> kvp in portData)
            {
                ports[i++] = kvp.Key + " " + kvp.Value;
            }
            return ports;
        }

        private void buttonRefresh_Click(object sender, EventArgs e)
        {
            // Update serial ports
            buttonRefresh.Font = new Font(buttonRefresh.Font, FontStyle.Regular);
            buttonRefresh.ForeColor = SystemColors.ControlText;
            buttonRefresh.BackColor = SystemColors.Control;

            string restore = buttonRefresh.Text;
            buttonRefresh.Text = "...";
            buttonRefresh.Cursor = Cursors.WaitCursor;
            UpdatePorts();
            buttonRefresh.Cursor = Cursors.Default;
            buttonRefresh.Text = restore;
        }

        private void MainForm_Shown(object sender, EventArgs e)
        {
            buttonRefresh_Click(null, null);
        }

        private void timerUpdate_Tick(object sender, EventArgs e)
        {
            // Update the command every second (it may include the current seconds as a value)
            Update(null, null);
            if (devicesChanged)
            {
                //buttonRefresh.Font = new Font(buttonRefresh.Font, FontStyle.Italic);
                //buttonRefresh.ForeColor = Color.White;
                buttonRefresh.BackColor = Color.Pink;

                //UpdatePorts();

            }
        }

    }
}

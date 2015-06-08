using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using WaxLib;
using System.IO;
using System.IO.Ports;
using System.Threading;

namespace WaxGui
{
    public partial class WaxMainForm : Form
    {
        public const string DEFAULT_LOG_FILE = "wax-{date}-{time}.csv";

        private WaxManager waxManager;

        public bool OscReceive { get; set; }
        public bool AutoConnect { get; set; }
        public bool MinimizeToTray { get; set; }
        public string PortName 
        { 
            get { return toolStripComboBoxPort.Text; } 
            set { toolStripComboBoxPort.Text = value; } 
        }
        public string OscHost { get; set; }
        public int OscPort { get; set; }
        private string logFile;
        public string LogFile
        { 
            get 
            {
                return logFile;
            } 
            set 
            {
                logFile = value;
                saveFileDialog.InitialDirectory = logFile.Length > 0 ? Path.GetDirectoryName(logFile) : "";
                saveFileDialog.FileName = logFile;    //Path.GetFileName(value);
                // Prevent self-changing
                if (toolStripTextBoxLogFile.Text != logFile)
                {
                    toolStripTextBoxLogFile.Text = logFile;
                    toolStripTextBoxLogFile.Select(toolStripTextBoxLogFile.Text.Length, 0);
                }
            } 
        }


        public WaxMainForm()
        {
            InitializeComponent();

            PortName = null;
            OscHost = OscTransmitter.DEFAULT_RECEIVER;
            OscPort = OscTransmitter.DEFAULT_PORT;
            LogFile = DEFAULT_LOG_FILE;
        }


        // Retrieve a list of ports, ordered numerically (not favouring built-ins COM1 and COM2)
        public string[] GetPorts()
        {
            // Check if port exists
            string[] ports = SerialPort.GetPortNames();

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

            // Sort by numerical component
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

            return ports;
        }



        // Find a port name 
        public string FindPort(string requestedPort)
        {
            string[] ports = GetPorts();

            // Use first item if no port specified
            if (requestedPort == null || requestedPort.Length == 0)
            {
                if (ports.Length > 0)
                {
                    requestedPort = ports[0];
                    Console.WriteLine("Port not specified on command-line, first port auto-selected: " + requestedPort);
                    //MessageBox.Show("Port not specified on command-line, first port auto-selected: " + portName);
                }
                else
                {
                    Console.WriteLine("Port not specified on command-line and no ports found in system.");
                    //MessageBox.Show("Port not specified on command-line and no ports found in system.");
                }
            }

            // Check port exists
            bool found = false;
            foreach (string p in ports)
            {
                found |= p.Equals(requestedPort, StringComparison.OrdinalIgnoreCase);
            }

            if (!found)
            {
                Console.WriteLine("Port not found: " + requestedPort);
                //MessageBox.Show("Error: Port not found (" + ((requestedPort == null || requestedPort.Length == 0) ? "-" : requestedPort)  + ")");
            }

            return requestedPort;
        }


        private void UpdatePorts()
        {
            toolStripComboBoxPort.Items.Clear();
            toolStripComboBoxPort.Items.AddRange(GetPorts());
            if (PortName.Length == 0)
            {
                string port = FindPort(PortName);
                if (port != null)
                {
                    //[dgj] PortName = port;
                }
            }
        }


        private void WaxMainForm_Load(object sender, EventArgs e)
        {
            UpdatePorts();

            if (AutoConnect)
            {
                toolStripButtonConnect.PerformClick();
            }

            if (MinimizeToTray)
            {
                this.WindowState = FormWindowState.Minimized;
            }
        }


        private void WaxMainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            // Remove old wax manager
            if (waxManager != null)
            {
                waxManager.Dispose();
                waxManager = null;
            }
            Thread.Sleep(500);
        }

        private void notifyIcon_Click(object sender, EventArgs e)
        {
            // Restore minimized window
            this.WindowState = FormWindowState.Normal;
        }

        private void WaxMainForm_Resize(object sender, EventArgs e)
        {
            if (MinimizeToTray)
            {
                if (WindowState == FormWindowState.Minimized)
                {
                    if (!notifyIcon.Visible || ShowInTaskbar)
                    {
                        ShowInTaskbar = false;
                        notifyIcon.Visible = true;

                        notifyIcon.BalloonTipText = "Running in the background...";
                        notifyIcon.BalloonTipTitle = "WAX Receiver";
                        notifyIcon.BalloonTipIcon = ToolTipIcon.Info;
                        notifyIcon.ShowBalloonTip(500);
                    }
                }
                else
                {
                    if (notifyIcon.Visible || !ShowInTaskbar)
                    {
                        ShowInTaskbar = true;
                        notifyIcon.Visible = false;
                    }
                }
            }

        }

        private void toolStripButtonConnect_Click(object sender, EventArgs e)
        {
            // Remove old wax manager
            if (waxManager != null)
            {
                waxManager.Dispose(); 
                waxManager = null;
            }

            if (!toolStripButtonConnect.Checked)
            {
                // Find port name
                if (PortName.Length != 0)
                {
                    string port = FindPort(PortName);
                    if (port != null) { PortName = port; }
                }

                waxManager = new WaxManager();

                if (PortName.Length > 0)
                {
                    // Create WAX manager
                    try
                    {
                        string initiallySend = null;
                        if (checkBoxSetMode.Checked)
                        {
                            // Set SLIP mode... [0=NONE,1=SLIP,2=TEXT,3=OSC]
                            initiallySend = "\r\nMODE=1\r\n";
                        }
                        waxManager.AddPacketSource(new SlipSource<WaxPacket>(waxManager.WaxPacketConverter, SlipSource<WaxPacket>.OpenSerialPort(PortName, initiallySend))); 
                        if (checkBoxBroadcast.Checked)
                        {                                      
                            waxManager.AddPacketDest(new StompDest<WaxPacket>(waxManager.WaxPacketConverter, "localhost"));
                        }
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(this, "Error: " + ex.Message, "Connection Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
                else
                {
                    //MessageBox.Show(this, "Error: Port not specified.", "Connection Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    waxManager.AddPacketSource(new StompSource<WaxPacket>(waxManager.WaxPacketConverter, "localhost"));
                    if (OscReceive)
                    {
                        waxManager.AddPacketSource(new OscSource<WaxPacket>(waxManager.WaxPacketConverter, "localhost:" + OscPort + "/wax"));
                    }
                }



                if (waxManager != null)
                {
                    // Set log file
                    string file = LogFile;
                    if (file != null && file.Length > 0)
                    {
                        DateTime now = DateTime.Now;
                        string date = now.ToString(@"yyyy-MM-dd");
                        string time = now.ToString(@"HH-mm-ss");
                        file = file.Replace("{date}", date);
                        file = file.Replace("{time}", time);
                        waxManager.AddPacketDest(new LogDest<WaxPacket>(waxManager.WaxPacketConverter, new StreamWriter(new FileStream(file, FileMode.Append)))); 
                    }

                    // Set WAX OSC transmission
                    if (OscHost != null && OscHost.Length > 0 && OscPort > 0)
                    {
                        OscTransmitter tinyOscTransmitter = new OscTransmitter(OscHost, OscPort);
                        waxManager.ReceivedPacket += (s, ea) =>
                        {
                            try
                            {
                                WaxSample[] samples = ea.Packet.Samples;
                                OscMessage[] messages = new OscMessage[samples.Length];
                                for (int i = 0; i < samples.Length; i++)
                                {
                                    messages[i] = new OscMessage("/wax", (int)ea.Packet.DeviceId, samples[i].Index, samples[i].X / 256.0f, samples[i].Y / 256.0f, samples[i].Z / 256.0f);
                                }
                                OscBundle bundle = new OscBundle(OscBundle.TIMESTAMP_NOW, messages);
                                tinyOscTransmitter.Send(bundle);
                            }
                            catch (Exception ex)
                            {
                                Console.Error.WriteLine("ERROR: Problem sending OSC packet - " + ex.Message);
                            }
                        };
                    }
                }

                // Set History panel to use WAX manager
                historyPanel.WaxManager = waxManager;

                // Update connection button
                toolStripButtonConnect.Checked = (waxManager != null);
            }
            else
            {
                // Update connection button
                toolStripButtonConnect.Checked = false;
            }

            // Update tool bar status
            bool connected = toolStripButtonConnect.Checked;
            toolStripLabelPort.Enabled = !connected;
            toolStripComboBoxPort.Enabled = !connected;
            toolStripLabelFile.Enabled = !connected;
            toolStripTextBoxLogFile.Enabled = !connected;
            toolStripButtonChooseFile.Enabled = !connected;
            // ...
        }

        private void toolStripButtonChooseFile_Click(object sender, EventArgs e)
        {
            //LogFile = toolStripTextBoxLogFile.Text;
            if (saveFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                LogFile = saveFileDialog.FileName;
            }
        }



        protected override void WndProc(ref Message m)
        {
            // Definitions from dbt.h and winuser.h
            const int WM_DEVICECHANGE = 0x0219;
            //const int DBT_DEVICEARRIVAL = 0x8000;

            if (m.Msg == WM_DEVICECHANGE)   // && m.WParam.ToInt32() == DBT_DEVICEARRIVAL
            {
                //int devType = Marshal.ReadInt32(m.LParam, 4);
                //if (devType == DBT_DEVTYP_...
                UpdatePorts();
            }
            base.WndProc(ref m);
        }

        private void toolStripTextBoxLogFile_Leave(object sender, EventArgs e)
        {
            LogFile = toolStripTextBoxLogFile.Text;
        }

        private void toolStripTextBoxLogFile_TextChanged(object sender, EventArgs e)
        {
            LogFile = toolStripTextBoxLogFile.Text;
        }

        private void timer_Tick(object sender, EventArgs e)
        {
            string status = "-";
            if (waxManager != null)
            {
                status = waxManager.DebugString;
            }
            toolStripStatusLabel.Text = "> " + status;
            toolStripTextBoxDebug.Text = status;
        }

        private void checkBoxSetMode_Click(object sender, EventArgs e)
        {

        }

        private void toolStripButtonSlow_Click(object sender, EventArgs e)
        {
            historyPanel.autoInvalidate = !toolStripButtonSlow.Checked;
        }

        private void toolStripButtonAlt_Click(object sender, EventArgs e)
        {
            historyPanel.scaleMode = toolStripButtonAlt.Checked ? 2 : 1;
        }


    }
}

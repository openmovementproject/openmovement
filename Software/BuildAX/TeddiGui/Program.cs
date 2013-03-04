using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Security.Permissions;
using System.IO;
using System.Threading;

namespace TeddiGui
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        [SecurityPermission(SecurityAction.Demand, Flags = SecurityPermissionFlag.ControlAppDomain)]
        static void Main(string[] args)
        {
            try
            {
                // Exception handling
                Application.ThreadException += Application_ThreadException;                         // UI thread exceptions
                Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);       // Force UI exceptions through our handler
                AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;     // non-UI thread exceptions

                // Run the application
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);

                // Command-line options
                bool autoConnect = false;
                string portName = null;
                string logFile = Path.GetDirectoryName(Application.ExecutablePath) + Path.DirectorySeparatorChar + "log.txt";
                string mapFile = "Data/CultureLab.csv";
                string stompReceiver = null;
                string oscTransmitter = null;
                bool oscCooked = false;
                int listenPort = -1;
                IList<MotionTracker> motionRun = new List<MotionTracker>();
                
                int positional = 0;
                for (int i = 0; i < args.Length; i++)
                {
                    if (args[i].ToLower() == "-connect") { autoConnect = true; }
                    else if (args[i].ToLower() == "-port") { portName = args[++i]; }
                    else if (args[i].ToLower() == "-log") { logFile = args[++i]; }
                    else if (args[i].ToLower() == "-map") { mapFile = args[++i]; }
                    else if (args[i].ToLower() == "-udp") { listenPort = int.Parse(args[++i]); }
                    else if (args[i].ToLower() == "-stomp") { stompReceiver = args[++i]; }
                    else if (args[i].ToLower() == "-oscraw") { oscTransmitter = args[++i]; oscCooked = false; }
                    else if (args[i].ToLower() == "-osccooked") { oscTransmitter = args[++i]; oscCooked = true; }
                    else if (args[i].ToLower() == "-motionrun") { int id = int.Parse(args[++i]); string action = args[++i]; motionRun.Add(new MotionTracker(id, action));  }
                    else if (args[i][0] == '-')
                    {
                        Console.Error.WriteLine("ERROR: Ignoring unknown option: " + args[i]);
                    }
                    else
                    {
                        //if (positional == 0) { logFile = args[i]; }
                        //else
                        {
                            Console.Error.WriteLine("ERROR: Ignoring positional parameter #" + (positional + 1) + ": " + args[i]);
                        }
                        positional++;
                    }
                }

                MainForm mainForm = new MainForm();
                mainForm.AutoConnect = autoConnect;
                mainForm.PortName = portName;
                mainForm.LogFile = logFile;
                mainForm.LoadFromFile(mapFile);
                if (listenPort != -1) { mainForm.StartListening(listenPort); }
                if (stompReceiver != null) { mainForm.StartStompReceiver(stompReceiver); }
                if (oscTransmitter != null) { mainForm.StartOscTransmitter(oscTransmitter, oscCooked); }
                mainForm.AddMotionRun(motionRun);

                Application.Run(mainForm);
            }
            catch (Exception ex)
            {
                string error =
                    "Sorry, a fatal application error occurred (exception in main function).\r\n\r\n" +
                    "Exception: " + ex.ToString() + "\r\n\r\n" +
                    "Stack trace: " + ex.StackTrace + "";
                MessageBox.Show(error, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
                Environment.Exit(-1);
            }
        }



        // Unhandled UI exceptions (can ignore and resume)
        private static void Application_ThreadException(object sender, ThreadExceptionEventArgs t)
        {
            DialogResult result = DialogResult.Abort;
            try
            {
                Exception ex = (Exception)t.Exception;
                string error =
                    "Sorry, an application error occurred (unhandled UI exception).\r\n\r\n" +
                    "Exception: " + ex.ToString() + "\r\n\r\n" +
                    "Stack trace: " + ex.StackTrace + "";
                result = MessageBox.Show(error, Application.ProductName, MessageBoxButtons.AbortRetryIgnore, MessageBoxIcon.Error);
            }
            catch { ; }
            finally
            {
                if (result == DialogResult.Abort) { Application.Exit(); }
            }
        }

        // Un-handled non-UI exceptions (cannot prevent the application from terminating)
        private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            try
            {
                Exception ex = (Exception)e.ExceptionObject;
                string error =
                    "Sorry, a fatal application error occurred (un-handled non-UI exception).\r\n\r\n" +
                    "Exception: " + ex.ToString() + "\r\n\r\n" +
                    "Stack trace: " + ex.StackTrace + "";
                MessageBox.Show(error, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            catch { ; }
            finally
            {
                Environment.Exit(-1);       // Not Application.Exit, this will prevent the Windows error message
            }
        }



    }
}

using System;
using System.Windows.Forms;
using System.Security.Permissions;
using System.Threading;
using System.IO;

namespace OMTesting
{
    public static class Program
    {
        public enum Mode
        {
            Setup,
            Recharge,
            RechargeClear,
        }

        static private bool logUiExceptionsOnly = false;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        [SecurityPermission(SecurityAction.Demand, Flags = SecurityPermissionFlag.ControlAppDomain)]
        static void Main(string[] args)
        {
            bool mutexCreated = false;
            System.Threading.Mutex mutex = new System.Threading.Mutex(true, @"Local\OMTesting", out mutexCreated);
            if (!mutexCreated)
            {
                MessageBox.Show("Another instance is already running.\r\nIt is only safe to run one at once.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try
            {
                // Default to only logging UI exceptions
                logUiExceptionsOnly = true;

                // Exception handling
                Application.ThreadException += Application_ThreadException;                         // UI thread exceptions
                Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);       // Force UI exceptions through our handler
                AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;     // non-UI thread exceptions

                // Run the application
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);

                // Command-line options
                string logFile = Path.GetDirectoryName(Application.ExecutablePath) + Path.DirectorySeparatorChar + "log.txt";
                bool autoAdd = false;
                Mode programMode = Mode.Setup;
                int minBattery = 85;
                int startDays = 1;
                int startHour = 0;
                int durationDays = 8;
                int endHour = 0;
                int debugMode = 0;

                int positional = 0;
                string loadFile = null;
                for (int i = 0; i < args.Length; i++)
                {
                    if (args[i].ToLower() == "-log") { logFile = args[++i]; }
                    else if (args[i].ToLower() == "-mode:setup") { programMode = Mode.Setup; }
                    else if (args[i].ToLower() == "-mode:recharge") { programMode = Mode.Recharge; }
                    else if (args[i].ToLower() == "-mode:rechargeclear") { programMode = Mode.RechargeClear; }
                    else if (args[i].ToLower() == "-auto") { autoAdd = true; }
                    else if (args[i].ToLower() == "-minbattery") { minBattery = int.Parse(args[++i]); }
                    else if (args[i].ToLower() == "-startdays") { startDays = int.Parse(args[++i]); }
                    else if (args[i].ToLower() == "-starthour") { startHour = int.Parse(args[++i]); }
                    else if (args[i].ToLower() == "-durationdays") { durationDays = int.Parse(args[++i]); }
                    else if (args[i].ToLower() == "-endhour") { endHour = int.Parse(args[++i]); }
                    else if (args[i].ToLower() == "-debugmode") { debugMode = int.Parse(args[++i]); }
                    else if (args[i][0] == '-' || args[i][0] == '/')
                    {
                        string error = "ERROR: Ignoring unknown option: " + args[i] + "\r\n(-minbattery / -startdays / -starthour / -durationdays / -endhour / -debugmode)";
                        Console.Error.WriteLine(error);
                        MessageBox.Show(null, error, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else
                    {
                        if (positional == 0) { loadFile = args[i]; }
                        else
                        {
                            string error = "ERROR: Ignoring positional parameter #" + (positional + 1) + ": " + args[i];
                            Console.Error.WriteLine(error);
                            MessageBox.Show(null, error, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                        positional++;
                    }
                }

                MainForm mainForm = new MainForm(loadFile, programMode, autoAdd, minBattery, startDays, startHour, durationDays, endHour, debugMode);
                //mainForm.LogFile = logFile;

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
            finally
            {
                mutex.Close();
            }
        }



        // Unhandled UI exceptions (can ignore and resume)
        private static void Application_ThreadException(object sender, ThreadExceptionEventArgs t)
        {
            DialogResult result = DialogResult.Abort;
            try
            {
                if (logUiExceptionsOnly)
                {
                    Console.WriteLine("EXCEPTION: " + ((Exception)t.Exception).ToString() + "");
                    result = DialogResult.Ignore;
                }
                else
                {
                    Exception ex = (Exception)t.Exception;
                    string error =
                        "Sorry, an application error occurred (unhandled UI exception).\r\n\r\n" +
                        "Exception: " + ex.ToString() + "\r\n\r\n" +
                        "Stack trace: " + ex.StackTrace + "";
                    result = MessageBox.Show(error, Application.ProductName, MessageBoxButtons.AbortRetryIgnore, MessageBoxIcon.Error);
                }
            }
            catch { ; }
            finally
            {
                if (result == DialogResult.Abort) { Application.Exit(); }
            }
        }

        // Unhandled non-UI exceptions (cannot prevent the application from terminating)
        private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            try
            {
                Exception ex = (Exception)e.ExceptionObject;
                string error =
                    "Sorry, a fatal application error occurred (unhandled non-UI exception).\r\n\r\n" +
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

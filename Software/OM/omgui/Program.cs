using System;
using System.Windows.Forms;
using System.Security.Permissions;
using System.Threading;
using System.IO;

namespace OmGui
{
    static class Program
    {
        static private bool logUiExceptionsOnly = false;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        [SecurityPermission(SecurityAction.Demand, Flags = SecurityPermissionFlag.ControlAppDomain)]
        static void Main(string[] args)
        {
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
                string configDumpFile = null; // Path.GetDirectoryName(Application.ExecutablePath) + Path.DirectorySeparatorChar + "dump.csv";

                int positional = 0;
                int uac = 0;
                for (int i = 0; i < args.Length; i++)
                {
                    if (args[i].ToLower() == "-log") { logFile = args[++i]; }
                    else if (args[i].ToLower() == "-dump") { configDumpFile = args[++i]; }
                    else if (args[i].ToLower() == "-uac:none") { uac = 0; }
                    else if (args[i].ToLower() == "-uac:ask")     { uac = 1; }
                    else if (args[i].ToLower() == "-uac:always")  { uac = 2; }
                    else if (args[i].ToLower() == "-uac:require") { uac = 3; }
                    else if (args[i][0] == '-' || args[i][0] == '/')
                    {
                        string error = "ERROR: Ignoring unknown option: " + args[i];
                        Console.Error.WriteLine(error);
                        MessageBox.Show(null, error, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else
                    {
                        //if (positional == 0) { loadFile = args[i]; }
                        //else
                        {
                            string error = "ERROR: Ignoring positional parameter #" + (positional + 1) + ": " + args[i];
                            Console.Error.WriteLine(error);
                            MessageBox.Show(null, error, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                        positional++;
                    }
                }

                MainForm mainForm = new MainForm(uac, configDumpFile);
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

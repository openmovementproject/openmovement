// Configure Wrapper
// Dan Jackson, 2012

/*
    // *** EXAMPLE ****
 
    // Create the command-line-interface wrapper
    Configure configure = new Configure(@"config.cmd");

    // Handler for completion (optional if not using asynchronous conversion)
    configure.Completed += (s,e) =>
    {
        if (!e.Result)
        {
            Console.WriteLine("Error: " + e.Error);
        }
        else
        {
            Console.WriteLine("Completed.");
        }
    };

    // Begin background configure
    configure.RunAsync(int deviceId);
*/


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace OMTesting
{
    public class Configure : IDisposable
    {
        // For clean up if destroyed
        private IList<Process> processList = new List<Process>();

        // Raw properties (should not need to access)
        public string Executable { get; protected set; }
        public int ExitCode { get; protected set; }
        public IDictionary<string, string> Results { get; protected set; }
        public uint Id { get; protected set; }
        public string Args { get; protected set; }
        //public string OutputFile { get; protected set; }

        // Status
        public bool IsBusy { get; protected set; }
        public bool Done { get; protected set; }
        public Exception Error { get; protected set; }
        public string ErrorMessages { get; protected set; }

        // Completed Event
        public class CompletedEventArgs : EventArgs
        {
            public CompletedEventArgs(Exception error, bool result, int exitCode, string errorMessages)
            {
                Error = error;
                Result = result;
                ExitCode = exitCode;
                ErrorMessages = errorMessages;
            }
            public Exception Error { get; protected set; }
            public bool Result { get; protected set; }
            public int ExitCode { get; protected set; }
            public string ErrorMessages { get; protected set; }
        }
        public delegate void CompletedEventHandler(object sender, CompletedEventArgs e);
        public event CompletedEventHandler Completed;


        public Configure(string executable, string args)
        {
            Id = 0;
            Args = args;
            //OutputFile = "";
            Executable = executable;
            ExitCode = int.MinValue;
            Results = new Dictionary<string, string>();
        }

        ~Configure()
        {
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        public void Cancel()
        {
            foreach (Process process in processList)
            {
                try
                {
                    if (!process.HasExited)
                    {
                        process.Kill();
                    }
                }
                catch { ; }
            }
            processList.Clear();
        }

        protected virtual void Dispose(bool disposing)
        {
            Cancel();
        }

        private void ParseLine(string line)
        {
            if (line.StartsWith("ERROR:"))
            {
                if (ErrorMessages == null) { ErrorMessages = ""; }
                ErrorMessages = ErrorMessages + line.Substring(6).Trim() + " | ";
            }
            return;
        }

        public bool ConfigureSync(uint id, int startDays, int startHour, int durationDays, int endHour)
        {
            ErrorMessages = null;
            try
            {
                Id = id;
                ExitCode = int.MinValue;
                Results.Clear();
                IsBusy = true;
                Done = false;
                Error = null;

                // Check if the executable exists
                if (!File.Exists(Executable))
                {
                    IsBusy = false;
                    Console.WriteLine("Conversion executable not found #" + id);
                    Exception ex = new Exception("Conversion executable not found: " + Executable);
                    Error = ex;
                    throw ex;
                }

                // Sort out args
                string args = Args;
                args = args.Replace("$id", Id.ToString());
                args = args.Replace("$startdays", startDays.ToString());
                args = args.Replace("$starthour", startHour.ToString());
                args = args.Replace("$durationdays", durationDays.ToString());
                args = args.Replace("$endhour", endHour.ToString());

                // Create the process structure
                ProcessStartInfo processInformation = new ProcessStartInfo();
                processInformation.FileName = Executable;
                processInformation.Arguments = args;
                processInformation.UseShellExecute = false;
                processInformation.WorkingDirectory = Directory.GetCurrentDirectory();
                processInformation.CreateNoWindow = true;
                processInformation.RedirectStandardError = true;
                processInformation.RedirectStandardOutput = true;

                // Create process
                Process process = new Process();
                process.EnableRaisingEvents = true;
                process.StartInfo = processInformation;

                // For clean-up if we're disposed
                processList.Add(process);
                process.Exited += (sender, e) => processList.Remove(process);

                // Start process
                try
                {
                    Console.WriteLine("PROCESS: " + Executable + " " + args);
                    process.Start();
                }
                catch (Exception e)
                {
                    IsBusy = false;
                    Exception ex = new Exception("Process exception.", e);
                    Error = ex;
                    throw ex;
                }

                // Parse output
                StringBuilder sb = new StringBuilder();

                if (processInformation.RedirectStandardError)
                {
                    while (!process.StandardError.EndOfStream) //  && !process.HasExited)
                    {
                        string line;
                        line = process.StandardError.ReadLine(); //blocking on this call with new version
                        if (line == null) { break; }
                        Console.WriteLine("[E] " + line);
                        ParseLine(line);
                    }
                }

                if (processInformation.RedirectStandardOutput)
                {
                    while (!process.StandardOutput.EndOfStream) //  && !process.HasExited)
                    {
                        string line;
                        line = process.StandardOutput.ReadLine(); //blocking on this call with new version
                        if (line == null) { break; }
                        Console.WriteLine("[O] " + line);
                        ParseLine(line);
                    }
                }

                // Check exit code
                process.WaitForExit();
                Results["_exit"] = process.ExitCode.ToString();

                if (Results.ContainsKey("_exit")) { int e = ExitCode; int.TryParse(Results["_exit"], out e); ExitCode = e; }

                Done = (ExitCode == 0);
                IsBusy = false;

                if (Completed != null)
                {
                    Completed.Invoke(this, new CompletedEventArgs(null, Done, ExitCode, ErrorMessages));
                }

                return Done;
            }
            catch (Exception e)
            {
                if (Completed != null)
                {
                    Error = e;
                    Completed.Invoke(this, new CompletedEventArgs(e, false, 0, ErrorMessages));
                }
                throw e;
            }
        }


        public void ConfigureAsync(uint id, int startDays, int startHour, int durationDays, int endHour)
        {
            Thread thread = new Thread(() => {
                try
                {
                    ConfigureSync(id, startDays, startHour, durationDays, endHour);
                }
                catch { ; }     // Swallow any re-thrown (ConvertSync will have called 'Completed' call-back with exception)
            });
            thread.Start();
        }


    }
}

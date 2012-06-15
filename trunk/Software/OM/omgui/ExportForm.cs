using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;

namespace OmGui
{
    public partial class ExportForm : Form
    {
        // Conversion Executable
        public const string EXECUTABLE_NAME = @"cwa-convert.exe";

        public ExportForm(string inputFilename, string downloadPath)
        {
            InitializeComponent();

            //saveFileDialog.InitialDirectory = downloadPath;

            textBoxSourceFile.Text = inputFilename;
            textBoxOutputFile.Text = Path.Combine(downloadPath, Path.ChangeExtension(Path.GetFileName(inputFilename), saveFileDialog.DefaultExt));
        }

        private void ExportForm_Load(object sender, EventArgs e)
        {
            if (Browse(Path.GetFileName(textBoxOutputFile.Text)) != DialogResult.OK) { this.Close(); }
        }

        private DialogResult Browse(string filename)
        {
            // Output file
            saveFileDialog.FileName = filename;
            DialogResult result = saveFileDialog.ShowDialog();
            if (result != DialogResult.OK || saveFileDialog.FileName.Trim().Length <= 0) { return result; }
            textBoxOutputFile.Text = saveFileDialog.FileName;
            return result;
        }

        private void buttonBrowse_Click(object sender, EventArgs e)
        {
            Browse(textBoxOutputFile.Text);
        }

        private void buttonConvert_Click(object sender, EventArgs e)
        {
            // Executable file
            string executableFile = Path.Combine(Application.StartupPath, EXECUTABLE_NAME);
            if (!File.Exists(executableFile))
            {
                MessageBox.Show(this, "This process requires the external executable " + EXECUTABLE_NAME + " for conversion to CSV.\r\n\r\nThe file was not found at:\r\n\r\n" + executableFile + "\r\n\r\nPlease locate the executable there and try again.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                return;
            }

            // Input file
            string inputFilename = textBoxSourceFile.Text;
            if (inputFilename == null || !File.Exists(inputFilename))
            {
                MessageBox.Show(this, "Cannot find the source file: \r\n\r\n" + inputFilename + "\r\n\r\nPlease ensure the file exists and try again.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                return;
            }

            // Convert the file
            ProcessStartInfo processInformation = new ProcessStartInfo();
            processInformation.FileName = executableFile;

            // <filename.cwa> [-s:accel|-s:gyro] [-v:float|-v:int] [-t:timestamp|-t:none|-t:sequence|-t:secs|-t:days|-t:serial|-t:excel] [-start 0] [-length <len>] [-step 1] [-out <outfile>]

            string args = "\"{infile}\" -f:csv -out \"{outfile}\"";
            args = args.Replace("{infile}", textBoxSourceFile.Text);
            args = args.Replace("{outfile}", textBoxOutputFile.Text);
            processInformation.Arguments = args;
            processInformation.UseShellExecute = false;
            //processInformation.CreateNoWindow = true;
            //processInformation.RedirectStandardError = true;
            //processInformation.RedirectStandardOutput = true;

            Process conversionProcess = new Process();
            conversionProcess.EnableRaisingEvents = true;
            conversionProcess.StartInfo = processInformation;

            try
            {
                conversionProcess.Start();
            }
            catch (Exception ex)
            {
                Trace.WriteLine("ERROR: Problem running conversion process: " + ex.Message);
            }

            this.Close();

            /*
            char[] buffer = new char[4];
            while (!conversionProcess.StandardError.EndOfStream)
            {
                int offset = 0;
                int length = conversionProcess.StandardError.ReadBlock(buffer, offset, buffer.Length);
                if (length > 0)
                {
                    string st = new string(buffer, offset, length);
                    Console.Out.Write(st);
                }
            }
            conversionProcess.WaitForExit();

            string transcription = null;
            int exitCode = conversionProcess.ExitCode;
            if (exitCode != 0 || !File.Exists(outputFilename))
            {
                Trace.WriteLine("Conversion error (" + exitCode + ")");
            }
            return outputFilename;
            */
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Close();
        }


    }
}

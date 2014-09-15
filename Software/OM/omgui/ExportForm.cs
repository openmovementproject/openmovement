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
        public const string EXECUTABLE_NAME = @"Plugins\Convert_CWA\cwa-convert.exe";

        public ExportForm(string inputFilename, string downloadPath, float blockStart, float blockCount)
        {
            InitializeComponent();

            saveFileDialog.InitialDirectory = downloadPath;

            textBoxSourceFile.Text = inputFilename;
            textBoxOutputFile.Text = Path.Combine(downloadPath, Path.ChangeExtension(Path.GetFileName(inputFilename), saveFileDialog.DefaultExt));

            if (blockStart >= 0)
            {
                textBoxBlockStart.Text = ((int)Math.Floor(blockStart)).ToString();
            }

            if (blockCount >= 0)
            {
                textBoxBlockCount.Text = ((int)Math.Ceiling(blockCount)).ToString();
            }
        }

        private void ExportForm_Load(object sender, EventArgs e)
        {
            //TS - [P] - Commented out because now just uses working folder.
            if (Browse(Path.GetFileName(textBoxOutputFile.Text)) != DialogResult.OK) { this.Close(); }
        }

        //TS - [P] - As explaind above this isn't 
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

            List<string> args = new List<string>();
            args.Add("\"" + textBoxSourceFile.Text + "\"");
            args.Add("-f:csv");
            args.Add("-out \"" + textBoxOutputFile.Text + "\"");

            // Stream
            if (radioButtonStreamAccel.Checked) { args.Add("-s:accel"); }
            if (radioButtonStreamGyro.Checked)  { args.Add("-s:gyro"); }

            // Values
            if (radioButtonValuesFloat.Checked) { args.Add("-v:float"); }
            if (radioButtonValuesInt.Checked)   { args.Add("-v:int"); }

            // Timestamp
            if (radioButtonTimeTimestamp.Checked) { args.Add("-t:timestamp"); }
            if (radioButtonTimeNone.Checked)      { args.Add("-t:none"); }
            if (radioButtonTimeSequence.Checked)  { args.Add("-t:sequence"); }
            if (radioButtonTimeSecs.Checked)      { args.Add("-t:secs"); }
            if (radioButtonTimeDays.Checked)      { args.Add("-t:days"); }
            if (radioButtonTimeSerial.Checked)    { args.Add("-t:serial"); }
            if (radioButtonTimeExcel.Checked)     { args.Add("-t:excel"); }
            if (radioButtonTimeMatlab.Checked)    { args.Add("-t:matlab"); }

            // Sub-sample
            if (textBoxSampleStart.Text.Length > 0)  { args.Add("-start " + textBoxSampleStart.Text); }
            if (textBoxSampleLength.Text.Length > 0) { args.Add("-length " + textBoxSampleLength.Text); }
            if (textBoxSampleStep.Text.Length > 0)   { args.Add("-step " + textBoxSampleStep.Text); }

            // Blocks
            if (textBoxBlockStart.Text.Length > 0) { args.Add("-blockstart " + textBoxBlockStart.Text); }
            if (textBoxBlockCount.Text.Length > 0) { args.Add("-blockcount " + textBoxBlockCount.Text); }

            // Construct arguments
            processInformation.Arguments = string.Join(" ", args.ToArray());
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

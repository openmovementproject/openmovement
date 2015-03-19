using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace OmGui
{
    /*
    public class ProcessingStep
    {
        public ProcessingStep(string executableName, string[] args)
        {
            this.ExecutableName = executableName;
            this.Args = args;
        }

        public string ExecutableName { get; protected set; }
        public string[] Args { get; protected set; }
    }
    */

    public partial class ProcessingForm : Form
    {
        private string executableName;
        private List<string> args;
        private string outputFile;
        private string finalFile;
        private bool autoClose;
        private int originalHeight;
        private const int expandedHeight = 400;

        public ProcessingForm(string executableName, List<string> args, string outputFile, string finalFile)
        {
            InitializeComponent();
            this.executableName = executableName;
            this.args = args;
            this.outputFile = outputFile;
            this.finalFile = finalFile;
            this.autoClose = true;
            buttonOK.Enabled = false;
            this.originalHeight = this.Height;
            this.DialogResult = DialogResult.Cancel;
        }


        private void AppendText(string data)
        {
            if (InvokeRequired) { this.Invoke(new MethodInvoker(() => { AppendText(data); })); return; }

            const int max = 8192;
            if (textBoxProgress.Text.Length + data.Length > max)
            {
                string txt = textBoxProgress.Text + data;
                txt = txt.Substring(txt.Length - max);
                textBoxProgress.Text = txt;
                textBoxProgress.Select(textBoxProgress.Text.Length, 0);
                textBoxProgress.ScrollToCaret();
            }
            else
            {
                textBoxProgress.Select(textBoxProgress.Text.Length, 0);
                textBoxProgress.SelectedText = data;
                textBoxProgress.Select(textBoxProgress.Text.Length, 0);
                textBoxProgress.ScrollToCaret();
            }
            Console.Out.Write(data);
        }

        public bool Execute()
        {
            // Executable file
            string executableFile = Path.Combine(Application.StartupPath, executableName);
            if (!File.Exists(executableFile))
            {
                MessageBox.Show(this, "This process requires the external executable " + executableName + ".\r\n\r\nThe file was not found at:\r\n\r\n" + executableFile + "\r\n\r\nPlease locate the executable there and try again.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                return false;
            }

            // Convert the file
            ProcessStartInfo processInformation = new ProcessStartInfo();
            processInformation.FileName = executableFile;

            // Construct arguments
            processInformation.Arguments = string.Join(" ", this.args.ToArray());
            processInformation.UseShellExecute = false;
            processInformation.CreateNoWindow = true;
            processInformation.RedirectStandardError = true;
            //processInformation.RedirectStandardOutput = true;

            Process conversionProcess = new Process();
            conversionProcess.EnableRaisingEvents = true;
            conversionProcess.StartInfo = processInformation;
            /*
            conversionProcess.OutputDataReceived += (sender, e) => {
                AppendText(e.Data);
            };
            conversionProcess.ErrorDataReceived += (sender, e) => {
                AppendText(e.Data);
            };
            */

            AppendText("<<<START: " + conversionProcess.StartInfo.FileName + " " + conversionProcess.StartInfo.Arguments + ">>>\n");
            try
            {
                conversionProcess.Start();
            }
            catch (Exception ex)
            {
                AppendText("<<<ERROR: " + ex.Message+ ">>>\n");
                Trace.WriteLine("ERROR: Problem running conversion process: " + ex.Message);
                MessageBox.Show(this, "Problem running conversion process " + ex.Message + ".", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                return false;
            }

            bool cancel = false;
            StringBuilder sb = new StringBuilder();
            DateTime lastAppended = DateTime.Now;
            while (!conversionProcess.StandardError.EndOfStream)
            {
                if (conversionProcess.StandardError.Peek() < 0)
                {
                    if (DateTime.Now - lastAppended > TimeSpan.FromMilliseconds(250))
                    {
                        lastAppended = DateTime.Now;
                        AppendText(sb.ToString());
                        sb.Remove(0, sb.Length);
                    }
                    System.Threading.Thread.Sleep(100);
                }
                else
                {
                    int ic = conversionProcess.StandardError.Read();
                    if (ic < 0) { break; }
                    char cc = (char)ic;
                    sb.Append(cc);
                }
                if (backgroundWorker.CancellationPending) 
                {
                    cancel = true;
                    conversionProcess.Kill();
                    break; 
                }
            }
            AppendText(sb.ToString()); 
            sb.Remove(0, sb.Length);

            AppendText("<<<WAIT>>>\n");
            conversionProcess.WaitForExit();

            int exitCode = conversionProcess.ExitCode;
            AppendText("<<<END: " + exitCode + ">>>\n");

            if (cancel || exitCode != 0)
            {
                if (cancel) { AppendText("<<<CANCELLED>>>\n"); }
                else { AppendText("<<<FAILED>>>\n"); }
                return false;
            }

            AppendText("<<<SUCCESS>>>\n");

            return true;
        }



        private bool cancelClose = true;

        private void ProcessingForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = cancelClose;
            //if (e.Cancel) { backgroundWorker.CancelAsync(); }
        }

        private void ProcessingForm_Load(object sender, EventArgs e)
        {
            backgroundWorker.DoWork += (bws, bwe) =>
            {
                if (!Execute())
                {
                    // Failed - show details
                    checkBoxDetail.Checked = true;
                }
                else
                {
                    //if (InvokeRequired) 
                    this.Invoke(new MethodInvoker(() => {

                        // If we have to rename the output file as the final step...
                        if (this.outputFile != null && this.finalFile != null)
                        {
                            for (;;)
                            {
                                try
                                {
                                    if (!File.Exists(this.outputFile))
                                    {
                                        AppendText("ERROR: Output file not found: " + this.outputFile);
                                        return;
                                    }
                                    if (File.Exists(this.finalFile))
                                    {
                                        AppendText("NOTE: Removing existing output file: " + this.finalFile);
                                        File.Delete(this.finalFile);
                                    }
                                    File.Move(this.outputFile, this.finalFile);
                                    break;  // Continue
                                }
                                catch (Exception ex)
                                {
                                    AppendText("ERROR: Problem renaming output file: " + this.finalFile + " -- " + ex.Message);

                                    DialogResult rdr = MessageBox.Show(null, "Problem writing output.\r\n\r\nDescription: " + ex.Message + "\r\n\r\nCheck if the file is open in an application:\r\n\r\n" + this.finalFile + "\r\n\r\nTry again?", "Cannot Overwrite - Retry?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1);
                                    // If we're not retrying...
                                    if (rdr != DialogResult.Yes)
                                    {
                                        if (rdr == DialogResult.Cancel) { return; }   // Cancel
                                        break;    // DialogResult.No -- Don't retry, but don't cancel (continue)
                                    }
                                }
                            }
                        }

                        buttonOK.Enabled = true;
                        cancelClose = false;
                        if (autoClose) { buttonOK_Click(null, null); }
                    })); 
                }
            };
            backgroundWorker.RunWorkerCompleted += (bws, bwe) =>
            {
                cancelClose = false;
                //this.Close();
            };

            progressBar.Visible = pictureBoxProgress.Visible = true;
            backgroundWorker.RunWorkerAsync();
        }


        private void timerUpdate_Tick(object sender, EventArgs e)
        {
            progressBar.Visible = pictureBoxProgress.Visible = backgroundWorker.IsBusy;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void checkBoxDetail_CheckedChanged(object sender, EventArgs e)
        {
            textBoxProgress.Visible = checkBoxDetail.Checked;

            if (checkBoxDetail.Checked)
            {
                if (this.Height < expandedHeight)
                {
                    this.Height = expandedHeight;
                }
            }
            else
            {
                if (this.Height > this.originalHeight)
                {
                    this.Height = this.originalHeight;
                }
            }
        }

    }
}

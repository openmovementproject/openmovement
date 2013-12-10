using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;

namespace OmGui
{
    public partial class ProgressBox : Form
    {
        protected BackgroundWorker backgroundWorker;


        protected override void WndProc(ref Message msg)
        {
            base.WndProc(ref msg);
            if (MainForm.queryCancelAutoPlayID != 0 && msg.Msg == MainForm.queryCancelAutoPlayID)
            {
                msg.Result = (IntPtr)1;    // Cancel autoplay
            }
        }

        public ProgressBox(string title, string message, BackgroundWorker backgroundWorker)
        {
            InitializeComponent();
            DialogResult = System.Windows.Forms.DialogResult.None;
            Text = title;
            Prompt = message;

            this.backgroundWorker = backgroundWorker;

            if (backgroundWorker != null)
            {
                if (!backgroundWorker.WorkerSupportsCancellation)
                {
                    buttonCancel.Enabled = false;
                }

                if (backgroundWorker.WorkerReportsProgress)
                {
                    Value = 0;
                    backgroundWorker.ProgressChanged += backgroundWorker_ProgressChanged;
                    backgroundWorker.RunWorkerCompleted += backgroundWorker_RunWorkerCompleted;
                }

                backgroundWorker.RunWorkerAsync();
            }
        }

        void backgroundWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            BeginInvoke(new Action(() =>
                {
                    Value = e.ProgressPercentage;
                    if (e.UserState != null)
                    {
                        string s = e.UserState.ToString();
                        if (s != null)
                        {
                            Prompt = s;
                        }
                    }
Application.DoEvents();

//Invalidate(true);
//Thread.Sleep(500);
                }
            ));

Application.DoEvents();

//Thread.Sleep(50);
        }

        void backgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            Result = e.Result;
            this.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.Close();
        }

        public object Result
        {
            get;
            protected set;
        }

        public string Prompt
        {
            get { return labelPrompt.Text; }
            set { labelPrompt.Text = value; }
        }

        public int Value
        {
            get { return progressBar.Value; }
            set
            {
                if (value < 0)
                {
                    if (progressBar.Style != ProgressBarStyle.Marquee) progressBar.Style = ProgressBarStyle.Marquee;
                    progressBar.Value = 0;
                }
                else
                {
                    if (progressBar.Style != ProgressBarStyle.Continuous) progressBar.Style = ProgressBarStyle.Continuous;
                    progressBar.Value = value;
                }
//progressBar.Invalidate();
            }
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            DialogResult = System.Windows.Forms.DialogResult.Cancel;
            if (backgroundWorker != null && backgroundWorker.WorkerSupportsCancellation)
            {
                backgroundWorker.CancelAsync();
            }
            Close();
        }

        private void ProgressBox_Load(object sender, EventArgs e)
        {

        }
    }
}

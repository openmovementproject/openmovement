using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace OmGui
{
    public partial class ProgressBox : Form
    {
        protected BackgroundWorker backgroundWorker;

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
            Invoke(new Action(() =>
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
                }
            ));
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
                    progressBar.Style = ProgressBarStyle.Marquee;
                    progressBar.Value = 0;
                }
                else
                {
                    progressBar.Style = ProgressBarStyle.Continuous;
                    progressBar.Value = value;
                }
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

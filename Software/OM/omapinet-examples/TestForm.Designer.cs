namespace omapinet_examples
{
    partial class TestForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TestForm));
            this.splitContainerLog = new System.Windows.Forms.SplitContainer();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.buttonRun = new System.Windows.Forms.Button();
            this.splitContainerLog.Panel1.SuspendLayout();
            this.splitContainerLog.Panel2.SuspendLayout();
            this.splitContainerLog.SuspendLayout();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainerLog
            // 
            this.splitContainerLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerLog.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainerLog.Location = new System.Drawing.Point(0, 0);
            this.splitContainerLog.Name = "splitContainerLog";
            this.splitContainerLog.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainerLog.Panel1
            // 
            this.splitContainerLog.Panel1.Controls.Add(this.panel1);
            // 
            // splitContainerLog.Panel2
            // 
            this.splitContainerLog.Panel2.Controls.Add(this.textBoxLog);
            this.splitContainerLog.Size = new System.Drawing.Size(515, 364);
            this.splitContainerLog.SplitterDistance = 280;
            this.splitContainerLog.TabIndex = 0;
            // 
            // textBoxLog
            // 
            this.textBoxLog.BackColor = System.Drawing.SystemColors.Window;
            this.textBoxLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBoxLog.Font = new System.Drawing.Font("Courier New", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxLog.Location = new System.Drawing.Point(0, 0);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ReadOnly = true;
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxLog.Size = new System.Drawing.Size(515, 80);
            this.textBoxLog.TabIndex = 0;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.buttonRun);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(515, 280);
            this.panel1.TabIndex = 0;
            // 
            // buttonRun
            // 
            this.buttonRun.Location = new System.Drawing.Point(12, 12);
            this.buttonRun.Name = "buttonRun";
            this.buttonRun.Size = new System.Drawing.Size(143, 42);
            this.buttonRun.TabIndex = 0;
            this.buttonRun.Text = "&Run";
            this.buttonRun.UseVisualStyleBackColor = true;
            this.buttonRun.Click += new System.EventHandler(this.buttonRun_Click);
            // 
            // TestForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(515, 364);
            this.Controls.Add(this.splitContainerLog);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "TestForm";
            this.Text = "OmApiNet Examples";
            this.Load += new System.EventHandler(this.TestForm_Load);
            this.splitContainerLog.Panel1.ResumeLayout(false);
            this.splitContainerLog.Panel2.ResumeLayout(false);
            this.splitContainerLog.Panel2.PerformLayout();
            this.splitContainerLog.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainerLog;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonRun;
        private System.Windows.Forms.TextBox textBoxLog;
    }
}


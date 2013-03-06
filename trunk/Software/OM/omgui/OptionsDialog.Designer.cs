namespace OmGui
{
    partial class OptionsDialog
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
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxDefaultFolder = new System.Windows.Forms.TextBox();
            this.btnBrowse = new System.Windows.Forms.Button();
            this.buttonBrowse2 = new System.Windows.Forms.Button();
            this.textBoxDefaultPlugin = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.buttonSetCurrent = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnOK
            // 
            this.btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOK.Location = new System.Drawing.Point(429, 173);
            this.btnOK.Margin = new System.Windows.Forms.Padding(4);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(100, 28);
            this.btnOK.TabIndex = 0;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.Location = new System.Drawing.Point(546, 173);
            this.btnCancel.Margin = new System.Windows.Forms.Padding(4);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(100, 28);
            this.btnCancel.TabIndex = 1;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(16, 15);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(153, 17);
            this.label1.TabIndex = 2;
            this.label1.Text = "Default Working Folder";
            // 
            // textBoxDefaultFolder
            // 
            this.textBoxDefaultFolder.Location = new System.Drawing.Point(16, 43);
            this.textBoxDefaultFolder.Margin = new System.Windows.Forms.Padding(4);
            this.textBoxDefaultFolder.Name = "textBoxDefaultFolder";
            this.textBoxDefaultFolder.Size = new System.Drawing.Size(409, 22);
            this.textBoxDefaultFolder.TabIndex = 3;
            // 
            // btnBrowse
            // 
            this.btnBrowse.Location = new System.Drawing.Point(433, 40);
            this.btnBrowse.Margin = new System.Windows.Forms.Padding(4);
            this.btnBrowse.Name = "btnBrowse";
            this.btnBrowse.Size = new System.Drawing.Size(100, 28);
            this.btnBrowse.TabIndex = 4;
            this.btnBrowse.Text = "Browse...";
            this.btnBrowse.UseVisualStyleBackColor = true;
            this.btnBrowse.Click += new System.EventHandler(this.btnBrowse_Click);
            // 
            // buttonBrowse2
            // 
            this.buttonBrowse2.Location = new System.Drawing.Point(433, 111);
            this.buttonBrowse2.Margin = new System.Windows.Forms.Padding(4);
            this.buttonBrowse2.Name = "buttonBrowse2";
            this.buttonBrowse2.Size = new System.Drawing.Size(100, 28);
            this.buttonBrowse2.TabIndex = 7;
            this.buttonBrowse2.Text = "Browse...";
            this.buttonBrowse2.UseVisualStyleBackColor = true;
            this.buttonBrowse2.Click += new System.EventHandler(this.buttonBrowse2_Click);
            // 
            // textBoxDefaultPlugin
            // 
            this.textBoxDefaultPlugin.Location = new System.Drawing.Point(16, 114);
            this.textBoxDefaultPlugin.Margin = new System.Windows.Forms.Padding(4);
            this.textBoxDefaultPlugin.Name = "textBoxDefaultPlugin";
            this.textBoxDefaultPlugin.Size = new System.Drawing.Size(409, 22);
            this.textBoxDefaultPlugin.TabIndex = 6;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(16, 86);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(140, 17);
            this.label2.TabIndex = 5;
            this.label2.Text = "Default Plugin Folder";
            // 
            // buttonSetCurrent
            // 
            this.buttonSetCurrent.Location = new System.Drawing.Point(546, 40);
            this.buttonSetCurrent.Margin = new System.Windows.Forms.Padding(4);
            this.buttonSetCurrent.Name = "buttonSetCurrent";
            this.buttonSetCurrent.Size = new System.Drawing.Size(100, 28);
            this.buttonSetCurrent.TabIndex = 8;
            this.buttonSetCurrent.Text = "Set Current";
            this.buttonSetCurrent.UseVisualStyleBackColor = true;
            this.buttonSetCurrent.Click += new System.EventHandler(this.buttonSetCurrent_Click);
            // 
            // OptionsDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(662, 216);
            this.Controls.Add(this.buttonSetCurrent);
            this.Controls.Add(this.buttonBrowse2);
            this.Controls.Add(this.textBoxDefaultPlugin);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnBrowse);
            this.Controls.Add(this.textBoxDefaultFolder);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOK);
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "OptionsDialog";
            this.Text = "OptionsDialog";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBoxDefaultFolder;
        private System.Windows.Forms.Button btnBrowse;
        private System.Windows.Forms.Button buttonBrowse2;
        private System.Windows.Forms.TextBox textBoxDefaultPlugin;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button buttonSetCurrent;
    }
}
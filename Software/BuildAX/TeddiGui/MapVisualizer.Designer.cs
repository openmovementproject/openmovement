namespace TeddiGui
{
    partial class MapVisualizer
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // MapVisualizer
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.BackColor = System.Drawing.Color.Black;
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "MapVisualizer";
            this.Size = new System.Drawing.Size(1011, 556);
            this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.MapVisualizer_KeyPress);
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.MapVisualizer_MouseDown);
            this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.MapVisualizer_MouseUp);
            this.ResumeLayout(false);

        }

        #endregion

    }
}

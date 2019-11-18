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
    public partial class Splash : Form
    {
        public event EventHandler Started;
        private bool hasStarted = false;

        public Splash()
        {
            InitializeComponent();
        }

        protected override CreateParams CreateParams
        {
            get
            {
                const int CS_DROPSHADOW = 0x20000;
                CreateParams createParams = base.CreateParams;
                createParams.ClassStyle |= CS_DROPSHADOW;
                return createParams;
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            e.Graphics.DrawRectangle(Pens.Gray, new Rectangle(0, 0, Width - 1, Height - 1));
        }

        private void Splash_Load(object sender, EventArgs e)
        {
            //labelStatus.Text = "Loading...";
        }

        private void Splash_Shown(object sender, EventArgs e)
        {
            // Run one timer tick (proves main loop is running)
            timer1.Enabled = true;
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            timer1.Enabled = false;
            if (!hasStarted)
            {
                hasStarted = true;
                Started?.Invoke(this, null);
            }
        }
    }
}

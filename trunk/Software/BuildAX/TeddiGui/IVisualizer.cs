using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace TeddiGui
{
    public partial class IVisualizer : UserControl
    {
        protected Manager manager;

        public IVisualizer()
            : this(null)
        {
        }

        public IVisualizer(Manager manager)
        {
            InitializeComponent();
            this.manager = manager;
            this.DoubleBuffered = true;
        }

        protected override void OnResize(EventArgs e)
        {
            Invalidate();
        }

    }
}

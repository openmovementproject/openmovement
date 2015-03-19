using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace OmGui
{
    public partial class ToolStripDirectoryChooser : ToolStripTextBox
    {
        public ToolStripDirectoryChooser()
        {
            InitializeComponent();
        }

        public override Size GetPreferredSize(Size constrainingSize)
        {
            if (IsOnOverflow || Owner.Orientation == Orientation.Vertical)
            {
                return DefaultSize;
            }

            // Total available width
            Int32 width = Owner.DisplayRectangle.Width;
            if (Owner.OverflowButton.Visible)
            {
                width = width - Owner.OverflowButton.Width - Owner.OverflowButton.Margin.Horizontal;
            }

            // Count number of boxes
            Int32 springBoxCount = 0;
            foreach (ToolStripItem item in Owner.Items)
            {
                // (which aren't overflowed)
                if (item.IsOnOverflow) continue;

                if (item is ToolStripDirectoryChooser)
                {
                    springBoxCount++;
                    width -= item.Margin.Horizontal;
                }
                else
                {
                    width = width - item.Width - item.Margin.Horizontal;
                }
            }

            // Share available space
            if (springBoxCount > 1) width /= springBoxCount;
            if (width < DefaultSize.Width) width = DefaultSize.Width;
            Size size = base.GetPreferredSize(constrainingSize);
            size.Width = width;

            return size;
        }
    }
}

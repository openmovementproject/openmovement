using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace OMTesting
{
    public class FormattedNumericUpDown : NumericUpDown
    {
        [DllImport("User32.dll")]
        static extern Boolean HideCaret(System.IntPtr hWnd);

        [DllImport("User32.dll")]
        static extern Boolean ShowCaret(System.IntPtr hWnd);

        private TextBox textBox;
        private List<int> lostList = new List<int>();
        private const int MaxLost = 10;

        public FormattedNumericUpDown() : base()
        {
            foreach (Control control in this.Controls)
            {
                if (control is TextBox)
                {
                    textBox = (TextBox)control;
                    break;
                }
            }

            if (textBox != null)
            {
                textBox.ReadOnly = true;
                textBox.HideSelection = true;
                textBox.Cursor = Cursors.Default;
            }

            NextDigitClears = true;
        }

        protected override void OnGotFocus(EventArgs e)
        {
            if (textBox != null)
            {
                HideCaret(textBox.Handle);
            }
            base.OnGotFocus(e);
        }

        protected override void OnLostFocus(EventArgs e)
        {
            if (textBox != null)
            {
                ShowCaret(textBox.Handle);
            }
            base.OnLostFocus(e);
        }

        protected override void OnTextBoxTextChanged(object source, EventArgs e)
        {
            TextBox textBox = source as TextBox;
            int val = 0;
            NextDigitClears = false;
            if (int.TryParse(textBox.Text, out val))
            {
                textBox.Text = val.ToString("D" + Maximum.ToString().Length);
            }
            else
            {
                base.OnTextBoxTextChanged(source, e);
            }
        }

        private bool NextDigitClears { get; set; }

        public void WasSubmitted()
        {
            NextDigitClears = true;
            lostList.Clear();
        }

        public void AddDigit(int digit)
        {
            if (NextDigitClears) { Value = 0; }
            int value = (int)Value * 10 + digit;
            int lost = value / ((int)Maximum + 1);
            lostList.Insert(0, lost);
            while (lostList.Count > MaxLost) { lostList.RemoveAt(MaxLost); }
            value %= ((int)Maximum + 1);
            Value = value;
        }

        public void RemoveDigit()
        {
            int lost = 0;
            if (lostList.Count > 0)
            {
                lost = lostList[0];
                lostList.RemoveAt(0);
            }

            Value = (int)(Value / 10) + lost * (((int)Maximum + 1) / 10);
        }
    }
}

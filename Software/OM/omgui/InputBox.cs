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
    public partial class InputBox : Form
    {
        public InputBox(string title, string prompt, string defaultValue = "")
        {
            InitializeComponent();
            Text = title;
            Prompt = prompt;
            Value = defaultValue;
        }

        public string Prompt
        {
            get { return labelPrompt.Text; }
            set { labelPrompt.Text = value; }
        }

        public string Value
        {
            get { return textBoxInput.Text; }
            set { textBoxInput.Text = value; textBoxInput.SelectAll(); }
        }

        private void buttonOk_Click(object sender, EventArgs e)
        {
            DialogResult = System.Windows.Forms.DialogResult.OK;
            Close();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            DialogResult = System.Windows.Forms.DialogResult.Cancel;
            Close();
        }

        private void InputBox_Load(object sender, EventArgs e)
        {
        }
    }
}

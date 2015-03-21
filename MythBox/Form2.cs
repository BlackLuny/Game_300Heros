using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MythBox
{
	public partial class Form2 : Form
	{
		public Form2()
		{
			InitializeComponent();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			//openFileDialog1.InitialDirectory = "c:\\";//注意这里写路径时要用c:\\而不是c:\
			openFileDialog1.Filter = "300英雄|300.exe";
			openFileDialog1.RestoreDirectory = true;
			openFileDialog1.FilterIndex = 1;

			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				textBox1.Text = openFileDialog1.FileName;
			}
		}
	}
}

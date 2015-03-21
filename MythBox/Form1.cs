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
	public partial class Form1 : Form
	{
		public Form1()
		{
			InitializeComponent();
		}

		private void 程序设置ToolStripMenuItem_Click(object sender, EventArgs e)
		{
			new Form2().ShowDialog();
		}
	}
}

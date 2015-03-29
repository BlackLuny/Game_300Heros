using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MythBox.View
{
	partial class Profile : Form
	{
		public Profile()
		{
			InitializeComponent();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			openFileDialog1.Filter = "300英雄|300.exe";
			openFileDialog1.RestoreDirectory = true;
			openFileDialog1.FilterIndex = 1;

			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				textBox1.Text = openFileDialog1.FileName;
			}
		}

		private void Form2_Load(object sender, EventArgs e)
		{
            textBox1.Text = Model.Service.GetInstance().Profile.GetValue("system", "GamePath");
		}

		private void Form2_FormClosed(object sender, FormClosedEventArgs e)
		{
			Model.Service.GetInstance().Profile.SetValue("system", "GamePath", textBox1.Text);
		}


	}
}

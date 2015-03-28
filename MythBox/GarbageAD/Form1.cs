using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GarbageAD
{
	public partial class Form1 : Form
	{
		public Form1()
		{
			InitializeComponent();
		}

        private void button1_Click(object sender, EventArgs e)
        {
            if (textBox1.Text == "")
            {
                MessageBox.Show("不能添加空数据哦");
                return;
            }
            listBox1.Items.Add(textBox1.Text);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            foreach (string match in MyPlugin.matchList)
            {
                listBox1.Items.Add(match);
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            List<string> matchList = new List<string>();

            foreach (object match in listBox1.Items)
            {
                matchList.Add((string)match);
            }

            MyPlugin.matchList = matchList;
        }

        private void 删除ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (listBox1.SelectedItem != null)
            {
                listBox1.Items.Remove(listBox1.SelectedItem);
            }
        }


	}
}

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using Microsoft.Win32;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;


namespace MythBox.View
{
	partial class Main : Form
	{
		public Main()
		{
			InitializeComponent();
		}

		private void 程序设置ToolStripMenuItem_Click(object sender, EventArgs e)
		{
			new Profile().ShowDialog();
		}

		private void listView1_DragEnter(object sender, DragEventArgs e)
		{
			if (e.Data.GetDataPresent(DataFormats.FileDrop))
			{
				e.Effect = DragDropEffects.All;
			}
			else
			{
				e.Effect = DragDropEffects.None;
			}
		}

		private void listView1_DragDrop(object sender, DragEventArgs e)
		{
			string[] DragFiles = (string[])e.Data.GetData(DataFormats.FileDrop, false);

			foreach (string DragFile in DragFiles)
			{
				
			}
		}


		private void Form1_Load(object sender, EventArgs e)
		{
            Model.Service.GetInstance().Startup();
            RefreshPluginView();
		}


        public void RefreshPluginView()
        {
            listView1.Items.Clear();

            foreach (Model.Plugin.SDK.PluginSdk plugin in Model.Service.GetInstance().Plugin.pluginList)
            {
                ListViewItem lv1 = listView1.Items.Add(plugin.pluginName);
                lv1.SubItems.Add(plugin.pluginAuthor);
                lv1.SubItems.Add(plugin.pluginVersion);

                if (plugin.IsEnabled == true)
                {
                    lv1.SubItems.Add("启用");
                }
                else
                {
                    lv1.SubItems.Add("禁用");
                }

                lv1.Tag = plugin;
            }
        }
		private void listView1_DoubleClick(object sender, EventArgs e)
		{
            if (listView1.SelectedItems.Count > 0)
            {
                Model.Plugin.SDK.PluginSdk plugin = (Model.Plugin.SDK.PluginSdk)listView1.SelectedItems[0].Tag;

                plugin.OnShowForm();
            }
		}

		private void button1_Click(object sender, EventArgs e)
		{
            string GamePath = Model.Service.GetInstance().Profile.GetValue("system", "GamePath");
            if (GamePath == "")
            {
                MessageBox.Show("找不到游戏路径,请手动设置游戏路径！");
                return;
            }
            Process.Start(GamePath, "LauncherShellExecuteGame");
		}

        private void 调试模式ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            new Debug.Analysis().Show();
        }

        private void 禁用ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count > 0)
            {
                Model.Plugin.SDK.PluginSdk plugin = (Model.Plugin.SDK.PluginSdk)listView1.SelectedItems[0].Tag;
                plugin.SetEnabled(false);

                RefreshPluginView();
            }
        }

        private void 启用ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count > 0)
            {
                Model.Plugin.SDK.PluginSdk plugin = (Model.Plugin.SDK.PluginSdk)listView1.SelectedItems[0].Tag;
                plugin.SetEnabled(true);

                RefreshPluginView();
            }
        }
	}
}

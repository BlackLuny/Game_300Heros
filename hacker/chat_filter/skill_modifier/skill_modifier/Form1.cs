﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace skill_modifier
{
	public partial class Form1 : Form
	{
		[DllImport("user32.dll")]
		private static extern long SendMessage(Int32 hwnd, Int32 msg, Int32 hwndFrom, ref COPYDATASTRUCT cd);

		[DllImport("user32.dll")]
		private static extern Int32 FindWindow(String classname, String text);

		[DllImport("extdll.dll", EntryPoint = "game_hook")]
		private static extern bool game_hook();

		const int WM_COPYDATA = 0x004A;
		[StructLayout(LayoutKind.Sequential)]
		public struct COPYDATASTRUCT
		{
			public uint dwData;
			public int cbData;
			public IntPtr lpData;
		}

		public Form1()
		{
			InitializeComponent();
		}

		private void Form1_Load(object sender, EventArgs e)
		{
			this.Text = "ExGameHook";
		}

		protected override void DefWndProc(ref Message m)
		{
			if (m.Msg == WM_COPYDATA)
			{
				COPYDATASTRUCT cds = new COPYDATASTRUCT();
				Type mytype = cds.GetType();
				cds = (COPYDATASTRUCT)m.GetLParam(mytype);//当前的消息

				//0xFFFF 原始技能ID
				//0xFFFF 新技能ID
				uint event_id = cds.dwData;

			}
			base.DefWndProc(ref m);
		}

		private bool PostToEVents(ushort source_id, ushort target_id)
		{
			bool rs = false;
			COPYDATASTRUCT cds = new COPYDATASTRUCT();

			cds.dwData = (((uint)source_id << 16) | (uint)target_id);
			cds.cbData = 0;
			cds.lpData = Marshal.AllocHGlobal(0);

			int hWnd = FindWindow("WWW_JUMPW_COM", null);

			if (hWnd != 0)
			{
				SendMessage(hWnd, WM_COPYDATA, 0, ref cds);
				rs = true;
			}

			Marshal.FreeHGlobal(cds.lpData);
			return rs;
		}

		private void button2_Click(object sender, EventArgs e)
		{
			try
			{
				if (game_hook() == false)
				{
					MessageBox.Show("初始化失败");
				}
				else
				{
					MessageBox.Show("初始化成功");
				}
			}
			catch
			{
				MessageBox.Show("找不到extdll.dll");
				
			}
		}

		private static byte[] HexToByte(string hexString)
		{
			string newstr = hexString.Replace(" ", "");
			byte[] returnBytes = new byte[newstr.Length / 2];
			for (int i = 0; i < returnBytes.Length; i++)
				returnBytes[i] = Convert.ToByte(newstr.Substring(i * 2, 2), 16);
			return returnBytes;
		}
	}
}
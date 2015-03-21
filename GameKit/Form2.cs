using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace GameKit
{
	public partial class Form2 : Form
	{
		public Form2()
		{
			InitializeComponent();
		}

		private void Form2_Load(object sender, EventArgs e)
		{
			this.Text = "GameKit_Protocol";
		}

		protected override void DefWndProc(ref Message m)
		{
			if (m.Msg == Windows.WM_COPYDATA)
			{
				COPYDATASTRUCT cds = new COPYDATASTRUCT();
				Type mytype = cds.GetType();
				cds = (COPYDATASTRUCT)m.GetLParam(mytype);//当前的消息

				byte[] bufs = new byte[cds.cbData];//注意，将结构体的成员单个处理再使用
				Marshal.Copy(cds.lpData, bufs, 0, bufs.Length);//复制消息

				GameMessageData MessageData = new GameMessageData();
				MessageData.buffer = bufs;
				MessageData.type = cds.dwData;
				lock (Form1.MsgQueue)
				{
					Form1.MsgQueue.Enqueue(MessageData);
				}
				//AddPackets(MessageData);
			}
			base.DefWndProc(ref m);
		}
	}
}

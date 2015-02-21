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
    public partial class Form1 : Form
    {
        [DllImport("user32.dll")]
        private static extern long SendMessage(Int32 hwnd, Int32 msg, Int32 hwndFrom, ref COPYDATASTRUCT cd);

        [DllImport("user32.dll")]
        private static extern Int32 FindWindow(String classname, String text);
        
        const int WM_COPYDATA = 0x004A;
        [StructLayout(LayoutKind.Sequential)]
        public struct COPYDATASTRUCT
        {
            public uint dwData;
            public int cbData;
            public IntPtr lpData;
        }

        public class GameMessageData
        {
            public byte[] buffer;
            public uint type;
        }

        private Queue<GameMessageData> MessageDataQueue = new Queue<GameMessageData>();

        public static string BytesToString(byte[] sb)
        {
            string temp = "";
            foreach (byte b in sb)
            {
                temp += String.Format("{0:X2} ", b);
            }
            return temp;
        }

        private static byte[] HexToByte(string hexString)
        {
            string newstr = hexString.Replace(" ", "");
            byte[] returnBytes = new byte[newstr.Length / 2];
            for (int i = 0; i < returnBytes.Length; i++)
                returnBytes[i] = Convert.ToByte(newstr.Substring(i * 2, 2), 16);
            return returnBytes;
        }

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Console.Title = "GameKit";
            this.Text = "GameKit_Protocol";


            //listView1.SetStyle(ControlStyles.OptimizedDoubleBuffer | ControlStyles.AllPaintingInWmPaint, true);
        }

        private void AddPackets(GameMessageData Data)
        {
            if (Data.type == 0)
            {
                if (checkBox2.Checked)
                {
                    ListViewItem lv = listView1.Items.Add(Data.type.ToString());
                    lv.SubItems.Add(BytesToString(Data.buffer));
                }
            }
            else
            {
                if (checkBox1.Checked)
                {
                    ListViewItem lv = listView1.Items.Add(Data.type.ToString());
                    lv.SubItems.Add(BytesToString(Data.buffer));
                }
            }
           
        }


        protected override void DefWndProc(ref Message m)
        {
            if (m.Msg == WM_COPYDATA)
            {
                COPYDATASTRUCT cds = new COPYDATASTRUCT();
                Type mytype = cds.GetType();
                cds = (COPYDATASTRUCT)m.GetLParam(mytype);//当前的消息

                byte[] bufs = new byte[cds.cbData];//注意，将结构体的成员单个处理再使用
                Marshal.Copy(cds.lpData, bufs, 0, bufs.Length);//复制消息

                GameMessageData MessageData = new GameMessageData();
                MessageData.buffer = bufs;
                MessageData.type = cds.dwData;

                MessageDataQueue.Enqueue(MessageData);

                //AddPackets(MessageData);
            }
            base.DefWndProc(ref m);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            listView1.Items.Clear();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            //listView1.BeginUpdate();
            while (MessageDataQueue.Count > 0)
            {
                AddPackets(MessageDataQueue.Dequeue());
            }
            //listView1.EndUpdate();
        }

        private void 复制ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count != 0)
            {
                ListViewItem lv = listView1.SelectedItems[0];
                Clipboard.SetDataObject(lv.SubItems[1].Text, true);
            }
            //Clipboard.SetFileDropList
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                byte[] data = HexToByte(textBox1.Text);

                COPYDATASTRUCT cds = new COPYDATASTRUCT();

                cds.dwData = (uint)1;

                cds.cbData = data.Length;

                cds.lpData = Marshal.AllocHGlobal(data.Length);

                Marshal.Copy(data, 0, cds.lpData, data.Length);

                int hWnd = FindWindow("WWW_JUMPW_COM", null);

                if (hWnd != 0)
                {
                    SendMessage(hWnd, WM_COPYDATA, 0, ref cds);
                }
            }
            catch
            {
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            try
            {
                byte[] data = HexToByte(textBox1.Text);

                COPYDATASTRUCT cds = new COPYDATASTRUCT();

                cds.dwData = (uint)0;

                cds.cbData = data.Length;

                cds.lpData = Marshal.AllocHGlobal(data.Length);

                Marshal.Copy(data, 0, cds.lpData, data.Length);

                int hWnd = FindWindow("WWW_JUMPW_COM", null);

                if (hWnd != 0)
                {
                    SendMessage(hWnd, WM_COPYDATA, 0, ref cds);
                }
            }
            catch
            {
            }
        }

    }
}

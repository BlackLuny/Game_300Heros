using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

using System.IO;

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

        public struct GamePacketTypes
        {
            public int length;
            public uint timestamp;
            public ushort identifier;
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
        }

        private void AddPackets(GameMessageData Data)
        {
            try
            {
                GamePacketTypes info = new GamePacketTypes();
                BinaryReader sr = new BinaryReader(new MemoryStream(Data.buffer));
                info.length = sr.ReadInt32();
                info.timestamp = sr.ReadUInt32();
                info.identifier = sr.ReadUInt16();



                if (Data.type == 0)
                {
                    if (checkBox2.Checked)
                    {
                        if (checkBox4.Checked)
                        {
                            foreach (string s in listBox2.Items)
                            {
                                ushort i = Convert.ToUInt16(s, 16);
                                if (i == info.identifier)
                                {
                                    return;
                                }
                            }

                            ListViewItem lv = listView1.Items.Add(Data.type.ToString());
                            lv.SubItems.Add(BytesToString(Data.buffer));
                            lv.SubItems.Add(String.Format("{0:X4}", info.identifier));
                        }
                        else
                        {
                            ListViewItem lv = listView1.Items.Add(Data.type.ToString());
                            lv.SubItems.Add(BytesToString(Data.buffer));
                            lv.SubItems.Add(String.Format("{0:X4}", info.identifier));
                        }
                    }
                }
                else
                {
                    if (checkBox1.Checked)
                    {
                        if (checkBox3.Checked)
                        {
                            foreach (string s in listBox1.Items)
                            {
                                ushort i = Convert.ToUInt16(s, 16);
                                if (i == info.identifier)
                                {
                                    return;
                                }
                            }

                            ListViewItem lv = listView1.Items.Add(Data.type.ToString());
                            lv.SubItems.Add(BytesToString(Data.buffer));
                            lv.SubItems.Add(String.Format("{0:X4}", info.identifier));
                        }
                        else
                        {
                            ListViewItem lv = listView1.Items.Add(Data.type.ToString());
                            lv.SubItems.Add(BytesToString(Data.buffer));
                            lv.SubItems.Add(String.Format("{0:X4}", info.identifier));
                        }
                    }
                }
            }
            catch
            {

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


        private void 复制IDToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count != 0)
            {
                ListViewItem lv = listView1.SelectedItems[0];
                Clipboard.SetDataObject(lv.SubItems[2].Text, true);
            }
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

        private ushort GetTextBoxIdentifier()
        {
            ushort rs = 0;

            try
            {
                rs = Convert.ToUInt16(textBox2.Text,16);
            }
            catch
            {
            }

            return rs;
        }
        private void button4_Click(object sender, EventArgs e)
        {
            ushort identifier = GetTextBoxIdentifier();

            if (identifier != 0)
            {
                listBox1.Items.Add(String.Format("{0:X4}", identifier));
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            ushort identifier = GetTextBoxIdentifier();

            if (identifier != 0)
            {
                listBox2.Items.Add(String.Format("{0:X4}", identifier));
            }
        }

        private void 删除ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (listBox1.SelectedItem != null)
            {
                listBox1.Items.Remove(listBox1.SelectedItem);
            }
        }

        private void 删除ToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            if (listBox2.SelectedItem != null)
            {
                listBox2.Items.Remove(listBox2.SelectedItem);
            }
        }
        public List<string> GetPacketArrays()
        {
            List<string> sl = new List<string>();


            string[] x = textBox3.Text.Split(new string[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);

            foreach (string i in x)
            {
                sl.Add(i);
            }

            return sl;
        }
        private void button7_Click(object sender, EventArgs e)
        {
            List<string> sl = GetPacketArrays();

            foreach (string x in sl)
            {
                try
                {
                    byte[] data = HexToByte(x);

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
        }

        private void button6_Click(object sender, EventArgs e)
        {
            List<string> sl = GetPacketArrays();

            foreach (string x in sl)
            {
                try
                {
                    byte[] data = HexToByte(x);

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
}

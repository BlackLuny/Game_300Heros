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
using System.Threading;

namespace MythBox.View.Debug
{
    public partial class Analysis : Form
    {
        public List<ushort> RecvFilterList = new List<ushort>();
        public List<ushort> SendFilterList = new List<ushort>();

        public Queue<ListViewItem> RefreshListViewQueue = new Queue<ListViewItem>();
        public Analysis()
        {
            InitializeComponent();
        }

        public delegate void Delegate_PostMessageHandler(object sender, Model.Plugin.SDK.Events.MessagePostArgs e);
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

        

        private void Form1_Load(object sender, EventArgs e)
        {
            Text = "GameKit";

            Model.Service.GetInstance().DebugEvents.PostReceiveMessageHandler += new EventHandler<Model.Plugin.SDK.Events.MessagePostArgs>(DebugEvents_PostReceiveMessageHandler);
            Model.Service.GetInstance().DebugEvents.PostSendMessageHandler += new EventHandler<Model.Plugin.SDK.Events.MessagePostArgs>(DebugEvents_PostSendMessageHandler);
        }


        private void Analysis_FormClosing(object sender, FormClosingEventArgs e)
        {
            Model.Service.GetInstance().DebugEvents.PostReceiveMessageHandler -= new EventHandler<Model.Plugin.SDK.Events.MessagePostArgs>(DebugEvents_PostReceiveMessageHandler);
            Model.Service.GetInstance().DebugEvents.PostSendMessageHandler -= new EventHandler<Model.Plugin.SDK.Events.MessagePostArgs>(DebugEvents_PostSendMessageHandler);
        }

        void DebugEvents_PostSendMessageHandler(object sender, Model.Plugin.SDK.Events.MessagePostArgs e)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new Delegate_PostMessageHandler(DebugEvents_PostSendMessageHandler), sender, e);
            }
            else
            {
                AddPackets(e.Message, 1);
            }
        }

        void DebugEvents_PostReceiveMessageHandler(object sender, Model.Plugin.SDK.Events.MessagePostArgs e)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new Delegate_PostMessageHandler(DebugEvents_PostReceiveMessageHandler), sender, e);
            }
            else
            {
                AddPackets(e.Message, 0);
            }
        }
        public void SyncMemData()
        {
            RecvFilterList.Clear();

            foreach (string s in listBox2.Items)
            {
                ushort i = Convert.ToUInt16(s, 16);
                RecvFilterList.Add(i);
            }

            SendFilterList.Clear();

            foreach (string s in listBox1.Items)
            {
                ushort i = Convert.ToUInt16(s, 16);
                SendFilterList.Add(i);
            }
        }

        private void AddListView(Model.Data.Message Message, int IsSend)
        {
            ListViewItem lv = new ListViewItem();
            lv.SubItems[0].Text = IsSend.ToString();
            lv.SubItems.Add(BytesToString(Message.data));
            lv.SubItems.Add(String.Format("{0:X4}", Message.identifier));

            lock (RefreshListViewQueue)
            {
                RefreshListViewQueue.Enqueue(lv);
            }
        }
        private void AddPackets(Model.Data.Message Message, int IsSend)
        {
            try
            {
                if (IsSend == 0)
                {
                    if (checkBox2.Checked)
                    {
                        if (checkBox4.Checked)
                        {
                            foreach (ushort i in RecvFilterList)
                            {
                                if (i == Message.identifier)
                                {
                                    return;
                                }
                            }

                            AddListView(Message, IsSend);
                        }
                        else
                        {
                            AddListView(Message, IsSend);
                        }
                    }
                }
                else
                {
                    if (checkBox1.Checked)
                    {
                        if (checkBox3.Checked)
                        {
                            foreach (ushort i in SendFilterList)
                            {
                                if (i == Message.identifier)
                                {
                                    return;
                                }
                            }

                            AddListView(Message, IsSend);
                        }
                        else
                        {
                            AddListView(Message, IsSend);
                        }
                    }
                }
            }
            catch
            {

            }

        }

        private void button1_Click(object sender, EventArgs e)
        {
            listView1.Items.Clear();
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

                Model.Data.MessageSender ser = new Model.Data.MessageSender();
                ser.IsSend = 1;
                ser.data = data;

                Model.Service.GetInstance().DelayChannel.PutQueue(ser);
                
                //send
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

                Model.Data.MessageSender ser = new Model.Data.MessageSender();
                ser.IsSend = 0;
                ser.data = data;

                Model.Service.GetInstance().DelayChannel.PutQueue(ser);
                //recv
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
                rs = Convert.ToUInt16(textBox2.Text, 16);
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

            SyncMemData();
        }

        private void button5_Click(object sender, EventArgs e)
        {
            ushort identifier = GetTextBoxIdentifier();

            if (identifier != 0)
            {
                listBox2.Items.Add(String.Format("{0:X4}", identifier));
            }

            SyncMemData();
        }

        private void 删除ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (listBox1.SelectedItem != null)
            {
                listBox1.Items.Remove(listBox1.SelectedItem);
                SyncMemData();
            }
        }

        private void 删除ToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            if (listBox2.SelectedItem != null)
            {
                listBox2.Items.Remove(listBox2.SelectedItem);
                SyncMemData();
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
                    //send

                    Model.Data.MessageSender ser = new Model.Data.MessageSender();
                    ser.IsSend = 1;
                    ser.data = data;

                    Model.Service.GetInstance().DelayChannel.PutQueue(ser);
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

                    Model.Data.MessageSender ser = new Model.Data.MessageSender();
                    ser.IsSend = 0;
                    ser.data = data;

                    Model.Service.GetInstance().DelayChannel.PutQueue(ser);
                    //recv
                }
                catch
                {
                }
            }
        }


        private void timer1_Tick(object sender, EventArgs e)
        {
            lock (RefreshListViewQueue)
            {
                listView1.BeginUpdate();

                while (RefreshListViewQueue.Count > 0)
                {
                    listView1.Items.Add(RefreshListViewQueue.Dequeue());
                }

                listView1.EndUpdate();
            }
        }



    }
}

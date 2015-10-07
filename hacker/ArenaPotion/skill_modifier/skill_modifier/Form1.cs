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

        public enum ZLibError : int
        {
            Z_OK = 0,
            Z_STREAM_END = 1,
            Z_NEED_DICT = 2,
            Z_ERRNO = (-1),
            Z_STREAM_ERROR = (-2),
            Z_DATA_ERROR = (-3),
            Z_MEM_ERROR = (-4),
            Z_BUF_ERROR = (-5),
            Z_VERSION_ERROR = (-6),
        }

        public enum ZLibCompressionLevel : int
        {
            Z_NO_COMPRESSION = 0,
            Z_BEST_SPEED = 1,
            Z_BEST_COMPRESSION = 9,
            Z_DEFAULT_COMPRESSION = (-1)
        }

        public class ZLib
        {
            [DllImport("zlib.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ZLibError compress(byte[] dest, ref int destLength, byte[] source, int sourceLength);
            [DllImport("zlib.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ZLibError compress2(byte[] dest, ref int destLength, byte[] source, int sourceLength, ZLibCompressionLevel level);
            [DllImport("zlib.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ZLibError uncompress(byte[] dest, ref int destLen, byte[] source, int sourceLen);
            [DllImport("zlib.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int compressBound(int sourceLen);


            //
        }   

        const int WM_COPYDATA = 0x004A;
        [StructLayout(LayoutKind.Sequential)]
        public struct COPYDATASTRUCT
        {
            public uint dwData;
            public int cbData;
            public IntPtr lpData;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct PackHeader
        {
            public int length;
            public int magic;
            public ushort identifier;
            public byte[] buffers;
        }

        public static MemoryStream MemoryStreamToReaderStream(MemoryStream memoryStream)
        {
            return new MemoryStream(memoryStream.GetBuffer(), 0, (int)memoryStream.Length);
        }
        public static byte[] MemoryStreamToBytes(MemoryStream memoryStream)
        {
            return new BinaryReader(MemoryStreamToReaderStream(memoryStream)).ReadBytes((int)memoryStream.Length);
        }

        public class JumpStream : BinaryWriter
        {
            private uint magic;
            private ushort identifier;

            public JumpStream(uint magic, ushort identifier):
                base(new MemoryStream())
            {
                this.magic = magic;
                this.identifier = identifier;

            }

            public override void Write(string value)
            {
                byte[] s = Encoding.GetEncoding(936).GetBytes(value);
                Write((ushort)(s.Length + 1));
                Write(s);
                Write((byte)0);
            }

            public byte[] GetBuffer()
            {
                byte[] sb = null;

                using (MemoryStream tmp = new MemoryStream())
                {
                    BinaryWriter ss = new BinaryWriter(tmp);
                    byte[] data = MemoryStreamToBytes((MemoryStream)base.BaseStream);

                    int length = data.Length + sizeof(ushort) + sizeof(uint) + sizeof(uint);

                    ss.Write((int)length);
                    ss.Write(magic);
                    ss.Write(identifier);
                    ss.Write(data);

                    
                    ss.Flush();

                    using (BinaryReader sr = new BinaryReader(new MemoryStream(tmp.GetBuffer())))
                    {
                        sb =  sr.ReadBytes((int)tmp.Position);
                    }
                }



                return sb;
            }
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

        private byte[] comprJumpwBuffer(byte[] inputs)
        {
            int outputLength = ZLib.compressBound(inputs.Length);
            byte[] outputBuffer = new byte[outputLength];

            if (ZLib.compress(outputBuffer, ref outputLength, inputs, inputs.Length) != ZLibError.Z_OK)
            {
                throw new Exception("compress failed");
            }

            using (BinaryReader sr = new BinaryReader(new MemoryStream(outputBuffer)))
            {
                return sr.ReadBytes(outputLength);
            }
        }


        private void SendBuffer(byte[] buffers, int IsSend)
        {

            COPYDATASTRUCT cds = new COPYDATASTRUCT();

            cds.dwData = (uint)IsSend;

            cds.cbData = buffers.Length;

            cds.lpData = Marshal.AllocHGlobal(buffers.Length);

            Marshal.Copy(buffers, 0, cds.lpData, buffers.Length);

            int hWnd = FindWindow("WWW_JUMPW_COM", null);

            if (hWnd != 0)
            {
                SendMessage(hWnd, WM_COPYDATA, 0, ref cds);
            }

            Marshal.FreeHGlobal(cds.lpData);
        }


        
        private byte[] BuilderJumpwPackage(byte[] s, ushort identifier)
        {
            byte[] rs = null;
            using (MemoryStream stream = new MemoryStream())
            {
                using (BinaryWriter sw = new BinaryWriter(stream))
                {
                    int packLength = s.Length + sizeof(ushort) + sizeof(uint) + sizeof(uint) + sizeof(byte);
                    sw.Write(packLength);
                    sw.Write((int)0);
                    sw.Write(identifier);
                    sw.Write(false);
                    sw.Write(s);
                    sw.Flush();

                    using (BinaryReader sr = new BinaryReader(new MemoryStream(stream.GetBuffer())))
                    {
                        rs = sr.ReadBytes((int)stream.Position);
                    }

                }
            }
            return rs;
        }

        protected void postBuyPotionPackage(int type)
        {
            byte[] buffer = null;

            switch (type)
            {
                case 0:     //狂暴
                    buffer = new byte[] { 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xA3, 0x41, 0x00, 0x96, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00 };
                    break;
                case 1:     //生命
                    buffer = new byte[] { 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xA3, 0x41, 0x00, 0x97, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00 };
                    break;
                case 2:     //吸血
                    buffer = new byte[] { 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xA3, 0x41, 0x00, 0x94, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00 };
                    break;
                case 3:     //隐身
                    buffer = new byte[] { 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xA3, 0x41, 0x00, 0x92, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00 };
                    break;
                case 4:     //急速
                    buffer = new byte[] { 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xA3, 0x41, 0x00, 0x93, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00 };
                    break;
                case 5:		//法力
                    buffer = new byte[] { 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xA3, 0x41, 0x00, 0x95, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00 };
                    break;
            }

            if (buffer != null)
            {
            	SendBuffer(buffer, 1);
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            postBuyPotionPackage(0);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            postBuyPotionPackage(1);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            postBuyPotionPackage(2);
        }

        private void button6_Click(object sender, EventArgs e)
        {
            postBuyPotionPackage(3);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            postBuyPotionPackage(4);
        }

        private void button7_Click(object sender, EventArgs e)
        {
            postBuyPotionPackage(5);
        }

        private void button8_Click(object sender, EventArgs e)
        {
            //getPotionBytes(6);
        }


    }
}

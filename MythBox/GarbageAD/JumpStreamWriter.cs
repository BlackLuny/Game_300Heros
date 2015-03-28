using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace GarbageAD
{
    public class JumpStreamWriter : BinaryWriter
    {
        private uint magic;
        private ushort identifier;

        public JumpStreamWriter(uint magic, ushort identifier) :
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
                byte[] data = ByteHelper.MemoryStreamToBytes((MemoryStream)base.BaseStream);

                int length = data.Length + sizeof(ushort) + sizeof(uint) + sizeof(uint);

                ss.Write((int)length);
                ss.Write(magic);
                ss.Write(identifier);
                ss.Write(data);


                ss.Flush();

                using (BinaryReader sr = new BinaryReader(new MemoryStream(tmp.GetBuffer())))
                {
                    sb = sr.ReadBytes((int)tmp.Position);
                }
            }



            return sb;
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace GarbageAD
{
    class JumpStreamReader : BinaryReader
    {
        public JumpStreamReader(byte[] inputs) :
            base(new MemoryStream(inputs))
        {

        }


        public override string ReadString()
        {
            try
            {
                int length = ReadInt16();
                byte[] sb = ReadBytes(length);

                return Encoding.GetEncoding(936).GetString(sb);
            }
            catch
            {
            }
            return "";
        }
    }
}

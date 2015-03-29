using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace MythBox.Model.Plugin.SDK
{
    public class JumpStreamReader : BinaryReader
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

                string rs = Encoding.GetEncoding(936).GetString(sb);
                return rs.Replace("\0", "");
            }
            catch
            {
            }
            return "";
        }
    }
}

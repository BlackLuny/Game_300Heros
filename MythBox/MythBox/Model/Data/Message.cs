using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace MythBox.Model.Data
{
    public class Message
    {
        /// <summary>
        /// 包长度
        /// </summary>
        public int length;

        /// <summary>
        /// 包标记
        /// </summary>
        public int magic;

        /// <summary>
        /// 包ID
        /// </summary>
        public ushort identifier;

        /// <summary>
        /// 包数据
        /// </summary>
        public byte[] data;

        public byte[] readerData;

        public static Message CreateByData(byte[] data)
        {
            Message msg = new Message();

            try
            {
                msg.data = data;
                using (BinaryReader sr = new BinaryReader(new MemoryStream(data)))
                {
                    msg.length = sr.ReadInt32();
                    msg.magic = sr.ReadInt32();
                    msg.identifier = sr.ReadUInt16();

                    int headerSize = sizeof(int) + sizeof(int) + sizeof(ushort);

                    msg.readerData = sr.ReadBytes(data.Length - headerSize);
                }
            }
            catch
            {
            }

            return msg;
        }
    }
}

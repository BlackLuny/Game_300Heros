using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.IO.Compression;
using System.Diagnostics;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.Serialization;
using ICSharpCode.SharpZipLib.Zip.Compression;
using ICSharpCode.SharpZipLib.Zip.Compression.Streams;

namespace FilePatch
{
    class JumpArchive
    {
        /// <summary>
        /// 无效的数据包文件异常
        /// </summary>
        public class InvalidPackageException : Exception
        {

        }
        /// <summary>
        /// 文件节点
        /// </summary>
        public class FileEntry
        {
            /// <summary>
            /// 文件名
            /// </summary>
            public string filename;
            /// <summary>
            /// 文件在数据结构的偏移
            /// </summary>
            public UInt32 offset;
            /// <summary>
            /// 压缩后大小
            /// </summary>
            public UInt32 compressSize;
            /// <summary>
            /// 压缩前的大小
            /// </summary>
            public UInt32 sourceSize;

            public bool inMemory = false;
            public byte[] data = null;
        }
        /// <summary>
        /// 文件标记头
        /// </summary>
        public class FileHeader
        {
            /// <summary>
            /// 文件头标记
            /// </summary>
            public string hdr;
            /// <summary>
            /// 文件数量
            /// </summary>
            public UInt32 fileCount;
        }

        [Serializable]
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct FileEntryMarshal
        {
            /// <summary>
            /// 文件名
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 260)]
            public char[] filename;
            /// <summary>
            /// 文件在数据结构的偏移
            /// </summary>
            [MarshalAs(UnmanagedType.U4, SizeConst = 4)]
            public UInt32 offset;
            /// <summary>
            /// 压缩后大小
            /// </summary>
            [MarshalAs(UnmanagedType.U4, SizeConst = 4)]
            public UInt32 compressSize;
            /// <summary>
            /// 压缩前的大小
            /// </summary>
            [MarshalAs(UnmanagedType.U4, SizeConst = 4)]
            public UInt32 sourceSize;

            /// <summary>
            /// 保留未用
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public byte[] reserved;
        }

        [Serializable]
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct FileHeaderMarshal
        {
            /// <summary>
            /// 文件头标记
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 50)]
            public char[] hdr;
            /// <summary>
            /// 文件数量
            /// </summary>
            [MarshalAs(UnmanagedType.U4, SizeConst = 4)]
            public UInt32 fileCount;
        }

        FileHeader header = new FileHeader();
        protected FileStream dataFileStream;
        public Dictionary<string, FileEntry> files;

        public JumpArchive()
        {

            files = new Dictionary<string, FileEntry>();
        }

        /// <summary>
        /// 使用zlib压缩一段数据
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        private byte[] compressBuffer(byte[] input)
        {
            byte[] result = null;

            using (MemoryStream outputStream = new MemoryStream())
            {
                using (MemoryStream inputStream = new MemoryStream(input))
                {
                    using (DeflaterOutputStream outZStream = new DeflaterOutputStream(outputStream, new Deflater(Deflater.DEFAULT_COMPRESSION)))
                    {
                        try
                        {
                            inputStream.CopyTo(outZStream);
                            outZStream.Flush();
                            outZStream.Finish();
                        }
                        finally
                        {
                            outputStream.Seek(0, SeekOrigin.Begin);
                            result = new byte[outputStream.Length];
                            outputStream.Read(result, 0, (int)outputStream.Length);
                        }
                    }
                }
            }

            return result;
        }


        /// <summary>
        /// 使用zlib解压一段数据
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        private byte[] uncompressBuffer(byte[] input)
        {
            byte[] result = null;

            using (MemoryStream outputStream = new MemoryStream())
            {
                using (MemoryStream inputStream = new MemoryStream(input))
                {
                    using (InflaterInputStream inputZStream = new InflaterInputStream(inputStream))
                    {
                        try
                        {
                            inputZStream.CopyTo(outputStream);
                            outputStream.Flush();
                        }
                        finally
                        {
                            outputStream.Seek(0, SeekOrigin.Begin);
                            result = new byte[outputStream.Length];
                            outputStream.Read(result, 0, (int)outputStream.Length);
                        }

                    }
                }
            }

            return result;
        }
        /// <summary>
        /// 将文件加入或替换到文件包
        /// </summary>
        /// <param name="filename"></param>
        /// <param name="data"></param>
        public void putFile(string filename, byte[] data)
        {
            if (files.ContainsKey(filename))
            {
                FileEntry entry = files[filename];
                byte[] comressData = compressBuffer(data);
                entry.sourceSize = (uint)data.Length;
                entry.compressSize = (uint)comressData.Length;
                entry.inMemory = true;
                entry.data = comressData;
            }
            else
            {
                FileEntry entry = new FileEntry();
                entry.filename = filename;
                byte[] comressData = compressBuffer(data);
                entry.sourceSize = (uint)data.Length;
                entry.compressSize = (uint)comressData.Length;
                entry.inMemory = true;
                entry.data = comressData;
                files.Add(entry.filename, entry);
            }
        }

        /// <summary>
        /// 从文件包中读取一个文件
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        public byte[] readFile(string filename)
        {
            if (files.ContainsKey(filename))
            {
                FileEntry entry = files[filename];
                if (entry.inMemory)
                {
                    return uncompressBuffer(entry.data);
                }
                else
                {
                    dataFileStream.Seek(entry.offset, SeekOrigin.Begin);
                    byte[] data = new byte[entry.compressSize];
                    dataFileStream.Read(data, 0, (int)entry.compressSize);
                    return uncompressBuffer(data);
                }
            }
            return null;
        }

        private static byte[] StructToBytes(object structObj)
        {
            int size = Marshal.SizeOf(structObj);
            byte[] bytes = new byte[size];

            IntPtr structPtr = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(structObj, structPtr, false);
            Marshal.Copy(structPtr, bytes, 0, size);
            Marshal.FreeHGlobal(structPtr);
            return bytes;
        }

        /// <summary>
        /// 保存文件包数据
        /// </summary>
        public void Save()
        {
            int index = 0;
            int structSize = Marshal.SizeOf(new FileEntryMarshal());
            int headerSize = Marshal.SizeOf(new FileHeaderMarshal());
            int totalHeaderSize = headerSize + structSize * files.Count;
            byte[] temp;

            FileEntryMarshal[] file_list = new FileEntryMarshal[files.Count];
            foreach (var i in files)
            {
                file_list[index].filename = i.Value.filename.PadRight(260, '\0').ToCharArray();
                file_list[index].compressSize = i.Value.compressSize;
                file_list[index].sourceSize = i.Value.sourceSize;
                file_list[index].reserved = new byte[0x20];

                if (!i.Value.inMemory)
                {
                    file_list[index].offset = i.Value.offset;
                    if (file_list[index].offset <= totalHeaderSize)
                    {
                        temp = new byte[file_list[index].compressSize];
                        dataFileStream.Seek(file_list[index].offset, SeekOrigin.Begin);
                        dataFileStream.Read(temp, 0, (int)file_list[index].compressSize);
                        dataFileStream.Seek(0, SeekOrigin.End);
                        file_list[index].offset = (uint)dataFileStream.Position;
                        dataFileStream.Write(temp, 0, temp.Length);
                    }
                }
                else
                {
                    dataFileStream.Seek(0, SeekOrigin.End);
                    file_list[index].offset = (uint)dataFileStream.Position;
                    dataFileStream.Write(i.Value.data, 0, i.Value.data.Length);
                }
                index++;
            }
            FileHeaderMarshal header = new FileHeaderMarshal();
            header.fileCount = (uint)files.Count;
            header.hdr = "DATA1.0".PadRight(50, '\0').ToCharArray();

            for (int i = 0; i < files.Count; i++)
            {
                dataFileStream.Seek(headerSize + (structSize * i), SeekOrigin.Begin);
                temp = StructToBytes(file_list[i]);
                dataFileStream.Write(temp, 0, temp.Length);
            }

            temp = StructToBytes(header);
            dataFileStream.Seek(0, SeekOrigin.Begin);
            dataFileStream.Write(temp, 0, temp.Length);

            dataFileStream.Flush();
        }

        /// <summary>
        /// 打开一个文件包
        /// </summary>
        /// <param name="filepath"></param>
        public void Open(string filepath)
        {
            dataFileStream = File.Open(filepath, FileMode.Open);

            BinaryReader sr = new BinaryReader(dataFileStream);

            byte[] hdr = sr.ReadBytes(50);

            header.hdr = Encoding.Default.GetString(hdr).TrimEnd('\0');
            header.fileCount = sr.ReadUInt32();

            if (header.hdr != "DATA1.0")
            {
                throw new InvalidPackageException();
            }

            for (int i = 0; i < header.fileCount; i++)
            {
                FileEntry file = new FileEntry();
                file.filename = Encoding.Default.GetString(sr.ReadBytes(260)).TrimEnd('\0');
                file.offset = sr.ReadUInt32();
                file.compressSize = sr.ReadUInt32();
                file.sourceSize = sr.ReadUInt32();
                sr.ReadBytes(32);
                files.Add(file.filename, file);
            }
        }
    }
}

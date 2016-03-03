using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;
using ICSharpCode.SharpZipLib.Zip;

using System.Threading;
using System.Runtime.InteropServices;

namespace FilePatch
{
    public partial class Form1 : Form
    {
        private delegate void LogDelegate(string s);
        private delegate void showTipDelegate(string s);
        JumpArchive jumpArc = null;
        public Form1()
        {
            InitializeComponent();
        }
        void ReOpen()
        {
            if (jumpArc != null)
            {
                jumpArc.Close();
            }

            try
            {
                jumpArc = new JumpArchive();
                jumpArc.Open(System.IO.Directory.GetCurrentDirectory() + "\\Data.jmp");
            }
            catch (FileNotFoundException)
            {
                MessageBox.Show("data.jmp不存在，请确认补丁程序是否放置到300英雄目录");
                Application.Exit();
            }
            catch (JumpArchive.InvalidPackageException)
            {
                MessageBox.Show("无效的文件包");
                Application.Exit();
            }
            catch (IOException)
            {
                MessageBox.Show("Data.jmp文件被占用,请确认是否开启了300英雄或补丁程序.");
                Application.Exit();
            }
            catch (Exception ex)
            {
                MessageBox.Show("未知错误" + ex.ToString());

                Application.Exit();
            }
        }
        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
                e.Effect = DragDropEffects.Link;
            else e.Effect = DragDropEffects.None;
        }

        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            textBox1.Text = ((System.Array)e.Data.GetData(DataFormats.FileDrop)).GetValue(0).ToString();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //if (!File.Exists(System.IO.Directory.GetCurrentDirectory() + "\\ICSharpCode.SharpZipLib.dll"))
            //{
            //    MessageBox.Show("ICSharpCode.SharpZipLib.dll不存在。程序无法运行。");
            //    Application.Exit();
            //}

            ReOpen();

            Log("300英雄补丁安装器 V1.4");
            Log("Author:201724");
            Log("Q群:528991906");

            Log("");

            Log("V1.4更新:");
            Log("[+]修复1.3的BUG");
            Log("");

            Log("V1.3更新:");
            Log("[+]修复了文件包内包含重复文件的报错的问题（外团补丁BUG）");
            Log("[+]去除ICSharpCode.SharpZipLib.dll文件");
            Log("[+]现在安装补丁会判断补丁和文件包内的文件是否相等");

            Log("");

            Log("V1.2更新:");
            Log("[!]修复打补丁时未选择文件报错的BUG");
            Log("[!]增加检测ICSharpCode.SharpZipLib.dll文件");

            Log("");

            Log("V1.1更新:");
            Log("[!]修复打补丁日志框闪烁问题");
            Log("[!]修复了多次打补丁会导致文件变大很多倍的问题");
            Log("[+]增加了重建文件包选项");
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                textBox1.Text = openFileDialog1.FileName;
            }
        }

        private void Log(string s)
        {
            if (this.InvokeRequired)
            {
                this.BeginInvoke(new LogDelegate(Log), new object[] { s });
            }
            else
            {
                textBox2.SuspendLayout();
                textBox2.AppendText(s + "\r\n");
                textBox2.SelectionStart = this.textBox2.Text.Length;
                textBox2.ScrollToCaret();
                textBox2.ResumeLayout();
            }
        }

        private void showTip(string s)
        {
            MessageBox.Show(s);
        }
        private void patchThread(object obj)
        {
            FileStream fileStream = (FileStream)obj;
            try
            {
                using (ZipInputStream s = new ZipInputStream(fileStream))
                {
                    ZipEntry theEntry;
                    while ((theEntry = s.GetNextEntry()) != null)
                    {
                        if (theEntry.IsFile)
                        {
                            byte[] buff = new byte[theEntry.Size];
                            s.Read(buff, 0, (int)theEntry.Size);
                            string newName = "..\\" + theEntry.Name.Replace("/", "\\");

                            jumpArc.putFile(newName, buff);
                            Log("补丁文件:" + newName);
                        }
                    }
                    jumpArc.Save();
                    Log("补丁安装成功");
                    this.BeginInvoke(new showTipDelegate(showTip), new object[] { "补丁安装成功" });
                }
            }
            catch (Exception ex)
            {
                this.BeginInvoke(new showTipDelegate(showTip), new object[] { "补丁安装失败" + ex.ToString() });
            }

            BeginInvoke(new rebuildCompleteDelegate(rebuildComplete));
        }
        private void button2_Click(object sender, EventArgs e)
        {
            if (!File.Exists(textBox1.Text))
            {
                MessageBox.Show("补丁文件不存在!", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            FileStream fileStream = File.OpenRead(textBox1.Text);
            Log("开始安装补丁..");
            this.button2.Enabled = false;
            this.button3.Enabled = false;
            new Thread(patchThread).Start(fileStream);

        }
        private void rebuildFilePackage()
        {
            try
            {
                string newFile = System.IO.Directory.GetCurrentDirectory() + "\\Data2.jmp";

                JumpArchive.FileHeaderMarshal header = new JumpArchive.FileHeaderMarshal();
                header.fileCount = (uint)jumpArc.files.Count;
                header.hdr = "DATA1.0".PadRight(50, '\0').ToCharArray();
                int structSize = Marshal.SizeOf(new JumpArchive.FileEntryMarshal());
                int headerSize = Marshal.SizeOf(new JumpArchive.FileHeaderMarshal());
                int dataStart = jumpArc.files.Count * structSize + headerSize;
                JumpArchive.FileEntryMarshal[] file_list = new JumpArchive.FileEntryMarshal[jumpArc.files.Count];
                int i = 0;
                byte[] temp;
                Log("开始重建数据文件....");
                using (FileStream outputStream = File.Open(newFile, FileMode.Create))
                {
                    outputStream.Seek(dataStart, SeekOrigin.Begin);
                    Log("处理文件数据....");
                    foreach (var node in jumpArc.files)
                    {
                        file_list[i].filename = node.Value.filename.PadRight(260, '\0').ToCharArray();
                        file_list[i].compressSize = node.Value.compressSize;
                        file_list[i].sourceSize = node.Value.sourceSize;
                        file_list[i].reserved = new byte[0x20];
                        file_list[i].offset = (uint)outputStream.Position;
                        jumpArc.dataFileStream.Seek(node.Value.offset, SeekOrigin.Begin);
                        temp = new byte[node.Value.compressSize];
                        jumpArc.dataFileStream.Read(temp, 0, (int)node.Value.compressSize);
                        outputStream.Write(temp, 0, (int)node.Value.compressSize);
                        i++;
                    }

                    Log("处理文件头信息....");
                    outputStream.Seek(0, SeekOrigin.Begin);
                    byte[] headerBytes = JumpArchive.StructToBytes(header);
                    outputStream.Write(headerBytes, 0, headerBytes.Length);

                    for (i = 0; i < jumpArc.files.Count; i++)
                    {
                        outputStream.Seek(headerSize + (structSize * i), SeekOrigin.Begin);
                        temp = JumpArchive.StructToBytes(file_list[i]);
                        outputStream.Write(temp, 0, temp.Length);
                    }

                    outputStream.Flush();
                    Log("重建完成....");
                }

                this.jumpArc.Close();
                this.jumpArc = null;
                File.Delete(System.IO.Directory.GetCurrentDirectory() + "\\Data.jmp");
                File.Move(System.IO.Directory.GetCurrentDirectory() + "\\Data2.jmp", System.IO.Directory.GetCurrentDirectory() + "\\Data.jmp");

                ReOpen();
                BeginInvoke(new rebuildCompleteDelegate(rebuildComplete));
            }
            catch (Exception ex)
            {
                MessageBox.Show("未知错误" + ex.ToString());
            }
        }
        private delegate void rebuildCompleteDelegate();
        private void rebuildComplete()
        {
            this.button2.Enabled = true;
            this.button3.Enabled = true;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            this.button2.Enabled = false;
            this.button3.Enabled = false;
            new Thread(rebuildFilePackage).Start();
        }
    }
}

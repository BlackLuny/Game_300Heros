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

namespace FilePatch
{
    public partial class Form1 : Form
    {
        private delegate void LogDelegate(string s);
        JumpArchive jumpArc;
        public Form1()
        {
            InitializeComponent();
            jumpArc = new JumpArchive();
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
            try
            {
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
            catch (Exception ex)
            {
                MessageBox.Show("未知错误" + ex.ToString());

                Application.Exit();
            }

            Log("300英雄补丁安装器 V1.0");
            Log("Author:201724");
            Log("Q群:528991906");
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

                textBox2.Text += s + "\r\n";
                textBox2.SelectionStart = this.textBox2.Text.Length;
                textBox2.ScrollToCaret();
                textBox2.ResumeLayout();
            }
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
                    MessageBox.Show("补丁安装成功！");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("补丁安装失败" + ex.ToString());
            }
        }
        private void button2_Click(object sender, EventArgs e)
        {
            if (!File.Exists(textBox1.Text))
            {
                MessageBox.Show("补丁文件不存在!", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            FileStream fileStream = File.OpenRead(textBox1.Text);
            Log("开始安装补丁..");

            new Thread(patchThread).Start(fileStream);

        }
    }
}

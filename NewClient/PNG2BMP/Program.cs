using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ConsoleApplication1
{
    class Program
    {

        static void Png2Bmp(string file)
        {
            Bitmap myImage;
            using (System.Drawing.Image PngImg = System.Drawing.Image.FromFile(file))
            {
                myImage = myImage = new Bitmap(PngImg.Width, PngImg.Height, PixelFormat.Format32bppArgb);
                using (Graphics g = Graphics.FromImage(myImage))
                {
                    g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
                    g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
                    g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighQuality;

                    g.DrawImage(PngImg, new Rectangle(0, 0, PngImg.Width, PngImg.Height));
                }
            }

            if (File.Exists(file))
            {
                File.Delete(file);
            }

            myImage.Save(file,ImageFormat.Bmp);
        }

        static void GetAllFileByDir(string DirPath)
        {
            //列举出所有文件,添加到AL
            foreach (string file in Directory.GetFiles(DirPath))
            {
                if (file.IndexOf(".png") >= 0)
                {
                    Png2Bmp(file);
                }
            }
            //列举出所有子文件夹,并对之调用GetAllFileByDir自己;
            foreach (string dir in Directory.GetDirectories(DirPath))
                GetAllFileByDir(dir);
        }
        static void Main(string[] args)
        {
            GetAllFileByDir(@"C:\《300英雄》\output");
        }
    }
}

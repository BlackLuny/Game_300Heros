using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Threading;
using System.Diagnostics;
using System.IO.Pipes;

namespace MythBox.Model
{
    class Service
    {
        /// <summary>
        /// 程序配置
        /// </summary>
        public Data.Profile Profile;

        /// <summary>
        /// 游戏后门(Core)通信程序
        /// </summary>
        public Channel.GameChannel Channel;

        /// <summary>
        /// 程序启动路径
        /// </summary>
        public string StartupPath;

        /// <summary>
        /// 插件管理
        /// </summary>
        public Plugin.Manager Plugin;

        /// <summary>
        /// 对象全局实例
        /// </summary>
        private static Service _Instance = new Service();

        /// <summary>
        /// 调试事件信息
        /// </summary>
        public Debug.Events DebugEvents = new Debug.Events();

        public Data.ChannelQueue DelayChannel = new Data.ChannelQueue();

        public Service()
        {
            try
            {
                this.StartupPath = Application.StartupPath + @"\";
                Profile = new Data.Profile(GetDataDirectory() + "data.sqlite");
                Channel = new Channel.GameChannel("300HeroBox", this);
                Plugin = new Plugin.Manager();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        /// <summary>
        /// 获取插件目录
        /// </summary>
        /// <returns></returns>
        public string GetPluginDirectory()
        {
            return this.StartupPath + @"plugins\";
        }
        /// <summary>
        /// 获取数据目录
        /// </summary>
        /// <returns></returns>
        public string GetDataDirectory()
        {
            return this.StartupPath + @"data\";
        }

        /// <summary>
        /// 获取日志目录
        /// </summary>
        /// <returns></returns>
        public string GetLogDirectory()
        {
            return this.StartupPath + @"log\";
        }
        /// <summary>
        /// 获取注入DLL的文件路径
        /// </summary>
        /// <returns></returns>
        public string GetBackdoorPath()
        {
            return this.StartupPath + @"core.dll";
        }

        /// <summary>
        /// 更新游戏目录信息
        /// </summary>
        private void UpdateGamePath()
        {
            string GamePath = Profile.GetValue("system", "GamePath");
            if (GamePath == "")
            {
                try
                {
                    RegistryKey Keys = Registry.CurrentUser.OpenSubKey(@"Software\Jumpw\300HeroClient");

                    GamePath = Keys.GetValue("300HeroPath").ToString();
                    if (GamePath != "")
                    {
                        GamePath += @"\300.exe";
                    }
                }
                catch
                {

                }
            }

            Profile.SetValue("system", "GamePath", GamePath);
        }

        /// <summary>
        /// 游戏DLL监视线程,兼容多游戏
        /// </summary>
        public void UpdateStateThread()
        {
            while (true)
            {
                Process[] processList = Process.GetProcesses();

                foreach (Process process in processList)
                {
                    if (process.ProcessName == "300")
                    {
                        bool isInjected = false;
                        foreach (ProcessModule module in process.Modules)
                        {
                            if (GetBackdoorPath() == module.FileName)
                            {
                                isInjected = true;
                            }
                        }

                        if (isInjected == false)
                        {
                            Game.Backdoor.GameHook(process.Id);
                        }
                    }
                }
                Thread.Sleep(1000);
            }
        }

        /// <summary>
        /// 启动服务
        /// </summary>
        public void Startup()
        {
            UpdateGamePath();
            Plugin.loadPluginFormDirectory(GetPluginDirectory());
            CreateThread(new ThreadStart(UpdateStateThread));

            Channel.Startup();
        }

        /// <summary>
        /// 创建一个线程
        /// </summary>
        /// <param name="threads"></param>
        private void CreateThread(ThreadStart threads)
        {
            Thread ts = new Thread(threads);
            ts.IsBackground = true;
            ts.Start();
        }


        /// <summary>
        /// 获取本对象的实例
        /// </summary>
        /// <returns></returns>
        public static Service GetInstance()
        {
            return _Instance;
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Diagnostics;
using MythBox.Model.Plugin.SDK;
using System.Windows.Forms;

using System.IO;
using System.Text.RegularExpressions;
using Newtonsoft.Json;

namespace GarbageAD
{
    public class MyPlugin : PluginSdk
	{
        /// <summary>
        /// 插件名称
        /// </summary>
        /// 
        public override string pluginName { get { return "垃圾广告屏蔽"; } }

        /// <summary>
        /// 插件版本
        /// </summary>
        public override string pluginVersion { get { return "1.0"; } }

        /// <summary>
        /// 插件作者
        /// </summary>
        public override string pluginAuthor { get { return "201724"; } }

        /// <summary>
        /// 插件别名_内部系统识别插件配置用
        /// </summary>
        public override string pluginCoreName { get { return "garbage_ad"; } }

        public StreamWriter logWriter;

        public static List<string> matchList = new List<string>();

        public void SaveMatch()
        {
            string js = JsonConvert.SerializeObject(matchList);
            SetValue("matchStr", js);
        }
        public void ReadMatch()
        {
            matchList = JsonConvert.DeserializeObject<List<string>>(GetValue("matchStr", "[]"));
        }
		public MyPlugin()
		{
            PreReceiveMessageHandler += new EventHandler<MythBox.Model.Plugin.SDK.Events.MessagePreArgs>(MyPlugin_PreReceiveMessageHandler);
            
		}

        /// <summary>
        /// 插件加载
        /// </summary>
        public override void OnLoadPlugin()
        {
            try
            {
                File.Delete(GetLogDirectory() + @"garbage_ad.log");
                logWriter = new StreamWriter(GetLogDirectory() + @"garbage_ad.log", true, Encoding.GetEncoding(936));
            }
            catch
            {

            }

            ReadMatch();
        }

        public bool IsMatch(string input, string match)
        {
            string format = match.Replace("*", ".*").Replace("?", ".?");
            Regex regex = new Regex(format, RegexOptions.IgnoreCase);
            return regex.Match(input).Success;
        }

        void MyPlugin_PreReceiveMessageHandler(object sender, MythBox.Model.Plugin.SDK.Events.MessagePreArgs e)
        {
            if (e.Message.identifier != 0x2EE2)
                return;

            try
            {
                using (JumpStreamReader sr = new JumpStreamReader(e.Message.readerData))
                {
                    byte unknownA = sr.ReadByte();
                    byte messageType = sr.ReadByte();

                    if (messageType != 0x12)
                        return;

                    sr.ReadBytes(11);   //ignore unknown

                    string playerName = sr.ReadString();

                    byte unknownB = sr.ReadByte();
                    byte unknownC = sr.ReadByte();

                    string chatText = sr.ReadString();

                    lock (logWriter)
                    {
                        try
                        {
                            logWriter.WriteLine(DateTime.Now.ToString() + ":      " + chatText);
                            logWriter.Flush();
                        }
                        catch
                        {

                        }
                    }

                    foreach (string match in matchList)
                    {
                        if (IsMatch(chatText, match) == true)
                        {
                            e.Cancel = true;
                            e.Blocked = true;
                            return;
                        }
                    }
                }
            }
            catch
            {

            }
        }


		/// <summary>
		/// 打开配置窗口
		/// </summary>
		public override void OnShowForm()
		{
			new Form1().ShowDialog();
            SaveMatch();
		}
	}
}

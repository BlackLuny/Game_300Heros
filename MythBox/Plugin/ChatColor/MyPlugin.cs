using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MythBox.Model.Plugin.SDK;
using System.Windows.Forms;

namespace ChatColor
{
    public class MyPlugin : PluginSdk
    {
        /// <summary>
        /// 插件名称
        /// </summary>
        /// 
        public override string pluginName { get { return "聊天颜色"; } }

        /// <summary>
        /// 插件版本
        /// </summary>
        public override string pluginVersion { get { return "1.1"; } }

        /// <summary>
        /// 插件作者
        /// </summary>
        public override string pluginAuthor { get { return "201724"; } }

        /// <summary>
        /// 插件别名_内部系统识别插件配置用
        /// </summary>
        public override string pluginCoreName { get { return "chat_color"; } }

        public static List<string> matchList = new List<string>();

        public string chatColor = "FFFFFFFF";

        public MyPlugin()
        {
            PreSendMessageHandler += new EventHandler<MythBox.Model.Plugin.SDK.Events.MessagePreArgs>(MyPlugin_PreSendMessageHandler);
        }

        void MyPlugin_PreSendMessageHandler(object sender, MythBox.Model.Plugin.SDK.Events.MessagePreArgs e)
        {
            if (e.Message.identifier == 0x2EE2)
            {
                try
                {
                    using (JumpStreamReader sr = new JumpStreamReader(e.Message.readerData))
                    {
                        byte unknownA = sr.ReadByte();
                        byte messageType = sr.ReadByte();

                        byte[] fillByteA = sr.ReadBytes(8);   //ignore unknown
                        string targetPlayer = sr.ReadString();
                        string playerName = sr.ReadString();

                        byte unknownB = sr.ReadByte();
                        byte unknownC = sr.ReadByte();

                        string chatText = sr.ReadString();
                        byte[] endBytes = sr.ReadBytes(12);

                        using (JumpStreamWriter sw = new JumpStreamWriter(e.Message.magic, e.Message.identifier))
                        {
                            sw.Write(unknownA);
                            sw.Write(messageType);
                            sw.Write(fillByteA);
                            sw.Write(targetPlayer);
                            sw.Write(playerName);

                            sw.Write(unknownB);
                            sw.Write(unknownC);

                            sw.Write(@"[color=" + chatColor + @"]" + chatText);
                            sw.Write(endBytes);

                            byte[] sb = sw.GetBuffer();
                            e.result = sb;
                        }
                        
                    }
                }
                catch
                {

                }
            }
        }

        /// <summary>
        /// 插件加载
        /// </summary>
        public override void OnLoadPlugin()
        {
            chatColor = GetValue("ChatColor", "FFFFFF00");
        }


        /// <summary>
        /// 打开配置窗口
        /// </summary>
        public override void OnShowForm()
        {
            ColorDialog cd = new ColorDialog();

            cd.AllowFullOpen = true;
            if (cd.ShowDialog() == DialogResult.OK)
            {
                chatColor = String.Format("{0:X2}{1:X2}{2:X2}{3:X2}", cd.Color.A, cd.Color.R, cd.Color.G, cd.Color.B);
                SetValue("ChatColor", chatColor);
            }
        }
    }
}

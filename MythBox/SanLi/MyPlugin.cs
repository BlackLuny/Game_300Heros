using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MythBox.Model.Plugin.SDK;


namespace SanLi
{
    public class MyPlugin : PluginSdk
    {
        /// <summary>
        /// 插件名称
        /// </summary>
        /// 
        public override string pluginName { get { return "三笠自动W"; } }

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
        public override string pluginCoreName { get { return "sanli_w"; } }


        public MyPlugin()
        {
            PreReceiveMessageHandler += new EventHandler<MythBox.Model.Plugin.SDK.Events.MessagePreArgs>(MyPlugin_PreReceiveMessageHandler);
        }

        /// <summary>
        /// 插件加载
        /// </summary>
        public override void OnLoadPlugin()
        {

        }

        void MyPlugin_PreReceiveMessageHandler(object sender, MythBox.Model.Plugin.SDK.Events.MessagePreArgs e)
        {
            if (e.Message.identifier == 0x2AFF)
            {
                //e.Blocked = true;
                e.Cancel = true;
                return;
            }
            if (e.Message.identifier == 0x69E8)
            {
                //e.Blocked = true;
                e.Cancel = true;
                return;
            }
            if (e.Message.identifier == 0x69E1)
            {
                //e.Blocked = true;
                e.Cancel = true;
                return;
            }

            if (e.Message.identifier == 0x0415)
            {
                //e.Blocked = true;
                e.Cancel = true;
                return;
            }
            if (e.Message.identifier == 0x0566)
            {
                //e.Blocked = true;
                e.Cancel = true;
                return;
            }
            //

            if (e.Message.identifier != 0x2B19)
                return;

            try
            {
                using (JumpStreamReader sr = new JumpStreamReader(e.Message.readerData))
                {
                    byte unknownA = sr.ReadByte();
                    byte unknownB = sr.ReadByte();
                    ushort skillId = sr.ReadUInt16();

                    if (skillId == 0x9D08)
                    {
                        SendMessage(e.sender, new byte[] { 0x25, 0x00, 0x00, 0x00, 0x13, 0x53, 0x08, 0xA0, 0xFA, 0x2A, 0x00, 0x13, 0x53, 0x08, 0xA0, 0xFF, 0xFF, 0xFF, 0xFF, 0x95, 0x0B, 0x33, 0x8E, 0x4C, 0x43, 0x78, 0x10, 0x3F, 0x43, 0x93, 0x57, 0xD1, 0xBE, 0xA2, 0x9F, 0x69, 0x3F });
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
        }
    }
}

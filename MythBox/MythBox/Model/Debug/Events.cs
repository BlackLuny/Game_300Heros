using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MythBox.Model.Debug
{
    public class Events
    {
        /// <summary>
        /// 调用游戏之后处理收到消息
        /// </summary>
        public event EventHandler<Plugin.SDK.Events.MessagePostArgs> PostReceiveMessageHandler;

        /// <summary>
        /// 调用游戏之后处理发送消息
        /// </summary>
        public event EventHandler<Plugin.SDK.Events.MessagePostArgs> PostSendMessageHandler;

        /// <summary>
        /// 事件处理
        /// </summary>
        /// <param name="e"></param>
        public void OnEvent(Plugin.SDK.Events.PluginArgs e)
        {
            try
            {
                switch (e.Event)
                {
                    case Plugin.SDK.PluginEvent.PostReceiveMessage:
                        OnPostReceiveMessage(e);
                        break;
                    case Plugin.SDK.PluginEvent.PostSendMessage:
                        OnPostSendMessage(e);
                        break;
                }
            }
            catch
            {

            }
        }


        /// <summary>
        /// 调用游戏之后处理收到消息
        /// </summary>
        /// <param name="e"></param>
        private void OnPostReceiveMessage(Plugin.SDK.Events.PluginArgs e)
        {
            if (PostReceiveMessageHandler != null)
            {
                PostReceiveMessageHandler(this, (Plugin.SDK.Events.MessagePostArgs)e);
            }
        }

        /// <summary>
        /// 调用游戏之后处理发送消息
        /// </summary>
        /// <param name="e"></param>
        private void OnPostSendMessage(Plugin.SDK.Events.PluginArgs e)
        {
            if (PostSendMessageHandler != null)
            {
                PostSendMessageHandler(this, (Plugin.SDK.Events.MessagePostArgs)e);
            }
        }
    }
}

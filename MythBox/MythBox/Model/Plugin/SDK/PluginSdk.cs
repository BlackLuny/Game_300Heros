using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MythBox.Model.Plugin.SDK
{
    /// <summary>
    /// 插件抽象类
    /// </summary>
    public abstract class PluginSdk : PluginInterface
    {
        public PluginSdk()
        {
            if (GetValue("IsEnabled", "Enable") == "Enable")
            {
                IsEnabled = true;
            }
            else
            {
                IsEnabled = false;
            }
        }

        public void SetEnabled(bool IsEnabled)
        {
            if (!IsEnabled)
            {
                SetValue("IsEnabled", "Disable");
            }
            else
            {
                SetValue("IsEnabled", "Enable");
            }

            if (GetValue("IsEnabled", "Enable") == "Enable")
            {
                this.IsEnabled = true;
            }
            else
            {
                this.IsEnabled = false;
            }
        }

        /// <summary>
        /// 插件加载
        /// </summary>
        public abstract void OnLoadPlugin();

        /// <summary>
        /// 打开配置窗口
        /// </summary>
        public abstract void OnShowForm();

        /// <summary>
        /// 获取配置信息
        /// </summary>
        /// <param name="key"></param>
        /// <param name="def"></param>
        /// <returns></returns>
        public string GetValue(string key, string def = "")
        {
            return Service.GetInstance().Profile.GetValue(pluginCoreName, key, def);
        }

        /// <summary>
        /// 设置配置信息
        /// </summary>
        /// <param name="key"></param>
        /// <param name="value"></param>
        public void SetValue(string key, string value)
        {
            Service.GetInstance().Profile.SetValue(pluginCoreName, key, value);
        }


        /// <summary>
        /// 让游戏发送一个消息到服务器
        /// </summary>
        /// <param name="ch"></param>
        /// <param name="Message"></param>
        public void SendMessage(object ch, byte[] Message)
        {
            Model.Service.GetInstance().Channel.PKT_UserSendMessage((Channel.MessageChannel.ChannelInstance)ch, Message);
        }


        /// <summary>
        /// 给游戏本地发送一个消息
        /// </summary>
        /// <param name="ch"></param>
        /// <param name="Message"></param>
        public void ReceiveMessage(object ch,byte[] Message)
        {
            Model.Service.GetInstance().Channel.PKT_UserReceiveMessage((Channel.MessageChannel.ChannelInstance)ch, Message);
        }


        /// <summary>
        /// 获取插件目录
        /// </summary>
        /// <returns></returns>
        public string GetPluginDirectory()
        {
            return Model.Service.GetInstance().StartupPath + @"plugins\";
        }
        /// <summary>
        /// 获取数据目录
        /// </summary>
        /// <returns></returns>
        public string GetDataDirectory()
        {
            return Model.Service.GetInstance().StartupPath + @"data\";
        }

        /// <summary>
        /// 获取日志目录
        /// </summary>
        /// <returns></returns>
        public string GetLogDirectory()
        {
            return Model.Service.GetInstance().StartupPath + @"log\";
        }


        /// <summary>
        /// 事件处理
        /// </summary>
        /// <param name="e"></param>
        public void OnEvent(Events.PluginArgs e)
        {
            switch (e.Event)
            {
                case PluginEvent.PreReceiveMessage:
                    OnPreReceiveMessage(e);
                    break;
                case PluginEvent.PreSendMessage:
                    OnPostSendMessage(e);
                    break;
                case PluginEvent.PostReceiveMessage:
                    OnPostReceiveMessage(e);
                    break;
                case PluginEvent.PostSendMessage:
                    OnPostSendMessage(e);
                    break;
                case PluginEvent.Update:
                    OnUpdateTimer(e);
                    break;
            }
        }


        /// <summary>
        /// 调用游戏之前处理收到消息
        /// </summary>
        /// <param name="e"></param>
        private void OnPreReceiveMessage(Events.PluginArgs e)
        {
            if (PreReceiveMessageHandler != null)
            {
                PreReceiveMessageHandler(this, (Events.MessagePreArgs)e);
            }
        }

        /// <summary>
        /// 调用游戏之后处理收到消息
        /// </summary>
        /// <param name="e"></param>
        private void OnPostReceiveMessage(Events.PluginArgs e)
        {
            if (PostReceiveMessageHandler != null)
            {
                PostReceiveMessageHandler(this, (Events.MessagePostArgs)e);
            }
        }


        /// <summary>
        /// 调用游戏之前处理发送消息
        /// </summary>
        /// <param name="e"></param>
        private void OnPreSendMessage(Events.PluginArgs e)
        {
            if (PreSendMessageHandler != null)
            {
                PreSendMessageHandler(this, (Events.MessagePreArgs)e);
            }
        }

        /// <summary>
        /// 调用游戏之后处理发送消息
        /// </summary>
        /// <param name="e"></param>
        private void OnPostSendMessage(Events.PluginArgs e)
        {
            if (PostSendMessageHandler != null)
            {
                PostSendMessageHandler(this, (Events.MessagePostArgs)e);
            }
        }

        /// <summary>
        /// 调用游戏之后处理发送消息
        /// </summary>
        /// <param name="e"></param>
        private void OnUpdateTimer(Events.PluginArgs e)
        {
            if (UpdateTimerHandler != null)
            {
                UpdateTimerHandler(this, (Events.UpdateArgs)e);
            }
        }

        //

        /// <summary>
        /// 插件名称
        /// </summary>
        public virtual string pluginName { get { return ""; } }

        /// <summary>
        /// 插件版本
        /// </summary>
        public virtual string pluginVersion { get { return ""; } }

        /// <summary>
        /// 插件作者
        /// </summary>
        public virtual string pluginAuthor { get { return ""; } }

        /// <summary>
        /// 插件别名_内部系统识别插件配置用
        /// </summary>
        public virtual string pluginCoreName { get { return ""; } }

        /// <summary>
        /// 调用游戏之前处理收到消息
        /// </summary>
        public event EventHandler<Events.MessagePreArgs> PreReceiveMessageHandler;

        /// <summary>
        /// 调用游戏之后处理收到消息
        /// </summary>
        public event EventHandler<Events.MessagePostArgs> PostReceiveMessageHandler;

        /// <summary>
        /// 调用游戏之前处理发送消息
        /// </summary>
        public event EventHandler<Events.MessagePreArgs> PreSendMessageHandler;

        /// <summary>
        /// 调用游戏之后处理发送消息
        /// </summary>
        public event EventHandler<Events.MessagePostArgs> PostSendMessageHandler;

        /// <summary>
        /// 游戏实时更新时钟
        /// </summary>
        public event EventHandler<Events.UpdateArgs> UpdateTimerHandler;

        /// <summary>
        /// 检查插件是否开启
        /// </summary>
        public bool IsEnabled;
    }
}

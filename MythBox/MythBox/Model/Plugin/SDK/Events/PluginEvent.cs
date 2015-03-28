using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MythBox.Model.Plugin.SDK
{
    public enum PluginEvent
    {
        /// <summary>
        /// 预处理收到的封包信息
        /// </summary>
        PreReceiveMessage,
        /// <summary>
        /// 投递已经收到的封包信息
        /// </summary>
        PostReceiveMessage,
        /// <summary>
        /// 预处理即将发送的封包信息
        /// </summary>
        PreSendMessage,
        /// <summary>
        /// 投递已经发送完成的封包信息
        /// </summary>
        PostSendMessage,

        /// <summary>
        /// 游戏定期更新缓存数据
        /// </summary>
        Update,
    }
}

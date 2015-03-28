using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MythBox.Model.Plugin.SDK.Events
{
    public class PluginArgs : EventArgs
    {
        /// <summary>
        /// 事件
        /// </summary>
        public PluginEvent Event;

        /// <summary>
        /// 终止其他插件执行
        /// </summary>
        public bool Cancel = false;

        public object sender;


        public PluginArgs(object sender)
        {
            this.sender = sender;
        }
    }
}

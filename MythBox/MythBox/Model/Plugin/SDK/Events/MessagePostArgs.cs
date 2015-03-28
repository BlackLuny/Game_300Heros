using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MythBox.Model.Plugin.SDK.Events
{
    public class MessagePostArgs : PluginArgs
    {
        public byte[] messages;
        public Model.Data.Message Message;

        public MessagePostArgs(byte[] messages,object sender):
            base(sender)
        {
            this.messages = messages;

            Message = Model.Data.Message.CreateByData(messages);
        }
    }
}

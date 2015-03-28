using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MythBox.Model.Plugin.SDK.Events
{
    public class MessagePreArgs : PluginArgs
    {
        public byte[] messages;
        public byte[] result;
        public bool Blocked;

        public Model.Data.Message Message;

        public MessagePreArgs(byte[] messages,object sender):
            base(sender)
        {
            this.messages = messages;
            this.result = messages;
            Blocked = false;
            Message = Model.Data.Message.CreateByData(messages);
        }

        public void SetResult(byte[] result)
        {
            this.result = result;
        }
    }
}

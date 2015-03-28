using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.Pipes;
namespace MythBox.Model.Data
{
    public class ChannelQueue
    {
        public Queue<MessageSender> senderQueue;

        public ChannelQueue()
        {
            senderQueue = new Queue<MessageSender>();
        }

        public void PutQueue(MessageSender sender)
        {
            lock (senderQueue)
            {
                senderQueue.Enqueue(sender);
            }
        }

        public MessageSender GetQueue()
        {
            lock (senderQueue)
            {
                if (senderQueue.Count > 0)
                {
                    return senderQueue.Dequeue();
                }
            }
            return null;
        }
    }
}

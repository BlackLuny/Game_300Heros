using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.IO;
using System.IO.Pipes;

using System.Threading;
using System.Diagnostics;


namespace MythBox.Model.Channel
{
    public class MessageChannel
    {
        private const byte CH_CALL = 0x01;
        private const byte CH_RET = 0x2;

        public class ChannelInstance
        {
            public NamedPipeServerStream channel;
            public BinaryReader sr;
            public BinaryWriter sw;
        }

        public class APICaller
        {
            public string apiName;
            public byte[] param;
            public byte[] result = new byte[0];
        }

        private delegate void CH_READMESSAGE_FUNC(IAsyncResult ar);

        private string channelName;


        public MessageChannel(string channelName)
        {
            this.channelName = channelName;
        }


        private Byte[] GetMessage(ChannelInstance ch)
        {
            while (true)
            {
                if (ch.sr.ReadByte() == CH_RET)
                {
                    int rs_len = ch.sr.ReadInt32();
                    byte[] rs = ch.sr.ReadBytes(rs_len);
                    return rs;
                }

                APICaller caller = new APICaller();
                byte[] apiNameBytes = ch.sr.ReadBytes(32);
                int paramLength = ch.sr.ReadInt32();
                byte[] paramBytes = new byte[0];
                if (paramLength > 0)
                {
                    paramBytes = ch.sr.ReadBytes(paramLength);
                }

                caller.apiName = Encoding.GetEncoding(936).GetString(apiNameBytes);
                caller.apiName = caller.apiName.Replace("\0", "");
                caller.param = paramBytes;

                NotifyCallerProcedure(ch, caller);

                ch.sw.Write(CH_RET);
                ch.sw.Write(caller.result.Length);
                if (caller.result.Length > 0)
                {
                    ch.sw.Write(caller.result);
                }
            }
        }

        public void NamedPipeWorkerThread(object e)
        {
            NamedPipeServerStream channel = e as NamedPipeServerStream;
            try
            {
                using (BinaryReader sr = new BinaryReader(channel))
                {
                    using (BinaryWriter sw = new BinaryWriter(channel))
                    {
                        ChannelInstance ch = new ChannelInstance();
                        ch.channel = channel;
                        ch.sr = sr;
                        ch.sw = sw;

                        NotifyCreateConnect(ch);
                        

                        while (true)
                        {
                            GetMessage(ch);
                        }
                    }
                }
            }
            catch (System.IO.EndOfStreamException)
            {
                ChannelInstance ch = new ChannelInstance();
                ch.channel = channel;

                NotifyCloseConnect(ch);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }

            channel.Close();
        }
        public void Startup()
        {
            NamedPipeServerStream channel = new NamedPipeServerStream(channelName, PipeDirection.InOut, NamedPipeServerStream.MaxAllowedServerInstances, PipeTransmissionMode.Byte, PipeOptions.Asynchronous);

            channel.BeginWaitForConnection(new AsyncCallback(delegate(IAsyncResult ar)
            {
                channel.EndWaitForConnection(ar);

                Thread t = new Thread(new ParameterizedThreadStart(NamedPipeWorkerThread));
                t.IsBackground = true;
                t.Start(channel);

                Startup();
            }), channel);
        }

        public void CallApi(ChannelInstance ch, string apiName, byte[] param, out byte[] output)
        {
            ch.sw.Write(CH_CALL);
            Byte[] s = Encoding.GetEncoding(936).GetBytes(apiName);
            Byte[] apiNameBytes = new Byte[32];
            for (int i = 0; i < s.Length; i++)
            {
                apiNameBytes[i] = s[i];
            }

            ch.sw.Write(apiNameBytes);

            ch.sw.Write(param.Length);

            if (param.Length > 0)
            {
                ch.sw.Write(param);
            }

            output = GetMessage(ch);
        }

        public virtual void NotifyCallerProcedure(ChannelInstance ch, APICaller caller)
        {

        }

        public virtual void NotifyCreateConnect(ChannelInstance ch)
        {

        }
        public virtual void NotifyCloseConnect(ChannelInstance ch)
        {

        }
    }
}

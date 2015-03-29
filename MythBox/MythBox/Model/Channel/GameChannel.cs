using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
namespace MythBox.Model.Channel
{
    class GameChannel : MessageChannel
    {
        private Service sv;

        public GameChannel(string channelName, Service sv) :
            base(channelName)
        {
            this.sv = sv;

        }

        public static MemoryStream MemoryStreamToReaderStream(MemoryStream memoryStream)
        {
            return new MemoryStream(memoryStream.GetBuffer(), 0, (int)memoryStream.Length);
        }
        public static byte[] GetBytes(Stream memoryStream)
        {
            return new BinaryReader(MemoryStreamToReaderStream((MemoryStream)memoryStream)).ReadBytes((int)((MemoryStream)memoryStream).Length);
        }


        public override void NotifyCallerProcedure(ChannelInstance ch, APICaller caller)
        {
            try
            {
                switch (caller.apiName)
                {
                    case "PKT_PreSendMessage":
                        PKT_PreSendMessage(ch, caller);
                        break;
                    case "PKT_PostSendMessage":
                        PKT_PostSendMessage(ch, caller);
                        break;
                    case "PKT_PreReceiveMessage":
                        PKT_PreReceiveMessage(ch, caller);
                        break;
                    case "PKT_PostReceiveMessage":
                        PKT_PostReceiveMessage(ch, caller);
                        break;
                    case "SYS_Update":
                        SYS_Update(ch, caller);
                        break;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        public override void NotifyCreateConnect(MessageChannel.ChannelInstance ch)
        {

        }

        public override void NotifyCloseConnect(MessageChannel.ChannelInstance ch)
        {

        }

        public void PKT_PreSendMessage(ChannelInstance ch, APICaller caller)
        {

            foreach (Plugin.SDK.PluginSdk plugin in sv.Plugin.pluginList)
            {
                if (!plugin.IsEnabled)
                    continue;

                try
                {
                    Plugin.SDK.Events.MessagePreArgs e = new Plugin.SDK.Events.MessagePreArgs(caller.param, ch);
                    e.Event = Plugin.SDK.PluginEvent.PreSendMessage;

                    plugin.OnEvent(e);



                    using (BinaryWriter output = new BinaryWriter(new MemoryStream()))
                    {
                        output.Write(Convert.ToInt32(e.Blocked));
                        output.Write(e.result);

                        output.Flush();

                        caller.result = GetBytes(output.BaseStream);
                    }

                    caller.param = e.result;

                    if (e.Cancel == true)
                    {
                        return;
                    }
                }
                catch
                {

                }
            }

            Plugin.SDK.Events.MessagePostArgs notif = new Plugin.SDK.Events.MessagePostArgs(caller.param, ch);
            notif.Event = Plugin.SDK.PluginEvent.PostSendMessage;
            sv.DebugEvents.OnEvent(notif);
        }

        public void PKT_PostSendMessage(ChannelInstance ch, APICaller caller)
        {
            Plugin.SDK.Events.MessagePostArgs e;

            foreach (Plugin.SDK.PluginSdk plugin in sv.Plugin.pluginList)
            {
                if (!plugin.IsEnabled)
                    continue;

                try
                {
                    e = new Plugin.SDK.Events.MessagePostArgs(caller.param, ch);
                    e.Event = Plugin.SDK.PluginEvent.PostSendMessage;


                    plugin.OnEvent(e);


                    if (e.Cancel == true)
                    {
                        return;
                    }
                }
                catch
                {

                }
            }


        }

        public void PKT_PreReceiveMessage(ChannelInstance ch, APICaller caller)
        {
            foreach (Plugin.SDK.PluginSdk plugin in sv.Plugin.pluginList)
            {
                if (!plugin.IsEnabled)
                    continue;

                try
                {
                    Plugin.SDK.Events.MessagePreArgs e = new Plugin.SDK.Events.MessagePreArgs(caller.param, ch);
                    e.Event = Plugin.SDK.PluginEvent.PreReceiveMessage;


                    plugin.OnEvent(e);

                    using (BinaryWriter output = new BinaryWriter(new MemoryStream()))
                    {
                        output.Write(Convert.ToInt32(e.Blocked));
                        output.Write(e.result);

                        output.Flush();

                        caller.result = GetBytes(output.BaseStream);
                    }

                    caller.param = e.result;

                    if (e.Cancel == true)
                    {
                        return;
                    }
                }
                catch
                {

                }
            }


            Plugin.SDK.Events.MessagePostArgs notif = new Plugin.SDK.Events.MessagePostArgs(caller.param, ch);
            notif.Event = Plugin.SDK.PluginEvent.PostReceiveMessage;
            sv.DebugEvents.OnEvent(notif);

        }

        public void PKT_PostReceiveMessage(ChannelInstance ch, APICaller caller)
        {
            Plugin.SDK.Events.MessagePostArgs e;

            foreach (Plugin.SDK.PluginSdk plugin in sv.Plugin.pluginList)
            {
                if (!plugin.IsEnabled)
                    continue;

                try
                {
                    e = new Plugin.SDK.Events.MessagePostArgs(caller.param, ch);
                    e.Event = Plugin.SDK.PluginEvent.PostReceiveMessage;

                    plugin.OnEvent(e);


                    if (e.Cancel == true)
                    {
                        return;
                    }
                }
                catch
                {

                }
            }



        }

        public void SYS_Update(ChannelInstance ch, APICaller caller)
        {
            Plugin.SDK.Events.UpdateArgs e;

            foreach (Plugin.SDK.PluginSdk plugin in sv.Plugin.pluginList)
            {
                if (!plugin.IsEnabled)
                    continue;

                try
                {
                    e = new Plugin.SDK.Events.UpdateArgs(ch);

                    plugin.OnEvent(e);

                    if (e.Cancel == true)
                    {
                        return;
                    }
                }
                catch
                {

                }
            }
            try
            {
                Data.MessageSender sender = null;
                sender = sv.DelayChannel.GetQueue();

                while (sender != null)
                {
                    if (sender.IsSend == 1)
                    {
                        PKT_UserSendMessage(ch, sender.data);
                    }
                    else
                    {
                        PKT_UserReceiveMessage(ch, sender.data);
                    }

                    sender = sv.DelayChannel.GetQueue();
                }
            }
            catch
            {

            }
        }

        public void PKT_UserSendMessage(ChannelInstance ch, byte[] data)
        {
            byte[] output;
            CallApi(ch, "PKT_UserSendMessage", data, out output);
        }

        public void PKT_UserReceiveMessage(ChannelInstance ch, byte[] data)
        {
            byte[] output;
            CallApi(ch, "PKT_UserReceiveMessage", data, out output);
        }
    }
}

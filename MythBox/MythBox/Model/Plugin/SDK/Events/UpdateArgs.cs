using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MythBox.Model.Plugin.SDK.Events
{
    public class UpdateArgs : PluginArgs
    {
        public UpdateArgs(object sender):
            base(sender)
        {
            Event = PluginEvent.Update;
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Reflection;

using MythBox.Model.Plugin.SDK;

namespace MythBox.Model.Plugin
{
    class Manager
    {
        public List<PluginSdk> pluginList = new List<PluginSdk>();

        /// <summary>
        /// 加载一个插件
        /// </summary>
        /// <param name="pluginPath">插件路径</param>
        public void LoadPlugin(string pluginPath)
        {
            using (FileStream fs = new FileStream(pluginPath, FileMode.Open, FileAccess.Read))
            {
                using (BinaryReader br = new BinaryReader(fs))
                {
                    byte[] pluginBytes = br.ReadBytes((int)fs.Length);

                    Assembly ab = Assembly.Load(pluginBytes);
                    Type[] types = ab.GetTypes();
                    foreach (Type t in types)
                    {
                        if (t.GetInterface("PluginInterface") != null)
                        {
                            PluginSdk pl = (PluginSdk)ab.CreateInstance(t.FullName);
                            pl.OnLoadPlugin();
                            pluginList.Add(pl);
                        }
                    }
                }
            }
        }

        public void loadPluginFormDirectory(string pluginDirecotry)
        {
            string[] files = Directory.GetFiles(pluginDirecotry);

            foreach (string file in files)
            {
                if (file.EndsWith(".DLL", StringComparison.OrdinalIgnoreCase))
                {
                    try
                    {
                        LoadPlugin(file);
                    }
                    catch
                    {

                    }
                }
            }
        }

    }
}

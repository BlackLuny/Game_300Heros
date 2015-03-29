using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace MythBox.Model.Game
{
    class Backdoor
    {
        /// <summary>
        /// 注入DLL到游戏
        /// </summary>
        /// <param name="processId"></param>
        /// <returns></returns>
        [DllImport("core.dll", EntryPoint = "GameHook")]
        public static extern bool GameHook(int processId);
    }
}

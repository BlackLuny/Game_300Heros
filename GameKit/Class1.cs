using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace GameKit
{
	public class Windows
	{
		public const int WM_COPYDATA = 0x004A;
	}


	[StructLayout(LayoutKind.Sequential)]
	public struct COPYDATASTRUCT
	{
		public uint dwData;
		public int cbData;
		public IntPtr lpData;
	}

	public class GameMessageData
	{
		public byte[] buffer;
		public uint type;
	}

	public struct GamePacketTypes
	{
		public int length;
		public uint timestamp;
		public ushort identifier;
	}
}

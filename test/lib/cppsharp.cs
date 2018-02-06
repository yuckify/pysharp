using System;
using System.Runtime.CompilerServices;

namespace cppsharp
{
	public class Object
	{
		//[MethodImplAttribute(MethodImplOptions.InternalCall)]
		//protected extern static void _cppsharp_Object_setMonoObject(IntPtr __arg, IntPtr obj);

		//protected IntPtr CppHandle { get { return __handle; } set { __handle = value; } }
		//protected bool CppFree { get { return __free; } set { __free = value; } }
		public virtual void OnCreate() { Console.WriteLine("OnCreate()"); }
		public virtual void OnUpdate() {}

		protected IntPtr CppHandle;
		protected bool CppFree;
	}
}


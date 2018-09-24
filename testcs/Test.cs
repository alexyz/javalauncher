using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    class Test
    {

        [DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr Create(IntPtr dir, IntPtr[] jreargs, int log);
		
		[DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr Run(IntPtr mainclassname, IntPtr[] mainargs, int log);
		
		[DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr Destroy(int log);

        public static IntPtr[] StringArrayToBSTRArray(string[] a)
        {
            IntPtr[] b = new IntPtr[a.Length + 1];
            for (int n = 0; n < a.Length; n++)
            {
                b[n] = Marshal.StringToBSTR(a[n]);
            }
            b[b.Length - 1] = IntPtr.Zero;
            return b;
        }

        private static void FreeBSTRArray (IntPtr[] a)
        {
            for (int n = 0; n < a.Length; n++)
            {
                if (a[n] != null && a[n] != IntPtr.Zero)
                {
                    Marshal.FreeBSTR(a[n]);
                }
            }
        }

		public static string Create2 (string jredir, string[] jreargs, bool log)
        {
            IntPtr a = Marshal.StringToBSTR(jredir);
            IntPtr[] b = StringArrayToBSTRArray(jreargs);
            IntPtr v = Create(a, b, log ? 1 : 0);
            Marshal.FreeBSTR(a);
            FreeBSTRArray(b);
            return v != null && v != IntPtr.Zero ? Marshal.PtrToStringBSTR(v) : null;
        }
		
		public static string Run2 (string mainclassname, string[] mainargs, bool log)
        {
            IntPtr c = Marshal.StringToBSTR(mainclassname);
            IntPtr[] d = StringArrayToBSTRArray(mainargs);
            IntPtr v = Run(c, d, log ? 1 : 0);
            Marshal.FreeBSTR(c);
            FreeBSTRArray(d);
            return v != null && v != IntPtr.Zero ? Marshal.PtrToStringBSTR(v) : null;
        }
		
		public static string Destroy2 (bool log)
        {
            IntPtr v = Destroy(log ? 1 : 0);
            return v != null && v != IntPtr.Zero ? Marshal.PtrToStringBSTR(v) : null;
        }

        static void Main(string[] args)
        {
            Console.WriteLine("Main");

            string dir = null;
            List<string> options = new List<string>();
            string mainclassname = null;
            List<string> mainargs = new List<string>();

            for (int n = 0; n < args.Length; n++) {
                if (n == 0) {
                    Console.WriteLine("Main: jvm dir = " + args[n]);
                    dir = args[n];
                } else if (args[n].StartsWith("-D")) {
                    Console.WriteLine("Main: jvm option = " + args[n]);
                    options.Add(args[n]);
                } else if (mainclassname == null) {
                    Console.WriteLine("Main: main class = " +  args[n]);
                    mainclassname = args[n];
                } else {
                    Console.WriteLine("Main: main class arg = " + args[n]);
                    mainargs.Add(args[n]);
                }
            }

            string e = Create2(dir, options.ToArray(), true);
			if (e != null) {
				Console.WriteLine("Main: create = " + e);
				return;
			}

			for (int n = 0; n < 2; n++) {
				e = Run2(mainclassname, mainargs.ToArray(), true);
				if (e != null) {
					Console.WriteLine("Main: run = " + e);
					return;
				}
			}

			e = Destroy2(true);
			if (e != null) {
				Console.WriteLine("Main: destroy = " + e);
				return;
			}
				
            Console.WriteLine("Main exit");
        }

    }
}

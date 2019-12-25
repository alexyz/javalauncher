using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace test
{
    /// <summary>
    /// C:\Dev\test\Project1\Project2\bin\Release>LauncherTest.exe 1 "c:\Dev\openjdk-11+28" C:\Dev\GitHub\javalauncher\java Hello 1 2 3
    /// </summary>
    public class Test2
    {
		
		[DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static void AlexSetLog(int log);

        [DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static int AlexCreateVm(IntPtr dir, IntPtr[] jreargs);
		
		[DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static int AlexRunMain(IntPtr mainclassname, IntPtr[] mainargs);
		
		[DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static int AlexDestroyVm();

        private static IntPtr[] StringArrayToBSTRArray(string[] a) {
            IntPtr[] b = new IntPtr[a.Length + 1];
            for (int n = 0; n < a.Length; n++) {
                b[n] = Marshal.StringToBSTR(a[n]);
            }
            b[b.Length - 1] = IntPtr.Zero;
            return b;
        }

        private static void FreeBSTRArray (IntPtr[] a) {
            for (int n = 0; n < a.Length; n++) {
                if (a[n] != null && a[n] != IntPtr.Zero) {
                    Marshal.FreeBSTR(a[n]);
                }
            }
        }
		
		public static void Log (int log) {
			AlexSetLog(log);
		}

		public static void Create (string jredir, string[] jreargs) {
            IntPtr a = Marshal.StringToBSTR(jredir);
            IntPtr[] b = StringArrayToBSTRArray(jreargs);
            int v = AlexCreateVm(a, b);
            Marshal.FreeBSTR(a);
            FreeBSTRArray(b);
            if (v != 0) {
				throw new Exception("could not create: " + v);
			}
        }
		
		public static void RunMain (string mainclassname, string[] mainargs) {
            IntPtr c = Marshal.StringToBSTR(mainclassname);
            IntPtr[] d = StringArrayToBSTRArray(mainargs);
            int v = AlexRunMain(c, d);
            Marshal.FreeBSTR(c);
            FreeBSTRArray(d);
            if (v != 0) {
				throw new Exception("could not run main: " + v);
			}
        }
		
		public static void Destroy () {
            int v = AlexDestroyVm();
            if (v != 0) {
				throw new Exception("could not run main: " + v);
			}
        }
		
        public static void Main(string[] args) {
			int log = 0;
            string dir = null;
            List<string> options = new List<string>();
            string mainclassname = null;
            List<string> mainargs = new List<string>();

            for (int n = 0; n < args.Length; n++) {
                if (n == 0) {
                    log = int.Parse(args[n]);
                } else if (n == 1) {
                    dir = args[n];
                } else if (n == 2) {
                    options.Add("-Djava.class.path=" + args[n]);
                } else if (args[n].StartsWith("-D")) {
                    options.Add(args[n]);
                } else if (mainclassname == null) {
                    mainclassname = args[n].Replace(".", "/");
                } else {
                    mainargs.Add(args[n]);
                }
            }
			
			Console.WriteLine("log = " + log);
			Console.WriteLine("jvmdir = " + dir);
			Console.WriteLine("jvmargs = " + string.Join(", ", options) + " count = " + options.Count());
			Console.WriteLine("mainclass = " +  mainclassname);
			Console.WriteLine("mainargs = " + string.Join(", ", mainargs) + " count = " + mainargs.Count());

            if (dir != null && options.Count > 0 && mainclassname != null) {
                Log(log);
                Create(dir, options.ToArray());
                RunMain(mainclassname, mainargs.ToArray());
                // wait for all threads to exit
                Destroy();

            } else {
                Console.WriteLine("Usage: LauncherTest <log> <jvmdir> <classpath> {-D<jvmarg>} <mainclass> {<mainarg>}");
            }

        }

    }
}

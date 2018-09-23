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
        //LAUNCHER_API const wchar_t *LaunchImpl(WCHAR *jredir, WCHAR **jreargs, WCHAR *mainclassname, WCHAR **mainargs);
        [DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr LaunchImpl(IntPtr dir, IntPtr[] jreargs, IntPtr mainclassname, IntPtr[] mainargs, int log);

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

        public static string Launch (string jredir, string[] jreargs, string mainclassname, string[] mainargs, bool log)
        {
            IntPtr a = Marshal.StringToBSTR(jredir);
            IntPtr[] b = StringArrayToBSTRArray(jreargs);
            IntPtr c = Marshal.StringToBSTR(mainclassname);
            IntPtr[] d = StringArrayToBSTRArray(mainargs);
            IntPtr v = LaunchImpl(a, b, c, d, log ? 1 : 0);
            Marshal.FreeBSTR(a);
            FreeBSTRArray(b);
            Marshal.FreeBSTR(c);
            FreeBSTRArray(d);
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

            string v1 = Launch(dir, options.ToArray(), mainclassname, mainargs.ToArray(), true);
            Console.WriteLine("Main: test1 = " + v1);

            string v2 = Launch(dir, options.ToArray(), mainclassname, mainargs.ToArray(), true);
            Console.WriteLine("Main: test2 = " + v2);

            Console.WriteLine("Main exit");
        }

    }
}

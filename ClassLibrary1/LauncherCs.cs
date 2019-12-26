using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace JavaLauncher
{
    /// <summary>
    /// C# wrapper for launcher dll
    /// </summary>
    public class LauncherCs
    {

        [DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void AlexSetLog(int log);

        [DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static int AlexCreateVm(IntPtr dir, IntPtr[] jreargs);

        /// <summary>
        /// main class package seperator must be /
        /// </summary>
        [DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static int AlexRunMain(IntPtr mainclassname, IntPtr[] mainargs);

        [DllImport("launcher.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static int AlexDestroyVm();

        private static IntPtr[] StringArrayToBSTRArray(string[] a) {
            IntPtr[] b = new IntPtr[a.Length + 1];
            for (int n = 0; n < a.Length; n++) {
                b[n] = Marshal.StringToBSTR(a[n]);
            }
            b[b.Length - 1] = IntPtr.Zero;
            return b;
        }

        private static void FreeBSTRArray(IntPtr[] a) {
            for (int n = 0; n < a.Length; n++) {
                if (a[n] != null && a[n] != IntPtr.Zero) {
                    Marshal.FreeBSTR(a[n]);
                }
            }
        }

        public static void Log(int log) {
            AlexSetLog(log);
        }

        public static void Create(string jredir, string[] jreargs) {
            IntPtr a = Marshal.StringToBSTR(jredir);
            IntPtr[] b = StringArrayToBSTRArray(jreargs);
            int v = AlexCreateVm(a, b);
            Marshal.FreeBSTR(a);
            FreeBSTRArray(b);
            if (v != 0) {
                throw new Exception("could not create: " + v);
            }
        }

        public static void RunMain(string mainclassname, string[] mainargs) {
            IntPtr c = Marshal.StringToBSTR(mainclassname);
            IntPtr[] d = StringArrayToBSTRArray(mainargs);
            int v = AlexRunMain(c, d);
            Marshal.FreeBSTR(c);
            FreeBSTRArray(d);
            if (v != 0) {
                throw new Exception("could not run main: " + v);
            }
        }

        public static void Destroy() {
            int v = AlexDestroyVm();
            if (v != 0) {
                throw new Exception("could not run main: " + v);
            }
        }

    }
}

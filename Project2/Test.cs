using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Threading;

namespace JavaLauncher.Test
{
    /// <summary>
    /// Project2\bin\Release\LauncherTestCs.exe -log=1 "c:\Dev\openjdk-11+28" java Hello 1 2 3
    /// Project2\bin\Release\LauncherTestCs.exe -log=1 -pipe=Meow "c:\Dev\openjdk-11+28" java HelloPipe Meow
    /// </summary>
    public class Test
    {

        public static void Main(string[] args) {
            WriteLine("Usage: LauncherTest [-log={0,1}] [-pipe=<name>] [-D<jvmarg>]... <jvmdir> <classpath> <mainclass> [<mainarg>]...");

            int log = 0;
            string jvmdir = null;
            List<string> jvmopts = new List<string>();
            string mainclassname = null;
            List<string> mainargs = new List<string>();
            string pipe = null;
            bool cp = false;

            for (int n = 0; n < args.Length; n++) {
                string a = args[n];
                if (a.StartsWith("-log=")) {
                    log = int.Parse(a.Substring(5));
                }
                else if (a.StartsWith("-pipe=")) {
                    pipe = a.Substring(6);
                }
                else if (args[n].StartsWith("-D")) {
                    jvmopts.Add(args[n]);
                }
                else if (jvmdir == null) {
                    jvmdir = a;
                }
                else if (!cp) {
                    jvmopts.Add("-Djava.class.path=" + args[n]);
                    cp = true;
                }
                else if (mainclassname == null) {
                    mainclassname = args[n].Replace(".", "/");
                } 
                else {
                    mainargs.Add(args[n]);
                }
            }
			
			WriteLine("log = " + log);
            WriteLine("pipe = " + pipe);
            WriteLine("jvmdir = " + jvmdir);
			WriteLine("jvmargs = " + string.Join(", ", jvmopts) + " count = " + jvmopts.Count());
			WriteLine("mainclass = " +  mainclassname);
			WriteLine("mainargs = " + string.Join(", ", mainargs) + " count = " + mainargs.Count());

            if (jvmdir != null && jvmopts.Count > 0 && mainclassname != null) {
                if (pipe != null) {
                    Thread t = new Thread(() => PipeStuff(pipe));
                    t.Start();
                }
                WriteLine("set log");
                LauncherCs.Log(log);
                WriteLine("create vm");
                LauncherCs.Create(jvmdir, jvmopts.ToArray());
                WriteLine("run main");
                LauncherCs.RunMain(mainclassname, mainargs.ToArray());
                // wait for all threads to exit
                WriteLine("destroy");
                LauncherCs.Destroy();
                WriteLine("exiting");

            } else {
                throw new Exception("insufficient arguments");
            }
        }

        private static void PipeStuff(string name) {
            WriteLine("opening pipe " + name);
            using (NamedPipeServerStream s = new NamedPipeServerStream(name, PipeDirection.InOut)) {
                WriteLine("wait for pipe connection");
                s.WaitForConnection();
                WriteLine("pipe connected");
                using (StreamReader sr = new StreamReader(s)) {
                    using (StreamWriter sw = new StreamWriter(s)) {
                        while (true) {
                            string l = sr.ReadLine();
                            WriteLine("read from pipe: " + l);
                            if (l != null) {
                                WriteLine("write to pipe: " + l.ToUpper());
                                sw.WriteLine(l.ToUpper());
                                sw.Flush();
                            } else {
                                break;
                            }
                        }
                    }
                }
                WriteLine("closing pipe " + name);
            }
        }

        private static void WriteLine(string a) {
            Console.WriteLine("[C#] {0}", a);
        }
    }
}

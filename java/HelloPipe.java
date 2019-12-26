import java.util.*;
import java.io.*;

public class HelloPipe {
	private static BufferedReader br;
	// read from console, write to pipe, read from pipe (until end of file)
	public static void main (String[] args) throws Exception {
		println("HelloPipe! args: " + Arrays.toString(args));
		br = new BufferedReader(new InputStreamReader(System.in));
		try (RandomAccessFile f = new RandomAccessFile("\\\\.\\pipe\\" + args[0], "rw")) {
			while (true) {
				println("read from console");
				String l = br.readLine();
				if (l != null) {
					println("write to pipe: " + l);
					f.writeBytes(l + System.lineSeparator());
					String v = f.readLine();
					println("read from pipe: " + v);
				} else {
					break;
				}
			}
			println("closing random access file");
		}
	}
	private static void println(String a) {
		System.out.println("[Java] " + a);
	}
}

import java.util.*;

public class HelloAsync {
	public static void main (String[] args) throws Exception {
		Thread t = new Thread() {
			public void run() {
				try { Thread.sleep(5000); } catch (Exception e) { e.printStackTrace(); }
				System.out.println("HelloAsync from Java! args: " + Arrays.toString(args));
			}
		};
		t.start();
		System.out.println("java main exiting");
	}
}

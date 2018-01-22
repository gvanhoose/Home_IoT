package code;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;

public class TX_Home
{
	private static final int DEV_PORT_NUMBER = 5002;
	
	public static void main( String[] args )
	{
		try
		{
			String devIP = args[0];
			String cmd = args[1];
			
			try
			{
				ServerSocket devSrvSck = new ServerSocket( DEV_PORT_NUMBER );
				Socket devSck = devSrvSck.accept();
				
				PrintWriter devPrinter = new PrintWriter( devSck.getOutputStream() );
				devPrinter.print( "<" + devIP + ">" + "{" + cmd + "}" );
				
				devPrinter.flush();
				devPrinter.close();
				
				devSck.close();
				devSrvSck.close();
			}
			catch( IOException e )
			{
				e.printStackTrace();
			}
		}
		catch( ArrayIndexOutOfBoundsException e )
		{
			System.out.println( "Insufficient args." );
			e.printStackTrace();
		}
		
	}
}

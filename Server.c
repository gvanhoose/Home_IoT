#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define RX_PORT 5001
#define TX_PORT 5000
#define DEV_PORT 5002
#define BACKLOG_MAX 0
#define BUFFER_LENGTH 256

#define ACCEPT_ERROR -2
#define READ_ERROR -3
#define SOCKET_ERROR -4
#define BIND_ERROR -5
#define LISTEN_ERROR -6
#define CONNECT_ERROR -7
#define SEND_ERROR -8
#define WRITE_ERROR -9
#define CHILD_ERROR -10
#define EXECV_ERROR -11

// Process waits in this function, returns data received from super client
int receiveMessage( int listenerFD, char* rxData )
{
	int clientFD;

	if( (clientFD = accept( listenerFD, (struct sockaddr*) NULL, NULL )) < 0 )
	{
//		perror( "receiveMsg>accept" );
		return ACCEPT_ERROR;
	}

	int bytesRead;

	if( (bytesRead = read( clientFD, rxData, BUFFER_LENGTH - 1 )) < 0 )
	{
//		perror( "receveMsg>read" );
		return READ_ERROR;
	}

	rxData[ bytesRead ] = '\0';

	return bytesRead;
}

// Returns a file descriptor to the receiving socket, or a negative number upon error. (errno is set)
int openSocketAndBind( struct sockaddr_in sockAddr )
{
	int listenerFD;

	if( (listenerFD = socket( AF_INET, SOCK_STREAM, 0 )) < 0 )
	{
//		perror( "opScktBnd>socket" );
		return SOCKET_ERROR;
	}

	memset( (char*) &sockAddr, '\0', sizeof( sockAddr ) );
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	sockAddr.sin_port = htons( RX_PORT );

	if( bind( listenerFD, (struct sockaddr*) &sockAddr, sizeof( sockAddr ) ) < 0 )
	{
//		perror( "opScktBnd>bind" );
		return BIND_ERROR;
	}

	if( listen( listenerFD, BACKLOG_MAX ) < 0 )
	{
//		perror( "opScktBnd>listen" );
		return LISTEN_ERROR;
	}

	return listenerFD;
}

/**
 * Prints in target a null-terminated substring bounded by, but not including, the first respective occurrences of the specified opening and closing delimiters of type char
 * The delimiters must be distinct (i.e.:  They cannot hold the same value)
 * The caller must guarantee that the target is at least (strlen(src)) chars in size
 * The caller must also guarantee that the src is null-terminated
 * substr( "Hello world!", char* target, 'H', 'l' ) would print in target the string "e\0"
 * substr( "Hello world!", char* target, 'l', 'o' ) would print in target the string "l\0"
 * substr( "Hello world!", char* target, 'o', 'l' ) would print in target the string " wor\0"
 * returns the size of the substring printed in target, 0 if no substring was found, and -1 if there was an error
 */
int substr( char* src, char* target, char openDelim, char closeDelim )
{
	if( src == NULL || target == NULL || openDelim == closeDelim )
	{
//		perror( "substr>Bad Input" );
		return -1;
	}

	char current;
	int openFound = 0, closeFound = 0, outSize = 0;

	while( (current = *(src++) ) != '\0' )
	{

		if( current == closeDelim && openFound )
		{
			closeFound = 1;
			break;
		}
		if( !openFound )
		{
			openFound = current == openDelim;
			continue;
		}
		*(target + outSize++) = current;

	}

	target[ outSize ] = '\0';

	return outSize;
}

// spawns a child process which runs a .jar program, which is configured to send the appropriate data to the IP address of the destination device
// In the future, txDevice will be a device ID, not an IP address
int sendDevData( char* txDevice,  char* command )
{

	int childPID, parentPID = getpid();
//	printf( "\tParent PID:  %d\n", parentPID );
	
	childPID = fork();

	if( childPID > 0 )	// parent code
	{
		wait( 0 );
//		printf( "\tChild %d is done\n", childPID );
		return 0;
	}
	if( childPID == 0 )	// child code
	{

		char* cmdLine [] = { "java", "-jar", "Dev_TX_Server.jar", txDevice, command, NULL };

		execvp( cmdLine[0], cmdLine );

//		perror( "execv" );
		return EXECV_ERROR;
		
	}
	if( childPID < 0 )
	{
//		perror( "sendDevData>fork childPID<0" );
		return CHILD_ERROR;
	}
	return 0;
}

int main()
{

//	char pid_str [32];
//	memset( pid_str, '\0', 32 );
//	snprintf( pid_str, 31, "Server PID:  %d", getpid() );
//
//	printf( "%s\n", pid_str );

	struct sockaddr_in sockAddr;

	int listenerFD;

	while( (listenerFD = openSocketAndBind( sockAddr )) < 0 )
	{
		sleep( 20 );
	}

	char rxData [ BUFFER_LENGTH ];
	char sender [ BUFFER_LENGTH ];
	char txLeng [ 4 ];
	char cmdTar [ BUFFER_LENGTH ];
	char cmdCmd [ BUFFER_LENGTH ];

	int recReturnValue;
	int loopNumber = 0;

	while( 1 )
	{

		memset( rxData, '\0', BUFFER_LENGTH );
		memset( sender, '\0', BUFFER_LENGTH );
		memset( txLeng, '\0', 4 );
		memset( cmdTar, '\0', BUFFER_LENGTH );
		memset( cmdCmd, '\0', BUFFER_LENGTH );

		if( (recReturnValue = receiveMessage( listenerFD, rxData )) >= 0 )
		{
			if( strcmp( "quit", rxData ) == 0 )
			{
				printf( "\"quit\" detected\n" );
				break;
			}

			substr( rxData, cmdTar, '<', '>' );
			substr( rxData, txLeng, '[', ']' );
			substr( rxData, cmdCmd, '{', '}' );
			substr( rxData, sender, '(', ')' );

			if( strcmp( "Network quit", cmdCmd ) == 0 )
			{
				// dispense network quit signal to network... all network devices need to reboot at that point, I guess...
				break;
			}

//			printf( "About to send\n" );
			if( sendDevData( cmdTar, cmdCmd ) < 0 )
			{
				perror( "main>sendDevData" );
				// some kind of error handling should go here...  or maybe it just needs to wait?
				// That's why I'm not exiting the server program just yet!
			}

			
		}
		else
		{
//			printf( "Reception error:  %d\n", recReturnValue );

		}
	}

	close( listenerFD );

	return 0;
}

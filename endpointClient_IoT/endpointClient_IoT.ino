
#include <ESP8266WiFi.h>
#include <string.h>

#define SERVER_IP_ADDR  "192.168.2.4"
#define DEV_ID "192.168.2.18"
#define DEV_PORT 5002
#define NETWORK_NAME "belkin.f3a"
#define NETWORK_PW "nice_try"
#define BUFFER_LENGTH 256

WiFiClient _devClient;

String _rxRawString = "";
char _rxData [ BUFFER_LENGTH ];
char _rxID [ BUFFER_LENGTH ];
char _rxCmd [ BUFFER_LENGTH ];


/**
 * Prints in target a null-terminated substring bounded by, but not including, the first respective occurrences of the specified opening and closing delimiters of type char
 * The delimiters must be distinct (i.e.:  They cannot hold the same value)
 * substr( "Hello world!", char* target, 'H', 'l' ) would print in target the string "e\0"
 * substr( "Hello world!", char* target, 'l', 'o' ) would print in target the string "l\0"
 * substr( "Hello world!", char* target, 'o', 'l' ) would print in target the string "wor\0"
 * returns the size of the substring printed in target, 0 if no substring was found, and -1 if there was an error
 */
int substr( char* src, char* target, char openDelim, char closeDelim )
{
  if( src == NULL || target == NULL || openDelim == closeDelim )
  {
    printf( "Bad input\n" );
    return -1;
  }

  char current;
  int openFound = 0, closeFound = 0, i = 0, outSize = 0;

  while( (current = *(src + i++) ) != '\0' )
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

void setup()
{
  Serial.begin( 115200 );

  Serial.print( "\nConnecting to:  \"" );
  Serial.print( NETWORK_NAME );
  Serial.println( "\"" );
//  Serial.print( "\" with password:  \"" );
//  Serial.print( pwd );
//  Serial.println( "\"" );

//  delay( 1000 );
  WiFi.begin( NETWORK_NAME, NETWORK_PW );
//  delay( 1000 );
  
  while( WiFi.status() != WL_CONNECTED )
  {
    Serial.println( "Waiting to connect" );
    delay( 500 );
    //WiFi.begin( ssid, password );
  }

  Serial.println( "WiFi Connected and ready to go!" );

  

  pinMode( 0, OUTPUT );
  digitalWrite( 0, LOW );
  memset( _rxData, '\0', BUFFER_LENGTH );
  memset( _rxID, '\0', BUFFER_LENGTH );
}

void loop()
{
  if( !_devClient.connected() )
  {
//    Serial.println( "Awaiting connection" );
    if( !_devClient.connect( SERVER_IP_ADDR, DEV_PORT ) )
    {
//      Serial.println( "Connection failed" );
      delay( 1000 );
    }
  }
  else
  {
    _devClient.setNoDelay( true );
    Serial.println( "Connection Ready" );
    while( _devClient.connected() )
    {
      _rxRawString = "";
      _rxRawString = _devClient.readStringUntil( '\0' );
      if( ((String) "").compareTo( _rxRawString ) != 0 )
      {
        Serial.print( "Received:  \"" );
        Serial.print( _rxRawString );
        Serial.println( "\"" );

        _rxRawString.toCharArray( _rxData, BUFFER_LENGTH );

        substr( _rxData, _rxID, '<', '>' );
        substr( _rxData, _rxCmd, '{', '}' );

        Serial.print( "\tTarget ID:  " );
        Serial.println( _rxID );
        Serial.print( "\tTarget Command:  " );
        Serial.println( _rxCmd );
  
        if( strcmp( _rxID, DEV_ID ) == 0 )
        {
          if( strcmp( _rxCmd, "Room Light 3 TOGGLE" ) == 0 )
          {
            if( digitalRead( 0 ) == LOW )
            {
              digitalWrite( 0, HIGH );
            }
            else
            {
              digitalWrite( 0, LOW );
            }
          }
        }

        memset( _rxData, '\0', BUFFER_LENGTH );
        memset( _rxID, '\0', BUFFER_LENGTH );
        memset( _rxCmd, '\0', BUFFER_LENGTH );
      }
    }
  }
  
}

/**
 * Steps:
 * 
 * WiFiClient _client
 * 
 * setup:
 *  WiFi.begin()      // connect to WiFi
 *  WiFi.status()     // spin on this while waiting
 *  
 * Loop:
 * 
 * // to receive data
 *  _client.connected  // ensure connection persists and if not...
 *  _client.connect    // establishes socket (IP address and port number go here!) like so:
 *  if( !_client.connected )
 *  {
 *    if( !_client.connect( SERVER_IP_ADDR, TX_PORT )
 *    {
 *      local_boolean connected = false;
 *    }
 *  }
 *  
 *  // to send data
 *  _client.print( (String) data );   // make sure _client.connected is true
 *  
 * 
 */


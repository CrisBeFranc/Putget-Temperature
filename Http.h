/* Mbed library for the mbed Lab Board - Socket Connection to PUT and GET Data
 * on/from a Server
 * Copyright (c) 2018 Cristian Camilo Benavides Franco - University of Kent
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef HTTP_H
#define HTTP_H

/** optional Defines :
  * #define some relevant data for the packages
  */
#define GET_SIZE 69
#define PUT_SIZE 67
#define SERVER_RESPONSE_SIZE 256


/** messages Defines
*   #define the Serial and LCD prints used on the program.
*/
#define NO_CONNECTION "Not Possible To Connect"
#define HOST_ADDR "shed.kent.ac.uk"
#define PUT "PUT"
#define GET "GET"
#define PATH_SHED "/IoT/shed"
#define PATH "/IoT/ccb24"
#define VERSION "HTTP/1.1"
#define CONNECTION "Connection: "
#define CONNECTION_TYPE "close"
#define HOST "Host: "
#define CONTENT_TYPE "Content-Type: "
#define TYPE "text/plain"
#define CONTENT_LENGHT "Content-Length: "
#define JUMP "\r\n"

/**
*   DEF: Thread Components Definition
*/

/** Send the Temperature to the Shed Server - User: ccb24
*
*/
void sendTemperatureToCCB24();

/** Receive the Temperature from the Shed Server - User: shed
*
*/
void getTemperatureFromShed();

/**
*   DEF: HTTP Packages Definition & Creation
*/

/** Format the Put Packet
*
* @param put_packet the HTTP PUT packet formatted
*                                                   *
*/
void createPutRequest(char* put_packet);

/** Format the Get Packet
*
* @param put_packet the HTTP GET packet formatted
*                                                   *
*/
void createGetRequest(char* get_packet);

/**
*   DEF: Generic Printings on Serial and LCD
*/

/** Print Device Network Information
*
*/
void printData();

/** Print LCD Layout
*
*/
void printMenuMBED();

/**
*   DEF: Logic to Send and Receive HTTP Packets
*/

/** HTTP PUT Functionalitie
*
* @param socket used to manage the PUT pipe communication.
*                                                   *
*/
void sendTemperature(TCPSocket &socket);

/** HTTP GET Functionalitie
*
* @param socket used to manage the GET pipe communication.
*                                                   *
*/
void recieveTemperature(TCPSocket &socket);


/**
*   DEF: Error Formatting, Actions and Trigger
*/

/** Print LCD No Connection Error
*
*/
void noConnectionError();

/** PUT Error
*
* @param recieved_code received code to manage the PUT responses.
*                                                   *
*/
void putError(char* recieved_code);

/** GET Error
*
* @param recieved_code received code to manage the GET responses.
*                                                   *
*/
void getError(char* recieved_code);

/** Socket Error
*
* @param socket that is attempting to stablish connection.
*                                                   *
*/
bool socketError(TCPSocket &socket);


#endif // HTTP_H
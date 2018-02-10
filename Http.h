#ifndef HTTP_H
#define HTTP_H

#define GET_SIZE 69
#define PUT_SIZE 67
#define SERVER_RESPONSE_SIZE 256

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

//THREADS LOGIC
void sendTemperatureToCCB24();
void getTemperatureFromShed();

// PACKAGE CREATION 
void createPutRequest(char* put_packet);
void createGetRequest(char* get_packet);

// PRINTINGS
void printData();
void printMenuMBED();

// FUNCTIONALITIES
void sendTemperature(TCPSocket &socket);
void recieveTemperature(TCPSocket &socket);

// ERRORS
void putError(char* recieved_code);
void getError(char* recieved_code);
void noConnectionError();
bool socketError(TCPSocket &socket);


#endif // HTTP_H
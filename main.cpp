/* Mbed library for the mbed Lab Board - Socket Connection to PUT and GET Data
 * on/from a Server
 * Copyright (c) 2018 Cristian Camilo Benavides Franco - University of Kent
 * Includes of C12832 and LM75B Libraries Designed By:
 * Peter Drescher and cstyles.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "mbed.h"
#include "EthernetInterface.h"
#include "LM75B.h"
#include "C12832.h"
#include "Http.h"

//Network interface
EthernetInterface net;

//Display - Using Arduino pin notation
C12832 lcd(D11, D13, D12, D7, D10);

//Temperature Sensor
LM75B sensor(D14, D15);

//Serial Connection
Serial pc(USBTX, USBRX);

//Speaker
PwmOut spkr(D6);

//Digital out - Led's
DigitalOut led_red(LED1);
DigitalOut led_green(LED2);
DigitalOut led_blue(LED3);

//Threads
Thread packagePUT;
Thread packageGET;

//Timer
Timer to_get, to_put;

//Flag Collision Control and LCD printing controll flag
bool is_bussy, menu_available = false, program_init = true;

//Control Variable
float sent_temperature = NULL;

//Sensor Data, Interrupts and Communication Program
int main()
{

    pc.baud(115200);
    to_get.start();
    to_put.start();
    sent_temperature = sensor.read();
    packagePUT.start(callback(sendTemperatureToCCB24));
    packageGET.start(callback(getTemperatureFromShed));

    pc.printf("Temperature Program\n\r");
    pc.printf("----------------------------------\n\r");

    if (net.connect() < 0) {

        noConnectionError();

    } else {

        //Bring up the ethernet interface.
        net.connect();

        //Print Data of Device on Terminal.
        printData();

        //Print On Display Temperature.
        printMenuMBED();

        while (true) {

            float rounded_up = sensor.read();

            // Update ff the reading changes significantly from the previous reading 120 sec has elapsed.
            // A significant change is greater-than or equal-to plus or minus 1.0 degree.
            if ((rounded_up - sent_temperature) >= 1.0 || (rounded_up - sent_temperature) <= -1.0) {

                pc.printf("Signal triggered because there was a change greater-than or equal-to plus or minus 1.0 degree. \n\r");
                pc.printf("Difference of = %.1f\n\r", (rounded_up - sent_temperature));
                sent_temperature = rounded_up;
                to_put.reset();
                packagePUT.signal_set(0x1);

            }

            //HTTP PUT to send a temperature reading to CCB24 at least once every 120 seconds.
            if ((to_put.read() * 10) >= 1200.0 || program_init) {
                sent_temperature = rounded_up;
                packagePUT.signal_set(0x1);
                to_put.reset();
            }

            //HTTP GET to receive a temperature reading from THE-SHED every 30 seconds.
            if ((to_get.read() * 10) >= 300.0 || program_init) {
                sent_temperature = rounded_up;
                packageGET.signal_set(0x1);
                program_init = false;
                to_get.reset();
            }
            wait(1.0);
        }

        // Bring down the ethernet interface
        net.disconnect();
    }
}

void sendTemperatureToCCB24()
{
    TCPSocket send_socket;
    while (true) {

        Thread::signal_wait(0x1);

        // Send Package
        pc.printf("PUT Package on CCB24 \n\r");
        pc.printf("----------------------------------\n\r");

        //Send temperature to server (CCB24) - PUT ACTION
        sendTemperature(send_socket);

        pc.printf("Sent Temperature = %.1f\n\r", sent_temperature);
        pc.printf("------------------------------------\n\r");
    }
}

void getTemperatureFromShed()
{
    //Creation of the Socket
    TCPSocket recieve_socket;

    while (true) {

        Thread::signal_wait(0x1);

        // GET Package
        pc.printf("GET Package from SHED \n\r");
        pc.printf("----------------------------------\n\r");

        //Recieve temperature from Shed - GET
        recieveTemperature(recieve_socket);

        pc.printf("------------------------------------\n\r");
    }
}

void printData()
{
    // Show the network address
    pc.printf("Device Information \n\r");
    const char *ip = net.get_ip_address();
    const char *mac = net.get_mac_address();
    const char *gateway = net.get_gateway();
    pc.printf("IP address: %s\n\r", ip ? ip : "None");
    pc.printf("MAC address: %s\n\r", mac ? mac : "None");
    pc.printf("Gateway: %s\n\n\r", gateway ? gateway : "None");
}

void printMenuMBED()
{
    lcd.rect(0, 0, 127, 31, 1);
    lcd.line(65, 0, 65, 31, 1);
    lcd.locate(7,3);
    lcd.printf("Local Temp");
    lcd.locate(72, 3);
    lcd.printf("Shed Temp");
    menu_available = true;
}

void createGetRequest(char* get_packet)
{
    // GET /IoT/shed HTTP/1.1\r\nConnection: Close\r\nHost: shed.kent.ac.uk\r\n\r\n"
    sprintf(get_packet,"%s %s %s%s%s%s%s%s%s%s%s ", GET, PATH_SHED, VERSION, JUMP, CONNECTION, CONNECTION_TYPE, JUMP, HOST, HOST_ADDR, JUMP, JUMP);
}

void createPutRequest(char* put_packet)
{
    // PUT /IoT/ccb24 HTTP/1.1\r\nContent-Type: text/plain\r\nContent-Length: ";
    sprintf(put_packet,"%s %s %s%s%s%s%s%s%", PUT, PATH, VERSION, JUMP, CONTENT_TYPE, TYPE, JUMP, CONTENT_LENGHT);
}

void recieveTemperature(TCPSocket &socket)
{
    if (!socketError(socket)) {

        //Initialization of get_socket
        socket.open(&net);
        socket.connect(HOST_ADDR, 80);

        char sbuffer[GET_SIZE];

        // Create and Send the HTTP GET Request
        createGetRequest(sbuffer);
        int scount = socket.send(sbuffer, sizeof sbuffer);
        pc.printf("sent %d [%.*s]\n\r", scount, strstr(sbuffer, "\r\n")-sbuffer, sbuffer);

        // Receive a simple http response and print out the response line
        char rbuffer[SERVER_RESPONSE_SIZE];
        int rcount = socket.recv(rbuffer, sizeof rbuffer);
        pc.printf("recv %d [%.*s]\n\r", rcount, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);

        // Extract the Code and trigger responses (if needed)
        char recieved_code[4];
        memcpy(recieved_code, strstr(rbuffer, "HTTP/1.1") + 9, 3);
        recieved_code[3] = '\0';
        getError(recieved_code);

        if (strcmp (recieved_code,"200") == 0) {

            const char *payload = strstr(rbuffer, "\r\n\r\n")-2;

            //Flush buffer
            memset(rbuffer, 0, SERVER_RESPONSE_SIZE);
            rcount = socket.recv(rbuffer, sizeof rbuffer);

            pc.printf("Received Temperature: %.*s\n\r", sizeof(payload), rbuffer);

            // Print on LCD C12832
            if (!menu_available) {

                // Print menu on LCD
                lcd.cls();
                printMenuMBED();

            }

            //Print Received Temperature From SHED
            lcd.locate(82,15);
            lcd.printf("%.*s C\n\r", sizeof(payload), rbuffer);
        }

        delete[] rbuffer;
        delete[] sbuffer;

        // Close the recieve_socket to return its memory and bring down the network interface
        socket.close();
    }
}


void sendTemperature(TCPSocket &socket)
{
    if (!is_bussy) {

        is_bussy = true;

        if (!socketError(socket)) {

            //Initialization of send_socket
            socket.open(&net);
            socket.connect(HOST_ADDR, 80);

            //Temperature Lecture
            float rounded_up = sensor.read();

            if (!menu_available) {

                // Print menu on LCD
                lcd.cls();
                printMenuMBED();

            }

            // Print data on LCD
            lcd.locate(17,15);
            lcd.printf("%.1f C\n", sensor.read());

            // Send an HTTP request
            char header[PUT_SIZE];
            createPutRequest(header);
            int size_header = strlen(header);

            int size_payload = sizeof(rounded_up);
            char lenght[sizeof(size_payload)];
            sprintf(lenght, "%d", size_payload);

            int size_content_lenght = strlen(lenght);

            char temperature[sizeof(rounded_up)+1];
            sprintf(temperature, "%.2f", rounded_up);

            int total = size_header + size_content_lenght + 4 + size_payload + 2;

            char *sbuffer = NULL;
            sbuffer = (char*)malloc(total);

            // Create the HTTP packet
            sprintf(sbuffer,"%s%s%s%s%s%s", header, lenght, JUMP, JUMP, temperature, JUMP);

            //pc.printf(sbuffer);
            //pc.printf("\n\r");

            int scount = socket.send(sbuffer, strlen(sbuffer));
            pc.printf("sent %d [%.*s]\n\r", scount, strstr(sbuffer, "\r\n")-sbuffer, sbuffer);

            // Recieve a simple http response and print out the response line
            char rbuffer[SERVER_RESPONSE_SIZE];
            int rcount = socket.recv(rbuffer, sizeof rbuffer);
            pc.printf("recv %d [%.*s]\n\r", rcount, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);

            // Extract the Code and trigger responses (if needed)
            char recieved_code[4];
            memcpy(recieved_code, strstr(rbuffer, "HTTP/1.1") + 9, 3);
            recieved_code[3] = '\0';
            putError(recieved_code);

            delete[] rbuffer;
            delete[] sbuffer;

            // Close the send_socket to return its memory and bring down the network interface
            socket.close();
        }

        is_bussy = false;
    }
}

void noConnectionError()
{
    //Print on display
    lcd.cls();
    lcd.rect(0, 0, 127, 31, 1);
    lcd.locate(1,3);
    lcd.printf("%s\n", NO_CONNECTION );
    menu_available = false;
}

bool socketError(TCPSocket &socket)
{
    bool response = false;
    if (socket.open(&net) < 0 || socket.connect(HOST_ADDR, 80) < 0) {
        noConnectionError();
        response = true;
    }
    return response;
}

void putError(char* recieved_code)
{
    pc.printf("CODE [%s]\n\r", recieved_code);

    if (strcmp (recieved_code,"200") != 0) {
        //Trigger alarm
        spkr.period(1.0/1000.0);
        spkr=0.5;

        //Print on display
        lcd.cls();
        lcd.rect(0, 0, 127, 31, 1);
        lcd.locate(32,15);
        lcd.printf("PUT ERROR = %s\n", recieved_code );
        menu_available = false;

        //Led blue activation
        led_green = 0;
        led_blue = 1;
        led_red = 0;

        wait(3.0);
        spkr=0.0;
    }
}

void getError(char* recieved_code)
{
    pc.printf("CODE [%s]\n\r", recieved_code);

    if (strcmp (recieved_code,"200") != 0) {
        //Trigger alarm
        spkr.period(1.0/3000.0);
        spkr=0.5;

        //Print on display
        lcd.cls();
        lcd.rect(0, 0, 127, 31, 1);
        lcd.locate(32,15);
        lcd.printf("GET ERROR = %s\n", recieved_code );
        menu_available = false;

        //Led red activation
        led_green = 0;
        led_blue = 0;
        led_red = 1;

        wait(3.0);
        spkr=0.0;
    }
}


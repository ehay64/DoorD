/**
 * doord.c
 *
 * This is a server daemon used to open and close a garage door.
 * Although it's intened purpose is to control a garage door, this 
 * program can actually be used to control any general purpose 
 * input/output pin (gpio) attached to the system. 
 *
 * Controlling of the gpio is done by writing to the appropriate 
 * value file in the file system. The location of the value file
 * may be different in different embedded systems, but it is 
 * located at:
 *      /sys/class/gpio/gpio$/value
 * in OpenWRT Attitude Adjustment, where '$' is the number of the gpio.
 * The gpio is treated as a monostable multivibrator. The gpio 
 * will remain off and when it's toggled, will turn on for 1 second.
 *
 * Clients control the gpio over the network. The gpio is toggled 
 * by sending the text "toggle", or even the character "t", to an 
 * instance of this daemon over TCP.
 *
 * So far, this program has no provision for handling multiple 
 * network clients. It is not the intention of this program to
 * be a general purpose server designed to handle multiple 
 * network connections. There is no plan for this to be implemented 
 * in the future. 
 *
 * To use this program, the constants "port" and "gpio" should
 * be modified to taste. Then after compilation, the executable 
 * should be started by either init, systemd, or rc.local on 
 * startup. Consult the man pages of your system to determine 
 * what is the best way to integrate the program. 
 *
 * Author: Eric Hay
 * Email: erichay@live.ca
 * Date Created: September 3rd, 2015
 * Date Modified: September 6th, 2015
 * GCC Compile Flags: -std=c11
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

/**
 * int startServer(int serverPort)
 *
 * Given a port, this function starts the current process listening for TCP connections on it.
 * Any IP can connect to this process.
 * What is returned is the file descriptor for the listening port.
 * This functions creates that descriptor and binds it to a sockaddr_in.
 * Returns:
 *      -1: Couldn't get a listening socket
 *      -2: Couldn't bind the socket
 *      -3: Couldn't begin listening on the socket
 *      >0: File descriptor for the socket
 */
int startServer(int serverPort);

void doForever(int listenSocket, FILE *gpioFile);

//Constants needed for this program. Customize to taste
const int port = 8765; //The port the daemon will listen in on
const int gpio = 7; //The gpio the daemon will control

int main(void)
{
    printf("doord: Garage Door Daemon\n");
    
    //The data that we'll need
    int listenSocket = startServer(port); //Creates and stores the listening file descriptor
    FILE *gpioFile; //Stores the file info for the gpio file
    
    //Check to make sure the listenSocket is sane
    switch (listenSocket)
    {
        case -1:
            printf("Error: couldn't get a listening socket\n");
            return 1;
            break;
            
        case -2:
            printf("Error: couldn't bind the socket\n");
            return 1;
            break;
            
        case -3:
            printf("Error: couldn't begin to listen on socket");
            return 1;
            break;
            
            //If the socket isn't one of the error codes, it's fine
        default:
            break;
    }
    
    //Attempt to open the gpio file
    char gpioDir[64];
    sprintf(gpioDir, "/sys/class/gpio/gpio%d/value\0", gpio);
    printf("Attempting to open: %s\n", gpioDir);
    gpioFile = fopen(gpioDir, "w");
    
    doForever(listenSocket, gpioFile);
    
    return 0;
}

int startServer(int serverPort)
{
    //The data that we'll need
    int listenSocket = 0; //Stores the listening file descriptor
    struct sockaddr_in addressInfo; //Holds the address info
    
    //Get the listening socket
    listenSocket = socket(PF_INET, SOCK_STREAM, 0); //Get the listening socket
    
    //If the listening socket is -1, then something went wrong
    if (listenSocket < 0)
    {
        return -1;
    }
    
    //Make the address info
    memset(&addressInfo, 0, sizeof(addressInfo)); //Clear the contents of addressInfo
    
    addressInfo.sin_family = AF_INET; //Type of connection we're going to be using
    addressInfo.sin_port = htons(serverPort); //Port we're going to be using
    //ip addre0bss we're going to connect to - in this case, any ip
    addressInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //Bind the listening port to our address info
    //If bind returns less than 0
    if (bind(listenSocket, (struct sockaddr*)&addressInfo, sizeof(addressInfo)) < 0)
    {
        return -2; //Something went wrong
    }
    
    //Start listening on the listening port
    if (listen(listenSocket, 10) == -1) //If listen returned -1
    {
        return -3; //Something went wrong
    }
    
    //If we've made it this far, we're good to go
    return listenSocket; //Return the file descriptor for the currently listening socket
}

void doForever(int listenSocket, FILE *gpioFile)
{
    int clientSocket; //Stores the client file descriptor
    char buffer[1024]; //Place to store messages from the client
    
    //Accept connections and wait for data
    while (1)
    {
        //This will wait for a connection to occur
        //It outputs the file descriptor for communication with the client
        clientSocket = accept(listenSocket, (struct sockaddr*)NULL, NULL);
        
        //Check to make sure the clientSocket is sane
        if (clientSocket < 0)
        {
            printf("Error: failed to get client socket\n");
        }
        else //Otherwise, start comms with the client
        {
            printf("Client connected on socket %d\n", clientSocket);
            
            //Clear the buffer
            memset(buffer, 0, sizeof(buffer));
            //Get message from the client
            read(clientSocket, buffer, sizeof(buffer));
            printf("Client has said: %s", buffer);
            
            //If the client has said "toggle"
            if (buffer[0] == 't')
            {
                //Turn on the gpio
                putc('1', gpioFile);
                fflush(gpioFile);
                //Wait a bit
                sleep(1);
                //Turn off the gpio
                putc('0', gpioFile);
                fflush(gpioFile);
            }
            //Close comms with the client
            close(clientSocket);
        }
    }
    
}

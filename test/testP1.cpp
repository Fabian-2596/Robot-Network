// Source by https://www.geeksforgeeks.org/socket-programming-cc/

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <string>

using namespace std;
const int PORT{8080};


int main()
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }


    //Send GET to Server
    char getbuffer[1024] = { 0 };
    string getMSG = "GET /status HTTP/1.1";
    if ((send(client_fd, getMSG.c_str(), strlen(getMSG.c_str()), MSG_NOSIGNAL)) == -1){return -1;}
    if ((read(client_fd, getbuffer, 1024 - 1)) == -1){return -1;} // subtract 1 for the null terminator at the end
    istringstream getStream{getbuffer}; 
    string errCode, html_status, http_version, response, temp;
    getStream >> http_version >> errCode >> html_status;
    while (getStream >> temp){response = response + temp;}
    
    if (errCode != "200"){return 1;}

    //Send POST to Server
    char postbuffer[1024] = { 0 };
    string postMSG = "POST /status HTTP/1.1\r\nContent-Type: text/plain\r\nContent-Length: 6\r\n\r\ntestP1";
    if ((send(client_fd, postMSG.c_str(), strlen(postMSG.c_str()), MSG_NOSIGNAL)) == -1){return -1;}
    if ((read(client_fd, postbuffer, 1024 - 1)) == -1){}

    //Check if POST msg was saved
    char postgetbuffer[1024] = { 0 };
    string postgetMSG = "GET /lastPOST HTTP/1.1";
    if ((send(client_fd, postMSG.c_str(), strlen(postMSG.c_str()), MSG_NOSIGNAL)) == -1){return -1;}
    if ((read(client_fd, postgetbuffer, 1024 - 1)) == -1){return -1;} // subtract 1 for the null terminator at the end
    string postResponse, postTemp;
    istringstream postgetStream{postgetbuffer}; 
    postgetStream >> http_version >> errCode >> html_status;
    response = "";
    while (postgetStream >> temp){postResponse = postResponse + postTemp;}
    if (errCode != "200"){return 1;}
    if (response != "testP1"){return 1;}

 
    // closing the connected socket
    close(client_fd);
    return 0;
    
}
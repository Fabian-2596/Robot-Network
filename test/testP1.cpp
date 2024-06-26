// Source by https://www.geeksforgeeks.org/socket-programming-cc/

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
const int PORT{8080};

int main()
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n GET Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    int loop{5};
    int i{0};
    while ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        cout << "\nGET Connection Failed Try " << i <<" of " << loop << endl;
        if (i >= loop)
        {
            return -1;
        }else{
            this_thread::sleep_for(chrono::seconds(2));
            ++i;
        }
    }

    // Send GET to Server
    char getbuffer[1024] = {0};
    string getMSG = "GET /status HTTP/1.1";
    if ((send(client_fd, getMSG.c_str(), strlen(getMSG.c_str()), MSG_NOSIGNAL)) == -1)
    {
        cout << "Get Send Failed" << endl;
        return -1;
    }
    if ((read(client_fd, getbuffer, 1024 - 1)) == -1)
    {
        cout << "Get Read Failed" << endl;
        return -1;
    } // subtract 1 for the null terminator at the end
    istringstream getStream{getbuffer};
    string errCode, html_status, http_version, response, temp;
    getStream >> http_version >> errCode >> html_status;
    while (getStream >> temp)
    {
        response = response + temp;
    }
    if (errCode != "200")
    {
        return 1;
    }

    close(client_fd);
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n POST Socket creation error \n");
        return -1;
    }
    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nPOST Connection Failed \n");
        return -1;
    }

    // Send POST to Server
    char postbuffer[1024] = {0};
    string postMSG = "POST /status HTTP/1.1\r\nContent-Type: text/plain\r\nContent-Length: 6\r\n\r\ntestP1";
    if ((send(client_fd, postMSG.c_str(), strlen(postMSG.c_str()), MSG_NOSIGNAL)) == -1)
    {
        cout << "POST Send Failed" << endl;
        return -1;
    }
    if ((read(client_fd, postbuffer, 1024 - 1)) == -1)
    {
        cout << "POST Read Failed" << endl;
        return -1;
    }

    close(client_fd);
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n POSTGET Socket creation error \n");
        return -1;
    }
    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nPOSTGET Connection Failed \n");
        return -1;
    }

    // Check if POST msg was saved
    char postgetbuffer[1024] = {0};
    string postgetMSG = "GET /lastPOST HTTP/1.1";
    if ((send(client_fd, postgetMSG.c_str(), strlen(postgetMSG.c_str()), MSG_NOSIGNAL)) == -1)
    {
        cout << "POSTGET Send Failed" << endl;
        return -1;
    }
    if ((read(client_fd, postgetbuffer, 1024 - 1)) == -1)
    {
        cout << "POSTGET Read Failed" << endl;
        return -1;
    } // subtract 1 for the null terminator at the end
    string postgetResponse, postgetTemp;
    istringstream postgetStream{postgetbuffer};
    postgetStream >> http_version >> errCode >> html_status;
    response = "";
    while (postgetStream >> postgetTemp)
    {
        postgetResponse = postgetResponse + postgetTemp;
    }
    if (errCode != "200")
    {
        return 1;
    }
    if (postgetResponse != "testP1")
    {
        return 1;
    }

    // closing the connected socket
    close(client_fd);
    return 0;
}
#include <iostream>
#include <vector>
#include <thread>
using namespace std;


const int ANZ_THREADS{1000};

void sendCurlToServer(int i)
{
    system("curl -X POST -d TESTPROTOKOLL localhost:8080");
}


int main()
{
    vector<thread> pool;
    
    for (int i = 0; i < ANZ_THREADS; i++)
        pool.push_back(thread(sendCurlToServer, i));

    for (thread &t : pool)
        if (t.joinable())
        {
            t.join();
        }
    return 0;
}
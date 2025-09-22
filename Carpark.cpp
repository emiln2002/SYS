#include <iostream>
#include <mutex>
#include <condtion_variable> 



void car()
{
    this_thread::sleep_for(chrono::seconds(1));
    {
        lock_guard<mutex> lk(cv_m);
        ScarIn == 1; 
        cerr << "Car at gate.. \n"
    }

}

void entryGate()
{
    unique_lock<mutex> lk(mtx); 
    std::cerr << "Entry gate ready... \n"; 
    cv.wait(lk, []{return SCarIn == 1});
}

int main(){
condition_variable cv; 
mutex mtx;
int SCarIn = 0;
int SGate = 0; 
int SCarThrough = 0; 

}

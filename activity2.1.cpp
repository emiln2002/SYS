#include <iostream>
#include <mutex>
#include <condition_variable> 
#include <thread>
#include <chrono>

std::condition_variable cv_entry;
std::condition_variable cv_exit;  
std::mutex entry_;
std::mutex exit_; 
int Car_request_entry = 0;
int Car_Through_entry = 0; 
int EntryGate_Open = 0; 

int Car_request_exit = 0;
int Car_Through_exit = 0; 
int ExitGate_Open = 0; 


void car()
{
    // Bil venter et sekund, sender efterfølgende signal om at den er ved gate. 
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lk(entry_);
        Car_request_entry = 1; 
        std::cerr << "Car at entry gate.. \n";
        cv_entry.notify_all();
    }

        // Bilen venter på at gate åbner, kører efterfølgende gennem gate
        // og sender signal om gennemkørsel til gate. 
    {
        std::unique_lock<std::mutex> lk(entry_); 
        std::cerr << "Car waiting for open entry gate... \n"; 
        cv_entry.wait(lk, [] {return EntryGate_Open == 1; });
        std::cerr << "Car driving through entry gate... \n";
        Car_Through_entry = 1; 
        cv_entry.notify_all();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cerr << "Car parked. \n"; 
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cerr << "Car finished parking :) \nDriving to exit gate \n"; 


    // ***** EXIT CAR CODE *****
    {
        std::lock_guard<std::mutex> lk(exit_);
        Car_request_exit = 1; 
        std::cerr << "Waiting for open exit gate... \n";
        cv_exit.notify_all();
    }

    {
        std::unique_lock<std::mutex> lk(exit_); 
        cv_exit.wait(lk, [] {return ExitGate_Open == 1; }); 
        std::cerr << "Car driving through exit gade... \n"; 
        Car_Through_exit = 1; 
        cv_exit.notify_all();
    }


    }


void entryGate()
{
    // Gate venter på signal fra ankommet bil, sender efterfølgende signal om open gate. 
    {
        std::unique_lock<std::mutex> lk(entry_); 
        std::cerr << "Entry gate ready... \n"; 
        cv_entry.wait(lk, [] {return Car_request_entry == 1; });
        std::cerr << "Opening entry gate... \n";
        EntryGate_Open = 1;
        Car_request_entry = 0; 
        std::cerr << "Entry gate open.. \n";
        cv_entry.notify_all();
    }

    // Gate venter på at bil kører gennem
    {
        std::unique_lock<std::mutex> lk(entry_);
        std::cerr << "Entry gate waiting for car to drive through... \n"; 
        cv_entry.wait(lk, [] {return Car_Through_entry == 1; });
        std::cerr << "Car has driven through entry gate \n";
    }

    {
        std::lock_guard<std::mutex> lk(entry_); 
        Car_Through_entry = 0; 
        std::cerr << "Entry gate closing.. \n"; 
        EntryGate_Open = 0; 
        std::cerr << "Entry gate closed \n"; 
    }
}

void exitGate(){
    {
        std::unique_lock<std::mutex> lk(exit_); 
        cv_exit.wait(lk, [] {return Car_request_exit == 1; }); 
        std::cerr << "Opening exit gate.. \n";
        ExitGate_Open = 1; 
        Car_request_exit = 0; 
        std::cerr << "Exit gate open \n"; 
        cv_exit.notify_all(); 
    }

    {
        std::unique_lock<std::mutex> lk(exit_);
        std::cerr << "Exit gate waiting for car to drive through... \n"; 
        cv_exit.wait(lk, [] {return Car_Through_exit == 1; });
        std::cerr << "Car has driven through exit gate \n";
    }

    {
        std::lock_guard<std::mutex> lk(exit_); 
        Car_Through_exit = 0; 
        std::cerr << "Exit gate closing... \n"; 
        ExitGate_Open = 0; 
        std::cerr << "Exit gate closed \n"; 
    }

}


int main(){
    while(1){
    std::thread carThread(car);
    std::thread entryGateThread(entryGate);
    std::thread exitGateThread(exitGate);

    carThread.join();
    entryGateThread.join();
    exitGateThread.join(); 
    }
}
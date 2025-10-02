#include <iostream>
#include <mutex>
#include <condition_variable> 
#include <thread>
#include <chrono>
#include <queue>
#include <vector>
#include <cstdlib>
#include <ctime>

struct Gate {
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<int> waitingCars;  // Nummer bil for at overholde eventuel kø
    bool isOpen = false;          // Om gate pt er åben
    bool shutdown = false;
};

Gate entryGate, exitGate;
int parkedCars = 0; 
int maxParkingspace = 69; 


void car(int id, Gate &entryGate, Gate &exitGate)
{
    
    {
        std::lock_guard<std::mutex> lk(entryGate.mtx);
        entryGate.waitingCars.push(id); 
        std::cout << "Car " << id << " arrived at entry gate. \n";
        entryGate.cv.notify_all();
    }

  
    {
        std::unique_lock<std::mutex> lk(entryGate.mtx); 

        if(parkedCars == maxParkingspace){
            std::cout << "Park-A-Lot full\n";
            std::cout << "Car with id: " << id << " waiting...\n"; 
        }

        entryGate.cv.wait(lk, [&] {return entryGate.isOpen && entryGate.waitingCars.front() == id && parkedCars < maxParkingspace; });
        // Kører gennem entry gate: 
        std::cout << "Car " << id << " is passing through entry gate! \n";
        entryGate.waitingCars.pop();          // car pops itself
        entryGate.isOpen = false; 
        parkedCars ++; 

    }

    int parkingTime = 1 + (rand() % 5);
    std::this_thread::sleep_for(std::chrono::seconds(parkingTime));


    // ***** EXIT CAR CODE *****
    {
        std::lock_guard<std::mutex> lk(exitGate.mtx);
        exitGate.waitingCars.push(id);
        std::cout << "Car " << id << " arrived at exit gate! \n";
        exitGate.cv.notify_all();
    }

    {
        std::unique_lock<std::mutex> lk(exitGate.mtx); 
        exitGate.cv.wait(lk, [&] {return exitGate.isOpen && exitGate.waitingCars.front() == id; });
        // Kører gennem exit gate: 
        std::cout << "Car " << id << " is passing through exit gate :)\n";
        exitGate.waitingCars.pop();          // car pops itself
        exitGate.isOpen = false; 
        parkedCars--;
        entryGate.cv.notify_all();
    }

    std::cout << "Car " << id << " has left Park-A-lot!\n";
}


void gate(Gate &gate, std::string name)
{
       while (true) {
        std::unique_lock<std::mutex> lk(gate.mtx);

        // Wait until there is at least one car i køen
        gate.cv.wait(lk, [&] { return gate.shutdown || !gate.waitingCars.empty();});
        
        if (gate.shutdown && gate.waitingCars.empty()) {
            std::cout << "Cars no longer in line at " << name << " gate\n";
            break;
        }

        // Process cars in line, one at a time
        while (!gate.waitingCars.empty()) {
            int currentCar = gate.waitingCars.front();

            if(&gate == &entryGate){
                gate.cv.wait(lk, [&] {return gate.shutdown || parkedCars < maxParkingspace;});
                if (gate.shutdown) break; 
            }

            // Åben gate
            gate.isOpen = true;
            std::cout << name << " gate open for car " << currentCar << "\n";
            gate.cv.notify_all();

            // Wait until the car passes
            gate.cv.wait(lk, [&] { 
                return gate.waitingCars.empty() || gate.waitingCars.front() != currentCar; });
            gate.isOpen = false;
            std::cout << name << " gate closed.\n";
        }
    }

}



int main() {
    srand(time(0));

    std::string entryName = "Entry";
    std::thread entryThread(gate, std::ref(entryGate), entryName);

    std::string exitName = "Exit";
    std::thread exitThread(gate, std::ref(exitGate), exitName);

    // Create car threads
    std::vector<std::thread> cars;
    for (int i = 0; i < 420; ++i) {
        cars.emplace_back(car, i, std::ref(entryGate), std::ref(exitGate));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Wait for all cars to finish
    for (auto &t : cars) {
        t.join();
    }

    // Tell gates to stop
    {
        std::lock_guard<std::mutex> lock(entryGate.mtx);
        entryGate.shutdown = true;
    }
    entryGate.cv.notify_all();

    {
        std::lock_guard<std::mutex> lock(exitGate.mtx);
        exitGate.shutdown = true;
    }
    exitGate.cv.notify_all();

    // Join gate threads
    entryThread.join();
    exitThread.join();


    std::cout << "Simulation finished, all threads closed.\n";
}
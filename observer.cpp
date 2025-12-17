#include "observer.h"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace std::chrono;

std::mutex DeathObserver::logMutex;
std::mutex ConsoleObserver::coutMutex;

std::unique_lock<std::mutex> DeathObserver::getLock() {
    return std::unique_lock<std::mutex>(logMutex);
}

void ConsoleObserver::onDeath(const std::string& killer, const std::string& victim) {
    std::lock_guard lock(coutMutex);
    
    auto now = system_clock::now();
    auto now_time = system_clock::to_time_t(now);
    
    std::cout << "[БОЙ] " << std::put_time(std::localtime(&now_time), "%H:%M:%S") 
              << " - " << killer << " убил " << victim << std::endl;
}

FileObserver::FileObserver(const std::string& fname) : filename(fname) {
    std::lock_guard lock(logMutex);
    logFile.open(filename, std::ios::app);
    
    if (logFile.is_open()) {
        auto now = system_clock::now();
        auto now_time = system_clock::to_time_t(now);
        
        logFile << "\n=== НАЧАЛО СЕССИИ ЛОГИРОВАНИЯ ===" << std::endl;
        logFile << "Время: " << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") 
                << std::endl;
    }
}

FileObserver::~FileObserver() {
    std::lock_guard lock(logMutex);
    if (logFile.is_open()) {
        auto now = system_clock::now();
        auto now_time = system_clock::to_time_t(now);
        
        logFile << "=== КОНЕЦ СЕССИИ ЛОГИРОВАНИЯ ===" << std::endl;
        logFile << "Время: " << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") 
                << std::endl;
        logFile.close();
    }
}

void FileObserver::onDeath(const std::string& killer, const std::string& victim) {
    std::lock_guard lock(logMutex);
    
    if (logFile.is_open()) {
        auto now = system_clock::now();
        auto now_time = system_clock::to_time_t(now);
        
        logFile << std::put_time(std::localtime(&now_time), "%H:%M:%S") 
                << " - " << killer << " убил " << victim << std::endl;
    }
}
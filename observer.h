#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <mutex>

class DeathObserver {
protected:
    static std::mutex logMutex;
    
public:
    virtual void onDeath(const std::string& killer, const std::string& victim) = 0;
    virtual ~DeathObserver() = default;
    
    static std::unique_lock<std::mutex> getLock();
};

class ConsoleObserver : public DeathObserver {
private:
    static std::mutex coutMutex;
    
public:
    void onDeath(const std::string& killer, const std::string& victim) override;
};

class FileObserver : public DeathObserver {
private:
    std::ofstream logFile;
    std::string filename;
    
public:
    FileObserver(const std::string& filename);
    ~FileObserver();
    
    void onDeath(const std::string& killer, const std::string& victim) override;
    
    bool isFileOpen() const { return logFile.is_open(); }
};
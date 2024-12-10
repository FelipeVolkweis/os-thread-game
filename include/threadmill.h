#ifndef THREADMILL_H
#define THREADMILL_H

#include <SFML/Graphics.hpp>
#include <map>
#include <thread>
#include <mutex>
#include <semaphore> 

#include <package.h>
#include <constants.h>

class Threadmill {
public:
    Threadmill(int y, float packageSpeed);
    ~Threadmill();

    void addPackage(int id);
    void removePackage(int id);
    void setPackageSpeed(float newSpeed);

    void clearPackages();
    void activate();

    void deactivate();
    int getAndResetLostPackages();

    void draw(sf::RenderWindow& window, sf::Font& font);

    std::map<int, Package> getPackages();

private:
    void run();

    std::map<int, Package> packages_;
    int y_;
    float packageSpeed_;
    sf::RectangleShape threadmillShape_;
    std::thread thread_;
    std::mutex mtx_;
    std::binary_semaphore semaphore_; 
    bool isActive_;
    std::atomic<bool> stop_;
    std::mutex lostMutex_;
    int lostPackages_;

public:
    static const int width = THREADMILL_WIDTH;
    static const int height = THREADMILL_HEIGHT;
};

#endif  // THREADMILL_H
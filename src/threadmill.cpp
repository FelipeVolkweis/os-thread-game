#include <threadmill.h>

Threadmill::Threadmill(int y, float packageSpeed) 
        : y_(y), packageSpeed_(packageSpeed), semaphore_(0), isActive_(false), stop_(false),   
          lostPackages_(0) { 
        threadmillShape_.setSize(sf::Vector2f(THREADMILL_WIDTH, THREADMILL_HEIGHT));
        threadmillShape_.setFillColor(THREADMILL_COLOR);
        threadmillShape_.setPosition(0.0f, y_);

        thread_ = std::thread(&Threadmill::run, this);
    }

Threadmill::~Threadmill() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        stop_ = true;
        isActive_ = false;
    }
    semaphore_.release();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Threadmill::addPackage(int id) {
    std::lock_guard<std::mutex> lock(mtx_);
    float startY = y_ + (THREADMILL_HEIGHT - PACKAGE_SIZE) / 2.0f;
    packages_.emplace(id, Package(id, PACKAGE_START_X, startY, packageSpeed_));
}

void Threadmill::removePackage(int id) {
    std::lock_guard<std::mutex> lock(mtx_);
    packages_.erase(id);
}

void Threadmill::setPackageSpeed(float newSpeed) {
    std::lock_guard<std::mutex> lock(mtx_);
    packageSpeed_ = newSpeed;
    for (auto& [id, package] : packages_) {
        package.setSpeed(newSpeed);
    }
}

void Threadmill::clearPackages() {
    std::lock_guard<std::mutex> lock(mtx_);
    packages_.clear();
}

void Threadmill::activate() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!isActive_) { 
            isActive_ = true;
            semaphore_.release();
        }
    }
}

void Threadmill::deactivate() {
    std::lock_guard<std::mutex> lock(mtx_);
    isActive_ = false;
}

int Threadmill::getAndResetLostPackages() {
    std::lock_guard<std::mutex> lock(lostMutex_);
    int temp = lostPackages_;
    lostPackages_ = 0;
    return temp;
}

void Threadmill::draw(sf::RenderWindow& window, sf::Font& font) {
    std::lock_guard<std::mutex> lock(mtx_);
    window.draw(threadmillShape_);

    std::vector<Package*> sortedPackages;
    for (auto& [id, package] : packages_) {
        if (package.isValid()) {
            sortedPackages.push_back(&package);
        }
    }

    std::sort(sortedPackages.begin(), sortedPackages.end(), [](Package* a, Package* b) {
        return a->getX() < b->getX();
    });

    std::vector<std::vector<Package*>> groups;
    std::vector<bool> grouped(sortedPackages.size(), false);
    for (size_t i = 0; i < sortedPackages.size(); ++i) {
        if (grouped[i]) continue;
        std::vector<Package*> group;
        group.push_back(sortedPackages[i]);
        grouped[i] = true;
        float x1 = sortedPackages[i]->getX();
        float x2 = x1 + PACKAGE_SIZE;
        for (size_t j = i + 1; j < sortedPackages.size(); ++j) {
            float centerX = sortedPackages[j]->getX() + PACKAGE_SIZE / 2.0f;
            if (centerX >= x1 && centerX <= x2) {
                group.push_back(sortedPackages[j]);
                grouped[j] = true;
            }
        }
        groups.push_back(group);
    }

    for (auto& [id, package] : packages_) {
        if (package.isValid()) {
            package.draw(window);
        }
    }

    for (auto& group : groups) {
        if (group.size() > 1) {
            Package* topPackage = nullptr;
            float maxX = -1.0f;
            for (auto* pkg : group) {
                if (pkg->getX() > maxX) {
                    maxX = pkg->getX();
                    topPackage = pkg;
                }
            }
            if (topPackage) {
                sf::Text countText;
                countText.setFont(font);
                countText.setCharacterSize(SCORE_TEXT_SIZE);
                countText.setFillColor(sf::Color::White);
                countText.setString(std::to_string(group.size()));
                float textX = topPackage->getX() + PACKAGE_SIZE / 2.0f;
                float textY = topPackage->getBounds().top - 20.0f;
                countText.setPosition(textX, textY);
                window.draw(countText);
            }
        }
    }
}

std::map<int, Package> Threadmill::getPackages() {
    std::lock_guard<std::mutex> lock(mtx_);
    return packages_; 
}

void Threadmill::run() {
    while (true) {
        semaphore_.acquire(); 
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (stop_) break;
        }

        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx_);
                if (!isActive_ || stop_) {
                    break; 
                }
            }

            float deltaTime = 0.016f; // Approx. 60 FPS
            {
                std::lock_guard<std::mutex> lock(mtx_);
                std::vector<int> packagesToRemove;
                for (auto& [id, package] : packages_) {
                    if (package.isValid()) {
                        package.update(deltaTime);
                        if (package.getX() > WIDTH) {
                            packagesToRemove.push_back(id);
                            {
                                std::lock_guard<std::mutex> lostLock(lostMutex_);
                                lostPackages_++;
                            }
                        }
                    }
                }
                for (int id : packagesToRemove) {
                    packages_.erase(id);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (stop_) break;
        }
    }
}
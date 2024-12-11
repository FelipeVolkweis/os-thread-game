#ifndef PLAYER_HH
#define PLAYER_HH

#include <SFML/Graphics.hpp>

#include <constants.h>
#include <package.h>

class Player {
public:
    Player(const std::vector<int>& laneYs);

    void switchLane(int direction);

    void handleInput(float deltaTime);

    void draw(sf::RenderWindow& window);

    float getLeftX() const;

    float getRightX() const;
    
    bool canGrabPackage(const Package& package) const;

    int getCurrentLane() const;

    static bool loadTexture();
private:
    sf::Sprite shape_;
    std::vector<int> laneYs_;
    int currentLane_;

    static sf::Texture texture_;
};

#endif // PLAYER_HH
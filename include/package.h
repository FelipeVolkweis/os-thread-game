#ifndef PACKAGE_H
#define PACKAGE_H

#include <SFML/Graphics.hpp>

class Package {
public:
    Package(int id, float startX, float startY, float speed);

    Package();

    int getId() const;

    bool isValid() const;

    void update(float deltaTime);

    void draw(sf::RenderWindow& window);

    float getX() const;

    sf::FloatRect getBounds() const;

    void setSpeed(float speed);

private:
    int id_;
    float x_;
    float y_;
    float speed_;
    sf::RectangleShape shape_;
};

#endif  // PACKAGE_H
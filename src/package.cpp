#include <package.h>
#include <constants.h>

Package::Package(int id, float startX, float startY, float speed) 
    : id_(id), x_(startX), y_(startY), speed_(speed) {
    shape_.setSize(sf::Vector2f(PACKAGE_SIZE, PACKAGE_SIZE));
    shape_.setFillColor(PACKAGE_COLOR);
    shape_.setPosition(x_, y_);
}

Package::Package() 
    : id_(INVALID), x_(0), y_(0), speed_(0) {
    shape_.setSize(sf::Vector2f(PACKAGE_SIZE, PACKAGE_SIZE));
    shape_.setFillColor(PACKAGE_COLOR);
    shape_.setPosition(x_, y_);
}

int Package::getId() const {
    return id_;
}

bool Package::isValid() const {
    return id_ != INVALID;
}

void Package::update(float deltaTime) {
    x_ += speed_ * deltaTime;
    shape_.setPosition(x_, y_);
}

void Package::draw(sf::RenderWindow& window) {
    window.draw(shape_);
}

float Package::getX() const {
    return x_;
}

sf::FloatRect Package::getBounds() const {
    return shape_.getGlobalBounds();
}

void Package::setSpeed(float speed) {
    speed_ = speed;
}
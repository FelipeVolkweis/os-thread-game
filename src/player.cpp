#include <iostream>

#include <player.h>

sf::Texture Player::texture_;

Player::Player(const std::vector<int> &laneYs) : laneYs_(laneYs), currentLane_(1) {
    // shape_.setSize(sf::Vector2f(PLAYER_SIZE, PLAYER_SIZE));
    // shape_.setFillColor(PLAYER_COLOR);
    if (!Player::loadTexture()) {
        std::cout << "Error loading player texture." << std::endl;
    }
    float scaleX = PLAYER_SIZE / texture_.getSize().x;
    float scaleY = PLAYER_SIZE / texture_.getSize().y;
    shape_.setScale(scaleX, scaleY);
    shape_.setTexture(texture_);
    shape_.setPosition(WIDTH / 2 - PLAYER_SIZE / 2,
                       laneYs_[currentLane_] + THREADMILL_HEIGHT + PLAYER_OFFSET_Y);
}

void Player::switchLane(int direction) {
    int newLane = currentLane_ + direction;
    if (newLane >= MIN_LANE && newLane <= MAX_LANE) {
        currentLane_ = newLane;
        shape_.setPosition(shape_.getPosition().x,
                           laneYs_[currentLane_] + THREADMILL_HEIGHT + PLAYER_OFFSET_Y);
    }
}

void Player::handleInput(float deltaTime) {
    float movement = PLAYER_SPEED * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        if (shape_.getPosition().x - movement >= 0)
            shape_.move(-movement, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        if (shape_.getPosition().x + movement + PLAYER_SIZE <= WIDTH)
            shape_.move(movement, 0);
    }
}

void Player::draw(sf::RenderWindow &window) {
    window.draw(shape_);
}

float Player::getLeftX() const {
    return shape_.getPosition().x;
}

float Player::getRightX() const {
    return shape_.getPosition().x + PLAYER_SIZE;
}

bool Player::canGrabPackage(const Package &package) const {
    float playerLeft = getLeftX();
    float playerRight = getRightX();
    float packageCenterX = package.getX() + (PACKAGE_SIZE / 2.0f);
    return (packageCenterX >= playerLeft) && (packageCenterX <= playerRight);
}

int Player::getCurrentLane() const {
    return currentLane_;
}

bool Player::loadTexture() {
    return texture_.loadFromFile(PLAYER_TEXTURE_PATH);
}
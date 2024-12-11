#include <iostream>

#include <player.h>

sf::Texture Player::texture_;

/**
 * @brief Construtor da classe Player.
 *
 * Inicializa um objeto Player com as posições das pistas fornecidas.
 * Carrega a textura do jogador e configura a escala e a posição da forma do jogador.
 *
 * @param laneYs Vetor contendo as posições Y das pistas.
 */
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

/**
 * @brief Altera a faixa do jogador.
 *
 * Esta função altera a faixa do jogador com base na direção fornecida.
 * A nova faixa é calculada adicionando a direção à faixa atual. Se a nova
 * faixa estiver dentro dos limites permitidos (MIN_LANE e MAX_LANE), a faixa
 * atual do jogador é atualizada e a posição do jogador é ajustada
 * de acordo.
 *
 * @param direction A direção para a qual o jogador deve mudar de faixa.
 *                  Pode ser um valor positivo (para a direita) ou negativo
 *                  (para a esquerda).
 */
void Player::switchLane(int direction) {
    int newLane = currentLane_ + direction;
    if (newLane >= MIN_LANE && newLane <= MAX_LANE) {
        currentLane_ = newLane;
        shape_.setPosition(shape_.getPosition().x,
                           laneYs_[currentLane_] + THREADMILL_HEIGHT + PLAYER_OFFSET_Y);
    }
}

/**
 * @brief Manipula a entrada do jogador para mover o personagem.
 *
 * Esta função verifica se as teclas de movimento (A, D, Esquerda, Direita) estão pressionadas
 * e move o personagem na direção correspondente. O movimento é limitado pelas bordas da tela.
 *
 * @param deltaTime O tempo decorrido desde a última atualização, usado para calcular a distância de movimento.
 */
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

/**
 * @brief Verifica se o jogador pode pegar o pacote.
 *
 * Esta função verifica se o jogador está na posição correta para pegar o pacote.
 * O jogador pode pegar o pacote se o centro do pacote estiver entre as coordenadas
 * esquerda e direita do jogador.
 *
 * @param package Referência constante para o objeto Package que representa o pacote.
 * @return true se o jogador pode pegar o pacote, false caso contrário.
 */
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
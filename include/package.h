#ifndef PACKAGE_H
#define PACKAGE_H

#include <SFML/Graphics.hpp>

/**
 * @class Package
 * @brief Representa um pacote que pode ser desenhado e atualizado em uma janela de renderização.
 *
 * A classe Package encapsula as propriedades e comportamentos de um pacote, incluindo sua posição,
 * velocidade e forma gráfica. Ela fornece métodos para obter o ID do pacote, verificar sua validade,
 * atualizar sua posição com base no tempo decorrido, desenhá-lo em uma janela de renderização,
 * e ajustar sua velocidade. Além disso, a classe gerencia uma textura estática compartilhada entre
 * todas as instâncias de Package.
 */
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

    static bool loadTexture();

private:
    int id_;
    float x_;
    float y_;
    float speed_;
    sf::Sprite shape_;

    static sf::Texture texture_;
};

#endif  // PACKAGE_H
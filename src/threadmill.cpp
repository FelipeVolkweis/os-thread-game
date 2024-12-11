#include <iostream>

#include <threadmill.h>

sf::Texture Threadmill::threadmillTexture_;
bool Threadmill::loaded = false;

/**
 * @brief Construtor da classe Threadmill.
 *
 * Inicializa uma instância da esteira com a posição vertical e a velocidade do pacote especificadas.
 * Configura a textura e a escala da esteira, e inicia a thread de execução.
 *
 * @param y Posição vertical da esteira.
 * @param packageSpeed Velocidade do pacote na esteira.
 */
Threadmill::Threadmill(int y, float packageSpeed)
    : y_(y), packageSpeed_(packageSpeed), semaphore_(0), isActive_(false), stop_(false),
      lostPackages_(0) {
    // threadmillShape_.setSize(sf::Vector2f(THREADMILL_WIDTH, THREADMILL_HEIGHT));
    // threadmillShape_.setFillColor(THREADMILL_COLOR);
    if (!loaded) {
        if (!loadTexture()) {
            std::cout << "Failed to load threadmill texture" << std::endl;
        } else {
            loaded = true;
        }
    }
    float scaleX = static_cast<float>(THREADMILL_WIDTH) / threadmillTexture_.getSize().x;
    float scaleY = static_cast<float>(THREADMILL_HEIGHT) / threadmillTexture_.getSize().y;
    threadmillShape_.setScale(scaleX, scaleY);
    threadmillShape_.setTexture(threadmillTexture_);
    threadmillShape_.setPosition(0.0f, y_);

    thread_ = std::thread(&Threadmill::run, this);
}

/**
 * @brief Destrutor da classe Threadmill.
 *
 * Este destrutor garante que a thread associada à instância de Threadmill
 * seja corretamente finalizada. Ele faz isso definindo a flag `stop_` como
 * verdadeira e `isActive_` como falsa dentro de um bloco protegido por um
 * mutex. Em seguida, libera o semáforo e, se a thread estiver em um estado
 * "joinable", chama `join` para esperar que a thread termine sua execução.
 */
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

/**
 * @brief Adiciona um pacote à esteira.
 * 
 * Esta função adiciona um novo pacote à lista de pacotes da esteira. 
 * O pacote é identificado por um ID único e é posicionado na coordenada 
 * inicial da esteira com uma velocidade predefinida.
 * 
 * @param id Identificador único do pacote.
 */
void Threadmill::addPackage(int id) {
    std::lock_guard<std::mutex> lock(mtx_);
    float startY = y_ + (THREADMILL_HEIGHT - PACKAGE_SIZE) / 2.0f;
    packages_.emplace(id, Package(id, PACKAGE_START_X, startY, packageSpeed_));
}

/**
 * @brief Remove um pacote da lista de pacotes.
 * 
 * Esta função remove um pacote identificado pelo seu ID da lista de pacotes.
 * A operação é protegida por um mutex para garantir a segurança em ambientes
 * multithread.
 * 
 * @param id O identificador do pacote a ser removido.
 */
void Threadmill::removePackage(int id) {
    std::lock_guard<std::mutex> lock(mtx_);
    packages_.erase(id);
}

/**
 * @brief Define a nova velocidade dos pacotes na esteira.
 * 
 * Esta função define a velocidade dos pacotes na esteira para um novo valor especificado.
 * A função é thread-safe, utilizando um mutex para garantir que a operação seja atômica.
 * 
 * @param newSpeed A nova velocidade a ser definida para os pacotes.
 */
void Threadmill::setPackageSpeed(float newSpeed) {
    std::lock_guard<std::mutex> lock(mtx_);
    packageSpeed_ = newSpeed;
    for (auto &[id, package] : packages_) {
        package.setSpeed(newSpeed);
    }
}

/**
 * @brief Limpa todos os pacotes armazenados.
 *
 * Esta função adquire um bloqueio no mutex `mtx_` para garantir que a operação
 * de limpeza seja realizada de forma segura em um ambiente multithread. Em seguida,
 * ela limpa o contêiner `packages_`, removendo todos os pacotes armazenados.
 */
void Threadmill::clearPackages() {
    std::lock_guard<std::mutex> lock(mtx_);
    packages_.clear();
}

/**
 * @brief Ativa a Threadmill.
 *
 * Esta função ativa a Threadmill se ela ainda não estiver ativa. 
 * Utiliza um lock_guard para garantir que a operação de ativação 
 * seja thread-safe. Se a Threadmill não estiver ativa, ela é 
 * marcada como ativa e um semáforo é liberado.
 */
void Threadmill::activate() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!isActive_) {
            isActive_ = true;
            semaphore_.release();
        }
    }
}

/**
 * @brief Desativa a Threadmill.
 *
 * Esta função desativa a Threadmill, definindo a variável isActive_ como false.
 * A função utiliza um std::lock_guard para garantir que a operação seja thread-safe,
 * bloqueando o mutex mtx_ durante a execução.
 */
void Threadmill::deactivate() {
    std::lock_guard<std::mutex> lock(mtx_);
    isActive_ = false;
}

/**
 * @brief Retorna o número de pacotes perdidos e reseta o contador.
 *
 * Esta função obtém o número atual de pacotes perdidos, reseta o contador de pacotes perdidos para zero
 * e retorna o número de pacotes perdidos antes do reset.
 *
 * @return int O número de pacotes perdidos antes do reset.
 */
int Threadmill::getAndResetLostPackages() {
    std::lock_guard<std::mutex> lock(lostMutex_);
    int temp = lostPackages_;
    lostPackages_ = 0;
    return temp;
}

/**
 * @brief Desenha o estado atual do Threadmill na janela fornecida.
 *
 * Esta função desenha a forma do Threadmill, os pacotes válidos e os textos de contagem de pacotes agrupados na janela fornecida.
 *
 * @param window Referência para a janela onde os elementos serão desenhados.
 * @param font Referência para a fonte usada para desenhar os textos de contagem.
 */
void Threadmill::draw(sf::RenderWindow &window, sf::Font &font) {
    std::lock_guard<std::mutex> lock(mtx_);
    window.draw(threadmillShape_);

    std::vector<Package *> sortedPackages;
    for (auto &[id, package] : packages_) {
        if (package.isValid()) {
            sortedPackages.push_back(&package);
        }
    }

    std::sort(sortedPackages.begin(), sortedPackages.end(),
              [](Package *a, Package *b) { return a->getX() < b->getX(); });

    std::vector<std::vector<Package *>> groups;
    std::vector<bool> grouped(sortedPackages.size(), false);
    for (size_t i = 0; i < sortedPackages.size(); ++i) {
        if (grouped[i])
            continue;
        std::vector<Package *> group;
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

    for (auto &[id, package] : packages_) {
        if (package.isValid()) {
            package.draw(window);
        }
    }

    for (auto &group : groups) {
        if (group.size() > 1) {
            Package *topPackage = nullptr;
            float maxX = -1.0f;
            for (auto *pkg : group) {
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

/**
 * @brief Obtém um mapa de pacotes.
 * 
 * Esta função retorna um mapa contendo os pacotes armazenados no objeto Threadmill.
 * A função é thread-safe, utilizando um lock_guard para garantir que o acesso ao
 * mapa de pacotes seja protegido por um mutex.
 * 
 * @return std::map<int, Package> Um mapa onde a chave é um inteiro e o valor é um objeto Package.
 */
std::map<int, Package> Threadmill::getPackages() {
    std::lock_guard<std::mutex> lock(mtx_);
    return packages_;
}

/**
 * @brief Método principal de execução da Threadmill.
 *
 * Este método é executado em um loop contínuo até que a sinalização de parada seja recebida.
 * Ele adquire um semáforo para controlar o acesso e utiliza mutexes para garantir a segurança
 * em operações concorrentes. O método processa pacotes, atualizando-os a cada iteração com
 * base em um deltaTime fixo, simulando aproximadamente 60 FPS. Pacotes que se tornam inválidos
 * ou ultrapassam uma largura definida são removidos e contabilizados como perdidos.
 *
 * @note O método utiliza um loop interno adicional para processar pacotes enquanto a Threadmill
 *       estiver ativa e não tiver recebido a sinalização de parada.
 */
void Threadmill::run() {
    while (true) {
        semaphore_.acquire();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (stop_)
                break;
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
                for (auto &[id, package] : packages_) {
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
            if (stop_)
                break;
        }
    }
}

bool Threadmill::loadTexture() {
    return threadmillTexture_.loadFromFile(THREADMILL_TEXTURE_PATH);
}
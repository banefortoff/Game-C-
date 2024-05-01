#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>

#include <iostream>

const int BLOCK_SIZE = 50;
const int WIDTH = 16;
const int HEIGHT = 10;
const int COLORS = 3;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 1024;
const sf::Color BACKGROUND_COLOR = sf::Color(64, 224, 208);
const sf::Color TILE_BACKGROUND_COLOR = sf::Color::Black;

enum class Color { Red, Green, Blue };

class Block {
public:
    Block(Color color = Color::Red) : color(color), isActive(true) {}

    void setColor(Color newColor) {
        color = newColor;
    }

    Color getColor() const {
        return color;
    }

    void setActive(bool active) {
        isActive = active;
    }

    bool isActiveBlock() const {
        return isActive;
    }

    bool isSameColor(const Block& other) const {
        return color == other.color;
    }

private:
    Color color;
    bool isActive;
};

class Game {
public:
    Game() : gameStarted(false) {

        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Error loading default font" << std::endl;
        }

        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        text.setPosition(10.f, 10.f);

        startButton.setSize(sf::Vector2f(100.f, 50.f));
        startButton.setPosition(SCREEN_WIDTH - 120.f, 10.f);
        startButton.setFillColor(sf::Color::Green);
        startButton.setOutlineColor(sf::Color::Black);
        startButton.setOutlineThickness(2.f);

        startButtonText.setFont(font);
        startButtonText.setString("Start");
        startButtonText.setFillColor(sf::Color::Black);

        sf::FloatRect textBounds = startButtonText.getLocalBounds();
        startButtonText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
        startButtonText.setPosition(startButton.getPosition() + sf::Vector2f(startButton.getSize().x / 2.0f, startButton.getSize().y / 2.0f));
    }

    void run() {
        sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Game", sf::Style::Titlebar | sf::Style::Close);
        while (window.isOpen()) {
            handleEvents(window);
            if (gameStarted) {
                render(window);
            } else {
                renderStartScreen(window);
            }
        }
    }

    private:
        std::vector<std::vector<Block>> blocks;
        sf::Font font;
        sf::Text text;
        sf::RectangleShape startButton;
        sf::Text startButtonText;
        bool gameStarted;

    void resetGame() {
        std::srand(std::time(nullptr));
        blocks.clear();
        for (int y = 0; y < HEIGHT; ++y) {
            std::vector<Block> row;
            for (int x = 0; x < WIDTH; ++x) {
                Color color = static_cast<Color>(std::rand() % COLORS);
                row.push_back(Block(color));
            }
            blocks.push_back(row);
        }
    }

    void handleEvents(sf::RenderWindow& window) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    if (startButton.getGlobalBounds().contains(mousePos)) {
                        gameStarted = true;
                        resetGame();
                    }
                    if (gameStarted) {
                        int x = (event.mouseButton.x - (SCREEN_WIDTH - WIDTH * BLOCK_SIZE) / 2) / BLOCK_SIZE;
                        int y = (event.mouseButton.y - (SCREEN_HEIGHT - HEIGHT * BLOCK_SIZE) / 2) / BLOCK_SIZE;
                        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                            removeConnectedBlocks(x, y);
                            moveBlocksDown();
                        }
                    }
                }
            }
            if (gameStarted && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    resetGame();
                }
            }
        }
    }

    void removeConnectedBlocks(int x, int y) {
        Color color = blocks[y][x].getColor();
        if (!blocks[y][x].isActiveBlock())
            return;

        std::vector<sf::Vector2i> toRemove;
        std::vector<sf::Vector2i> queue{{x, y}};
        while (!queue.empty()) {
            sf::Vector2i current = queue.back();
            queue.pop_back();
            int cx = current.x;
            int cy = current.y;
            if (cx < 0 || cx >= WIDTH || cy < 0 || cy >= HEIGHT || !blocks[cy][cx].isActiveBlock())
                continue;
            if (!blocks[cy][cx].isSameColor(blocks[y][x]))
                continue;
            toRemove.push_back(current);
            blocks[cy][cx].setActive(false);
            queue.push_back({cx - 1, cy});
            queue.push_back({cx + 1, cy});
            queue.push_back({cx, cy - 1});
            queue.push_back({cx, cy + 1});
        }

        bool hasNeighbor = false;
        for (const auto& pos : toRemove) {
            int cx = pos.x;
            int cy = pos.y;
            if ((cx - 1 >= 0 && blocks[cy][cx - 1].isSameColor(blocks[cy][cx])) ||
                (cx + 1 < WIDTH && blocks[cy][cx + 1].isSameColor(blocks[cy][cx])) ||
                (cy - 1 >= 0 && blocks[cy - 1][cx].isSameColor(blocks[cy][cx])) ||
                (cy + 1 < HEIGHT && blocks[cy + 1][cx].isSameColor(blocks[cy][cx]))) {
                hasNeighbor = true;
                break;
            }
        }

        if (toRemove.size() >= 3 && hasNeighbor) {
            for (const auto& pos : toRemove) {
                blocks[pos.y][pos.x].setActive(false);
            }
        } else {
            for (const auto& pos : toRemove) {
                blocks[pos.y][pos.x].setActive(true);
            }
        }

    }

    void moveBlocksDown() {
        for (int x = 0; x < WIDTH; ++x) {
            int emptySpaces = 0;
            for (int y = HEIGHT - 1; y >= 0; --y) {
                if (!blocks[y][x].isActiveBlock()) {
                    emptySpaces++;
                    blocks[y][x].setColor(Color::Red);
                } else if (emptySpaces > 0) {
                    blocks[y + emptySpaces][x] = blocks[y][x];
                    blocks[y][x].setActive(false);
                }
            }
        }
    }

    void render(sf::RenderWindow& window) {
        window.clear(BACKGROUND_COLOR);

        sf::RectangleShape background(sf::Vector2f(WIDTH * BLOCK_SIZE, HEIGHT * BLOCK_SIZE));
        background.setFillColor(TILE_BACKGROUND_COLOR);
        background.setPosition((SCREEN_WIDTH - WIDTH * BLOCK_SIZE) / 2, (SCREEN_HEIGHT - HEIGHT * BLOCK_SIZE) / 2);
        window.draw(background);

        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                if (blocks[y][x].isActiveBlock()) {
                    sf::RectangleShape tile(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
                    tile.setPosition(x * BLOCK_SIZE + (SCREEN_WIDTH - WIDTH * BLOCK_SIZE) / 2,
                                        y * BLOCK_SIZE + (SCREEN_HEIGHT - HEIGHT * BLOCK_SIZE) / 2);
                    switch (blocks[y][x].getColor()) {
                        case Color::Red:
                            tile.setFillColor(sf::Color::Red);
                            break;
                        case Color::Green:
                            tile.setFillColor(sf::Color::Green);
                            break;
                        case Color::Blue:
                            tile.setFillColor(sf::Color::Blue);
                            break;
                    }
                    window.draw(tile);

                    sf::RectangleShape border(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
                    border.setPosition(tile.getPosition());
                    border.setFillColor(sf::Color::Transparent);
                    border.setOutlineThickness(2);
                    border.setOutlineColor(sf::Color::Black);
                    window.draw(border);
                }
            }
        }

        window.draw(text);
        window.draw(startButton);
        window.draw(startButtonText);
        window.display();
    }

    void renderStartScreen(sf::RenderWindow& window) {
        window.clear(BACKGROUND_COLOR);
        window.draw(startButton);
        window.draw(startButtonText);
        window.display();
    }
};

int main() {
    Game game;
    game.run();

    return 0;
}

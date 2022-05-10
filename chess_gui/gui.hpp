/*
 * Hotkeys:
 *  Escape   (close)
 *  C        (clear)
 *  F        (unlock fps)
 *  Num1     (set active color to blue)
 *  Numpad1  (set active color to blue)
 *  Num2     (set active color to red)
 *  Numpad2  (set active color to red)
 *  MouseL   (add point)
 *  MouseR   (remove point)
 */

#include <SFML/Graphics.hpp>
#include <array>
#include <cassert>
#include <iostream>
#include <vector>

const std::string image = "SFML/chess_gui/ChessPieces.png";

class Window : public sf::RenderWindow {
   public:
    Window(uint32_t resolution, uint32_t fps_max);
    void toggleFpsLock();

   private:
    const uint32_t fpsMax;
    bool is_fps_locked = true;
};

class Processing {
   public:
    Processing(Window& window, sf::Vector2u board_size = sf::Vector2u(8, 8),
               sf::Color light = sf::Color(220, 220, 160),
               sf::Color dark = sf::Color(80, 80, 40));
    void update();

   private:
    enum Piece {
        king_white,
        queen_white,
        bishop_white,
        knight_white,
        rook_white,
        pawn_white,
        king_black,
        queen_black,
        bishop_black,
        knight_black,
        rook_black,
        pawn_black
    };
    Window& window;
    const sf::Vector2u boardSize;
    const sf::Color light, dark;
    sf::Texture pieces_texture_;
    float piece_size_px;
    std::vector<sf::Sprite> pieces_sprites_;
    std::string FEN;

    void drawBoard(float cell_size) const;
    void loadPieces(std::string path = image);
    void drawPieces(float cell_size);
};

class Events {
   public:
    Events(Window& window, Processing& processing)
        : window(window), processing_(processing) {}
    void handle();

   private:
    sf::Event event_;
    Window& window;
    Processing& processing_;
    int point_category_ = -1;

    void handleKeyboard();
    void handleMouse();
};

Window::Window(uint32_t resolution, uint32_t fps_max)
    : sf::RenderWindow(sf::VideoMode(resolution, resolution), "Chess GUI"),
      fpsMax(fps_max) {
    setFramerateLimit(fps_max);
    setMouseCursorVisible(true);
}

void Window::toggleFpsLock() {
    is_fps_locked = !is_fps_locked;
    setFramerateLimit(is_fps_locked ? fpsMax : 0);
}

Processing::Processing(Window& window, sf::Vector2u board_size, sf::Color light,
                       sf::Color dark)
    : window(window),
      boardSize(board_size),
      light(light),
      dark(dark),
    //   FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
    FEN("rnbqkbnr/ppp2ppp/4p3/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 1") {
    loadPieces();
}

void Processing::update() {
    float cell_size = static_cast<float>(std::min(
        window.getSize().x / boardSize.x, window.getSize().y / boardSize.y));
    window.clear();
    drawBoard(cell_size);
    drawPieces(cell_size);
    window.display();
}

void Processing::drawBoard(float cell_size) const {
    sf::RectangleShape cell({cell_size, cell_size});
    sf::Vector2u pos;
    for (pos.x = 0; pos.x < boardSize.x; ++pos.x) {
        for (pos.y = 0; pos.y < boardSize.y; ++pos.y) {
            cell.setFillColor((pos.x ^ pos.y) & 1 ? dark : light);
            cell.setPosition(static_cast<sf::Vector2f>(pos) * cell_size);
            window.draw(cell);
        }
    }
}

void Processing::loadPieces(std::string path) {
    assert(pieces_texture_.loadFromFile(path));
    int s = static_cast<int>(pieces_texture_.getSize().x / 6);
    for (int j = 0; j < 2; ++j) {
        for (int i = 0; i < 6; ++i) {
            pieces_sprites_.push_back(sf::Sprite(pieces_texture_));
            pieces_sprites_.back().setTextureRect(
                sf::IntRect(i * s, j * s, s, s));
        }
    }
    piece_size_px = static_cast<float>(s);
}

void Processing::drawPieces(float cell_size) {
    int index = 0;
    for (auto i : FEN) {
        if (std::isdigit(i)) {
            index += i - '0';
        } else if (i == '/') {
            assert(index % 8 == 0);
        } else if (std::isalpha(i)) {
            Piece piece = king_white;
            switch (i) {
                case 'K':
                    piece = king_white;
                    break;
                case 'k':
                    piece = king_black;
                    break;
                case 'Q':
                    piece = queen_white;
                    break;
                case 'q':
                    piece = queen_black;
                    break;
                case 'R':
                    piece = rook_white;
                    break;
                case 'r':
                    piece = rook_black;
                    break;
                case 'B':
                    piece = bishop_white;
                    break;
                case 'b':
                    piece = bishop_black;
                    break;
                case 'N':
                    piece = knight_white;
                    break;
                case 'n':
                    piece = knight_black;
                    break;
                case 'P':
                    piece = pawn_white;
                    break;
                case 'p':
                    piece = pawn_black;
                    break;
            }
            pieces_sprites_[piece].setScale(
                {cell_size / piece_size_px, cell_size / piece_size_px});
            pieces_sprites_[piece].setPosition(
                static_cast<float>(index % 8) * cell_size,
                static_cast<float>(index / 8) * cell_size);
            window.draw(pieces_sprites_[piece]);
            ++index;
        } else {
            assert(index == 64);
            break;
        }
    }
}

void Events::handle() {
    while (window.pollEvent(event_)) {
        switch (event_.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::Resized:
                window.setView(sf::View(sf::FloatRect(
                    0.f, 0.f, static_cast<float>(event_.size.width),
                    static_cast<float>(event_.size.height))));
                break;
            case sf::Event::KeyPressed:
                handleKeyboard();
                break;
            case sf::Event::MouseButtonPressed:
                handleMouse();
                break;
            default:
                break;
        }
    }
}

void Events::handleKeyboard() {
    switch (event_.key.code) {
        case sf::Keyboard::Escape:
            window.close();
            break;
        case sf::Keyboard::F:
            window.toggleFpsLock();
            break;
        default:
            break;
    }
}

void Events::handleMouse() {}

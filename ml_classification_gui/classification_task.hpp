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
#include <iostream>
#include <vector>

class Window : public sf::RenderWindow {
   public:
    Window(uint32_t fps_max);
    void toggleFpsLock();

   private:
    const uint32_t fpsMax;
    bool is_fps_locked = true;
};

template <typename Classifier>
class Processing {
   public:
    Processing(Window& window, float learning_rate, float points_radius);
    void addPoint(sf::Vector2f, int);
    void removePoint(sf::Vector2f);
    void clear();
    void update(uint32_t eras);

   private:
    Window& window;
    Classifier classifier_;
    std::vector<sf::Vector2f> points_pos_;
    std::vector<int> points_category_;  // is binary ([-1, 1])
    const float pointRadius;

    std::array<float, 2> posScaled(sf::Vector2f) const;
    void trainClassifier(uint32_t eras);
    int predict(sf::Vector2u) const;
    void drawBackground() const;
    void drawForeground() const;
};

template <typename Classifier>
class Events {
   public:
    Events(Window& window, Processing<Classifier>& processing)
        : window(window), processing_(processing) {}
    void handle();

   private:
    sf::Event event;
    Window& window;
    Processing<Classifier>& processing_;
    int point_category_ = -1;

    void handleKeyboard();
    void handleMouse();
};

Window::Window(uint32_t fps_max)
    : sf::RenderWindow(sf::VideoMode::getDesktopMode(), "Classification",
                       sf::Style::Fullscreen),
      fpsMax(fps_max) {
    setFramerateLimit(fps_max);
    setMouseCursorVisible(true);
}

void Window::toggleFpsLock() {
    is_fps_locked = !is_fps_locked;
    setFramerateLimit(is_fps_locked ? fpsMax : 0);
}

template <typename Classifier>
Processing<Classifier>::Processing(Window& window, float learning_rate,
                                   float point_radius)
    : window(window), classifier_(learning_rate), pointRadius(point_radius) {}

template <typename Classifier>
void Processing<Classifier>::addPoint(sf::Vector2f pos, int category) {
    points_pos_.push_back({pos.x, pos.y});
    points_category_.push_back(category);
}

template <typename Classifier>
void Processing<Classifier>::removePoint(sf::Vector2f pos) {
    auto sq = [](float x) { return x * x; };
    for (size_t i = 0; i < points_pos_.size(); ++i) {
        if (sq(pointRadius) >
            sq(points_pos_[i].x - pos.x) + sq(points_pos_[i].y - pos.y)) {
            ptrdiff_t i_ = static_cast<ptrdiff_t>(i);
            points_pos_.erase(points_pos_.begin() + i_);
            points_category_.erase(points_category_.begin() + i_);
        }
    }
}

template <typename Classifier>
void Processing<Classifier>::clear() {
    points_pos_.clear();
    points_category_.clear();
    classifier_.initialize();
}

template <typename Classifier>
void Processing<Classifier>::update(uint32_t eras) {
    trainClassifier(eras);
    window.clear();
    drawBackground();
    drawForeground();
    window.display();
}

template <typename Classifier>
std::array<float, 2> Processing<Classifier>::posScaled(sf::Vector2f pos) const {
    sf::Vector2f scale(static_cast<sf::Vector2f>(window.getSize()));
    return {2 * pos.x / scale.x - 1, 2 * pos.y / scale.y - 1};
}

template <typename Classifier>
void Processing<Classifier>::trainClassifier(uint32_t eras) {
    std::vector<std::array<float, 2>> points_pos_scaled(points_pos_.size());
    for (size_t i = 0; i != points_pos_.size(); ++i) {
        points_pos_scaled[i] = posScaled(points_pos_[i]);
    }
    classifier_.fit(points_pos_scaled, points_category_, eras);
}

template <typename Classifier>
int Processing<Classifier>::predict(sf::Vector2u pos) const {
    return classifier_.predict(
        posScaled(static_cast<sf::Vector2f>(sf::Vector2u(pos.x, pos.y))));
}

template <typename Classifier>
void Processing<Classifier>::drawBackground() const {
    sf::VertexArray line(sf::Lines, 2);
    sf::Vector2u size = window.getSize();
    for (uint32_t i = 0; i != size.y; ++i) {
        int category = predict({0, i});
        line[0].position = {0, static_cast<float>(i)};
        line[0].color = line[1].color =
            category == 1 ? sf::Color::Magenta : sf::Color::Cyan;
        if (category != predict({size.x - 1, i})) {
            uint32_t l = 0, r = size.x, m;
            while (r - l > 1) {
                m = (l + r) / 2;
                (category == predict({m, i}) ? l : r) = m;
            }
            line[1].position = {static_cast<float>(l), static_cast<float>(i)};
            window.draw(line);
            std::swap(line[0].position, line[1].position);
            line[0].color = line[1].color =
                !(category == 1) ? sf::Color::Magenta : sf::Color::Cyan;
        }
        line[1].position = {static_cast<float>(size.x - 1),
                            static_cast<float>(i)};
        window.draw(line);
    }
}

template <typename Classifier>
void Processing<Classifier>::drawForeground() const {
    static sf::CircleShape shape_(pointRadius);
    for (size_t i = 0; i != points_pos_.size(); ++i) {
        shape_.setFillColor(points_category_[i] == 1 ? sf::Color::Red
                                                     : sf::Color::Blue);
        shape_.setPosition(points_pos_[i] -
                           sf::Vector2f(pointRadius, pointRadius));
        window.draw(shape_);
    }
}

template <typename Classifier>
void Events<Classifier>::handle() {
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
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

template <typename Classifier>
void Events<Classifier>::handleKeyboard() {
    switch (event.key.code) {
        case sf::Keyboard::Escape:
            window.close();
            break;
        case sf::Keyboard::C:
            processing_.clear();
            break;
        case sf::Keyboard::F:
            window.toggleFpsLock();
            break;
        case sf::Keyboard::Num1:
            point_category_ = -1;
            break;
        case sf::Keyboard::Numpad1:
            point_category_ = -1;
            break;
        case sf::Keyboard::Num2:
            point_category_ = 1;
            break;
        case sf::Keyboard::Numpad2:
            point_category_ = 1;
            break;
        default:
            break;
    }
}

template <typename Classifier>
void Events<Classifier>::handleMouse() {
    sf::Vector2f pos(static_cast<sf::Vector2f>(
        sf::Vector2i(event.mouseButton.x, event.mouseButton.y)));
    switch (event.mouseButton.button) {
        case sf::Mouse::Left:
            processing_.addPoint(pos, point_category_);
            break;
        case sf::Mouse::Right:
            processing_.removePoint(pos);
            break;
        default:
            break;
    }
}

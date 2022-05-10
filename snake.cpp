#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <vector>

class Frame {
 public:
  Frame(const sf::Vector2u&);
  const sf::Image& get_image() const { return image; }
  uint32_t GetColor(uint32_t, uint32_t) const;
  void SetColor(uint32_t, uint32_t, uint32_t);
  void Clear();
  void Randomize();
  void Update();
  static uint32_t FlipColor(uint32_t);

  const uint32_t width;
  const uint32_t height;

 private:
  sf::Image image;
  uint32_t* image_ptr;
};

struct GUI {
  sf::RenderWindow window;
  sf::Texture texture;
  sf::Sprite sprite;
  const sf::Clock clock;
  const uint32_t cell_size;
  const uint32_t fps_max;
  bool is_paused;

  GUI(uint32_t cell_size, uint32_t fps_max);
  void display(const Frame&);
};

class Events {
 public:
  Events(GUI& gui, Frame& table) : gui(gui), table(table) {}
  void handle();

 private:
  sf::Event event;
  GUI& gui;
  Frame& table;

  void handle_keyboard();
  void handle_mouse();
};

//......................................................................
int main() {
  const uint32_t cell_size = 40, fps_max = 30;

  GUI gui(cell_size, fps_max);

  Frame table(gui.window.getSize() / cell_size);

  Events events(gui, table);

  sf::Time time_calc;
  sf::Clock clock_calc;
  size_t frame_counter = 0;

  while (gui.window.isOpen() && ++frame_counter <= 1000) {
    gui.display(table);
    events.handle();

    clock_calc.restart();
    if (!gui.is_paused) table.Update();
    time_calc += clock_calc.getElapsedTime();
  }
  std::cout << "resol  " << gui.window.getSize().x
            << 'x' << gui.window.getSize().y << '\n';
  std::cout << "total  " << gui.clock.getElapsedTime().asMilliseconds() << '\n';
  std::cout << "calc   " << time_calc.asMilliseconds() << '\n';
  return 0;
}
/*
 * Hotkeys:
 *  Escape   (close)
 *  C        (Clear)
 *  N        (new table with random cells)
 *  P        (pause and show mouse coursor)
 *  F        (unlock fps)
 */

Frame::Frame(
    const sf::Vector2u& size) : width(size.x),
                                height(size.y) {
  image.create(size.x, size.y);
  image_ptr = reinterpret_cast<uint32_t*>(
      const_cast<sf::Uint8*>(image.getPixelsPtr()));
  Randomize();
}

uint32_t Frame::GetColor(uint32_t i, uint32_t j) const {
  //return image.getPixel(i, j);
  return image_ptr[i + j * width];
}

void Frame::SetColor(uint32_t i, uint32_t j, uint32_t color) {
  //image.setPixel(i, j, sf::Color(color));
  image_ptr[i + j * width] = color;
}

void Frame::Clear() {
  std::fill_n(image_ptr, width * height, 0xff000000);
}

void Frame::Randomize() {
  
  for (uint32_t j = 0; j < 0x100; j += 0x10) {
    SetColor(j >> 4, 0, 0xff000000 | (j << 16) | (j << 8) | j);
  }
  // static std::mt19937_64 rnd;
  // uint64_t num = 0;
  // for (uint32_t j = 0; j != height; ++j) {
  //   for (uint32_t i = 0; i != width; ++i, num >>= 1) {
  //     if ((i & 0x3f) == 0) num = rnd();
  //     SetColor(i, j, num & 1 ? 0xffffffff : 0xff000000);
  //   }
  // }
}

void Frame::Update() {
  for (uint32_t i = width - 1; i != 0; --i) {
    SetColor(i, 0, GetColor(i - 1, 0));
  }
  SetColor(0, 0, GetColor(width - 1, 0));
  // for (uint32_t j = 0; j != height; ++j) {
  //   for (uint32_t i = 0; i != width; ++i) {
  //     SetColor(i, j, FlipColor(GetColor(i, j)));
  //   }
  // }
}

uint32_t Frame::FlipColor(uint32_t color) {
  return ~color | 0xff000000;
}

GUI::GUI(uint32_t cell_size, uint32_t fps_max)
    : window(sf::VideoMode(sf::VideoMode(1200, 800)),  //::getDesktopMode()),
             "Eyep", sf::Style::Default),
      cell_size(cell_size),
      fps_max(fps_max),
      is_paused(false) {
  window.setFramerateLimit(fps_max);
  window.setMouseCursorVisible(false);
  sprite.setScale(sf::Vector2f(static_cast<float>(cell_size),
                               static_cast<float>(cell_size)));
}

void GUI::display(const Frame& table) {
  window.clear();
  texture.loadFromImage(table.get_image());
  sprite.setTexture(texture, false);
  window.draw(sprite);
  window.display();
}

void Events::handle() {
  while (gui.window.pollEvent(event)) {
    switch (event.type) {
      case sf::Event::Closed:
        gui.window.close();
        break;
      case sf::Event::KeyPressed:
        handle_keyboard();
        break;
      case sf::Event::MouseButtonPressed: {
        handle_mouse();
        break;
      }
      default:
        break;
    }
  }
}

void Events::handle_keyboard() {
  switch (event.key.code) {
    case sf::Keyboard::Escape:
      gui.window.close();
      break;
    case sf::Keyboard::N:
      table.Randomize();
      break;
    case sf::Keyboard::C:
      table.Clear();
      break;
    case sf::Keyboard::F:
      static bool unlocked = false;
      unlocked = !unlocked;
      gui.window.setFramerateLimit(unlocked ? 0 : gui.fps_max);
      break;
    case sf::Keyboard::P:
      gui.is_paused = !gui.is_paused;
      gui.window.setMouseCursorVisible(gui.is_paused);
      break;
    default:
      break;
  }
}

void Events::handle_mouse() {
  sf::Vector2i p = sf::Mouse::getPosition() - gui.window.getPosition();
  uint32_t x = static_cast<uint32_t>(p.x) / gui.cell_size;
  uint32_t y = static_cast<uint32_t>(p.y) / gui.cell_size;
  table.SetColor(x, y, Frame::FlipColor(table.GetColor(x, y)));
}

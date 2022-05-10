#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <vector>

class CellTable {
 public:
  CellTable(const sf::Vector2u&);
  const sf::Image& get_image() const { return image; }
  bool get_state(int, int) const;
  void set_state(int, int, bool);
  void clear();
  void randomize();
  void update();

  const int width;
  const int height;

 private:
  sf::Image image;
  uint32_t* image_ptr;
  std::vector<uint8_t> neighbors;

  void fix_neighbors(int, int);
};

struct GUI {
  sf::RenderWindow window;
  sf::Texture texture;
  sf::Sprite sprite;
  const sf::Clock clock;
  const float cell_size;
  const unsigned int fps_max;
  bool is_paused;
  // window(sf::VideoMode(x, y), "Conway's Game of Life")

  GUI(float cell_size, unsigned int fps_max);
  void display(const CellTable&);
};

class Events {
 public:
  Events(GUI& gui, CellTable& table) : gui(gui), table(table) {}
  void handle();

 private:
  sf::Event event;
  GUI& gui;
  CellTable& table;

  void handle_keyboard();
  void handle_mouse();
};

int main() {
  const unsigned int cell_size = 1, fps_max = 0;
  GUI gui(cell_size, fps_max);
  CellTable table(gui.window.getSize() / cell_size);
  Events events(gui, table);

  sf::Time calc_time;
  sf::Clock cl;

  int frame_counter = 0;
  while (gui.window.isOpen() && ++frame_counter <= 40) {
    gui.display(table);
    events.handle();

    cl.restart();

    if (!gui.is_paused) table.update();

    calc_time += cl.getElapsedTime();
  }
  std::cout << gui.clock.getElapsedTime().asMilliseconds() << '\n';
  std::cout << calc_time.asMilliseconds() << '\n';
  return 0;
}
/*
 * Hotkeys:
 *  Escape   (close)
 *  C        (clear)
 *  N        (new table with random cells)
 *  P        (pause and show mouse coursor)
 *  F        (unlock fps)
 */

CellTable::CellTable(
    const sf::Vector2u& size) : width(size.x),
                                height(size.y),
                                neighbors(size.x * size.y) {
  image.create(size.x, size.y);
  image_ptr = reinterpret_cast<uint32_t*>(
      const_cast<sf::Uint8*>(image.getPixelsPtr()));
  randomize();
}

bool CellTable::get_state(int i, int j) const {
  //return image.getPixel(i, j) == sf::Color::White;
  return image_ptr[i + j * width] == 0xFFFFFFFF;
}

void CellTable::set_state(int i, int j, bool state) {
  //image.setPixel(i, j, (state ? sf::Color::White : sf::Color::Black));
  image_ptr[i + j * width] = state ? 0xFFFFFFFF : 0xFF000000;
}

void CellTable::clear() {
  image.create(image.getSize().x, image.getSize().y);
}

void CellTable::randomize() {
  static std::mt19937_64 rnd;
  uint64_t num = 0;
  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i, num >>= 1) {
      if ((i & 0x3F) == 0) num = rnd();
      set_state(i, j, num & 1);
    }
  }
}

void CellTable::update() {
  std::fill(neighbors.begin(), neighbors.end(), uint8_t());
  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
      if (get_state(i, j)) fix_neighbors(i, j);
    }
  }
  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
      int index = i + j * width;
      if (get_state(i, j)) {
        if (neighbors[index] < 2 || neighbors[index] > 3) {
          set_state(i, j, false);
        }
      } else if (neighbors[index] == 3) {
        set_state(i, j, true);
      }
    }
  }
}

void CellTable::fix_neighbors(int i, int j) {
  const int offsets[8][2] = {
      {-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
  for (auto offset : offsets) {
    int io = i + offset[0];
    int jo = j + offset[1];
    if (io < 0) io += width;
    if (io >= width) io -= width;
    if (jo < 0) jo += height;
    if (jo >= height) jo -= height;
    ++neighbors[io + jo * width];
  }
}

GUI::GUI(float cell_size, unsigned int fps_max)
    : window(sf::VideoMode(sf::VideoMode::getDesktopMode()),
             "Conway's Game of Life", sf::Style::Fullscreen),
      sprite(),
      cell_size(cell_size),
      fps_max(fps_max),
      is_paused(false) {
  window.setFramerateLimit(fps_max);
  window.setMouseCursorVisible(false);
  sprite.setScale({cell_size, cell_size});
}

void GUI::display(const CellTable& table) {
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
      table.randomize();
      break;
    case sf::Keyboard::C:
      table.clear();
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
  int x = p.x / static_cast<int>(gui.cell_size);
  int y = p.y / static_cast<int>(gui.cell_size);
  table.set_state(x, y, !table.get_state(x, y));
}

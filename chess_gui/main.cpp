#include "gui.hpp"

int main() {
    uint32_t fps_max = 0;

    Window window(720, fps_max);
    Processing processing(window);
    Events events(window, processing);

    // uint32_t frame_counter = 0;

    while (window.isOpen()) {
        events.handle();
        processing.update();
    }
    return 0;
}

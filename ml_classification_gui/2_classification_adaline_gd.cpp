#include "adaline_gd.hpp"
#include "classification_task.hpp"

int main() {
    float points_radius = 10, learning_rate = 0.01f;
    uint32_t fps_max = 30, eras_per_frame = 15;

    Window window(fps_max);
    Processing<AdalineGD<2>> processing(window, learning_rate, points_radius);
    Events events(window, processing);

    while (window.isOpen()) {
        events.handle();
        processing.update(eras_per_frame);
    }
    return 0;
}

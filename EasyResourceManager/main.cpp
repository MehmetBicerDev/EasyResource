#include "ui.h"

int main()
{
    ui::init();

    while (1)
    {
        ui::update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }
}


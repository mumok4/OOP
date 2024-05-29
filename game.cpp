#include "gameplay.cpp"
#include <Windows.h>
#include <locale>

int main() {
    // Устанавливаем кодировку консоли для корректного отображения русских символов
    SetConsoleCP(1251); 
    SetConsoleOutputCP(1251);

    // Устанавливаем локаль для потоков ввода и вывода
    std::wcout.imbue(std::locale(""));
    std::wcin.imbue(std::locale(""));

    // Создаем объект игры и запускаем игровой цикл
    Game game;
    game.play();

    return 0;
}

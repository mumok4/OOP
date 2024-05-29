Пояснительная записка к коду
Описание программы
Программа представляет собой текстовую игру для двух игроков, в которой игроки поочередно добавляют буквы на игровое поле, пытаясь составить слова из словаря. Игра написана на языке C++ и использует кириллицу для ввода и вывода. Программа позволяет пользователям выбирать размер игрового поля, задавать начальное слово и следить за ходом игры, включая ведение журнала состояний игры.

Основные компоненты программы
Программа состоит из следующих основных компонентов:

Файл main.cpp: Содержит функцию main(), которая инициализирует кодировку консоли для корректного отображения русских символов, создает объект игры и запускает игровой цикл.
Файл gameplay.cpp: Содержит реализацию класса Game, который включает в себя логику игры.
Файл gameplay.h: Заголовочный файл, объявляющий класс Game и его методы.
main.cpp
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

Функция main() выполняет следующие действия:

Устанавливает кодировку консоли для корректного отображения и ввода русских символов с помощью функций SetConsoleCP(1251) и SetConsoleOutputCP(1251).
Устанавливает локаль для потоков ввода и вывода для корректной работы с русскими символами.
Создает объект класса Game и запускает игровой цикл, вызывая метод play().

gameplay.cpp

#include "gameplay.h"
#include <locale>
#include <codecvt>
#include <Windows.h>

// Конструктор класса Game
Game::Game() : currentPlayer(0), passCount(0), isFirstMove(true) {
    logFile.open("game_log.txt");
    logFile.imbue(std::locale(logFile.getloc(), new std::codecvt_utf8<wchar_t>));
    loadDictionary("dictionary.txt");

    std::wcout << L"Введите размер игрового поля (например, 5 для 5x5): ";
    std::wcin >> boardSize;
    board.resize(boardSize, std::vector<wchar_t>(boardSize, L' '));
    scores.resize(2, 0);

    std::wstring initialWord;
    std::wcout << L"Введите начальное слово: ";
    std::wcin >> initialWord;
    toUpper(initialWord);

    // Размещение начального слова на доске
    int start = (boardSize - initialWord.size()) / 2;
    for (int i = 0; i < initialWord.size(); ++i) {
        board[boardSize / 2][start + i] = initialWord[i];
    }
    usedWords.insert(initialWord);
    logGameState();
}

// Загрузка словаря из файла
void Game::loadDictionary(const std::string& filename) {
    std::wifstream file(filename);
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));
    std::wstring word;
    while (std::getline(file, word)) {
        dictionary.insert(word);
    }
}

// Печать текущего состояния доски
void Game::printBoard() {
    for (const auto& row : board) {
        for (wchar_t cell : row) {
            std::wcout << (cell == L' ' ? L'.' : cell) << L' ';
        }
        std::wcout << std::endl;
    }
}

// Проверка допустимости хода
bool Game::isValidMove(int x, int y, wchar_t letter) {
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize || board[x][y] != ' ') {
        return false;
    }

    if (isFirstMove) {
        return true;
    }

    if (isAdjacent(x, y)) {
        return true;
    }

    board[x][y] = letter;
    std::set<std::wstring> words = extractWordsFromBoard();
    board[x][y] = L' ';

    for (const std::wstring& word : words) {
        if (!isValidWord(word)) {
            return false;
        }
    }
    return true;
}

// Проверка допустимости слова
bool Game::isValidWord(const std::wstring& word) {
    return dictionary.find(word) != dictionary.end();
}

// Проверка допустимости префикса слова
bool Game::isWordPrefixValid(const std::wstring& word) {
    for (const auto& dictWord : dictionary) {
        if (dictWord.compare(0, word.length(), word) == 0) {
            return true;
        }
    }
    return false;
}

// Осуществление хода игроком
void Game::makeMove() {
    int x, y;
    wchar_t letter;

    while (true) {
        std::wcout << L"Игрок " << currentPlayer + 1 << L", введите x, y, букву: ";
        std::wcin >> x >> y >> letter;
        letter = towupper(letter);

        if (isValidMove(x, y, letter)) {
            board[x][y] = letter;
            std::set<std::wstring> words = extractWordsFromBoard();
            for (const std::wstring& word : words) {
                if (isValidWord(word)) {
                    usedWords.insert(word);
                    if (!isFirstMove) {
                        scores[currentPlayer] += word.length();
                    }
                }
            }
            isFirstMove = false;
            currentPlayer = (currentPlayer + 1) % 2;
            passCount = 0;
            logGameState();
            break;
        }
        else {
            std::wcout << L"Недопустимый ход, попробуйте снова." << std::endl;
        }
    }
}

// Пропуск хода
void Game::passTurn() {
    passCount++;
    currentPlayer = (currentPlayer + 1) % 2;
    logGameState();
}

// Запись текущего состояния игры в лог-файл
void Game::logGameState() {
    logFile << L"Текущее состояние доски:\n";
    for (const auto& row : board) {
        for (wchar_t cell : row) {
            logFile << (cell == L' ' ? L'.' : cell) << L' ';
        }
        logFile << std::endl;
    }
    logFile << L"Очки: ";
    for (int i = 0; i < 2; ++i) {
        logFile << L"Игрок " << i + 1 << L": " << scores[i] << L" ";
    }
    logFile << L"\n\n";
}

// Извлечение всех слов с доски
std::set<std::wstring> Game::extractWordsFromBoard() {
    std::set<std::wstring> words;

    // Извлечение горизонтальных слов
    for (int i = 0; i < boardSize; ++i) {
        std::wstring word;
        for (int j = 0; j < boardSize; ++j) {
            if (board[i][j] != L' ') {
                word += board[i][j];
            }
            else if (!word.empty()) {
                if (word.length() > 1) {
                    words.insert(word);
                }
                word.clear();
            }
        }
        if (!word.empty() && word.length() > 1) {
            words.insert(word);
        }
    }

    // Извлечение вертикальных слов
    for (int j = 0; j < boardSize; ++j) {
        std::wstring word;
        for (int i = 0; i < boardSize; ++i) {
            if (board[i][j] != L' ') {
                word += board[i][j];
            }
            else if (!word.empty()) {
                if (word.length() > 1) {
                    words.insert(word);
                }
                word.clear();
            }
        }
        if (!word.empty() && word.length() > 1) {
            words.insert(word);
        }
    }

    return words;
}

// Проверка, прилегает ли клетка к другой букве
bool Game::isAdjacent(int x, int y) {
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize && board[nx][ny] != L' ') {
                return true;
            }
        }
    }
    return false;
}

// Основной игровой цикл
void Game::play() {
    while (passCount < 6) {
        printBoard();
        std::wcout << L"Ход игрока " << currentPlayer + 1 << L"." << std::endl;

        wchar_t choice;
        std::wcout << L"Введите 'm' для хода, 'p' для пропуска: ";
        std::wcin >> choice;

        if (choice == L'm') {
            makeMove();
        }
        else if (choice == L'p') {
            passTurn();
        }
        else {
            std::wcout << L"Недопустимый выбор, попробуйте снова." << std::endl;
        }
    }

    std::wcout << L"Игра окончена. Итоговые очки:" << std::endl;
    for (int i = 0; i < 2; ++i) {
        std::wcout << L"Игрок " << i + 1 << L": " << scores[i] << std::endl;
    }
    logFile.close();
}

// Преобразование строки в верхний регистр
void Game::toUpper(std::wstring& str) {
    for (auto& ch : str) {
        ch = towupper(ch);
    }
}

gameplay.h

#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <fstream>

class Game {
public:
    Game();
    void play();

private:
    void loadDictionary(const std::string& filename);
    void printBoard();
    bool isValidMove(int x, int y, wchar_t letter);
    bool isValidWord(const std::wstring& word);
    bool isWordPrefixValid(const std::wstring& word);
    void makeMove();
    void passTurn();
    void logGameState();
    std::set<std::wstring> extractWordsFromBoard();
    bool isAdjacent(int x, int y);
    void toUpper(std::wstring& str);

    int currentPlayer;
    int passCount;
    bool isFirstMove;
    int boardSize;
    std::vector<std::vector<wchar_t>> board;
    std::vector<int> scores;
    std::wofstream logFile;
    std::set<std::wstring> dictionary;
    std::set<std::wstring> usedWords;
    std::wstring initialWord;
};

#endif

Описание классов и методов
Конструктор Game::Game():

Инициализирует переменные currentPlayer, passCount, и isFirstMove.
Открывает файл для логирования состояния игры.
Загружает словарь из файла.
Запрашивает у пользователя размер игрового поля и начальное слово.
Размещает начальное слово в центре игрового поля и записывает начальное состояние игры в лог.
Метод Game::loadDictionary:

Загружает слова из указанного файла в множество dictionary.
Метод Game::printBoard:

Выводит текущее состояние игрового поля на консоль.
Метод Game::isValidMove:

Проверяет, является ли ход допустимым, проверяя границы поля, пустоту клетки, прилегание к другим буквам и формирование допустимых слов.
Метод Game::isValidWord:

Проверяет, содержится ли слово в словаре.
Метод Game::isWordPrefixValid:

Проверяет, является ли заданная строка префиксом какого-либо слова из словаря.
Метод Game::makeMove:

Обрабатывает ход игрока: считывает координаты и букву, проверяет допустимость хода, обновляет поле, подсчитывает очки, переключает ход на следующего игрока и записывает состояние игры в лог.
Метод Game::passTurn:

Обрабатывает пропуск хода: увеличивает счетчик пропусков, переключает ход на следующего игрока и записывает состояние игры в лог.
Метод Game::logGameState:

Записывает текущее состояние игрового поля и очки игроков в лог-файл.
Метод Game::extractWordsFromBoard:

Извлекает все горизонтальные и вертикальные слова с игрового поля.
Метод Game::isAdjacent:

Проверяет, прилегает ли клетка к другой букве.
Метод Game::play:

Основной игровой цикл, который продолжается до шести последовательных пропусков ходов. Выводит текущее состояние поля, запрашивает действие у текущего игрока (ход или пропуск), и завершает игру, выводя итоговые очки.
Метод Game::toUpper:

Преобразует строку в верхний регистр.

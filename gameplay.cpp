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
            // Сохраняем текущее состояние доски
            auto previousBoard = board;

            board[x][y] = letter;
            std::set<std::wstring> words = extractWordsFromBoard();

            int pointsEarned = 0;
            for (const std::wstring& word : words) {
                if (isValidWord(word) && usedWords.find(word) == usedWords.end()) {
                    usedWords.insert(word);
                    if (!isFirstMove) {
                        pointsEarned += word.length();
                    }
                }
            }

            if (pointsEarned > 0 || isFirstMove) {
                scores[currentPlayer] += pointsEarned;
                isFirstMove = false;
                currentPlayer = (currentPlayer + 1) % 2;
                passCount = 0;
                logGameState();
                break;
            }
            else {
                board = previousBoard; // Возвращаем доску в предыдущее состояние
                std::wcout << L"Новые слова не образованы, попробуйте снова." << std::endl;
            }
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

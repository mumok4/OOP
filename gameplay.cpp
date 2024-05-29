#include "gameplay.h"
#include <locale>
#include <codecvt>
#include <Windows.h>

// ����������� ������ Game
Game::Game() : currentPlayer(0), passCount(0), isFirstMove(true) {
    logFile.open("game_log.txt");
    logFile.imbue(std::locale(logFile.getloc(), new std::codecvt_utf8<wchar_t>));
    loadDictionary("dictionary.txt");

    std::wcout << L"������� ������ �������� ���� (��������, 5 ��� 5x5): ";
    std::wcin >> boardSize;
    board.resize(boardSize, std::vector<wchar_t>(boardSize, L' '));
    scores.resize(2, 0);

    std::wstring initialWord;
    std::wcout << L"������� ��������� �����: ";
    std::wcin >> initialWord;
    toUpper(initialWord);

    // ���������� ���������� ����� �� �����
    int start = (boardSize - initialWord.size()) / 2;
    for (int i = 0; i < initialWord.size(); ++i) {
        board[boardSize / 2][start + i] = initialWord[i];
    }
    usedWords.insert(initialWord);
    logGameState();
}

// �������� ������� �� �����
void Game::loadDictionary(const std::string& filename) {
    std::wifstream file(filename);
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));
    std::wstring word;
    while (std::getline(file, word)) {
        dictionary.insert(word);
    }
}

// ������ �������� ��������� �����
void Game::printBoard() {
    for (const auto& row : board) {
        for (wchar_t cell : row) {
            std::wcout << (cell == L' ' ? L'.' : cell) << L' ';
        }
        std::wcout << std::endl;
    }
}

// �������� ������������ ����
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

// �������� ������������ �����
bool Game::isValidWord(const std::wstring& word) {
    return dictionary.find(word) != dictionary.end();
}

// �������� ������������ �������� �����
bool Game::isWordPrefixValid(const std::wstring& word) {
    for (const auto& dictWord : dictionary) {
        if (dictWord.compare(0, word.length(), word) == 0) {
            return true;
        }
    }
    return false;
}

// ������������� ���� �������
void Game::makeMove() {
    int x, y;
    wchar_t letter;

    while (true) {
        std::wcout << L"����� " << currentPlayer + 1 << L", ������� x, y, �����: ";
        std::wcin >> x >> y >> letter;
        letter = towupper(letter);

        if (isValidMove(x, y, letter)) {
            // ��������� ������� ��������� �����
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
                board = previousBoard; // ���������� ����� � ���������� ���������
                std::wcout << L"����� ����� �� ����������, ���������� �����." << std::endl;
            }
        }
        else {
            std::wcout << L"������������ ���, ���������� �����." << std::endl;
        }
    }
}


// ������� ����
void Game::passTurn() {
    passCount++;
    currentPlayer = (currentPlayer + 1) % 2;
    logGameState();
}

// ������ �������� ��������� ���� � ���-����
void Game::logGameState() {
    logFile << L"������� ��������� �����:\n";
    for (const auto& row : board) {
        for (wchar_t cell : row) {
            logFile << (cell == L' ' ? L'.' : cell) << L' ';
        }
        logFile << std::endl;
    }
    logFile << L"����: ";
    for (int i = 0; i < 2; ++i) {
        logFile << L"����� " << i + 1 << L": " << scores[i] << L" ";
    }
    logFile << L"\n\n";
}

// ���������� ���� ���� � �����
std::set<std::wstring> Game::extractWordsFromBoard() {
    std::set<std::wstring> words;

    // ���������� �������������� ����
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

    // ���������� ������������ ����
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

// ��������, ��������� �� ������ � ������ �����
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

// �������� ������� ����
void Game::play() {
    while (passCount < 6) {
        printBoard();
        std::wcout << L"��� ������ " << currentPlayer + 1 << L"." << std::endl;

        wchar_t choice;
        std::wcout << L"������� 'm' ��� ����, 'p' ��� ��������: ";
        std::wcin >> choice;

        if (choice == L'm') {
            makeMove();
        }
        else if (choice == L'p') {
            passTurn();
        }
        else {
            std::wcout << L"������������ �����, ���������� �����." << std::endl;
        }
    }

    std::wcout << L"���� ��������. �������� ����:" << std::endl;
    for (int i = 0; i < 2; ++i) {
        std::wcout << L"����� " << i + 1 << L": " << scores[i] << std::endl;
    }
    logFile.close();
}

// �������������� ������ � ������� �������
void Game::toUpper(std::wstring& str) {
    for (auto& ch : str) {
        ch = towupper(ch);
    }
}

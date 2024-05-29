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
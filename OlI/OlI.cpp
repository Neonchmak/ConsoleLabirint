#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <stack>
#include <queue>
#include <ctime>
#include <conio.h>

using namespace std;

class Vector2 {
public:
    int x;
    int y;

    Vector2(int x = 0, int y = 0) : x(x), y(y) {}

    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator*(int scalar) const {
        return Vector2(x * scalar, y * scalar);
    }
};

// Размеры лабиринта
const Vector2 mazeSize(101, 51);
const int INIT_HEALTH = 50;
const int COINS_FOR_BONUS = 2;
const int HEALTH_BONUS = 5;
const int ENEMY_DAMAGE = 15;
const int TRAP_PENALTY = 3;

// Игровые объекты
const char WALL = '#';
const char EMPTY = ' ';
const char PLAYER = '@';
const char COIN = 'C';
const char ENEMY = 'E';
const char EXIT = 'X';
const char TRAP = 'T';

// Движение
const Vector2 directions[] = { Vector2(0, -1), Vector2(-1, 0), Vector2(0, 1), Vector2(1, 0) };

// Цвета для консоли
const int COLOR_WALL = 15;
const int COLOR_PLAYER = 11;
const int COLOR_COIN = 14;
const int COLOR_ENEMY = 12;
const int COLOR_EXIT = 10;
const int COLOR_TRAP = 13;

// Состояние игры
Vector2 playerPos;
int health = INIT_HEALTH;
int coinsCollected = 0;
int coinsSinceLastBonus = 0;
bool gameOver = false;

// Работа с цветами
void SetColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Завершение игры
void GameOver(char ending) {
    gameOver = true;
    system("cls");

    if (ending == 'E') {
        cout << "Победа! Вы прошли лабиринт!\n";
    }
    else if (ending == 'D') {
        cout << "Поражение(\n";
        cout << "Вы погибли!\n";
    }
    else if (ending == 'Q') {
        cout << "Игра завершена\n";
    }

    if (coinsCollected >= 0) {
        cout << "Собрано монет: " << coinsCollected << endl;
    }
    else {
        cout << "Монеты в долг: " << -coinsCollected << endl;
    }
    cout << "Осталось здоровья: " << health << endl;
}

// Создание пустых квадратов в лаберинте
void ClearSquare(vector<vector<char>>& maze, int startIndex) {
    for (int i = startIndex + 10; i > startIndex; i--)
        for (int j = startIndex + 20; j > startIndex; j--)
            maze[i][j] = EMPTY;
}

// DFS генерация лабиринта
void GenerateMazeDFS(vector<vector<char>>& maze) {
    maze.assign(mazeSize.y, vector<char>(mazeSize.x, WALL));

    stack<Vector2> st;
    st.push(Vector2(1, 1));
    maze[1][1] = EMPTY;

    while (!st.empty()) {
        Vector2 current = st.top();
        st.pop();

        int dirs[] = { 0, 1, 2, 3 };
        for (int i = 0; i < 4; i++) swap(dirs[i], dirs[rand() % 4]);

        for (int dir : dirs) {
            Vector2 next = current + directions[dir] * 2;

            if (next.x > 0 && next.x < mazeSize.x - 1 && next.y > 0 && next.y < mazeSize.y - 1 && maze[next.y][next.x] == WALL) {
                maze[next.y][next.x] = EMPTY;
                maze[current.y + directions[dir].y][current.x + directions[dir].x] = EMPTY;
                st.push(next);
            }
        }
    }
    ClearSquare(maze, 5);
}

// BFS проверка проходимости лабиринта 
bool IsMazePassableBFS(const vector<vector<char>>& maze) {
    vector<vector<bool>> visited(mazeSize.y, vector<bool>(mazeSize.x, false));
    queue<Vector2> q;

    q.push(Vector2(1, 1));
    visited[1][1] = true;

    while (!q.empty()) {
        Vector2 current = q.front();
        q.pop();

        if (maze[current.y][current.x] == EXIT) return true;

        for (int dir = 0; dir < 4; dir++) {
            Vector2 next = current + directions[dir];

            if (next.x < 0 || next.x >= mazeSize.x || next.y < 0 || next.y >= mazeSize.y) continue;
            if (maze[next.y][next.x] == WALL || maze[next.y][next.x] == ENEMY || maze[next.y][next.x] == TRAP) continue;
            if (visited[next.y][next.x]) continue;

            visited[next.y][next.x] = true;
            q.push(next);
        }
    }
    return false;
}

// Инициализация лабиринта 
void SetupMaze(vector<vector<char>>& maze) {
    do {
        GenerateMazeDFS(maze);
        maze[mazeSize.y - 2][mazeSize.x - 2] = EXIT;

        for (int i = 0; i < 10;) {
            Vector2 pos(rand() % (mazeSize.x - 2) + 1, rand() % (mazeSize.y - 2) + 1);
            if (maze[pos.y][pos.x] == EMPTY) {
                maze[pos.y][pos.x] = COIN;
                i++;
            }
        }

        for (int i = 0; i < 5;) {
            Vector2 pos(rand() % (mazeSize.x - 2) + 1, rand() % (mazeSize.y - 2) + 1);
            if (maze[pos.y][pos.x] == EMPTY) {
                maze[pos.y][pos.x] = ENEMY;
                i++;
            }
        }

        for (int i = 0; i < 3;) {
            Vector2 pos(rand() % (mazeSize.x - 2) + 1, rand() % (mazeSize.y - 2) + 1);
            if (maze[pos.y][pos.x] == EMPTY) {
                maze[pos.y][pos.x] = TRAP;
                i++;
            }
        }

    } while (!IsMazePassableBFS(maze));
}

// Отрисовка лабиринта
void DrawMaze(const vector<vector<char>>& maze) {
    system("cls");

    // Ограничиваем отрисовку видимой областью вокруг игрока
    int viewRadius = 30;
    int startX = max(0, playerPos.x - viewRadius);
    int endX = min(mazeSize.x, playerPos.x + viewRadius + 1);
    int startY = max(0, playerPos.y - viewRadius);
    int endY = min(mazeSize.y, playerPos.y + viewRadius + 1);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            char c = maze[y][x];
            if (c == WALL) SetColor(COLOR_WALL);
            else if (c == PLAYER) SetColor(COLOR_PLAYER);
            else if (c == COIN) SetColor(COLOR_COIN);
            else if (c == ENEMY) SetColor(COLOR_ENEMY);
            else if (c == EXIT) SetColor(COLOR_EXIT);
            else if (c == TRAP) SetColor(COLOR_TRAP);
            cout << c;
        }
        cout << endl;
    }

    SetColor(COLOR_WALL);

    cout << "\nЗдоровье: " << health << endl;
    cout << "Монеты: " << coinsCollected << " (след. бонус через "
        << (COINS_FOR_BONUS - coinsSinceLastBonus) << ")" << endl;
    cout << "Управление: WASD, выход: Q" << endl;
    cout << "Позиция: (" << playerPos.x << ", " << playerPos.y << ")" << endl;
}

// Обработка контакта с монетой
void HitCoin() {
    coinsCollected++;
    coinsSinceLastBonus++;

    if (coinsSinceLastBonus >= COINS_FOR_BONUS) {
        health += HEALTH_BONUS;
        coinsSinceLastBonus = 0;
    }
}

// Обработка контакта с ловушой
void HitTrap(vector<vector<char>>& maze) {
    coinsCollected -= TRAP_PENALTY;
    DrawMaze(maze);
    cout << "\nВы наступили на ловушку! Потеряно 3 монеты!\n";
    Sleep(1500);
}

// Обработка контакта с врагом
void HitEnemy(vector<vector<char>>& maze) {
    health -= ENEMY_DAMAGE;
    if (health <= 0) {
        GameOver('D'); return;
    }
    DrawMaze(maze);
    cout << "\nВы встретились с врагом! Потеряно 15 очков здоровья!\n";
    Sleep(1500);
}

// Движение игрока
void MovePlayer(vector<vector<char>>& maze, Vector2 newPos) {
    if (newPos.x < 0 || newPos.x >= mazeSize.x || newPos.y < 0 || newPos.y >= mazeSize.y) return;

    char target = maze[newPos.y][newPos.x];
    if (target == WALL) return;

    maze[playerPos.y][playerPos.x] = EMPTY;
    playerPos = newPos;
    maze[playerPos.y][playerPos.x] = PLAYER;

    if (target == COIN) HitCoin();
    else if (target == ENEMY) HitEnemy(maze);
    else if (target == TRAP) HitTrap(maze);
    else if (target == EXIT) GameOver('E');
}

// Функция для настройки консоли
void SetupConsole() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Устанавливаем размер буфера консоли
    COORD bufferSize = { mazeSize.x + 1, mazeSize.y + 10 };
    SetConsoleScreenBufferSize(hOut, bufferSize);

    // Устанавливаем размер окна консоли
    SMALL_RECT rect = { 0, 0, mazeSize.x, mazeSize.y + 5 };
    SetConsoleWindowInfo(hOut, TRUE, &rect);

    // Скрываем курсор
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

int main() {
    setlocale(0, "");
    srand(static_cast<unsigned int>(time(nullptr)));

    // Настраиваем консоль перед началом игры
    SetupConsole();

    vector<vector<char>> maze;
    SetupMaze(maze);
    playerPos = Vector2(1, 1);
    maze[playerPos.y][playerPos.x] = PLAYER;

    while (!gameOver) {
        DrawMaze(maze);
        int input = _getch();

        switch (tolower(input)) {
        case 'w': MovePlayer(maze, playerPos + Vector2(0, -1)); break;
        case 'a': MovePlayer(maze, playerPos + Vector2(-1, 0)); break;
        case 's': MovePlayer(maze, playerPos + Vector2(0, 1)); break;
        case 'd': MovePlayer(maze, playerPos + Vector2(1, 0)); break;
        case 'q': GameOver('Q'); break;
        }
    }
    return 0;
}
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

const Vector2 MAZE_SIZE(131,41);
const int INIT_HEALTH = 50;
const int COINS_FOR_BONUS = 2;
const int HEALTH_BONUS = 5;
const int ENEMY_DAMAGE = 15;
const int TRAP_PENALTY = 3;

const char WALL = '#';
const char EMPTY = ' ';
const char PLAYER = '@';
const char COIN = 'C';
const char ENEMY = 'E';
const char BOSS = 'B';
const char EXIT = 'X';
const char TRAP = 'T';

const int dx[] = { 0, -1, 0, 1 };
const int dy[] = { -1, 0, 1, 0 };

const int COLOR_WALL = 15;
const int COLOR_PLAYER = 11;
const int COLOR_COIN = 14;
const int COLOR_ENEMY = 12;
const int COLOR_BOSS = 6;
const int COLOR_EXIT = 10;
const int COLOR_TRAP = 13;

int playerX, playerY;
int health = INIT_HEALTH;
int coinsCollected = 0;
int coinsSinceLastBonus = 0;
bool gameOver = false;

//----------------------------------Prototypes----------------------------------

void SetColor(int color);

void GameOver(char ending);

//----------------------------------Maze generation----------------------------------

// Creating rooms
void Createroom(vector<vector<char>>& maze, Vector2 cords) {
    const int roomWidth = 20; 
    const int roomHeight = 10; 

    // Проверка
    if (cords.x + roomWidth >= MAZE_SIZE.x || cords.y + roomHeight >= MAZE_SIZE.y)
        return;

    // Стены
    for (int y = cords.y; y <= cords.y + roomHeight; y++) {
        for (int x = cords.x; x <= cords.x + roomWidth; x++) {
            if (y == cords.y || y == cords.y + roomHeight || x == cords.x || x == cords.x + roomWidth) 
                maze[y][x] = WALL;
            else 
                maze[y][x] = EMPTY;
            
        }
    }

    //Спавн босса
    maze[cords.y + roomHeight / 2][cords.x + roomWidth / 2] = BOSS;

    // Выходы
    int centerX = cords.x + roomWidth / 2;
    int centerY = cords.y + roomHeight / 2;

    maze[cords.y][centerX] = EMPTY;
    maze[cords.y + roomHeight][centerX] = EMPTY;
    maze[centerY][cords.x] = EMPTY;
    maze[centerY][cords.x + roomWidth] = EMPTY;
}

// Maze generation
void GenerateMaze(vector<vector<char>>& maze) {
    maze.assign(MAZE_SIZE.y, vector<char>(MAZE_SIZE.x, WALL));

    const int roomWidth = 15;
    const int roomHeight = 7;
    const int hMargin = 10;
    const int vMargin = 5;

    stack<pair<int, int>> st;
    st.push({ 1,1 });
    maze[1][1] = EMPTY;

    //Комнаты 1
    Createroom(maze, Vector2(10, 5));
    Createroom(maze, Vector2(55, 5));
    Createroom(maze, Vector2(95, 5));
    //Комнаты 2
    Createroom(maze, Vector2(10, 20));
    Createroom(maze, Vector2(55, 20));
    Createroom(maze, Vector2(95, 20));

    while (!st.empty()) {
        int x = st.top().first;
        int y = st.top().second;
        st.pop();

        int dirs[] = { 0,1,2,3 };
        for (int i = 0; i < 4; i++) swap(dirs[i], dirs[rand() % 4]);

        for (int dir : dirs) {
            int nx = x + dx[dir] * 2;
            int ny = y + dy[dir] * 2;

            if (ny > 0 && ny < MAZE_SIZE.y - 1 && nx > 0 && nx < MAZE_SIZE.x - 1 && maze[ny][nx] == WALL) {
                maze[ny][nx] = EMPTY;
                maze[y + dy[dir]][x + dx[dir]] = EMPTY;
                st.push({ nx,ny });
            }

        }

    }
}

// Patency check
bool IsMazePassable(const vector<vector<char>>& maze) {
    vector<vector<bool>> visited(MAZE_SIZE.y, vector<bool>(MAZE_SIZE.x, false));
    queue<pair<int, int>> q;

    q.push({ 1, 1 });
    visited[1][1] = true;

    while (!q.empty()) {
        int x = q.front().first;
        int y = q.front().second;
        q.pop();

        if (maze[y][x] == EXIT) return true;

        for (int dir = 0; dir < 4; dir++) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            if (ny <= 0 || ny >= MAZE_SIZE.y - 1 || nx <= 0 || nx >= MAZE_SIZE.x - 1) continue;
            if (maze[ny][nx] == WALL || maze[ny][nx] == ENEMY || maze[ny][nx] == TRAP) continue;
            if (visited[ny][nx]) continue;

            visited[ny][nx] = true;
            q.push({ nx, ny });
        }
    }
    return false;
}

// Creating maze
void SetupMaze(vector<vector<char>>& maze) {
    do {
        GenerateMaze(maze);

        maze[MAZE_SIZE.y - 2][MAZE_SIZE.x - 2] = EXIT;

        for (int i = 0; i < 20;) {
            int x = rand() % (MAZE_SIZE.x - 2) + 1;
            int y = rand() % (MAZE_SIZE.y - 2) + 1;
            if (maze[y][x] == EMPTY) {
                maze[y][x] = COIN;
                i++;
            }
        }

        for (int i = 0; i < 15;) {
            int x = rand() % (MAZE_SIZE.x - 2) + 1;
            int y = rand() % (MAZE_SIZE.y - 2) + 1;
            if (maze[y][x] == EMPTY) {
                maze[y][x] = ENEMY;
                i++;
            }
        }

        for (int i = 0; i < 10;) {
            int x = rand() % (MAZE_SIZE.x - 2) + 1;
            int y = rand() % (MAZE_SIZE.y - 2) + 1;
            if (maze[y][x] == EMPTY) {
                maze[y][x] = TRAP;
                i++;
            }
        }

    } while (!IsMazePassable(maze));

}

//----------------------------------Rendering----------------------------------

// Screen update
void SetCur(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
};

// Rendering
void DrawMaze(const vector<vector<char>>& maze) {

    SetCur(0, 0);

    for (int y = 0; y < MAZE_SIZE.y; y++) {
        for (int x = 0; x < MAZE_SIZE.x; x++) {

            char c = maze[y][x];
            if (c == WALL) SetColor(COLOR_WALL);
            else if (c == PLAYER) SetColor(COLOR_PLAYER);
            else if (c == COIN) SetColor(COLOR_COIN);
            else if (c == ENEMY) SetColor(COLOR_ENEMY);
            else if (c == BOSS) SetColor(COLOR_BOSS);
            else if (c == EXIT) SetColor(COLOR_EXIT);
            else if (c == TRAP) SetColor(COLOR_TRAP);
            cout << c;
        }
        cout << '\n';
    }

    SetColor(COLOR_WALL);
    cout << "\nЗдоровье: " << health << endl;
    cout << "Монеты: " << coinsCollected << "(след. бонус через "
        << (COINS_FOR_BONUS - coinsSinceLastBonus) << ")\n";
    cout << "Управление WASD, выход Q\n";
}

// Set color
void SetColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

//----------------------------------Contact Check----------------------------------


void ContactCoin() {
    coinsCollected++;
    coinsSinceLastBonus++;

    if (coinsSinceLastBonus >= COINS_FOR_BONUS) {
        health += HEALTH_BONUS;
        coinsSinceLastBonus = 0;
    }
}

void ContactTrap(vector<vector<char>>& maze) {
    coinsCollected -= TRAP_PENALTY;
    DrawMaze(maze);
    cout << "\nВы наступили на ловушку! Потеряно 3 монеты!\n";
    Sleep(2000);
}

void ContactEnemy(vector<vector<char>>& maze) {
    health -= ENEMY_DAMAGE;
    if (health <= 0) {
        GameOver('D'); return;
    }
    system("cls");
    DrawMaze(maze);
    cout << "\nВы встретились с врагом! Потеряно 15 очков здоровья!\n";
    Sleep(2000);
}

void ContactBoss(vector<vector<char>>& maze) {
    health -= ENEMY_DAMAGE;
    if (health <= 0) {
        GameOver('D'); return;
    }
    system("cls");
    DrawMaze(maze);
    cout << "\nВы встретились с брссом! Потеряно 20 очков здоровья, полученно 5 монет!\n";
    coinsCollected += 5;
    Sleep(2000);
}

//----------------------------------Player movement----------------------------------
void MovePlayer(vector<vector<char>>& maze, int newX, int newY) {
    if (newX <= 0 || newX >= MAZE_SIZE.x - 1 || newY <= 0 || newY >= MAZE_SIZE.y - 1)
        return;

    char target = maze[newY][newX];
    if (target == WALL) return;

    maze[playerY][playerX] = EMPTY;
    playerX = newX;
    playerY = newY;
    maze[playerY][playerX] = PLAYER;

    if (target == COIN) ContactCoin();
    else if (target == ENEMY) ContactEnemy(maze);
    else if (target == TRAP) ContactTrap(maze);
    else if (target == BOSS) ContactBoss(maze);
    else if(target == EXIT) GameOver('E');
}

// Ending the game
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

    cout << "Собрано монет: " << coinsCollected << endl;
    cout << "Осталось здоровья: " << health << endl;
}

//----------------------------------Main----------------------------------
int main() {
    setlocale(0, "");
    srand(static_cast<unsigned int>(time(nullptr)));

    vector<vector<char>> maze;
    SetupMaze(maze);
    playerX = playerY = 1;
    maze[playerY][playerX] = PLAYER;

    while (!gameOver) {
        DrawMaze(maze);
        int input = _getch();

        switch (tolower(input)) {
        case 'w': MovePlayer(maze, playerX, playerY - 1); break;
        case 'a': MovePlayer(maze, playerX - 1, playerY); break;
        case 's': MovePlayer(maze, playerX, playerY + 1); break;
        case 'd': MovePlayer(maze, playerX + 1, playerY); break;
        case 'q': GameOver('Q'); break;
        }
    }
    return 0;
}



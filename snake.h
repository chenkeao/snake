#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

// height of the game window
#define GAME_WIN_HEIGHT 15

// width of the game window
#define GAME_WIN_WIDTH 60

// width of the log window
#define LOG_WIN_HEIGHT 7

// width of the log window
#define LOG_WIN_WIDTH 60

// descriptor, used for select function
#define STDIN 0

#define MAX_GRADE 50

#define GAME_TITLE " Snake Game "
#define GAME_GRADE " Grade: %d "
#define GAME_DIFF " Diff: %d "
#define LOG_TITLE " Log "
#define PLAY_AGAIN "Play Again?"
#define GAME_OVER "Game Over: Your Garde:%d"
#define GAME_WIN "Oh! You win! Your Garde:%d"

#define WELCOME "Snake Game V0.1 "
#define CHOOSEDIFF "choose difficulty(1 - 5):"
#define SHOWDIFF "difficulty:%d"
#define CONTINUE "press any key to continue the game"

#define INFO1 "\'*\': 1 score"
#define INFO2 "\'@\': 3 score"
#define INFO3 "\'%%\': 5 score"
// direction of every snake's node
enum DIRE
{
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
};

static char foodTypeChar[] = {'*', '@', '%'};
static char SnakeShape[] = {'^', 'v', '<', '>'};
static int foodReward[] = {1, 3, 5};

typedef struct
{
    int grade;
    //难度系数 difficulty factor
    int difficulty;
    //是否存在食物
    int exsitFood;
    //游戏窗口
    WINDOW *gameWin;
    WINDOW *logWin;
} Game, *GamePtr;

//组成蛇的节点
typedef struct SnakeNode
{
    int x;
    int y;
    enum DIRE dire;
    struct SnakeNode *next;
} SnakeNode, *SnakeNodePtr;

//蛇
typedef struct
{
    enum DIRE dire;
    int length;
    // head of the snake
    SnakeNodePtr head;
} Snake, *SnakePtr;

typedef struct
{
    int type;
    int x, y;
    int reward;
} Food, *FoodPtr;

/**
 * @brief 初始化游戏， 重置分数、难度、游戏窗口、日志窗口
 *
 * @param game
 * @return int 0: success, other: fail
 */
void gameInit(GamePtr game);

WINDOW *newWindown(int height, int width, int left, int top);

/**
 * @brief 更新游戏状态
 *
 * @return int 0: go on, bigger: win, smaller: fail
 */
int gameRefresh(GamePtr game, SnakePtr snake);

void snakeInit(SnakePtr snake);

/**
 * @brief 移动蛇
 *
 * @param snake
 */
void snakeMove(SnakePtr snake);

/**
 * @brief 检查蛇的状态
 *
 * @param snake
 * @param food
 * @return int 0: go on. bigger: reward, smaller: fail
 */
int snakeCheck(SnakePtr snake, FoodPtr food);

void snakeGrow(SnakePtr snake);

void gameOver(GamePtr game, char *msg);

SnakeNodePtr snakeNodeInit(SnakeNodePtr *node, int x, int y, enum DIRE dire);

void snakeDraw(GamePtr game, SnakePtr snake);

void logAdd(GamePtr game, char *msg);

void lineClean(GamePtr game, int lineIndex);

void foodDraw(GamePtr game, FoodPtr food);

void foodInit(FoodPtr food, SnakePtr snake);

void gameDestroy(GamePtr game);

void snakeDestroy(SnakePtr snake);

void welcome(GamePtr game);

void clearGameWin(GamePtr game);

/**
 * @brief Get the Random number
 *
 * @param min 最小值
 * @param max 最大值
 * @return int
 */
int getRandom(int min, int max);

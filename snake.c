/**
 * @file snake.c
 * @author chenkeao@foxmail.com
 * @brief  Snake game. run on the terminal
 * @version 0.1
 * @date 2022-05-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "snake.h"

int main(int argc, char const *argv[])
{
    /**
     * 1. 声明几个必须的变量
     * 2. 初始化游戏窗口，日志窗口，蛇
     * 3. 循环更新游戏
     *  3.1 如果不存在食物，生成一个食物
     *  3.2 监听标准输入
     *  3.3 根据不同的输入改变蛇的前进状态
     *  3.4 移动蛇
     *  3.5 检查蛇的状态：包括是否碰壁，是否碰到自身，是否吃到食物
     * 4. 退出循环则说明游戏结束
     */

    Game game;
    gameInit(&game);
    Snake snake;
    snakeInit(&snake);

    welcome(&game);

    int status;

    while ((status = gameRefresh(&game, &snake)) == 0)
        ;

    if (status == -1)
    {
        gameOver(&game, GAME_OVER);
    }
    else if (status > 0)
    {
        gameOver(&game, GAME_OVER);
    }

    gameDestroy(&game);
    snakeDestroy(&snake);
    getch();
    endwin();
    return 0;
}

/**
 * @brief 核心函数, 每次执行代表了游戏的一次轮回, 蛇会移动一步
 *  主要流程
 *  1 检查是否存在食物, 没有食物则在游戏窗口中生成一个食物
 *  2 监听标准输入(键盘), 并阻塞一定的时间(根据游戏难度确定), 如果这段时间内没有监听到输入, 则结束阻塞继续执行；如果监听到了用户输入, 则判断是否是方向键, 如果不是方向键就忽略, 如果是方向键就改变蛇的行进方向.
 *  注: 如果按了按键, 相当于打断了阻塞态, 直接继续, 连续按的话相当于加速了蛇的移动.
 *  3 接着程序会判断蛇的方向, 通过将蛇尾移动到蛇头的方式每次前进一格.
 *  4 移动后判断游戏状态, 分如如下几种:
 *      4.1 无事发生, 蛇没有吃到食物也没有触碰身体和边缘, 函数返回0
 *      4.2 吃到食物, 蛇身体增长1, 分数根据食物的类型增加, 如果达到了胜利条件, 返回一个正数, 否则返回0
 *      4.3 检测碰到了自己的身体或者边缘, 函数返回-1
 *
 * @param game
 * @param snake
 * @return int
 */
int gameRefresh(GamePtr game, SnakePtr snake)
{
    static Food food;
    if (!game->exsitFood)
    {
        foodInit(&food, snake);
        foodDraw(game, &food);
    }

    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN, &set);

    struct timeval timeout;
    timeout.tv_sec = 0;
    //难度系数 × (0.1秒)www
    //难度系数： 难度1 为最简单，因为两次移动之间间隔最长，移动速度也就越慢
    timeout.tv_usec = (6 - game->difficulty) * 100 * 1000 * 0.5;

    if (select(1, &set, NULL, NULL, &timeout) < 0)
    {
        return -1;
    }
    if (FD_ISSET(STDIN, &set))
    {
        int key = getch();
        switch (key)
        {
        case 'w':
            (snake->dire == DOWN) ?: (snake->dire = UP);
            break;
        case 'a':
            (snake->dire == RIGHT) ?: (snake->dire = LEFT);
            break;
        case 's':
            (snake->dire == UP) ?: (snake->dire = DOWN);
            break;
        case 'd':
            (snake->dire == LEFT) ?: (snake->dire = RIGHT);
            break;
        default:
            break;
        }
    }
    snakeMove(snake);
    snakeDraw(game, snake);

    int res = snakeCheck(snake, &food);
    switch (res)
    {
    //情况一：正常前进
    case 0:
        break;
        //情况二：发生碰撞
    case -1:
        return -1;
        //情况三：吃到食物, res: 食物的奖励
    default:
        game->exsitFood = false;
        snakeGrow(snake);
        snakeDraw(game, snake);
        game->grade += food.reward;
        if (game->grade >= MAX_GRADE)
        {
            return game->grade;
        }
        break;
    }
    return 0;
}

/**
 * @brief 向前移动一步, 根据方向判断应该向哪里移动, 例如:应该向左移动就将尾节点的坐标更新为: (头节点的x-1, 头节点的y)
 *
 * @param snake
 */
void snakeMove(SnakePtr snake)
{
    int headX = snake->head->x;
    int headY = snake->head->y;
    SnakeNodePtr tail = snake->head->next;
    switch (snake->dire)
    {
    case UP:
        tail->y = headY - 1;
        tail->x = headX;
        tail->dire = UP;
        break;
    case LEFT:
        tail->x = headX - 1;
        tail->y = headY;
        tail->dire = LEFT;
        break;
    case DOWN:
        tail->y = headY + 1;
        tail->x = headX;
        tail->dire = DOWN;
        break;
    case RIGHT:
        tail->x = headX + 1;
        tail->y = headY;
        tail->dire = RIGHT;
        break;
    default:
        break;
    }
    //通过将尾节点移动到头节点，实现一格一格的移动
    snake->head = tail;
}

/**
 * @brief 检查蛇的状态
 *
 * @param snake
 * @param food
 * @return int 0: go on. bigger: reward, smaller: fail
 */
int snakeCheck(SnakePtr snake, FoodPtr food)
{
    SnakeNodePtr head = snake->head;
    if (head->x <= 0 || head->x >= GAME_WIN_WIDTH - 1 || head->y <= 0 || head->y >= GAME_WIN_HEIGHT - 1)
    {
        return -1;
    }
    SnakeNodePtr node = head->next;
    for (int i = 0; i < snake->length - 1; ++i, node = node->next)
    {
        if (head->y == node->y && head->x == node->x)
        {
            return -1;
        }
    }
    if (head->x == food->x && head->y == food->y)
    {
        return food->reward;
    }
    return 0;
}

/**
 * @brief 蛇吃到食物后长度+1, 新增的节点会添加到尾部, 根据尾部的方向不同, 坐标也不同, 例如:如果尾部正在向左移动, 那么新增的节点的坐标应该是(尾部的x+1, 尾部的y), 根据尾部判断坐标的原因在于, 头和尾的前进方向可能并不一样, 例如:
 * >>>>>>>
 *    <<<v
 *
 * @param snake
 */
void snakeGrow(SnakePtr snake)
{
    SnakeNodePtr node = (SnakeNodePtr)malloc(sizeof(SnakeNode));
    SnakeNodePtr tail = snake->head->next;
    int tailX = tail->x;
    int tailY = tail->y;
    node->dire = tail->dire;
    node->next = tail;
    snake->head->next = node;

    switch (tail->dire)
    {
    case UP:
        node->x = tailX;
        node->y = tailY + 1;
        break;
    case LEFT:
        node->x = tailX + 1;
        node->y = tailY;
        break;
    case DOWN:
        node->x = tailX;
        node->y = tailY - 1;
        break;
    case RIGHT:
        node->x = tailX - 1;
        node->y = tailY;
        break;
    default:
        break;
    }

    snake->length++;
}

/**
 * @brief 显示欢迎界面
 * 1. 显示程序名称
 * 2. 选择游戏难度
 * 3. 确认开始
 *
 * @param game
 */
void welcome(GamePtr game)
{
    int diff = -1;
    int lineIndex = 3;
    mvwprintw(game->gameWin, lineIndex++, (GAME_WIN_WIDTH - strlen(WELCOME)) / 2, WELCOME);
    mvwprintw(game->gameWin, lineIndex++, (GAME_WIN_WIDTH - strlen(INFO1)) / 2, INFO1);
    mvwprintw(game->gameWin, lineIndex++, (GAME_WIN_WIDTH - strlen(INFO2)) / 2, INFO2);
    mvwprintw(game->gameWin, lineIndex++, (GAME_WIN_WIDTH - strlen(INFO3)) / 2, INFO3);
    wrefresh(game->gameWin);

    mvwprintw(game->gameWin, lineIndex++, (GAME_WIN_WIDTH - strlen(CHOOSEDIFF)) / 2, CHOOSEDIFF);
    wrefresh(game->gameWin);
    while ((diff = getch()) > 5 + '0' || diff < 1 + '0')
        ;
    game->difficulty = diff - '0';
    mvwprintw(game->gameWin, lineIndex++, (GAME_WIN_WIDTH - strlen(SHOWDIFF)) / 2, SHOWDIFF, game->difficulty);
    mvwprintw(game->gameWin, lineIndex++, (GAME_WIN_WIDTH - strlen(CONTINUE)) / 2, CONTINUE);

    wrefresh(game->gameWin);
    getch();

    clearGameWin(game);
}

/**
 * @brief 初始化游戏
 * 1. 调用ncurses的函数进入ncurses模式
 * 2. 创建游戏窗口, 设置游戏默认参数
 * 3. 创建日志窗口
 * 4. 刷新界面, 显示内容
 *
 * @param game
 */
void gameInit(Game *game)
{
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    refresh();

    game->gameWin = newwin(GAME_WIN_HEIGHT, GAME_WIN_WIDTH, 1, 3);
    game->grade = 0;
    game->difficulty = 5;
    game->exsitFood = 0;
    box(game->gameWin, 0, 0);
    mvwprintw(game->gameWin, 0, (GAME_WIN_WIDTH - strlen(GAME_TITLE)) / 2, GAME_TITLE);
    mvwprintw(game->gameWin, GAME_WIN_HEIGHT - 1, 2, GAME_GRADE, game->grade);
    mvwprintw(game->gameWin, GAME_WIN_HEIGHT - 1, 30, GAME_DIFF, game->difficulty);

    game->logWin = newwin(LOG_WIN_HEIGHT, LOG_WIN_WIDTH, GAME_WIN_HEIGHT + 1, 3);
    box(game->logWin, 0, 0);
    mvwprintw(game->logWin, 0, (LOG_WIN_WIDTH - strlen(LOG_TITLE)) / 2, LOG_TITLE);

    wrefresh(game->gameWin);
    wrefresh(game->logWin);
}

/**
 * @brief 初始化一条蛇, 实际上是一个循环链表
 * 注: 在内存中链表的顺序如果是: 1->2->3->4, 那么在屏幕上显示时会显示: 1->4->3->2
 * 这是为了更方便的找到蛇头和蛇尾, 因为移动的方法是将蛇尾移动到蛇头, 所以经常要用到蛇尾的指针, 这种直接将蛇头和蛇尾连接的方式可以快速的找到蛇尾. 另一个好处是: 移动的时候, 只需要更新蛇尾的坐标, 并让头节点指向蛇尾, 这样蛇尾就成了蛇头, 整个结构也不会发生变化
 * 下面的数字表示在屏幕上显示的顺序, 1表示蛇头
 * snake->1->4->3->2-
 *       ^__________|
 *          |
 *          | snakeMove()
 *          V
 * snake->4->3->2->1-   这时, 根据在屏幕上的显示顺序重新对节点编号, snake->1->4->3->2-
 *       ^__________|                                               ^__________|
 * 可见原本处于3位置的节点变成了4, 原来处于2位置的节点变成了3, 原本的1变成了2.
 * @param snake
 */
void snakeInit(SnakePtr snake)
{
    snake->length = 4;
    snake->dire = LEFT;

    snakeNodeInit(
        &snakeNodeInit(
             &snakeNodeInit(
                  &snakeNodeInit(
                       &snake->head, 50, 5, LEFT)
                       ->next,
                  53, 5, LEFT)
                  ->next,
             52, 5, LEFT)
             ->next,
        51, 5, LEFT)
        ->next = snake->head;
}

/**
 * @brief 初始化一个节点
 *
 * @param node
 * @param x
 * @param y
 * @param dire
 * @return SnakeNodePtr
 */
SnakeNodePtr snakeNodeInit(SnakeNodePtr *node, int x, int y, enum DIRE dire)
{
    (*node) = (SnakeNodePtr)malloc(sizeof(struct SnakeNode));
    (*node)->x = x;
    (*node)->y = y;
    (*node)->dire = dire;
    (*node)->next = NULL;
    return *node;
}

/**
 * @brief 清空游戏窗口并打印游戏结束的通知
 *
 * @param game
 * @param msg
 */
void gameOver(GamePtr game, char *msg)
{
    clearGameWin(game);
    mvwprintw(game->gameWin, (int)(GAME_WIN_HEIGHT / 2), GAME_WIN_WIDTH / 2 - strlen(msg) / 2, msg, game->grade);
    mvwprintw(game->gameWin, (int)(GAME_WIN_HEIGHT / 2 + 1), 20, "Press any key to quit...");
    wrefresh(game->gameWin);
}

/**
 * @brief 将蛇在游戏窗口中显示出来, 遍历链表, 根据逐节点显示. 函数中有两个static变量, 是为了记录上一次尾部的坐标, 因为移动方式是将尾部移动到蛇首部, 自然需要将原来的尾部显示为空白字符, 否则就会显示一条不断增长的蛇
 *
 * @param game
 * @param snake
 */
void snakeDraw(Game *game, SnakePtr snake)
{
    static int tailX = 0;
    static int tailY = 0;
    if (tailX != 0 && tailY != 0)
    {
        mvwaddch(game->gameWin, tailY, tailX, ' ');
    }

    SnakeNodePtr node = snake->head->next;
    for (int i = 0; i < snake->length; i++)
    {
        mvwaddch(game->gameWin, node->y, node->x, SnakeShape[node->dire]);
        node = node->next;
    }
    tailX = snake->head->next->x;
    tailY = snake->head->next->y;
    wrefresh(game->gameWin);
}

/**
 * @brief 在窗口上显示食物
 *
 * @param game
 * @param food
 */
void foodDraw(GamePtr game, FoodPtr food)
{
    game->exsitFood = true;
    mvwaddch(game->gameWin, food->y, food->x, foodTypeChar[food->type]);
    wrefresh(game->gameWin);
}

/**
 * @brief 检查食物的出现位置, 不能出现在蛇的身体上
 *
 * @param food
 * @param snake
 * @return int
 */

int checkFoodPos(FoodPtr food, SnakePtr snake)
{
    SnakeNodePtr p = snake->head;
    for (int i = 0; i < snake->length; i++, p = p->next)
    {
        if (food->x == p->x && food->y == p->y)
        {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief 生成一个食物
 *
 * @param food
 * @param snake
 */
void foodInit(FoodPtr food, SnakePtr snake)
{
    srand(time(NULL));
    do
    {
        food->x = getRandom(4, GAME_WIN_WIDTH - 2);
        food->y = getRandom(2, GAME_WIN_HEIGHT - 2);
    } while (!checkFoodPos(food, snake));

    food->type = getRandom(0, 2);
    food->reward = foodReward[food->type];
}

void logAdd(Game *game, char *msg)
{
}

void lineClean(Game *game, int lineIndex)
{
}

/**
 * @brief 通过填充空字符的方式清空游戏窗口的内容
 *
 * @param game
 */
void clearGameWin(GamePtr game)
{
    for (int i = 1; i < GAME_WIN_WIDTH - 1; i++)
    {
        for (int j = 1; j < GAME_WIN_HEIGHT - 1; j++)
        {
            mvwaddch(game->gameWin, j, i, ' ');
        }
    }
}

/**
 * @brief 销毁两个窗口
 *
 * @param game
 */
void gameDestroy(GamePtr game)
{
    delwin(game->gameWin);
    delwin(game->logWin);
}

/**
 * @brief 释放组成蛇身体的节点
 *
 * @param snake
 */
void snakeDestroy(SnakePtr snake)
{
    SnakeNodePtr p = NULL;
    SnakeNodePtr node = snake->head;

    for (int i = 0; i < snake->length; i++)
    {
        p = node;
        node = node->next;
        free(p);
    }
    snake = NULL;
}

/**
 * @brief Get the Random number
 *
 * @param min 最小值
 * @param max 最大值
 * @return int
 */
int getRandom(int min, int max)
{
    int num;
    num = (rand() % (max - min + 1)) + min;
    return num;
}
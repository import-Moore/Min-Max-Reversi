#include<iostream>
#include<algorithm>
#include<fstream>
#include<strstream>
#include<graphics.h>
#include<windows.h>

#define GRID_SIZE 50
#define DTX 36
#define DTY 91
#define PHOTO_W 474
#define PHOTO_H 584
#define SIZE 8
#pragma comment(lib, "winmm.lib")


using namespace std;
/*ai策略修改历程
1. 直接在第一个可行点下棋
2. 在使对手可下的点最少的地方下棋
3. 在优先级最大的点落子，如果有多个，那么就在使对方平均优先级最小的点落子
4. 尝试1 minmax算法
*/


//黑棋  1 白棋  -1
int board[8][8] = {}, lastboard[500][8][8] = {}, rounds = 0;
static int dx[8] = { -1,1,0,0,-1,1,-1,1 }, dy[8] = { 0,0,1,-1,1,1,-1,-1 };//搜寻用的------上，下，右，左，右上，右下，左上，左下
bool endf1 = 0, endf2 = 0, endgame = 0;
int aicolor, humancolor, color, oppx = -1, oppy = -1, sum1 = 0, sum2 = 0;
int priority[8][8] = { {500,-25,10, 5, 5,10,-25,500},
                       {-25,-45, 1, 1, 1, 1,-45,-25},
                       { 10,  1, 3, 2, 2, 3,  1, 10},
                       {  5,  1, 2, 1, 1, 2,  1,  5},
                       {  5,  1, 2, 1, 1, 2,  1,  5},
                       { 10,  1, 3, 2, 2, 3,  1, 10},
                       {-25,-45, 1, 1, 1, 1,-45,-25},
                       {500,-25,10, 5, 5,10,-25,500}, };
const int weight[SIZE][SIZE] = {
    20, -3, 11,  8,  8, 11, -3, 20,
    -3, -7, -4,  1,  1, -4, -7, -3,
    11, -4,  2,  2,  2,  2, -4, 11,
    8,  1,  2, -3, -3,  2,  1,  8,
    8,  1,  2, -3, -3,  2,  1,  8,
    11, -4,  2,  2,  2,  2, -4, 11,
    -3, -7, -4,  1,  1, -4, -7, -3,
    20, -3, 11,  8,  8, 11, -3, 20,
};
int goaldepth = 5;
IMAGE img_bg;
ExMessage msg;
/*————————————————————————————————*/
//以下是规则函数
bool checkOK(int x, int y, int color)//大概可以check legal
{
    if (board[x][y] != 0 || (x >= 8 || x < 0 || y >= 8 || y < 0 || !(color == 1 || color == -1)))
        return 0;
    bool checklegal = 0; int i;
    for (i = 0; i < 8; i++)//上，下，右，左，右上，右下，左上，左下
    {
        int nx, ny;
        bool flag = 0;
        for (int j = 1; j < 8; j++)
        {
            nx = x + j * dx[i]; ny = y + j * dy[i];
            if (nx >= 8 || nx < 0 || ny >= 8 || ny < 0 || (board[nx][ny] == color && !flag) || board[nx][ny] == 0)
                break;
            if (board[nx][ny] == color * (-1))
                flag = 1;
            if (flag && board[nx][ny] == color)
            {
                return 1;
            }
        }
    }
    return 0;
}

bool checkOK2(int x, int y, int color, int state[8][8])//大概可以check legal
{
    if (state[x][y] != 0 || (x >= 8 || x < 0 || y >= 8 || y < 0 || !(color == 1 || color == -1))) return 0;

    bool checklegal = 0; int i;
    for (i = 0; i < 8; i++)
    {
        int nx, ny;
        bool flag = 0;
        for (int j = 1; j < 8; j++)
        {
            nx = x + j * dx[i]; ny = y + j * dy[i];
            if (nx >= 8 || nx < 0 || ny >= 8 || ny < 0 || (state[nx][ny] == color && !flag) || state[nx][ny] == 0)
                break;
            if (state[nx][ny] == color * (-1))
                flag = 1;
            if (flag && state[nx][ny] == color)
            {
                checklegal = 1;
                break;
            }
        }
    }
    if (checklegal) return 1;
    return 0;
}

void eat(int x, int y, int color)//测试了10次 吃子功能大概正确
{
    int i;  
    bool checklegal = 0;
    for (i = 0; i < 8; i++)
    {
        int nx, ny;
        bool flag = 0;
        for (int j = 1; j < 8; j++)
        {
            nx = x + j * dx[i]; ny = y + j * dy[i];
            if (nx >= 8 || nx < 0 || ny >= 8 || ny < 0 || (board[nx][ny] == color && !flag) || board[nx][ny] == 0)
                break;
            if (board[nx][ny] == color * (-1))
                flag = 1;
            if (flag && board[nx][ny] == color)
            {
                for (int tx = x, ty = y, m = 1; m < j; m++)
                {
                    tx += dx[i]; ty += dy[i];
                    board[tx][ty] = color;
                }
                board[x][y] = color;//走到这一步才能说明落子有效 才给它正式落子
                checklegal = 1;
                break;
            }
        }
    }
    return;
}

void eat2(int x, int y, int color, int state[8][8])//测试了10次 吃子功能大概正确
{
    int i;  bool checklegal = 0;
    for (i = 0; i < 8; i++)
    {
        int nx, ny;
        bool flag = 0;
        for (int j = 1; j < 8; j++)
        {
            nx = x + j * dx[i]; ny = y + j * dy[i];
            if (nx >= 8 || nx < 0 || ny >= 8 || ny < 0 || (state[nx][ny] == color && !flag) || state[nx][ny] == 0)
                break;
            if (state[nx][ny] == color * (-1))
                flag = 1;
            if (flag && state[nx][ny] == color)
            {
                for (int tx = x, ty = y, m = 1; m < j; m++)
                {
                    tx += dx[i]; ty += dy[i];
                    state[tx][ty] = color;
                }
                state[x][y] = color;//走到这一步才能说明落子有效 才给它正式落子
                checklegal = 1;
                break;
            }
        }
    }
    return;
}

int botwinorlose(int color)
{
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j] == color)
                sum1++;
            if (board[i][j] == color * (-1))
                sum2++;
        }
    if (sum1 > sum2)//赢
        return -1;
    if (sum1 < sum2)//失败
        return 1;
    if (sum1 == sum2)
        return 0;
}

bool ban(int color)//判断是否禁手
{
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (checkOK(i, j, color))
                return 0;
    return 1;
}

void printava(int color)//打出可行点位
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (checkOK(i, j, color))
            {
                int x = (j + 0.5) * GRID_SIZE + DTX;
                int y = (i + 0.5) * GRID_SIZE + DTY;
                setcolor(LIGHTBLUE);
                circle(x, y, 20);
            }
        }
    }
    return;
}

void cun(int b[8][8], int nb[8][8])//存盘方便模拟,实质上是一个复制函数
{
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            nb[i][j] = b[i][j];
}
/*——————————————————————————————*/
//以下是图形界面
void iniboard()
{
    board[3][3] = -1; board[3][4] = 1; board[4][3] = 1; board[4][4] = -1;
}

void printboard()
{
    cleardevice();
    loadimage(&img_bg, L"res\\黑白棋棋盘带菜单.jpg");
    putimage(0, 0, &img_bg);
    //绘制棋子
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] != 0) {
                //求格子中心坐标
                int x = (j + 0.5) * GRID_SIZE + DTX;
                int y = (i + 0.5) * GRID_SIZE + DTY;
                //绘制棋子
                if (board[i][j] == -1) {
                    setfillcolor(WHITE);
                }
                else if (board[i][j] == 1) {
                    setfillcolor(BLACK);
                }
                solidcircle(x, y, 20);
            }
        }
    }

}

void aithink(int color);
void newgame()//初始界面
{
    //音乐
    mciSendString(L"play res\\开场.mp3 repeat", 0, 0, 0);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = 0;
        }
    }
    iniboard();
    printboard();
    int key = MessageBox(NULL, L"是否选择先手黑棋？\n选择“是”为黑棋，选择“否”为白棋\n规则介绍请按左上角", L"请选择您方执子颜色", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND);
    if (key == 6) {
        color = 1;
    }
    else if (key == 7) {
        color = -1;
    }
    humancolor = color;
    aicolor = color * (-1);
    rounds = 0;
    sum1 = 0; sum2 = 0;
    endgame = 0;
    if (humancolor == -1) {
        aithink(aicolor);
        printboard();
        rounds++;
    }
    char c[3] = {};
    wchar_t wc[3];
    MultiByteToWideChar(CP_ACP, 0, c, -1, wc, 3);
    while (1) {
        InputBox(wc, 2, L"1：新手  初出茅庐\n2：中级  渐入佳境\n3：大师  百战百胜\n4：宗师  决胜千里", L"选择难度", L"只能输入一位数字哦~", 250, 95);
        if (wc[0] >= '1' && wc[0] <= '9') {
            goaldepth = wc[0] - '0';
            break;
        }
    }
    mciSendString(L"play res\\斗地主.mp3 repeat", 0, 0, 0);
    mciSendString(L"close res\\开场.mp3", NULL, 0, NULL);
}

void mousemessage() {
    int mx, my, x, y, lastx = -1, lasty = -1, left, top;
    //bool flag = 0;
    while (1) {
        msg = getmessage(EX_MOUSE | EX_KEY);
        switch (msg.message) {
        case WM_MOUSEMOVE: {
            mx = msg.x;
            my = msg.y;
            //cout << mx << " " << my << endl;
            x = (my - DTY) / GRID_SIZE;
            y = (mx - DTX) / GRID_SIZE;
            break;
        }
        case WM_LBUTTONDOWN: {           
            //规则介绍
            if (mx >= 8 && mx <= 48 && my >= 18 && my <= 69) {
                int key = MessageBox(NULL, L"棋盘为 8X8。\n黑白双方交替落子，但落子位置必须保证能让自己的子“夹”住对方的子来翻转对方的棋子，使之成为自己的子，同时不考虑连锁反应。\n若没有这种位置则己方跳过回合。双方均无子可下的时候游戏结束，数出双方棋子数，棋多者胜。\n黑方为先手。", L"黑白棋规则介绍", MB_OK | MB_ICONQUESTION | MB_SETFOREGROUND);
            }
            //悔棋
            else if (mx >= 47 && mx <= 108 && my >= 510 && my <= 565) {
                if (rounds == 0) {
                    int key = MessageBox(NULL, L"已悔棋到初始棋盘，您不能再继续悔棋", L"悔棋失败", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
                    break;
                }
                cun(lastboard[rounds - 1], board);
                rounds--;
                printboard();
                printava(color);
                break;
            }
            //存档
            else if (mx >= 155 && mx <= 213 && my >= 510 && my <= 565) {
                ofstream outf("cundang.txt");
                if (!outf) {
                    int key = MessageBox(NULL, L"Open error!", L"Error", MB_OK | MB_ICONHAND | MB_SETFOREGROUND);
                    break;
                }
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        outf << board[i][j] << ' ';
                    }
                    outf << endl;
                }
                outf << humancolor << endl;
                outf << rounds << endl;
                for (int t = 0; t < rounds; ++t) {
                    for (int i = 0; i < 8; ++i) {
                        for (int j = 0; j < 8; ++j) {
                            outf << lastboard[t][i][j] << " ";
                        }
                        outf << endl;
                    }
                }

                outf.close();
                int key = MessageBox(NULL, L"存档成功！", L"存档", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
                break;
            }
            //读档
            else if (mx >= 259 && mx <= 320 && my >= 510 && my <= 565) {
                int key = MessageBox(NULL, L"读档将会覆盖当前棋盘，是否确认读档？", L"请确认", MB_YESNO | MB_ICONINFORMATION | MB_SETFOREGROUND);
                if (key == 6) {
                    rounds = 0;
                    ifstream inf("cundang.txt");
                    if (!inf) {
                        int key = MessageBox(NULL, L"Open error!", L"Error", MB_OK | MB_ICONHAND | MB_SETFOREGROUND);

                        break;
                    }
                    for (int i = 0; i < 8; ++i) {
                        for (int j = 0; j < 8; ++j) {
                            inf >> board[i][j];
                        }
                    }
                    inf >> humancolor >> rounds;
                    for (int t = 0; t < rounds; ++t) {
                        for (int i = 0; i < 8; ++i) {
                            for (int j = 0; j < 8; ++j) {
                                inf >> lastboard[t][i][j];
                            }
                        }
                    }
                    inf.close();
                    aicolor = humancolor * (-1);
                    color = humancolor;
                    printboard();
                    printava(color);
                }
            }
            //认输
            else if (mx >= 364 && mx <= 426 && my >= 510 && my <= 565) {
                int key = MessageBox(NULL, L"只差一点就要成功了，确定要认输吗？", L"请确认", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND);
                if (key == 6) {//YES                 
                    endgame = 1;
                    return;
                }
            }
            //新游戏
            else if (mx >= 173 && mx <= 302 && my >= 27 && my <= 69) {
                int key = MessageBox(NULL, L"确定要放弃当前游戏开始新游戏吗？\n建议先存档", L"请确认", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND);
                if (key == 6) {
                    mciSendString(L"close res\\斗地主.mp3 repeat", 0, 0, 0);
                    newgame();
                    cun(board, lastboard[rounds]);
                    printava(color);
                    endf1 = 0;
                    endf2 = 0;
                }
            }
            //落子
            else {
                oppx = (my - DTY) / GRID_SIZE;
                oppy = (mx - DTX) / GRID_SIZE;
                if (checkOK(oppx, oppy, color)) {
                    eat(oppx, oppy, color);
                    printboard();
                    return;
                }
                else if (mx - DTX >= 0 && my - DTY >= 0 && oppx >= 0 && oppx <= 7 && oppy >= 0 && oppy <= 7) {
                    int key = MessageBox(NULL, L"不符合规则,请重新落子在可行点位蓝色圆圈内", L"提示", MB_OK | MB_ICONQUESTION | MB_SETFOREGROUND);
                }
            }
        }//case鼠标左键点击的大括号
        }//switch的大括号       
    }//死循环的大括号       
}

/*——————————————————————————————————*/
//以下是策略函数

int positionvalue(int state[8][8])//计算ai方color的value
{
    int value = 0;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (state[i][j] == aicolor)
                value += priority[i][j];
    return value;
}

int moves(int color, int state[8][8])//行动方 也可以在玩家（对手）视角评估，那就取反
{
    int move = 0;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (checkOK2(i, j, color, state))
                move++;
    int k;
    if (color == aicolor)  k = 1;
    else if (color == humancolor)  k = -1;
    return move * k;
}

int stable(int state[8][8])//只统计ai的边角个数+四面八方都有子的个数
{
    int stable = 0;
    for (int i = 0; i < 8; i++)
    {
        if (state[i][0] == aicolor)    stable++;
        if (state[i][7] == aicolor)    stable++;
    }
    for (int j = 0; j < 8; j++)
    {
        if (state[0][j] == aicolor)    stable++;
        if (state[7][j] == aicolor)    stable++;
    }
    //边角（算了两次角，相当于又强调了一次角）
    for (int i = 1; i < 7; i++)
    {
        for (int j = 1; j < 7; j++)//只算中间的
        {
            if (state[i][j] = aicolor)
            {
                int c;
                for (c = 0; c < 8; c++)
                {
                    bool f = 0;
                    for (int q = 0; q < 8; q++)
                    {
                        int nx = i + dx[c], ny = i + dy[c];
                        if (nx >= 8 || nx < 0 || ny >= 8 || ny < 0)    break;
                        if (state[nx][ny] == humancolor)
                        {
                            f = 1; break;
                        }
                    }
                    if (!f)  break;
                }
                if (c == 8)    stable++;
            }
        }
    }//这个函数正确性存疑！
    return stable;
}

int evaluation2(int board[8][8], int color) {//state棋盘，ai的color
    int aisum = 0, humansum = 0, aifront = 0, humanfront = 0;
    int aimobil = 0, humanmobil = 0;
    int myplayer = aicolor;
    double vtiles = 0, vfront = 0, vmobil = 0, vcorner = 0, vclose = 0;
    int map_weight = 0;

    int i, j;
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++) {
            if (checkOK2(i, j, myplayer, board)) { aimobil++; continue; }//ai的可行点位
            else if (checkOK2(i, j, (-1) * myplayer, board)) { humanmobil++; continue; }//玩家的可行点位
            else if (board[i][j] == myplayer) {//ai的子，以下计算ai的前沿子
                map_weight += weight[i][j]; aisum++;
                if (i > 0 && i < 7 && j>0 && j < 7) {
                    if (board[i - 1][j] == 0) { aifront++; continue; }
                    else if (board[i + 1][j] == 0) { aifront++; continue; }
                    else if (board[i][j - 1] == 0) { aifront++; continue; }
                    else if (board[i][j + 1] == 0) { aifront++; continue; }
                    else if (board[i - 1][j - 1] == 0) { aifront++; continue; }
                    else if (board[i - 1][j + 1] == 0) { aifront++; continue; }
                    else if (board[i + 1][j - 1] == 0) { aifront++; continue; }
                    else if (board[i + 1][j + 1] == 0) { aifront++; continue; }
                }
                else if (i == 0 || i == 7) {
                    if (j > 0) {
                        if (board[i][j - 1] == 0) { aifront++; continue; }
                    }
                    else if (j < 7) {
                        if (board[i][j + 1] == 0) { aifront++; continue; }
                    }
                }
                else if (j == 0 || j == 7) {
                    if (i > 0) {
                        if (board[i - 1][j] == 0) { aifront++; continue; }
                    }
                    else if (i < 7) {
                        if (board[i + 1][j] == 0) { aifront++; continue; }
                    }
                }

            }
            else if (board[i][j] == -myplayer) {//玩家的子，以下计算玩家的前沿子
                map_weight -= weight[i][j]; humansum++;
                if (i > 0 && i < 7 && j>0 && j < 7) {
                    if (board[i - 1][j] == 0) { humanfront++; continue; }
                    else if (board[i + 1][j] == 0) { humanfront++; continue; }
                    else if (board[i][j - 1] == 0) { humanfront++; continue; }
                    else if (board[i][j + 1] == 0) { humanfront++; continue; }
                    else if (board[i - 1][j - 1] == 0) { humanfront++; continue; }
                    else if (board[i - 1][j + 1] == 0) { humanfront++; continue; }
                    else if (board[i + 1][j - 1] == 0) { humanfront++; continue; }
                    else if (board[i + 1][j + 1] == 0) { humanfront++; continue; }
                }
                else if (i == 0 || i == 7) {
                    if (j > 0) {
                        if (board[i][j - 1] == 0) { humanfront++; continue; }
                    }
                    else if (j < 7) {
                        if (board[i][j + 1] == 0) { humanfront++; continue; }
                    }
                }
                else if (j == 0 || j == 7) {
                    if (i > 0) {
                        if (board[i - 1][j] == 0) { humanfront++; continue; }
                    }
                    else if (i < 7) {
                        if (board[i + 1][j] == 0) { humanfront++; continue; }
                    }
                }
            }
        }

    if (aisum > humansum)
        vtiles = (100.0 * aisum) / (aisum + humansum);
    else if (aisum < humansum)
        vtiles = -(100.0 * humansum) / (aisum + humansum);
    if (aifront > humanfront)
        vfront = -(100.0 * aifront) / (aifront + humanfront);
    else if (aifront < humanfront)
        vfront = (100.0 * humanfront) / (aifront + humanfront);

    if (aimobil > humanmobil)
        vmobil = (100.0 * aimobil) / (aimobil + humanmobil);
    else if (aimobil < humanmobil)
        vmobil = -(100.0 * humanmobil) / (aimobil + humanmobil);

    vcorner = board[0][0] * myplayer + board[0][7] * myplayer + board[7][0] * myplayer + board[7][7] * myplayer;
    vcorner = 25 * vcorner;

    if (board[0][0] == 0) {
        vclose += board[0][1] * myplayer;
        vclose += board[1][0] * myplayer;
        vclose += board[1][1] * myplayer;
    }
    if (board[0][7] == 0) {
        vclose += board[0][6] * myplayer;
        vclose += board[1][7] * myplayer;
        vclose += board[1][6] * myplayer;
    }
    if (board[7][0] == 0) {
        vclose += board[6][0] * myplayer;
        vclose += board[7][1] * myplayer;
        vclose += board[6][1] * myplayer;
    }
    if (board[7][7] == 0) {
        vclose += board[7][6] * myplayer;
        vclose += board[6][7] * myplayer;
        vclose += board[6][6] * myplayer;
    }
    vclose = -12.5 * vclose;

    double score = ((10 * vtiles) + (801.724 * vcorner) + (382.026 * vclose) + (78.922 * vmobil) + (74.396 * vfront) + (10 * map_weight)) * 100;

    return int(score);
}

int alphabeta(int a, int b, int depth, int state[8][8], int color);
void aithink(int color)
{
    int copy[8][8];
    cun(board, copy);
    int possible[64][8][8], cnt = 0;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (checkOK(i, j, color))
            {
                eat(i, j, color);
                cun(board, possible[cnt++]);
                cun(copy, board);
            }
    int max = -1e8, r;
    for (int i = 0; i < cnt; i++)
    {  //声明  int alphabeta(int a, int b, int depth, int state[8][8], int color)
        int temp = alphabeta( -1e8,   1e8,         0,     possible[i], color * (-1));
        if (temp > max)
        {
            max = temp;
            r = i;
        }
    }
    //cout << "\n\nmax=" << max << '\n';
    cun(possible[r], board);
}

int alphabeta(int a, int b, int depth, int state[8][8], int color)//ai的color
{
    int copy[8][8];
    cun(state, copy);
    int possible[64][8][8], cnt = 0;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (checkOK2(i, j, color, state))
            {
                eat2(i, j, color, state);
                cun(state, possible[cnt++]);
                cun(copy, state);
            }
    //以上是列举出能够下的点，存到possible[cnt]里面
    if (depth == goaldepth || cnt == 0)
    {
        int k = evaluation2(state, color);
        //cout<<k<<' ';
        return k;//只进行最后一步的估值，前面的节点直接通过策略选择后面的值
    }

    else if (color == aicolor)//这一步是ai方 max策略
    {
        int _max = -1e8;
        for (int i = 0; i < cnt; i++)
        {
            int value = alphabeta(a, b, depth + 1, possible[i], color * (-1));

            _max = max(_max, value);
            a = max(a, value);

            if (b <= a)    break;
        }
        return _max;
    }

    else if (color == humancolor)
    {
        int _min = 1e8;
        for (int i = 0; i < cnt; i++)
        {
            int value = alphabeta(a, b, depth + 1, possible[i], color * (-1));

            _min = min(_min, value);
            b = min(_min, value);

            if (b <= a)    break;
        }
        return _min;
    }
}

int main()
{
    //创建一个窗口
    initgraph(PHOTO_W, PHOTO_H);//474,584  
    //加载背景图片
    loadimage(&img_bg, L"res\\黑白棋棋盘带菜单.jpg");
    putimage(0, 0, &img_bg);
    //加载音乐
    mciSendString(L"open res\\开场.mp3", 0, 0, 0);
    mciSendString(L"open res\\斗地主.mp3", 0, 0, 0);
    mciSendString(L"open res\\败.mp3", 0, 0, 0);
    mciSendString(L"open res\\胜.mp3", 0, 0, 0);
loop:
    newgame();
    while (!(endf1 && endf2))
    {
        cun(board, lastboard[rounds]);
        printava(color);
        if (!ban(color))
        {
            endf1 = 0;
            mousemessage();
            if (endgame) {
                break;
            }
        }

        else
        {
            int key = MessageBox(NULL, L"您已被禁手，请等待AI先行", L"提示", MB_OK | MB_ICONQUESTION | MB_SETFOREGROUND);
            endf1 = 1;
        }

        color *= -1;//       切换到ai上来
		Sleep(1000);//延时1秒

        if (!ban(color))
        {
            endf2 = 0;
            aithink(color);
            printboard();
        }
        else
        {
            if (!endf1) {
                int key = MessageBox(NULL, L"AI已被禁手，请您再下一子", L"提示", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
                endf2 = 1;
            }
            else {
                break;
            }
        }
        color *= -1;
        rounds++;
    }
    mciSendString(L"close res\\斗地主.mp3", NULL, 0, NULL);
    int judge = botwinorlose(humancolor), Key;
    char str[100] = {};
    ostrstream strout(str, 100);
    strout << "您的棋子数：" << sum1 << "    bot的棋子数：" << sum2 << endl;
    wchar_t wstr[100];
    MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, 100);
    if (judge == 1 || endgame) {
        mciSendString(L"play res\\败.mp3", 0, 0, 0);
        strout << "您被我的bot打败了！" << endl;
        strout << "是否再来一局？" << endl;
        Key = MessageBox(NULL, wstr, L"败于垂成", MB_YESNO | MB_ICONSTOP | MB_SETFOREGROUND);

    }
    else if (judge == -1) {
        mciSendString(L"play res\\胜.mp3", 0, 0, 0);
        strout << "恭喜！您打败了我的bot。" << endl;
        strout << "是否再来一局？" << endl;
        Key = MessageBox(NULL, wstr, L"功成名就", MB_YESNO | MB_ICONINFORMATION | MB_SETFOREGROUND);

    }
    else if (judge == 0) {
        mciSendString(L"play res\\胜.mp3", 0, 0, 0);
        strout << "和局！" << endl;
        strout << "是否再来一局？" << endl;
        Key = MessageBox(NULL, wstr, L"以和为贵", MB_YESNO | MB_ICONINFORMATION | MB_SETFOREGROUND);

    }
    if (Key == 6) {
        endf1 = 0; endf2 = 0;
        mciSendString(L"close res\\败.mp3", NULL, 0, NULL);
        mciSendString(L"close res\\胜.mp3", NULL, 0, NULL);
        goto loop;
    }
    closegraph();
    return 0;
}
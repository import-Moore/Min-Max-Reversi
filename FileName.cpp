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
/*ai�����޸�����
1. ֱ���ڵ�һ�����е�����
2. ��ʹ���ֿ��µĵ����ٵĵط�����
3. �����ȼ����ĵ����ӣ�����ж������ô����ʹ�Է�ƽ�����ȼ���С�ĵ�����
4. ����1 minmax�㷨
*/


//����  1 ����  -1
int board[8][8] = {}, lastboard[500][8][8] = {}, rounds = 0;
static int dx[8] = { -1,1,0,0,-1,1,-1,1 }, dy[8] = { 0,0,1,-1,1,1,-1,-1 };//��Ѱ�õ�------�ϣ��£��ң������ϣ����£����ϣ�����
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
/*����������������������������������������������������������������*/
//�����ǹ�����
bool checkOK(int x, int y, int color)//��ſ���check legal
{
    if (board[x][y] != 0 || (x >= 8 || x < 0 || y >= 8 || y < 0 || !(color == 1 || color == -1)))
        return 0;
    bool checklegal = 0; int i;
    for (i = 0; i < 8; i++)//�ϣ��£��ң������ϣ����£����ϣ�����
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

bool checkOK2(int x, int y, int color, int state[8][8])//��ſ���check legal
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

void eat(int x, int y, int color)//������10�� ���ӹ��ܴ����ȷ
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
                board[x][y] = color;//�ߵ���һ������˵��������Ч �Ÿ�����ʽ����
                checklegal = 1;
                break;
            }
        }
    }
    return;
}

void eat2(int x, int y, int color, int state[8][8])//������10�� ���ӹ��ܴ����ȷ
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
                state[x][y] = color;//�ߵ���һ������˵��������Ч �Ÿ�����ʽ����
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
    if (sum1 > sum2)//Ӯ
        return -1;
    if (sum1 < sum2)//ʧ��
        return 1;
    if (sum1 == sum2)
        return 0;
}

bool ban(int color)//�ж��Ƿ����
{
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (checkOK(i, j, color))
                return 0;
    return 1;
}

void printava(int color)//������е�λ
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

void cun(int b[8][8], int nb[8][8])//���̷���ģ��,ʵ������һ�����ƺ���
{
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            nb[i][j] = b[i][j];
}
/*������������������������������������������������������������*/
//������ͼ�ν���
void iniboard()
{
    board[3][3] = -1; board[3][4] = 1; board[4][3] = 1; board[4][4] = -1;
}

void printboard()
{
    cleardevice();
    loadimage(&img_bg, L"res\\�ڰ������̴��˵�.jpg");
    putimage(0, 0, &img_bg);
    //��������
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] != 0) {
                //�������������
                int x = (j + 0.5) * GRID_SIZE + DTX;
                int y = (i + 0.5) * GRID_SIZE + DTY;
                //��������
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
void newgame()//��ʼ����
{
    //����
    mciSendString(L"play res\\����.mp3 repeat", 0, 0, 0);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = 0;
        }
    }
    iniboard();
    printboard();
    int key = MessageBox(NULL, L"�Ƿ�ѡ�����ֺ��壿\nѡ���ǡ�Ϊ���壬ѡ�񡰷�Ϊ����\n��������밴���Ͻ�", L"��ѡ������ִ����ɫ", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND);
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
        InputBox(wc, 2, L"1������  ����é®\n2���м�  ����Ѿ�\n3����ʦ  ��ս��ʤ\n4����ʦ  ��ʤǧ��", L"ѡ���Ѷ�", L"ֻ������һλ����Ŷ~", 250, 95);
        if (wc[0] >= '1' && wc[0] <= '9') {
            goaldepth = wc[0] - '0';
            break;
        }
    }
    mciSendString(L"play res\\������.mp3 repeat", 0, 0, 0);
    mciSendString(L"close res\\����.mp3", NULL, 0, NULL);
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
            //�������
            if (mx >= 8 && mx <= 48 && my >= 18 && my <= 69) {
                int key = MessageBox(NULL, L"����Ϊ 8X8��\n�ڰ�˫���������ӣ�������λ�ñ��뱣֤�����Լ����ӡ��С�ס�Է���������ת�Է������ӣ�ʹ֮��Ϊ�Լ����ӣ�ͬʱ������������Ӧ��\n��û������λ���򼺷������غϡ�˫�������ӿ��µ�ʱ����Ϸ����������˫���������������ʤ��\n�ڷ�Ϊ���֡�", L"�ڰ���������", MB_OK | MB_ICONQUESTION | MB_SETFOREGROUND);
            }
            //����
            else if (mx >= 47 && mx <= 108 && my >= 510 && my <= 565) {
                if (rounds == 0) {
                    int key = MessageBox(NULL, L"�ѻ��嵽��ʼ���̣��������ټ�������", L"����ʧ��", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
                    break;
                }
                cun(lastboard[rounds - 1], board);
                rounds--;
                printboard();
                printava(color);
                break;
            }
            //�浵
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
                int key = MessageBox(NULL, L"�浵�ɹ���", L"�浵", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
                break;
            }
            //����
            else if (mx >= 259 && mx <= 320 && my >= 510 && my <= 565) {
                int key = MessageBox(NULL, L"�������Ḳ�ǵ�ǰ���̣��Ƿ�ȷ�϶�����", L"��ȷ��", MB_YESNO | MB_ICONINFORMATION | MB_SETFOREGROUND);
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
            //����
            else if (mx >= 364 && mx <= 426 && my >= 510 && my <= 565) {
                int key = MessageBox(NULL, L"ֻ��һ���Ҫ�ɹ��ˣ�ȷ��Ҫ������", L"��ȷ��", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND);
                if (key == 6) {//YES                 
                    endgame = 1;
                    return;
                }
            }
            //����Ϸ
            else if (mx >= 173 && mx <= 302 && my >= 27 && my <= 69) {
                int key = MessageBox(NULL, L"ȷ��Ҫ������ǰ��Ϸ��ʼ����Ϸ��\n�����ȴ浵", L"��ȷ��", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND);
                if (key == 6) {
                    mciSendString(L"close res\\������.mp3 repeat", 0, 0, 0);
                    newgame();
                    cun(board, lastboard[rounds]);
                    printava(color);
                    endf1 = 0;
                    endf2 = 0;
                }
            }
            //����
            else {
                oppx = (my - DTY) / GRID_SIZE;
                oppy = (mx - DTX) / GRID_SIZE;
                if (checkOK(oppx, oppy, color)) {
                    eat(oppx, oppy, color);
                    printboard();
                    return;
                }
                else if (mx - DTX >= 0 && my - DTY >= 0 && oppx >= 0 && oppx <= 7 && oppy >= 0 && oppy <= 7) {
                    int key = MessageBox(NULL, L"�����Ϲ���,�����������ڿ��е�λ��ɫԲȦ��", L"��ʾ", MB_OK | MB_ICONQUESTION | MB_SETFOREGROUND);
                }
            }
        }//case����������Ĵ�����
        }//switch�Ĵ�����       
    }//��ѭ���Ĵ�����       
}

/*��������������������������������������������������������������������*/
//�����ǲ��Ժ���

int positionvalue(int state[8][8])//����ai��color��value
{
    int value = 0;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (state[i][j] == aicolor)
                value += priority[i][j];
    return value;
}

int moves(int color, int state[8][8])//�ж��� Ҳ��������ң����֣��ӽ��������Ǿ�ȡ��
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

int stable(int state[8][8])//ֻͳ��ai�ı߽Ǹ���+����˷������ӵĸ���
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
    //�߽ǣ��������νǣ��൱����ǿ����һ�νǣ�
    for (int i = 1; i < 7; i++)
    {
        for (int j = 1; j < 7; j++)//ֻ���м��
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
    }//���������ȷ�Դ��ɣ�
    return stable;
}

int evaluation2(int board[8][8], int color) {//state���̣�ai��color
    int aisum = 0, humansum = 0, aifront = 0, humanfront = 0;
    int aimobil = 0, humanmobil = 0;
    int myplayer = aicolor;
    double vtiles = 0, vfront = 0, vmobil = 0, vcorner = 0, vclose = 0;
    int map_weight = 0;

    int i, j;
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++) {
            if (checkOK2(i, j, myplayer, board)) { aimobil++; continue; }//ai�Ŀ��е�λ
            else if (checkOK2(i, j, (-1) * myplayer, board)) { humanmobil++; continue; }//��ҵĿ��е�λ
            else if (board[i][j] == myplayer) {//ai���ӣ����¼���ai��ǰ����
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
            else if (board[i][j] == -myplayer) {//��ҵ��ӣ����¼�����ҵ�ǰ����
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
    {  //����  int alphabeta(int a, int b, int depth, int state[8][8], int color)
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

int alphabeta(int a, int b, int depth, int state[8][8], int color)//ai��color
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
    //�������оٳ��ܹ��µĵ㣬�浽possible[cnt]����
    if (depth == goaldepth || cnt == 0)
    {
        int k = evaluation2(state, color);
        //cout<<k<<' ';
        return k;//ֻ�������һ���Ĺ�ֵ��ǰ��Ľڵ�ֱ��ͨ������ѡ������ֵ
    }

    else if (color == aicolor)//��һ����ai�� max����
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
    //����һ������
    initgraph(PHOTO_W, PHOTO_H);//474,584  
    //���ر���ͼƬ
    loadimage(&img_bg, L"res\\�ڰ������̴��˵�.jpg");
    putimage(0, 0, &img_bg);
    //��������
    mciSendString(L"open res\\����.mp3", 0, 0, 0);
    mciSendString(L"open res\\������.mp3", 0, 0, 0);
    mciSendString(L"open res\\��.mp3", 0, 0, 0);
    mciSendString(L"open res\\ʤ.mp3", 0, 0, 0);
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
            int key = MessageBox(NULL, L"���ѱ����֣���ȴ�AI����", L"��ʾ", MB_OK | MB_ICONQUESTION | MB_SETFOREGROUND);
            endf1 = 1;
        }

        color *= -1;//       �л���ai����
		Sleep(1000);//��ʱ1��

        if (!ban(color))
        {
            endf2 = 0;
            aithink(color);
            printboard();
        }
        else
        {
            if (!endf1) {
                int key = MessageBox(NULL, L"AI�ѱ����֣���������һ��", L"��ʾ", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
                endf2 = 1;
            }
            else {
                break;
            }
        }
        color *= -1;
        rounds++;
    }
    mciSendString(L"close res\\������.mp3", NULL, 0, NULL);
    int judge = botwinorlose(humancolor), Key;
    char str[100] = {};
    ostrstream strout(str, 100);
    strout << "������������" << sum1 << "    bot����������" << sum2 << endl;
    wchar_t wstr[100];
    MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, 100);
    if (judge == 1 || endgame) {
        mciSendString(L"play res\\��.mp3", 0, 0, 0);
        strout << "�����ҵ�bot����ˣ�" << endl;
        strout << "�Ƿ�����һ�֣�" << endl;
        Key = MessageBox(NULL, wstr, L"���ڴ���", MB_YESNO | MB_ICONSTOP | MB_SETFOREGROUND);

    }
    else if (judge == -1) {
        mciSendString(L"play res\\ʤ.mp3", 0, 0, 0);
        strout << "��ϲ����������ҵ�bot��" << endl;
        strout << "�Ƿ�����һ�֣�" << endl;
        Key = MessageBox(NULL, wstr, L"��������", MB_YESNO | MB_ICONINFORMATION | MB_SETFOREGROUND);

    }
    else if (judge == 0) {
        mciSendString(L"play res\\ʤ.mp3", 0, 0, 0);
        strout << "�;֣�" << endl;
        strout << "�Ƿ�����һ�֣�" << endl;
        Key = MessageBox(NULL, wstr, L"�Ժ�Ϊ��", MB_YESNO | MB_ICONINFORMATION | MB_SETFOREGROUND);

    }
    if (Key == 6) {
        endf1 = 0; endf2 = 0;
        mciSendString(L"close res\\��.mp3", NULL, 0, NULL);
        mciSendString(L"close res\\ʤ.mp3", NULL, 0, NULL);
        goto loop;
    }
    closegraph();
    return 0;
}
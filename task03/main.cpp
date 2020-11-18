#include <iostream>
#include <thread>
#include <Windows.h>
#include <ctime>
#include <mutex>
#include <chrono>

using namespace std;

const int map_h = 18, map_w = 40;

int block_h, block_w;

const int max_num_groups = 10, max_group_size = 10;
int num_groups, group_size;

string map_data =
              "-----OOOOOOOOOO-------------------------\n"
              "-------OOOOOO--------OOOOOOOOOOO--OOO---\n"
              "OO----------------OOOOOOOOOOOOOOOOOOOO--\n"
              "OOOOO-------OOOOOOOOOOOOOOOOOOOOOO------\n"
              "OOOOO---OOOOOOOOOOOOOOOOOOOOOOOOO-------\n"
              "OOO---OOOOOOOOOOOOOOOOOOOOOOOOOOO-------\n"
              "O-------OOOOOOOOOOOOOOOOOOOOOOOOOOO---OO\n"
              "OO---------OOOOOOOOOOOOOOOOOOOOOOO---OOO\n"
              "OOO----------OOOOOOOOOOOOOOOOOO-------OO\n"
              "OOOO----OOOO--OOOOOOOOOOOOOOOOOO------OO\n"
              "OOOOO--OOOOOOOOOOOOOOOOOOOOOOOOOO------O\n"
              "O------OOOOOOOOOOOOOOOOOXOOOOO--OO----OO\n"
              "OO----OOOOOO---OOOOOOOOOOOOOOO---O----OO\n"
              "OO---OOOOO----OOOOOOOOOOOOOOOOO------OOO\n"
              "OOO---OOO------OOOOOOOOOOOOOOOO-------OO\n"
              "OOO-----------OOOOOOOO--O-OOOO-------OOO\n"
              "OOOO------OOOOOOOOOOOO------------OOOOOO\n"
              "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n";

/*
string map_data = "---OOOOO----OO--\n"
                  "-OOOOOOOO--OOXO-\n"
                  "--OOOOOOOOOO----\n"
                  "--OOOOOOOOOOOO--\n"
                  "---OOOOOOOOOOO--\n"
                  "---OOOOOOOOOOO--\n"
                  "-----OOOOOOOOO--\n"
                  "---OOOOOOOOOO---\n"
                  "-OOOOOOOOOOOO---\n"
                  "-OOOOOOOOOOOOO--\n"
                  "--OOOO---OOOOO--\n"
                  "---OO-----O-----";
*/

char map[map_h][map_w];

int pos[max_num_groups][max_group_size][2];

int x_color = 64, sea_color = 7;

int num_blocks_h, num_blocks_w;

const chrono::milliseconds wait_time = 1s;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int max_curr_task;

int inline color(int i, int j) {
    i = i / block_h;
    j = j / block_w;
    return 3+i+j;
}

void build_map() {
    for (int i = 0; i < map_h; ++i) {
        for (int j = 0; j < map_w; ++j) {
            map[i][j] = map_data[i * (map_w + 1) + j];
        }
    }
}


void print_map() {

    for (int id = 0; id < num_groups; ++id) {
        for (int k = 0; k < group_size; ++k) {
            map[pos[id][k][0]][pos[id][k][1]] = (char)((int)'0' + id);
        }
    }

    for (int i = 0; i < map_h; ++i) {
        for (int j = 0; j < map_w; ++j) {
            if (map[i][j] == 'X') {
                SetConsoleTextAttribute(hConsole, x_color);
                cout << map[i][j];
            }
            else if ('0' <= map[i][j] && map[i][j] <= '0' + num_groups) {
                SetConsoleTextAttribute(hConsole, (11 + map[i][j]-'0') * 16 );
                cout << 'P';
            }
            else if (map[i][j] == '-') {
                SetConsoleTextAttribute(hConsole, 0);
                cout << map[i][j];
            }
            else
            {
                SetConsoleTextAttribute(hConsole, color(i, j) + 6);
                cout << map[i][j];
            }
        }
        cout << "\n";
    }

    for (int i = 0; i < num_groups; ++i) {
        for (int j = 0; j < group_size; ++j) {
            map[pos[i][j][0]][pos[i][j][1]] = '.';
        }
    }
}

int curr_task = 0;

mutex curr_task_lock;
mutex printing_lock;


void set_chars(int id, char c) {
    for (int j = 0; j < group_size; ++j) {
        map[pos[id][j][0]][pos[id][j][1]] = c;
    }
}

bool stop = false;

void find_X(int id) {
    while (true) {

        curr_task_lock.lock();

        int task = curr_task;
        curr_task = task + 1;

        curr_task_lock.unlock();

        if (stop || curr_task >= max_curr_task) {
            return;
        }

        int x = task / num_blocks_w, y = task % num_blocks_w;

        printing_lock.lock();
        SetConsoleTextAttribute(hConsole, sea_color);
        if (task < max_curr_task) {
            cout << "group #" << id + 1 << " is on field " << task + 1 << ".\n";
        }
        printing_lock.unlock();

        int free_space;

        while (!stop) {
            free_space = 0;

            for (int i = x * block_h; i < (x + 1) * block_h; ++i) {
                for (int j = y * block_w; j < (y + 1) * block_w; ++j) {
                    if (map[i][j] == 'O')
                        free_space++;
                }
            }

            if (free_space == 0) {
                break;
            }

            if (free_space >= group_size)
                free_space = group_size;

            printing_lock.lock();
            set_chars(id, '.');

            for (int i = 0; i < free_space; ++i) {
                int a, b;
                do {
                    a = x * block_h + abs(rand() % block_h);
                    b = y * block_w + abs(rand() % block_w);
                    if (map[a][b] == 'X') {
                        SetConsoleTextAttribute(hConsole, sea_color);
                        cout << "group #" << id+1 << " found the treasure!" << endl;
                        stop = true;
                        break;
                    }
                } while (map[a][b] != 'O');
                map[a][b] = 'P';
                pos[id][i][1] = b;
                pos[id][i][0] = a;
            }

            printing_lock.unlock();
            this_thread::sleep_for(wait_time);
        }
    }
}

void printing() {
    while (!stop) {
        this_thread::sleep_for(wait_time);
        printing_lock.lock();
        cout << endl;

        print_map();
        printing_lock.unlock();
    }
}


int main() {
    srand(time(nullptr));
    build_map();

    cout << "Enter block height and width: ";
    cin >> block_h >> block_w;

    cout << "Enter the amount of groups and group size: ";
    cin >> num_groups >> group_size;

    num_blocks_h = (map_h / block_h), num_blocks_w = (map_w / block_w);
    max_curr_task = num_blocks_h * num_blocks_w;

    thread groups[max_num_groups];
    thread print_thread(printing);

    for (int i = 0; i < num_groups; ++i) {
        groups[i] = thread {find_X, i};
    }

    for (int i = 0; i < num_groups; ++i) {
        groups[i].join();
    }

    stop = true;
    print_thread.join();

    SetConsoleTextAttribute(hConsole, sea_color);
    cout << "Done";
    getchar();
    return 0;
}

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <string>
#include <vector>

using namespace std;

template<int initial_value>
class semaphore
{
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = initial_value;

public:
    void post() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        ++count_;
        condition_.notify_one();
    }

    void wait() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        while(!count_) // Handle spurious wake-ups.
            condition_.wait(lock);
        --count_;
    }

    bool try_wait() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if(count_) {
            --count_;
            return true;
        }
        return false;
    }

    // Нужно для интерфейса std::lock
    void lock() {  wait(); }
    void unlock() {  post(); }
    bool try_lock() {  return try_wait(); }
};

int max_iter;

int curr_iter = 0;

string table[5] = {
                    "F--P1--F--P2",
                    "|         |",
                    "P5        F",
                    "|         |",
                    "F--P4--F--P3"
};

pair<int, int> fork_pos[] {
        {0,7}, {2, 10}, {4,7}, {4,0}, {0,0}
};

void change_fork_visibility(int fork_id) {
    pair<int, int> pos = fork_pos[fork_id];
    char* fork = &table[pos.first][pos.second];

    if (*fork == 'F')
        *fork = '.';
    else
        *fork = 'F';
}

// тут <параметр> - начальное значение счетчика семафора.
semaphore<1> forks[5];

semaphore<1> sync;

thread philosophers[5];

// вывод всех событий происходит только в print_iteration().
vector<string> events;

void print_iteration() {
    while (curr_iter < max_iter) {
        {
            sync.wait();
            cout << "time: " << curr_iter << "s\n";
            for (auto &str : events) {
                cout << str << "\n";
            }
            for (auto &str : table) {
                cout << str << "\n";
            }
            cout << "\n";
            events.clear();
            sync.post();
        }

        curr_iter++;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

bool take_or_put_forks(int curr_id, int prev_id) {
    if (curr_iter >= max_iter) {
        forks[curr_id].post();
        forks[prev_id].post();
        sync.post();
        return true;
    }

    change_fork_visibility(curr_id);
    change_fork_visibility(prev_id);
    return false;
}

void eat(int ph_id) {
    while (curr_iter < max_iter) {
        int prev_id = (ph_id + 4) % 5;

        // Ждем пока оба семафора не освободятся
        lock(forks[ph_id], forks[prev_id]);

        // Задержка для "реакции" философов (для более интересного вывода)
        this_thread::sleep_for(chrono::milliseconds( (rand() % 400) + 300) );

        // Доступ к этому блоку всегда есть только у 1 процесса для синхронизации и правильного вывода
        {
            sync.wait();

            if (take_or_put_forks(ph_id, prev_id))
                break;

            events.emplace_back("philosopher " + to_string(ph_id + 1) + " started eating");

            sync.post();
        }

        this_thread::sleep_for(chrono::milliseconds( (rand() % 2000) + 1000) );

        {
            sync.wait();

            if (take_or_put_forks(ph_id, prev_id))
                break;

            events.emplace_back("philosopher " + to_string(ph_id + 1) + " finished eating");

            sync.post();
        }

        forks[ph_id].post();
        forks[prev_id].post();
    }
}


int main() {
    srand(time(nullptr));

    while (true) {
        cout << "Enter maximum amount of iterations: ";
        cin >> max_iter;
        if (max_iter <= 0) {
            cout << "\nWrong format. max_iter should be > 0.\n";
        }
        else break;
    }
    cout << "\n";
    thread printing_thread (print_iteration);

    for (int i = 0; i < 5; ++i) {
        philosophers[i] = thread {eat, i};
    }

    for (auto &philosopher : philosophers) {
        philosopher.join();
    }

    printing_thread.join();

    return 0;
}

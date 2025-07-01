#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

const float M_PI = 3.14159265358979323846f;
const int width = 800;
const int height = 600;
const float V = 5.0f;
const int N = 3;
const int SIMULATION_TIME = 20;
std::mutex coutMutex;

struct Point {
    float x, y;
};

float getRandomFloat(float min, float max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

class Rabbit {
protected:
    Point pos;
    float vx, vy;
    int id;

public:
    Rabbit(int id_) : id(id_) {
        pos.x = getRandomFloat(0, width);
        pos.y = getRandomFloat(0, height);
        changeDirection();
    }

    virtual void run() = 0;

protected:
    void changeDirection() {
        float angle = getRandomFloat(0, 2 * M_PI);
        vx = V * cos(angle);
        vy = V * sin(angle);
    }

    void move() {
        pos.x += vx;
        pos.y += vy;

        if (pos.x <= 0 || pos.x >= width) vx = -vx;
        if (pos.y <= 0 || pos.y >= height) vy = -vy;
    }

    void printPosition(const string& type) {
        lock_guard<mutex> lock(coutMutex);
        cout << type << " #" << id << ": (" << pos.x << ", " << pos.y << ")" << endl;
    }
};

class RegularRabbit : public Rabbit {
public:
    using Rabbit::Rabbit;

    void run() override {
        auto lastDirectionChange = steady_clock::now();

        while (true) {
            move();
            printPosition("Regular rabbit");

            auto now = steady_clock::now();
            if (duration_cast<seconds>(now - lastDirectionChange).count() >= N) {
                changeDirection();
                lastDirectionChange = now;
            }

            this_thread::sleep_for(milliseconds(100));
        }
    }
};

class AlbinoRabbit : public Rabbit {
public:
    using Rabbit::Rabbit;

    AlbinoRabbit(int id_) : Rabbit(id_) {
        vx = V;
        vy = 0;
    }

    void run() override {
        while (true) {
            pos.x += vx;

            if (pos.x <= 0 || pos.x >= width)
                vx = -vx;

            printPosition("Albino rabbit");

            this_thread::sleep_for(milliseconds(100));
        }
    }
};

int main() {
    const int numRegular = 3;
    const int numAlbinos = 2;

    vector<thread> threads;

    for (int i = 0; i < numRegular; ++i) {
        RegularRabbit* r = new RegularRabbit(i + 1);
        threads.emplace_back([r]() { r->run(); });
    }

    for (int i = 0; i < numAlbinos; ++i) {
        AlbinoRabbit* a = new AlbinoRabbit(i + 1);
        threads.emplace_back([a]() { a->run(); });
    }

    this_thread::sleep_for(seconds(SIMULATION_TIME));

    cout << "Simulation finished. Press Enter to exit..." << endl;
    cin.get();
    return 0;
}

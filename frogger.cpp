#include "lib/console.h"
#include "lib/frame.h"
#include "lib/grid.h"
#include "lib/input.h"
#include "lib/render.h"
#include <chrono>

using namespace std;

static UserInput userInput{};
static constexpr int FPS{60};

class Car
{
public:
    Car(int _x, int _y, int _speed, int _framesPerMove)
    {;
        x = _x;
        y = _y;
        speed = _speed;
        framesPerMove = _framesPerMove;
        xStart = _speed < 0 ? grid.GetWidth()-1 : 0;
    }

    void Update()
    {
        int _x = x;
        int _y = y;
        
        frames++;
        if (frames >= framesPerMove)
        {
            x += speed;
            if (grid.IsOutOfBounds(x, y)) 
                x = xStart;
            frames = 0;
        }
        
        grid.SetTile(_x, _y);
        grid.SetTile(x, y, ascii);
    }

    static constexpr char ascii = 'C';

private:
    int xStart{};
    int x{};
    int y{};
    int speed{};
    int frames{};
    int framesPerMove{};
};

class Lane
{
public:
    Lane(int _y, int carCount)
    {
        y = _y;
        int fpm = (rand() % 10)+1;
        int space = grid.GetWidth() / carCount;
        int speed = rand() > 1073741824 ? 1 : -1;
        for (int i = 0; i < carCount; i++)
            cars.push_back(Car{space*i, y, speed, fpm});
    }

    void Update()
    {
        for (auto& car : cars)
            car.Update();
    }

private:
    int y{};
    int frames{};
    vector<Car> cars{};
};

class Pavement
{
public:
    Pavement(int _x, int _y)
    {
        x = _x;
        y = _y;
    }

    void Update()
    {
        grid.SetTile(x, y, ascii);
    }

    static constexpr char ascii = '.';

private:
    int x{};
    int y{};
};

class Infrastructure
{
public:
    Infrastructure(int laneCount)
    {
        for (int i = 0; i < grid.GetWidth()-1; i++)
        {
            pavements.emplace_back(Pavement{i, 0});
            pavements.emplace_back(Pavement{i, grid.GetHeight()-1});
        }
            
        int space = grid.GetHeight() / laneCount;
        for (int i = 0; i < laneCount; i++)
            lanes.emplace_back(Lane{space*i+1, 5});
    }

    void Update()
    {
        for (auto& lane : lanes)
            lane.Update();
        for (auto& pavement : pavements)
            pavement.Update();
    }

private:
    vector<Lane> lanes{};
    vector<Pavement> pavements{};
};

class Player
{
public:
    Player(int _x, int _y)
    {
        x = _y;
        y = _y;
        xSpawn = x;
        ySpawn = y;
    }

    void Update()
    {
        if (grid.IsCollision(x, y, Car::ascii))
        {
            grid.SetTile(x, y);
            Respawn();
            grid.SetTile(x, y, ascii);
            return;
        }

        if (y == 0)
            win = true;

        int _x = x;
        int _y = y;

        if (userInput == UserInput::Left) x--;
        else if (userInput == UserInput::Right) x++;
        else if (userInput == UserInput::Up) y--;
        else if (userInput == UserInput::Down) y++;

        if (grid.IsOutOfBounds(x, y))
        {
            x = _x;
            y = _y;
        }

        if (grid.IsCollision(x, y, Car::ascii))
        {
            Respawn();
        }

        grid.SetTile(_x, _y);
        grid.SetTile(x, y, ascii);
    }

    bool Wins() const { return win; }
    bool Loses() const { return lives == 0; }

    void Respawn()
    {
        x = xSpawn;
        y = ySpawn;
        lives--;
    }

    static constexpr char ascii = '@';

private:
    int x{};
    int y{};
    int xSpawn{};
    int ySpawn{};
    bool win{};
    int lives{3};
};

class Game
{
public:
    void Update()
    {
        infrastructure.Update();
        player.Update();
    }

    bool PlayerWins() const { return player.Wins(); }
    bool PlayerLoses() const { return player.Loses(); }

    Player player{grid.GetWidth()/2, grid.GetHeight()-1};
    Infrastructure infrastructure{grid.GetHeight()/2-1};
};

int main()
{
    srand(time(NULL));
    Console console{};
    Input input{};
    Frame frame{FPS};
    Render render{console};
    Game game{};

    while(1)
    {
        frame.limit();
        userInput = input.Read();
        if (userInput == UserInput::Quit) return 0;
        game.Update();
        if (game.PlayerWins())
        {
            console.moveCursor(grid.GetHeight()/2, grid.GetWidth()/2-4);
            console.print("Ribbit!");
            console.moveCursor(grid.GetHeight()/2+1, grid.GetWidth()/2-7);
            console.print("You've won!!!");
            break;
        }
        else if (game.PlayerLoses())
        {
            console.moveCursor(grid.GetHeight()/2, grid.GetWidth()/2-5);
            console.print("You lose.");
            console.moveCursor(grid.GetHeight()/2+1, grid.GetWidth()/2-17);
            console.print("Too bad frogs don't have 9 lives!");
            break;
        }
        render.Draw(grid.GetTiles());
    }

    frame = {1};
    frame.limit();
    frame.limit();
    frame.limit();
    frame.limit();

    return 0;
}
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <math.h>
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 900
#define colSize 450
#define rowSize 450
#define SQUARE_COL(row, column) Color(255 * sin((column + row * colSize)), 255 * tan((column + row * colSize)), 255 * cos((column + row * colSize)))
using namespace std;
using namespace sf;
RenderWindow window(VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Tester");
RenderTexture bufferText;
Sprite bufferSprite;
Shader shader;
float SQUARE_SIZE = (float)window.getSize().x / (float)colSize;
Clock simClock, frameClock;
float timeFrameStep = 60.0f;
Time updateInterval = seconds(1.0f / timeFrameStep);
Time frameUpdateInterval = seconds(1.0f);
Time timeEnd, frameTimeEnd;
int frameCount = 0;
bool grid[rowSize * colSize] = {false};
bool gridBuffer[rowSize * colSize] = {false};
int lastHovered = -1;
bool pause = true;
bool run = true;
VertexArray square(Quads, 4 * rowSize * colSize);
void addQuad(Vector2f pos, VertexArray *va, Color color, float size, int *index);
void clearBuffer();
void addQuad(Vector2f pos, VertexArray *va, float size, int *index);
bool isAlive(int row, int column, bool buffer);
void setCell(int row, int column, bool alive, bool bufferSet);
int aliveNeighbors(int row, int column);
void step();
void update();
void init();
void eventLoop();
void printGrid(bool buffer);
int loop_mod(int dividend, int divisor);

int main()
{
    srand(time(NULL));
    window.setVerticalSyncEnabled(true);
    shader.loadFromFile("./update.frag", Shader::Fragment);
    if (!shader.isAvailable())
    {
        cout << "Failed to load shader" << endl;
        return 0;
    }
    simClock.restart();
    frameClock.restart();
    bufferText.create(SCREEN_WIDTH, SCREEN_HEIGHT);
    bufferText.setRepeated(true);
    bufferSprite.setPosition(0, 0);
    init();
    while (run)
    {
        eventLoop();
        timeEnd = simClock.getElapsedTime();
        if (!pause && timeEnd >= updateInterval)
        {
            step();
            update();
            simClock.restart();
        }
        bufferText.draw(square);
        bufferText.display();
        bufferSprite.setTexture(bufferText.getTexture());

        shader.setUniform("texture", bufferText.getTexture());
        shader.setUniform("resolution", Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
        shader.setUniform("rowSize", rowSize);
        shader.setUniform("colSize", colSize);
        window.draw(bufferSprite, &shader);
        window.display();

        frameCount++;
        frameTimeEnd = frameClock.getElapsedTime();
        if (frameTimeEnd >= frameUpdateInterval)
        {
            // cout << frameCount / frameTimeEnd.asSeconds() << endl;
            frameCount = 0;
            frameTimeEnd = Time::Zero;
            frameClock.restart();
        }
    }
    return 0;
}

void addQuad(Vector2f pos, VertexArray *va, Color color, float size, int *index)
{
    (*va)[*index].position = pos;
    (*va)[*index + 1].position = Vector2f(pos.x + size, pos.y);
    (*va)[*index + 2].position = Vector2f(pos.x + size, pos.y + size);
    (*va)[*index + 3].position = Vector2f(pos.x, pos.y + size);

    (*va)[*index].color = color;
    (*va)[*index + 1].color = color;
    (*va)[*index + 2].color = color;
    (*va)[*index + 3].color = color;
    *index += 4;
}
void clearBuffer()
{
    for (int i = 0; i < rowSize; i++)
        for (int j = 0; j < colSize; j++)
            gridBuffer[i * colSize + j] = false;
}
void addQuad(Vector2f pos, VertexArray *va, float size, int *index)
{
    (*va)[*index].position = pos;
    (*va)[*index + 1].position = Vector2f(pos.x + size, pos.y);
    (*va)[*index + 2].position = Vector2f(pos.x + size, pos.y + size);
    (*va)[*index + 3].position = Vector2f(pos.x, pos.y + size);

    float offset = (float)(rand()) / RAND_MAX;
    Color color(255, 255, 255);
    (*va)[*index].color = color;
    (*va)[*index + 1].color = color;
    (*va)[*index + 2].color = color;
    (*va)[*index + 3].color = color;
    *index += 4;
}
bool isAlive(int row, int column, bool buffer)
{
    int adjustedRow = loop_mod(row, rowSize);
    int adjustedCol = loop_mod(column, colSize);
    if (!buffer)
        return grid[(adjustedRow * colSize + adjustedCol)];
    else
        return gridBuffer[(adjustedRow * colSize + adjustedCol)];
}
void setCell(int row, int column, bool alive, bool bufferSet)
{
    if ((column >= 0 && column < colSize) && (row >= 0 && row < rowSize))
    {
        int firstVertex = (row * rowSize + column) * 4;
        if (bufferSet)
        {
            gridBuffer[(row * colSize + column)] = alive;
        }
        else
        {
            grid[(row * colSize + column)] = alive;
            if (alive)
            {
                square[firstVertex].color = SQUARE_COL(row, column);
                square[firstVertex + 1].color = SQUARE_COL(row, column);
                square[firstVertex + 2].color = SQUARE_COL(row, column);
                square[firstVertex + 3].color = SQUARE_COL(row, column);
            }
            else
            {
                square[firstVertex].color = Color::Black;
                square[firstVertex + 1].color = Color::Black;
                square[firstVertex + 2].color = Color::Black;
                square[firstVertex + 3].color = Color::Black;
                // square[firstVertex].color = Color::White;
                // square[firstVertex + 1].color = Color::White;
                // square[firstVertex + 2].color = Color::White;
                // square[firstVertex + 3].color = Color::White;
            }
        }
    }
}
int aliveNeighbors(int row, int column)
{
    int aliveCount = 0;
    for (int r = row - 1; r <= row + 1; r++)
    {
        for (int c = column - 1; c <= column + 1; c++)
        {
            if (isAlive(r, c, false) && !(r == row && c == column))
                aliveCount++;
        }
    }
    return aliveCount;
}
void step()
{
    clearBuffer();
    for (int row = 0; row < rowSize; row++)
    {
        for (int column = 0; column < colSize; column++)
        {
            bool newVal = isAlive(row, column, false);
            int neighborCount = aliveNeighbors(row, column);
            if (newVal)
            {
                if (neighborCount < 2 || neighborCount > 3)
                {
                    newVal = false;
                }
            }
            else
            {
                if (neighborCount == 3)
                    newVal = true;
            }
            setCell(row, column, newVal, true);
        }
    }
}
void update()
{
    for (int row = 0; row < rowSize; row++)
    {
        for (int column = 0; column < colSize; column++)
        {
            setCell(row, column, gridBuffer[(row * colSize + column)], false);
        }
    }
}
void init()
{
    int k = 0;
    for (int i = 0; i < colSize; i++)
    {
        for (int j = 0; j < rowSize; j++)
        {
            Color c;
            if ((float)(rand()) / RAND_MAX > 0.5)
            {
                c = Color::Black;
                grid[i * rowSize + j] = false;
            }

            else
            {
                c = SQUARE_COL(i, j);
                grid[i * rowSize + j] = true;
            }
            addQuad(Vector2f((j * SQUARE_SIZE), (i * SQUARE_SIZE)), &square, c, SQUARE_SIZE, &k);
            // addQuad(Vector2f(0 + (i * SQUARE_SIZE), 0 + (j * SQUARE_SIZE)), &square, SQUARE_SIZE, &k);
        }
    }
}
void eventLoop()
{
    Event event;
    while (window.pollEvent(event))
    {
        switch (event.type)
        {
        case Event::Closed:
            window.close();
            run = false;
            break;
        case Event::MouseButtonPressed:
            if (window.hasFocus())
            {
                Vector2i mousepos = Mouse::getPosition(window);
                Vector2i index = Vector2i((int)(mousepos.x / SQUARE_SIZE), (int)(mousepos.y / SQUARE_SIZE));
                if (event.mouseButton.button == Mouse::Left)
                {
                    setCell(index.y, index.x, !isAlive(index.y, index.x, false), false);
                }
                else if (event.mouseButton.button == Mouse::Right)
                {
                    cout << aliveNeighbors(index.y, index.x) << endl;
                }
            }
            break;
        case Event::KeyPressed:
            if (event.key.code == Keyboard::Space)
            {
                step();
            }
            if (event.key.code == Keyboard::Enter)
            {
                update();
            }
            if (event.key.code == Keyboard::P)
                pause = !pause;
            if (event.key.code == Keyboard::Up)
            {
                if (timeFrameStep != 1)
                    timeFrameStep += 10;
                else
                    timeFrameStep = 10;
                updateInterval = seconds(1.0f / timeFrameStep);
            }
            if (event.key.code == Keyboard::Down)
            {
                if (timeFrameStep > 10)
                    timeFrameStep -= 10;
                else
                    timeFrameStep = 1;
                updateInterval = seconds(1.0f / timeFrameStep);
            }
            break;
        }
    }
}
void printGrid(bool buffer)
{
    for (int r = 0; r <= rowSize; r++)
    {
        for (int c = 0; c <= colSize; c++)
        {
            cout << isAlive(r, c, buffer) << ' ';
        }
        cout << endl;
    }
}
int loop_mod(int dividend, int divisor)
{
    int result = dividend % divisor;
    if (result < 0)
    {
        result += divisor;
    }
    else if (result >= divisor)
    {
        result -= divisor;
    }
    return result;
}
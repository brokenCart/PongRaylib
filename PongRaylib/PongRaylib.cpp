#include "raylib.h"
#include "raymath.h"
#include <algorithm>

float RandomFloat0To1() {
    return GetRandomValue(0, 1000000) / 1000000.0f;
}

// TODO: better random unit vector (restricted to certain region)
Vector2 RandomUnitVector2() {
    float angle = RandomFloat0To1() * 2.0f * PI;
    return { cosf(angle), sinf(angle) };
}

int main() {
    InitWindow(800, 450, "Pong!");
    SetTargetFPS(60);

    Vector2 playerPos = { 25, 200 };
    Vector2 botPos = { 755, 200 };
    Vector2 ballPos = { 400, 225 };

    const float paddleSpeed = 300.0f;
    const float ballSpeed = 200.0f;

    Vector2 ballVel = Vector2Scale(RandomUnitVector2(), ballSpeed);

    int nPlayerScore = 0;
    int nBotScore = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // INPUT
        if (IsKeyDown(KEY_DOWN)) playerPos.y = std::min(playerPos.y + paddleSpeed * dt, 350.0f);
        if (IsKeyDown(KEY_UP)) playerPos.y = std::max(playerPos.y - paddleSpeed * dt, 0.0f);

        // GAME LOGIC
        // Move the ball
        ballPos += Vector2Scale(ballVel, dt);

        // Make the bot follow the ball
        if (ballPos.x > 400) {
            float step = std::min(paddleSpeed * dt, std::abs(botPos.y - ballPos.y));
            if (ballPos.y > botPos.y) {
                botPos.y = std::min(botPos.y + step, 350.0f);
            }
            else if (ballPos.y < botPos.y) {
                botPos.y = std::max(botPos.y - step, 0.0f);
            }
        }

        // TODO: Collision of ball and paddle

        // Reflect the ball off the boundaries
        // Elastic collision -> angle of incidence = angle of reflection
        if (ballPos.y < 0.0f) { // Upper Boundary
            // Normal vector is downwards
            // https://math.stackexchange.com/questions/13261/how-to-get-a-reflection-vector
            Vector2 normalVec = Vector2(0, 1);
            Vector2 incidentUnitVec = Vector2Normalize(ballVel);
            Vector2 reflectedUnitVec = incidentUnitVec - Vector2Scale(normalVec, 2 * Vector2DotProduct(incidentUnitVec, normalVec));
            ballVel = Vector2Scale(reflectedUnitVec, ballSpeed);
        }
        else if (ballPos.y > 420.0f) { // Lower Boundary
            // Normal vector is upwards
            Vector2 normalVec = Vector2(0, -1);
            Vector2 incidentUnitVec = Vector2Normalize(ballVel);
            Vector2 reflectedUnitVec = incidentUnitVec - Vector2Scale(normalVec, 2 * Vector2DotProduct(incidentUnitVec, normalVec));
            ballVel = Vector2Scale(reflectedUnitVec, ballSpeed);
        }

        // Reset the ball position & velocity if beyong playing area
        // and increment the scores.
        if (ballPos.x < 0.0f) {
            ballPos = { 400, 225 };
            ballVel = Vector2Scale(RandomUnitVector2(), ballSpeed);
            nBotScore++;
        }
        else if (ballPos.x > 780.0f) {
            ballPos = { 400, 225 };
            ballVel = Vector2Scale(RandomUnitVector2(), ballSpeed);
            nPlayerScore++;
        }

        // RENDER
        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangleV(playerPos, { 20, 100 }, RAYWHITE);
        DrawRectangleV(botPos, { 20, 100 }, RAYWHITE);
        DrawRectangleV(ballPos, { 20, 20 }, MAROON);
        // TODO: display middle line
        // TODO: display scores
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

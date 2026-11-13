#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <string>

// Window
constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 450;
constexpr int TARGET_FPS = 60;

// Paddles
constexpr float PADDLE_WIDTH = 20.0f;
constexpr float PADDLE_HEIGHT = 100.0f;
constexpr float PADDLE_MARGIN = 25.0f; // Gap between a paddle and its side of the screen
constexpr float PADDLE_START_Y = 200.0f;
constexpr float PADDLE_MIN_Y = 0.0f;
constexpr float PADDLE_MAX_Y = SCREEN_HEIGHT - PADDLE_HEIGHT;
constexpr float PLAYER_X = PADDLE_MARGIN;
constexpr float BOT_X = SCREEN_WIDTH - PADDLE_MARGIN - PADDLE_WIDTH;
constexpr float PADDLE_SPEED = 300.0f;

// Ball
constexpr float BALL_SIZE = 20.0f;
constexpr float BALL_SPEED = 200.0f;
constexpr float BALL_START_X = SCREEN_WIDTH / 2.0f;
constexpr float BALL_START_Y = SCREEN_HEIGHT / 2.0f;
constexpr float BALL_MIN_Y = 0.0f;
constexpr float BALL_MAX_Y = SCREEN_HEIGHT - BALL_SIZE;
constexpr float BALL_MIN_X = 0.0f;
constexpr float BALL_MAX_X = SCREEN_WIDTH - BALL_SIZE;

// Bot only chases the ball once it crosses this x position
constexpr float BOT_TRACKING_X = SCREEN_WIDTH / 2.0f;

// Middle line
constexpr float MIDDLE_LINE_X = 397.0f;
constexpr float MIDDLE_LINE_DASH_WIDTH = 1.0f;
constexpr float MIDDLE_LINE_DASH_HEIGHT = 25.0f;
constexpr float MIDDLE_LINE_DASH_SPACING = 50.0f;

// Score text
constexpr int SCORE_FONT_SIZE = 40;
constexpr int SCORE_Y = 40;
constexpr int PLAYER_SCORE_X = SCREEN_WIDTH / 4;
constexpr int BOT_SCORE_X = SCREEN_WIDTH * 3 / 4;

// Random
constexpr int RANDOM_RESOLUTION = 1000000;

float RandomFloat0To1() {
    return GetRandomValue(0, RANDOM_RESOLUTION) / static_cast<float>(RANDOM_RESOLUTION);
}

// TODO: better random unit vector (restricted to certain region)
Vector2 RandomUnitVector2() {
    float angle = RandomFloat0To1() * 2.0f * PI;
    return { cosf(angle), sinf(angle) };
}

bool PaddleBallCollision(Vector2& paddle, Vector2& ball) {
    // https://www.jeffreythompson.org/collision-detection/rect-rect.php
    return (
        paddle.x + PADDLE_WIDTH >= ball.x &&
        paddle.x <= ball.x + BALL_SIZE &&
        paddle.y + PADDLE_HEIGHT >= ball.y &&
        paddle.y <= ball.y + BALL_SIZE
    );
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong!");
    SetTargetFPS(TARGET_FPS);

    const Vector2 paddleSize = { PADDLE_WIDTH, PADDLE_HEIGHT };
    const Vector2 ballSize = { BALL_SIZE, BALL_SIZE };
    const Vector2 ballStartPos = { BALL_START_X, BALL_START_Y };

    Vector2 playerPos = { PLAYER_X, PADDLE_START_Y };
    Vector2 botPos = { BOT_X, PADDLE_START_Y };
    Vector2 ballPos = ballStartPos;

    Vector2 ballVel = Vector2Scale(RandomUnitVector2(), BALL_SPEED);

    int nPlayerScore = 0;
    int nBotScore = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // INPUT
        if (IsKeyDown(KEY_DOWN)) playerPos.y = std::min(playerPos.y + PADDLE_SPEED * dt, PADDLE_MAX_Y);
        if (IsKeyDown(KEY_UP)) playerPos.y = std::max(playerPos.y - PADDLE_SPEED * dt, PADDLE_MIN_Y);

        // GAME LOGIC
        // Move the ball
        ballPos += Vector2Scale(ballVel, dt);

        // Make the bot follow the ball
        if (ballPos.x > BOT_TRACKING_X) {
            float step = std::min(PADDLE_SPEED * dt, std::abs(botPos.y - ballPos.y));
            if (ballPos.y > botPos.y) {
                botPos.y = std::min(botPos.y + step, PADDLE_MAX_Y);
            }
            else if (ballPos.y < botPos.y) {
                botPos.y = std::max(botPos.y - step, PADDLE_MIN_Y);
            }
        }

        // Collision of ball and paddle
        if (ballPos.x < SCREEN_WIDTH / 2.0f && PaddleBallCollision(playerPos, ballPos)) {
            Vector2 normalVec = Vector2(1, 0);
            Vector2 incidentUnitVec = Vector2Normalize(ballVel);
            Vector2 reflectedUnitVec = incidentUnitVec - Vector2Scale(normalVec, 2 * Vector2DotProduct(incidentUnitVec, normalVec));
            ballVel = Vector2Scale(reflectedUnitVec, BALL_SPEED);
        }

        if (ballPos.x > SCREEN_WIDTH / 2.0f && PaddleBallCollision(botPos, ballPos)) {
            Vector2 normalVec = Vector2(-1, 0);
            Vector2 incidentUnitVec = Vector2Normalize(ballVel);
            Vector2 reflectedUnitVec = incidentUnitVec - Vector2Scale(normalVec, 2 * Vector2DotProduct(incidentUnitVec, normalVec));
            ballVel = Vector2Scale(reflectedUnitVec, BALL_SPEED);
        }

        // Reflect the ball off the boundaries
        // Elastic collision -> angle of incidence = angle of reflection
        if (ballPos.y < BALL_MIN_Y) { // Upper Boundary
            // Normal vector is downwards
            // https://math.stackexchange.com/questions/13261/how-to-get-a-reflection-vector
            Vector2 normalVec = Vector2(0, 1);
            Vector2 incidentUnitVec = Vector2Normalize(ballVel);
            Vector2 reflectedUnitVec = incidentUnitVec - Vector2Scale(normalVec, 2 * Vector2DotProduct(incidentUnitVec, normalVec));
            ballVel = Vector2Scale(reflectedUnitVec, BALL_SPEED);
        }
        else if (ballPos.y > BALL_MAX_Y) { // Lower Boundary
            // Normal vector is upwards
            Vector2 normalVec = Vector2(0, -1);
            Vector2 incidentUnitVec = Vector2Normalize(ballVel);
            Vector2 reflectedUnitVec = incidentUnitVec - Vector2Scale(normalVec, 2 * Vector2DotProduct(incidentUnitVec, normalVec));
            ballVel = Vector2Scale(reflectedUnitVec, BALL_SPEED);
        }

        // Reset the ball position & velocity if beyong playing area
        // and increment the scores.
        if (ballPos.x < BALL_MIN_X) {
            ballPos = ballStartPos;
            ballVel = Vector2Scale(RandomUnitVector2(), BALL_SPEED);
            nBotScore++;
        }
        else if (ballPos.x > BALL_MAX_X) {
            ballPos = ballStartPos;
            ballVel = Vector2Scale(RandomUnitVector2(), BALL_SPEED);
            nPlayerScore++;
        }

        // RENDER
        BeginDrawing();
        ClearBackground(BLACK);

        // Display middle line
        for (float y = 0; y <= SCREEN_HEIGHT; y += MIDDLE_LINE_DASH_SPACING) {
            DrawRectangleV(Vector2{ MIDDLE_LINE_X, y }, { MIDDLE_LINE_DASH_WIDTH, MIDDLE_LINE_DASH_HEIGHT }, RAYWHITE);
        }

        // Display scores
        DrawText(TextFormat("%i", nPlayerScore), PLAYER_SCORE_X, SCORE_Y, SCORE_FONT_SIZE, RAYWHITE);
        DrawText(TextFormat("%i", nBotScore), BOT_SCORE_X, SCORE_Y, SCORE_FONT_SIZE, RAYWHITE);

        DrawRectangleV(playerPos, paddleSize, RAYWHITE);
        DrawRectangleV(botPos, paddleSize, RAYWHITE);
        DrawRectangleV(ballPos, ballSize, MAROON);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

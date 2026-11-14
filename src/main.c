/*
 * Copyright (c) 2018, 2019 Amine Ben Hassouna <amine.benhassouna@gmail.com>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without
 * limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
 * ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_audio.h>

// Define MAX and MIN macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define RANDINT(min, max) (rand() % (max - min + 1) + min)

// Define screen dimensions
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600
#define FONT "/usr/share/fonts/truetype/freefont/FreeSans.ttf"

// Source - https://stackoverflow.com/a/44105089
// Posted by baz, modified by community. See post 'Timeline' for change history
// Retrieved 2026-11-14, License - CC BY-SA 3.0

float float_rand( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

///////////////////////

struct paddle {
    float x;
    float y;
    float w;
    float h;
    float dy;
};


struct paddle paddle1 = {
    40,
    SCREEN_HEIGHT / 2 - (50 / 2),
    10,
    50,
    0
};

struct paddle paddle2 = {
    SCREEN_WIDTH - 40,
    SCREEN_HEIGHT / 2 - (50 / 2),
    10,
    50,
    0
};

uint64_t samples_played = 0;
float frequency = 493.88;


int paused = 1;

int p1_pts = 0;
int p2_pts = 0;

//  plrX_btns = [up, down];
int plr1_btns[2] = {0, 0};
int plr2_btns[2] = {0, 0};

float ball_dx = 0;
float ball_dy = 0;
float ball_x;
float ball_y; 

SDL_Rect paddleLeft;
SDL_Rect paddleRight;
SDL_Rect divider;
SDL_Rect ball;
SDL_Rect rect;
SDL_Rect overlay_rect;
TTF_Font *font;
SDL_Surface* textSurface;
SDL_AudioDeviceID audio_device_id;

SDL_Color text_white = {255, 255, 255, 255};

void setup() {
    // Position rects for paddles.
    paddleLeft.w = paddle1.w;
    paddleLeft.h = paddle1.h;
    paddleLeft.x = paddle1.x;
    paddleLeft.y = paddle1.y;

    paddleRight.w = paddle2.w;
    paddleRight.h = paddle2.h;
    paddleRight.x = paddle2.x;
    paddleRight.y = paddle2.y;

    // Position square for ball

    ball.w = 10;
    ball.h = 10;
    ball.x = SCREEN_WIDTH / 2 - (ball.w / 2);
    ball.y = SCREEN_HEIGHT / 2 - (ball.h / 2);
    ball_x = ball.x;
    ball_y = ball.y;

    // Misc

    // divider.w = 4;
    // divider.h = SCREEN_HEIGHT;
    // divider.x = SCREEN_WIDTH / 2 - (divider.w / 2);
    // divider.y = 0;

}

void draw(SDL_Renderer *renderer) {
    // Initialize renderer color black for the background
    // SDL_SetRenderDrawColor(renderer, 43, 75, 111, 0xFF);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0X00, 0xFF);

    // Clear screen
    SDL_RenderClear(renderer);

    // SDL_SetRenderDrawColor(renderer, 75, 105, 139, 0xFF);
    

    // Set renderer color to white
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    // Draw filled square

    for (int i = 0; i < SCREEN_HEIGHT / 20; i++) {
        if (i % 2 == 0) {
            continue;
        }
        rect.w = 4;
        rect.h = 10;
        rect.x = SCREEN_WIDTH / 2 - (rect.w / 2);
        rect.y = i * 20;
        SDL_RenderFillRect(renderer, &rect);
    }
    // SDL_RenderFillRect(renderer, &divider);
    SDL_RenderFillRect(renderer, &paddleLeft);
    SDL_RenderFillRect(renderer, &paddleRight);
    SDL_RenderFillRect(renderer, &ball);

    char p1_pts_c[20];
    char p2_pts_c[20];
    snprintf(p1_pts_c, sizeof(p1_pts_c), "%d", p1_pts);
    snprintf(p2_pts_c, sizeof(p1_pts_c), "%d", p2_pts);

    textSurface = TTF_RenderText_Solid(font, p1_pts_c, text_white);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = (SCREEN_WIDTH / 4) - (textRect.w / 2);
    textRect.y = 5 + (textRect.h / 2);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textSurface = TTF_RenderText_Solid(font, p2_pts_c, text_white);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = (SCREEN_WIDTH / 2) + (SCREEN_WIDTH / 4) - (textRect.w / 2);
    textRect.y = 5 + (textRect.h / 2);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    
    if (paused) {
        overlay_rect.w = SCREEN_WIDTH;
        overlay_rect.h = SCREEN_HEIGHT;
        overlay_rect.x = 0;
        overlay_rect.y = 0;

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 150);
        SDL_RenderFillRect(renderer, &rect);

        textSurface = TTF_RenderText_Solid(font,"Paused", text_white);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textRect.w = textSurface->w;
        textRect.h = textSurface->h;
        textRect.x = (SCREEN_WIDTH / 2) - (textRect.w / 2);
        textRect.y = (SCREEN_HEIGHT / 2) - (textRect.h);
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        textSurface = TTF_RenderText_Solid(font,"Space to start!", text_white);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textRect.w = textSurface->w;
        textRect.h = textSurface->h;
        textRect.x = (SCREEN_WIDTH / 2) - (textRect.w / 2);
        textRect.y = (SCREEN_HEIGHT / 2);
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    // Update screen
    SDL_RenderPresent(renderer);
   
}


int ball_out = 0;
int ball_out_time = 0;
void reset_ball(int side) {
    // printf("BOT: %d, BO: %d", ball_out_time, ball_out);
    if (ball_out) {
        if (SDL_GetTicks() - ball_out_time >= 1000) {
            ball_out_time = 1; // now
            ball_out = 0;

            ball_dx = 0;
            ball_dy = 0;

            ball_x = SCREEN_WIDTH / 2 - 5;
            ball_y = RANDINT(100, SCREEN_HEIGHT - 100);

            switch (side)
            {
            case 0:
                ball_dx = -0.8;
                break;
            
            case 1:
                ball_dx = 0.8;
                break;
            }
            
            switch (RANDINT(0, 1))
            {
            case 0:
                ball_dy = float_rand(-0.3, -0.6);
                break;
            
            case 1:
                ball_dy = float_rand(0.3, 0.6);
                break;
            }
            
        }
    }
}

int last_audio = 0;
void beep() {
    last_audio = SDL_GetTicks();
    SDL_PauseAudioDevice(audio_device_id, 0);
}

unsigned int audio_dur = 32;
float paddleSpeed = 0.9;
void update_pos() {
    if (SDL_GetTicks() - last_audio > audio_dur) {
        SDL_PauseAudioDevice(audio_device_id, 1);
    }

    paddle1.dy = 0;
    paddle2.dy = 0;

    if (paddle1.y > SCREEN_HEIGHT - paddle1.h) {
        paddle1.y = SCREEN_HEIGHT - paddle1.h;
    }
    if (paddle1.y < 0) {
        paddle1.y = 0;
    }

    if (paddle2.y > SCREEN_HEIGHT - paddle2.h) {
        paddle2.y = SCREEN_HEIGHT - paddle2.h;
    }
    if (paddle2.y < 0) {
        paddle2.y = 0;
    }

    if (plr1_btns[0] == 1) {
        paddle1.dy = -paddleSpeed;
    }
    if (plr1_btns[1] == 1) {
        paddle1.dy = paddleSpeed;
    }
    if (plr2_btns[0] == 1) {
        paddle2.dy = -paddleSpeed;
    }
    if (plr2_btns[1] == 1) {
        paddle2.dy = paddleSpeed;
    }

    if (ball_y < 0 || ball_y > SCREEN_HEIGHT - 10) {
        ball_dy = -ball_dy * 1.05;
        audio_dur = 16;
        frequency = 246.94;
        beep();
    } 

    if (ball_x > paddle1.x && 
        ball_x < paddle1.x + paddle1.w && 
        ball_y + ball.h > paddle1.y && 
        ball_y < paddle1.y + paddle1.h) 
    {
        ball_x = paddle1.x + paddle1.w + 1;
        /*if (ball_dy > paddle1.y + 5 + (paddle1.h / 2)) {
            ball_dy = ball_dy * 1.05 + 0.5;
        } else if (ball_dy < paddle1.y - 5 + (paddle1.h / 2)) {
            ball_dy = ball_dy * 1.05 - 0.5;
        } else */ 
        ball_dy = ball_dy * 1.05;
        if (ball_dy < paddle1.y + 5 + (paddle1.h / 2) && ball_dy > paddle1.y - 5 + (paddle1.h / 2)) {
            ball_dy = float_rand(-0.1, 0.1);
        }
        audio_dur = 32;
        frequency = 493.88;
        beep();
        ball_dx = -ball_dx;
    }

    if (ball_x > paddle2.x && 
        ball_x < paddle2.x + paddle2.w && 
        ball_y + ball.h > paddle2.y && 
        ball_y < paddle2.y + paddle2.h) 
    {   
        ball_x = paddle2.x - ball.w - 1;
        /* if (ball_dy > paddle2.y + 5 + (paddle2.h / 2)) {
            ball_dy = ball_dy * 1.05 + 0.5;
        } else if (ball_dy < paddle2.y - 5 + (paddle2.h / 2)) {
            ball_dy = ball_dy * 1.05 - 0.5;
        } else */ 
        ball_dy = ball_dy * 1.05;
        if (ball_dy < paddle2.y + 5 + (paddle2.h / 2) && ball_dy > paddle2.y - 5 + (paddle2.h / 2)) {
            ball_dy = float_rand(-0.1, 0.1);
        }
        ball_x = paddle2.x;
        ball_dy = ball_dy + (paddle2.dy * 0.5);
        audio_dur = 32;
        frequency = 493.88;
        beep();
        ball_dx = -ball_dx;
    }

    if (ball_x > SCREEN_WIDTH) {
        if (!ball_out) {
            p1_pts++;
            ball_out_time = SDL_GetTicks();
            ball_out = 1;
            audio_dur = 250;
            frequency = 246.94;
            beep();
        }
        reset_ball(1);
    }
    if (ball_x < 0) {
        if (!ball_out) {
            p2_pts++;
            ball_out_time = SDL_GetTicks();
            ball_out = 1;
            audio_dur = 250;
            frequency = 246.94;
            beep();
        }
        reset_ball(0);
    }

    ball_x += ball_dx;
    ball_y += ball_dy;


    ball.y = ball_y;
    ball.x = ball_x;

    paddle1.y += paddle1.dy;
    paddle2.y += paddle2.dy;

    paddleLeft.y = paddle1.y;
    paddleRight.y = paddle2.y;
}


void audio_callback(void* userdata, uint8_t* stream, int len)
{
    uint64_t* samples_played = (uint64_t*)userdata;
    float* fstream = (float*)(stream);

    static const float volume = 0.2;

    for(int sid = 0; sid < (len / 8); ++sid)
    {
        double time = (*samples_played + sid) / 44100.0;
        double x = 2.0 * M_PI * time * frequency;
        double sample = (sin(x) >= 0.0) ? volume : -volume;
        fstream[2 * sid + 0] = sample; /* L */
        fstream[2 * sid + 1] = sample; /* R */
    }

    *samples_played += (len / 8);
}

int main(int argc, char* argv[])
{
    // Unused argc, argv
    (void) argc;
    (void) argv;

    srand(time(NULL));

    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not be initialized!\n"
               "SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    if (TTF_Init() < 0)
    {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    font = TTF_OpenFont(FONT, 32);

    if(SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr,
            "Error initializing SDL. SDL_Error: %s\n",
            SDL_GetError()
        );
        return -1;
    }

    SDL_AudioSpec audio_spec_want, audio_spec;
    SDL_memset(&audio_spec_want, 0, sizeof(audio_spec_want));

    audio_spec_want.freq     = 44100;
    audio_spec_want.format   = AUDIO_F32;
    audio_spec_want.channels = 2;
    audio_spec_want.samples  = 512;
    audio_spec_want.callback = audio_callback;
    audio_spec_want.userdata = (void*)&samples_played;

    audio_device_id = SDL_OpenAudioDevice(
        NULL, 0,
        &audio_spec_want, &audio_spec,
        SDL_AUDIO_ALLOW_FORMAT_CHANGE
    );



#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
    // Disable compositor bypass
    if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
    {
        printf("SDL can not disable compositor bypass!\n");
        return 0;
    }
#endif

    // Create window
    SDL_Window *window = SDL_CreateWindow(
        "Pong",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if(!window)
    {
        printf("Window could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        // Create renderer
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); //  | SDL_RENDERER_PRESENTVSYNC
        if(!renderer)
        {
            printf("Renderer could not be created!\n"
                   "SDL_Error: %s\n", SDL_GetError());
        }
        else
        {

            setup();
            // Event loop exit flag
            int quit = 0;

            ball_dy = float_rand(-0.4, 0.4);
            switch (RANDINT(0, 1))
            {
            case 0:
                ball_dx = 1;
                break;
            
            case 1:
                ball_dx = -1;
                break;
            }

            // SDL_PauseAudioDevice(audio_device_id, 0);
            // Event loop
            while(!quit)
            {
                SDL_Event event;

                // Wait indefinitely for the next available event
                while (SDL_PollEvent(&event))
                {
                    int code = event.key.keysym.scancode;
                    switch (event.type)
                    {
                    case SDL_KEYDOWN:
                        // printf("Key press detected\n");
                        if (code == 41) {
                            quit = 1;
                        }
                        if (code == 44) {
                            paused = !paused;
                        }
                        
                        // printf("\n%d\n", code);

                        if (code == 4) { // A
                            plr1_btns[0] = 1;
                        }
                        if (code == 7) { // D
                            plr1_btns[1] = 1;
                        }

                        if (code == 15) { // L
                            plr2_btns[0] = 1;
                        }
                        if (code == 13) { // J
                            plr2_btns[1] = 1;
                        }

                        break;

                    case SDL_KEYUP:
                        // printf("Key release detected\n");
                        // int code1 = event.key.keysym.scancode;

                        if (code == 4) { // A
                            plr1_btns[0] = 0;
                        }
                        if (code == 7) { // D
                            plr1_btns[1] = 0;
                        }

                        if (code == 15) { // L
                            plr2_btns[0] = 0;
                        }
                        if (code == 13) { // J
                            plr2_btns[1] = 0;
                        }

                        break;

                    case SDL_QUIT:
                        quit = 1;
                        break;
                    default:
                        break;
                    }
                }
                if (!paused) { 
                    update_pos();
                }
                draw(renderer);
                
                SDL_Delay(1);
            }

            // Destroy renderer
            SDL_DestroyRenderer(renderer);
        }

        // Destroy window
        SDL_DestroyWindow(window);
    }

    // Quit SDL
    SDL_Quit();

    return 0;
}

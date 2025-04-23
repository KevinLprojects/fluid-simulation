#include "render.h"
#include "constants.h"
#include "mutexes.h"
#include "structures.h"

#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

CircleData circles[NUM_PARTICLES];

pthread_t update;

void* physicsUpdate(void* arg) {
    while (1) {
        pthread_mutex_lock(&renderDataMutex);
        for (int i = 0; i < NUM_PARTICLES; i++) {
            circles[i].x += 0.01 * (rand() / (float)RAND_MAX - 0.5);
            circles[i].y += 0.01 * (rand() / (float)RAND_MAX - 0.5);
            circles[i].r += 0.1 * (rand() / (float)RAND_MAX - 0.5);
            circles[i].g += 0.1 * (rand() / (float)RAND_MAX - 0.5);
            circles[i].b += 0.1 * (rand() / (float)RAND_MAX - 0.5);

            if (circles[i].x < 0) {
                circles[i].x = 0;
            } 
            else if (circles[i].x > 1) {
                circles[i].x = 1;
            }

            if (circles[i].y < 0) {
                circles[i].y = 0;
            } 
            else if (circles[i].y > 1) {
                circles[i].y = 1;
            }

            if (circles[i].r < 0) {
                circles[i].r = 0;
            } 
            else if (circles[i].r > 1) {
                circles[i].r = 1;
            }

            if (circles[i].g < 0) {
                circles[i].g = 0;
            } 
            else if (circles[i].g > 1) {
                circles[i].g = 1;
            }

            if (circles[i].b < 0) {
                circles[i].b = 0;
            } 
            else if (circles[i].b > 1) {
                circles[i].b = 1;
            }
        }
        pthread_mutex_unlock(&renderDataMutex);
        usleep(16000); // ~60 fps
    }
    return NULL;
}

int main() {
    srand((unsigned int)time(NULL));

    for (int i = 0; i < NUM_PARTICLES; i++) {
            circles[i].x = rand() / (float)RAND_MAX;
            circles[i].y = rand() / (float)RAND_MAX;
            circles[i].r = rand() / (float)RAND_MAX;
            circles[i].g = rand() / (float)RAND_MAX;
            circles[i].b = rand() / (float)RAND_MAX;
    }
    
    pthread_create(&update, NULL, physicsUpdate, NULL);
    loop();
}
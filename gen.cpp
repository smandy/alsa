#include "SDL2/SDL_audio.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

double angle;
uint64_t counter;

// Do all the init stuff
void init_sdl(void) {
  if (SDL_Init(SDL_INIT_AUDIO) < 0)
    exit(-1);
  atexit(SDL_Quit);
}
double Hz = 6000;

// Creates the sine wave. To supress ripple, the wave runs continuously by using
// an incemental counter
void Callback(void *userdata, uint8_t *stream, int len) {
  double pi = 3.1415;
  // uint8_t *waveptr;
  // double Hz=220;
  Hz *= 0.96;
  if (Hz < 400)
    Hz = 6000;
  // double L = 1024;
  double A = 65000;
  double SR = 44100;
  double F = 2 * pi * Hz / SR;
  int16_t *s2 = (int16_t *)stream;
  for (int z = 0; z < (len / 2); z++) {
    counter++;
    angle += F;
    s2[z] = (uint8_t)A * sin(angle);
    if (counter % 16284 == 0) {
      std::cout << counter << " hz=" << Hz << " len=" << len << std::endl;
    };
  }
}

void play(void) {
  // sound_buffer = new uint8_t[1024];
  // sound_len= 1024;
  SDL_AudioSpec spec;
  spec.freq = 44100;
  spec.format = AUDIO_S16SYS;
  spec.channels = 1;
  spec.silence = 0;
  spec.samples = 1024;
  spec.padding = 0;
  spec.size = 0;
  spec.userdata = 0;
  spec.callback = Callback;
  if (SDL_OpenAudio(&spec, NULL) < 0) {
    printf("Kann audio nicht ?ffnen: %s\n", SDL_GetError());
    exit(-1);
  }
  SDL_PauseAudio(0);
}

int main(int argc, char *argv[]) {
  std::cout << "STartup" << std::endl;
  init_sdl();
  play();
  std::cout << "delay" << std::endl;
  SDL_Delay(20000);
  std::cout << "return" << std::endl;
  return 0;
}

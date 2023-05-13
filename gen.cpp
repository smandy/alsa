#include "SDL2/SDL_audio.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <math.h>
#include <stdlib.h>

double angle;
uint64_t counter;
double Hz = 6000;

// Do all the init stuff
void init_sdl(void) {
  if (SDL_Init(SDL_INIT_AUDIO) < 0)
    exit(-1);
  atexit(SDL_Quit);
}

const float m1 = 0.90;
float mult = m1;

const double NOISE_AMPL = 5000;
const int SR = 44100;

void callback(void *userdata, uint8_t *stream, int len) {
  double pi = 3.1415;
  Hz *= mult;
  if (Hz < 100)
    mult = 1 / m1;
  else if (Hz > 6000)
    mult = m1;

  double A = INT16_MAX - NOISE_AMPL;
  //double SR = 44100;
  double F = 2 * pi * Hz / SR;
  auto s2 = reinterpret_cast<int16_t *>(stream);
  for (int z = 0; z < (len / 2); z++) {
    counter++;
    angle += F;
    s2[z] = (int16_t)(A * sin(angle)) + NOISE_AMPL * static_cast<float>(rand()) / RAND_MAX;
    if ((counter & ((1 << 14) - 1)) == 0) {
      std::cout << counter << " hz=" << Hz << " len=" << len << std::endl;
    }
  }
}

void play(void) {
  SDL_AudioSpec spec;
  spec.freq = SR;
  spec.format = AUDIO_S16SYS;
  spec.channels = 1;
  spec.silence = 0;
  spec.samples = 1024;
  spec.padding = 0;
  spec.size = 0;
  spec.userdata = 0;
  spec.callback = callback;
  if (SDL_OpenAudio(&spec, NULL) < 0) {
    printf("Kann audio nicht ?ffnen: %s\n", SDL_GetError());
    exit(-1);
  }
  SDL_PauseAudio(0);
}

int main(int argc, char *argv[]) {
  std::cout << "Startup" << std::endl;
  init_sdl();
  play();
  std::cout << "Delay" << std::endl;
  SDL_Delay(20000);
  std::cout << "Return" << std::endl;
  return 0;
}

env = Environment(CPPFLAGS = ['-g','-std=c++23'],
                  LIBS = ['asound', 'm',
                          'SDL2main',
                          'SDL2',
                          'SDL2_mixer'])

env.Program('alsa.c')
#env.Program('main.cpp')
#env.Program('amidi.cpp')
env.Program('my_midi.cpp')
env.Program('rawmidi.c')
env.Program('tut.cpp')
env.Program('alsaread.c')
env.Program('nonblocking.c')
env.Program('gen.cpp')

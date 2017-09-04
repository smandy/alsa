
#include "alloca.h"
#include <alsa/asoundlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

static void error(const char *format, ...) {
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  putc('\n', stderr);
}

static void list_device(snd_ctl_t *ctl, int card, int device) {
  snd_rawmidi_info_t *info;
  const char *name;
  const char *sub_name;
  int subs, subs_in, subs_out;
  int sub;
  int err;

  snd_rawmidi_info_alloca(&info);
  snd_rawmidi_info_set_device(info, device);

  snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
  err = snd_ctl_rawmidi_info(ctl, info);
  if (err >= 0)
    subs_in = snd_rawmidi_info_get_subdevices_count(info);
  else
    subs_in = 0;

  snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
  err = snd_ctl_rawmidi_info(ctl, info);
  if (err >= 0)
    subs_out = snd_rawmidi_info_get_subdevices_count(info);
  else
    subs_out = 0;

  subs = subs_in > subs_out ? subs_in : subs_out;
  if (!subs)
    return;

  for (sub = 0; sub < subs; ++sub) {
    snd_rawmidi_info_set_stream(info,
                                sub < subs_in ? SND_RAWMIDI_STREAM_INPUT
                                              : SND_RAWMIDI_STREAM_OUTPUT);
    snd_rawmidi_info_set_subdevice(info, sub);
    err = snd_ctl_rawmidi_info(ctl, info);
    if (err < 0) {
      error("cannot get rawmidi information %d:%d:%d: %s\n", card, device, sub,
            snd_strerror(err));
      return;
    }
    name = snd_rawmidi_info_get_name(info);
    sub_name = snd_rawmidi_info_get_subdevice_name(info);
    if (sub == 0 && sub_name[0] == '\0') {
      printf("%c%c  hw:%d,%d    %s", sub < subs_in ? 'I' : ' ',
             sub < subs_out ? 'O' : ' ', card, device, name);
      if (subs > 1)
        printf(" (%d subdevices)", subs);
      putchar('\n');
      break;
    } else {
      printf("%c%c  hw:%d,%d,%d  %s\n", sub < subs_in ? 'I' : ' ',
             sub < subs_out ? 'O' : ' ', card, device, sub, sub_name);
    }
  }
}

struct my_chan {
  std::string port;
  snd_rawmidi_t *input;
  snd_rawmidi_t **inputp;

public:
  my_chan(const std::string s) : port(s) {
    std::cout << "Woot " << s << std::endl;
    inputp = &input;
    std::cout << "ptr=" << input << " *ptr=" << inputp << std::endl;
  }
};

int main(int argc, char *argv[]) {
  // device_list();
  // list_card_devices(1);
  snd_ctl_t *ctl;
  int device = -1;
  int card = -1;
  int err;
  if ((err = snd_card_next(&card)) < 0) {
    error("cannot determine card number: %s", snd_strerror(err));
    return 0;
  }
  std::cout << "Card is " << std::to_string(card) << std::endl;
  auto x = snd_ctl_open(&ctl, "hw:1", 0);
  std::cout << " x is " << x << std::endl;
  auto x2 = snd_ctl_rawmidi_next_device(ctl, &device);
  std::cout << "x2 is " << std::to_string(x2) << std::endl;
  std::cout << "device is " << device << std::endl;
  //list_device(ctl, x, device);

  // snd_rawmidi_t *nput, **inputp;
  // snd_rawmidi_t *output, **outputp;

  std::vector<my_chan> ports = { {"hw:1,0,0"} }; //, {"hw:1,0,1"}, {"hw:1,0,2"} };
  //  std::cout << "Input is " << input << std::endl;

  for ( auto& chan : ports) {
    std::cout << "port is " << chan.port << std::endl;
    std::cout << "input is " << chan.input << " ptr=" << chan.inputp << std::endl;
    if ((err = snd_rawmidi_open(chan.inputp, nullptr, chan.port.c_str(), SND_RAWMIDI_NONBLOCK)) <
        0) {
      error("cannot open port \"%s\": %s", chan.port.c_str(), snd_strerror(err));
    }
    std::cout << "input is " << chan.input << " ptr=" << chan.inputp << std::endl;
    snd_rawmidi_read(chan.input, NULL, 0); /* trigger reading */
  }
  //std::cout << "Input is " << input << std::endl;


  int read = 0;
  int npfds;
  struct pollfd *pfds;

  npfds = 1 + snd_rawmidi_poll_descriptors_count(ports[0].input);
  pfds = (pollfd *)alloca(npfds * sizeof(struct pollfd));
  pfds[0].fd = -1;

  std::cout << "npfds is " << std::endl;

  snd_rawmidi_poll_descriptors(ports[0].input, &pfds[1], npfds - 1);

  // signal(SIGINT, sig_handler);

  std::cout << "Main loop" << std::endl;

  for (;;) {
    // std::cout << "Poll" << std::endl;
    unsigned char buf[256];
    int i, length;
    unsigned short revents;

    err = poll(pfds, npfds, -1);
    if (err < 0) {
      error("poll failed: %s", strerror(errno));
      break;
    }

    err = snd_rawmidi_poll_descriptors_revents(ports[0].input, &pfds[1], npfds - 1,
                                               &revents);
    if (err < 0) {
      error("cannot get poll events: %s", snd_strerror(errno));
      break;
    }
    if (revents & (POLLERR | POLLHUP))
      break;
    if (!(revents & POLLIN)) {
      if (pfds[0].revents & POLLIN)
        continue;
    }

    err = snd_rawmidi_read(ports[0].input, buf, sizeof(buf));
    if (err == -EAGAIN)
      continue;
    if (err < 0) {
      error("cannot read from port \"%s\": %s", "foo",
            snd_strerror(err));
      break;
    }
    length = 0;
    for (i = 0; i < err; ++i)
      // if ((buf[i] != MIDI_CMD_COMMON_CLOCK &&
      //      buf[i] != MIDI_CMD_COMMON_SENSING) ||
      //     (buf[i] == MIDI_CMD_COMMON_CLOCK && !ignore_clock) ||
      //     (buf[i] == MIDI_CMD_COMMON_SENSING && !ignore_active_sensing))
      buf[length++] = buf[i];
    if (length == 0)
      continue;
    read += length;

    for (i = 0; i < length; ++i) {
      std::cout << std::hex << buf[i] << " ";
      // print_byte(buf[i]);
    };
    std::cout << std::endl;
  }
};

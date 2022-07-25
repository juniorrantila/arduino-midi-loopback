#include <jack/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <jack/jack.h>
#include <jack/midiport.h>

using u8  = unsigned char;
using u16 = unsigned short;
using i8  = char;
using i16 = short;
using f32 = float;

int process(jack_nframes_t, void*)
{
    return 0;
}


int main(int argc, char const* argv[])
{
    auto arduino_name = "/dev/ttyACM0";
    if (argc == 2)
        arduino_name = argv[1];

    int fd = open(arduino_name, O_RDONLY);
    auto client = jack_client_open("arduino-midi-loopback", JackNullOption, nullptr);
    jack_set_process_callback(client, process, nullptr);
    auto midi_port = jack_port_register(client, "midi-out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    jack_activate(client);

    u8 midi_data[4];
    while (1) {
        if (read(fd, midi_data, sizeof(midi_data)) == sizeof(midi_data)) {
            // If any byte skipped
            if (midi_data[3] != 0) {
                char next_zero = 1;
                while(next_zero) // Find end of packet
                    read(fd, &next_zero, 1);
                continue;
            }

            printf("midi packet: %.2x %.2x %.2x\n",
                    midi_data[0], midi_data[1], midi_data[2]);

            auto port_buffer = jack_port_get_buffer(midi_port, 0);
            jack_midi_clear_buffer(port_buffer);
            jack_midi_event_write(port_buffer, 1, midi_data, sizeof(midi_data));
        }
    }
}

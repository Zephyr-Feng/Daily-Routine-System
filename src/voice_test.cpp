#include <iostream>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>

using namespace std;

termios orig_termios;
pid_t rec_pid = 0;

void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    cout << endl;
}

void set_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(reset_terminal);
    termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void start_recording() {
    rec_pid = fork();
    if (rec_pid == 0) {
        execlp("arecord", "arecord", "-f", "S16_LE", "-r", "16000",
               "-c", "1", "/tmp/voice_input.wav", nullptr);
        exit(1);
    }
}

void stop_recording() {
    if (rec_pid > 0) {
        kill(rec_pid, SIGINT);
        int status;
        waitpid(rec_pid, &status, 0);
        rec_pid = 0;
    }
}

string transcribe() {
    string result;
    FILE* fp = popen(
        "cd ~/Routine_System && "
        "./whisper.cpp/build/bin/whisper-cli -m whisper.cpp/models/ggml-base.bin "
        "-l zh -np /tmp/voice_input.wav 2>/dev/null", "r");
    if (!fp) return "Error: failed to run whisper";

    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) {
        char* ts = strstr(buf, "] ");
        if (ts) result += string(ts + 2);
    }
    pclose(fp);
    return result;
}

int main() {
    cout << "=== Voice Input Test ===" << endl;
    cout << "Press [SPACE] to start/stop recording" << endl;
    cout << "Press [q] to quit" << endl;
    cout << endl;

    set_raw_mode();
    bool recording = false;

    while (true) {
        char c;
        int n = read(STDIN_FILENO, &c, 1);
        if (n <= 0) continue;
        if (c == 'q' || c == 'Q') break;

        if (c == ' ') {
            if (!recording) {
                recording = true;
                cout << "\n[Recording... press SPACE to stop]" << flush;
                start_recording();
            } else {
                recording = false;
                cout << "\n[Transcribing...]" << endl;
                stop_recording();
                string text = transcribe();
                cout << "Result: " << text << endl;
                cout << "\nPress [SPACE] to record, [q] to quit" << endl;
            }
        }
    }

    reset_terminal();
    return 0;
}

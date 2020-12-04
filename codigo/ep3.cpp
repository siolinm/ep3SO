#include <time.h>

#include <fstream>
#include <iomanip>

#include "commands.hpp"
#include "util.hpp"

using namespace std;

#define PROMPT cout << "[ep3]: ";

void parse(string &command) {
    if (command == MOUNT) {
        mount();
    } else if (command == COPY) {
        cp();
    } else if (command == MKDIR) {
        mkdir();
    } else if (command == RMDIR) {
        rmdir();
    } else if (command == CAT) {
        cat();
    } else if (command == TOUCH) {
        touch();
    } else if (command == RM) {
        rm();
    } else if (command == LS) {
        ls();
    } else if (command == FIND) {
        find();
    } else if (command == DF) {
        df();
    } else if (command == UMOUNT) {
        umount();
    }
}

int main() {
    string command;
    PROMPT;

    while ((cin >> command) && command != EXIT) {
        parse(command);
        PROMPT;
    }
    cout << "\n";

    root.libera();

    return 0;
}

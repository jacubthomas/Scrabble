#include <iostream>

#include "scrabble.h"
#include "scrabble_config.h"

using namespace std;


// You may use this code as is for testing although you may want to write
// other driver programs for unit testing different parts of the game.
int main(int argc, char** argv) {


    try {
        Scrabble scrabble(ScrabbleConfig::read("config/config.txt"));
        scrabble.main();
    } catch (const FileException& e) {
        cerr << e.what() << endl;
        return 1;
    }


    return 0;
}


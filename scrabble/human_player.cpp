#include "human_player.h"
#include "exceptions.h"
#include "formatting.h"
#include "move.h"
#include "place_result.h"
#include "rang.h"
#include "tile_kind.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>
#include<iterator>

using namespace std;

// This method is fully implemented.
inline string &to_upper(string &str) {
  transform(str.begin(), str.end(), str.begin(), ::toupper);
  return str;
}

// Returns a VALID move that can be executed (can be placed, exchanged, or
// passed).
Move HumanPlayer::get_move(const Board &board,
                           const Dictionary &dictionary) const {
  // 80 is max # of chars in a line
  string raw_line;
  char peek_char;
  string input;
  TileKind* to_tile;
  vector<TileKind> tiles_leaving;
  Direction direction(Direction::NONE);
  size_t row = 1000000;
  size_t column = 1000000;
  size_t index = 0;
  size_t count = 0;
    bool is_valid = true;
    
  while (1) {
    // handle/parse user input
    try {
      cout << "Your move, options include: PASS, EXCHANGE (tiles), or PLACE ( "
              "- or |, row, column, tiles) "
           << endl;
      // retrieve entire user command
      getline(cin, raw_line);

      // convert to ss for parsing
      stringstream ss(raw_line);

      // clear potential failbit in cin
      cin.clear();

      // get first string for which should determine move type
      ss >> input;

      //  check for equality operator that ignores case if possible
      if (input == "PASS") {
        Move valid_move = Move();
        return valid_move;
      } else if (input == "EXCHANGE") {
          
        // while input stream is not empty
        while (ss.rdbuf()->in_avail()) {
            
            // handles exception for singular tile that does not exist in hand (Repeat remove is handled elsewhere)
            if( count == 0 ){
                try {
                    ss >> input;
                    while(count < input.size()){
                        to_tile = new TileKind(this->hand_tiles.lookup_tile(input[count]));
                        tiles_leaving.push_back(*to_tile);
                        count++;
                    }
              } catch (out_of_range &e) {
                cout << e.what() << endl;
                cout << "Tile(s) passed for removal do not match those in hand"
                     << endl;
                  string temp;
                  getline(cin, temp);
                  cin.clear();
                throw;
              }
            }
          // mischief managed
          delete to_tile;
        }
        // is_valid will stay true if input assessments pass, otherwise throw
        // an exception
        if (tiles_leaving.size() > this->count_tiles())
          is_valid = false;
          
        if (is_valid) {
          Move valid_move = Move(tiles_leaving);
          return valid_move;
        } else {
          throw CommandException(
              "Invalid Exchange. Please ensure correct tiles "
              "are passed for exchange.");
        }
      }

      else if (input == "PLACE") {
        
          // retrieve direction
          if(direction == Direction::NONE){
              ss >> input;
              if(input == "|") direction = Direction::DOWN;
              else if(input == "-") direction = Direction::ACROSS;
          }
          // retrieve starting position (row/column) for place
          if(row == 1000000 || column == 1000000) {
              if (row == 1000000) {
                  ss >> row;
                  row -= 1;
            } if (column == 1000000){
                ss >> column;
                column--;
            }
              // size_t cannot be (-), just need to check input not bigger than board
              if( row > board.rows || column > board.columns)
                  throw CommandException("Invalid Place. Please enter starting row & column." );
          }
          // retrieve tiles
          if(tiles_leaving.size() == 0) {
              try {
                  ss >> input;
                  count = 0;
                  while(count < input.size()){
                      
                      // instantiate tile object from input char, then push onto place vector<tile>
                      to_tile = new TileKind(this->hand_tiles.lookup_tile(input[count]));
                      
                      // assign the next char in input stream to blank tile, then increment twice
                      if(input[count] == '?'){
                          to_tile->assigned = input[count+1];
                          count++;
                      }
                      // push onto tiles_vector & increment
                      tiles_leaving.push_back(*to_tile);
                      count++;
                  }
              } catch (out_of_range &e) {
                cout << e.what() << endl;
                cout << "Tile(s) passed for place do not match those in hand" << endl;
                  string temp;
                  getline(cin, temp);
                  cin.clear();
                throw;
              }
              
            // mischief managed
            delete to_tile;
          }
        }
        
        // is_valid will stay true if input assessments pass, otherwise throw an
        // exception
        bool is_valid = true;
        if (is_valid) {
          Move valid_move = Move(tiles_leaving, row, column, direction);
            // assess via test_place if placement on board holds
                PlaceResult PR = board.test_place(valid_move);
            if (PR.valid){
                if(!dictionary.is_word(PR.words[0])){
                    throw MoveException("Word not in dictionary :/");
                }
            return valid_move;
            }
          else
            throw CommandException("Invalid move. Placement error.");
        }
        // assessments did not pass throw exception and repeat
        // also, cleans up heap memory from tile instantiation as they are
        // passed by value, not reference
        else
          throw CommandException(
              "Invalid move. Please ensure correct tiles are passed for place "
              "and reverify placement position/direction.");
        
    } catch (CommandException &e) {
      cout << e.what() << endl;
      string temp;
      getline(cin, temp);
      cin.clear();
      continue;
    } catch (MoveException &e) {
        cout << e.what() << endl;
        continue;
      }
      catch (out_of_range &e) {
        continue;
    }
  }
}

vector<TileKind> HumanPlayer::parse_tiles(string &letters) const {
  // TODO: begin implementation here.
}

Move HumanPlayer::parse_move(string &move_string) const {
  // TODO: begin implementation here.
}
bool HumanPlayer::is_human() const{
    return true;
}
// This function is fully implemented.
void HumanPlayer::print_hand(ostream &out) const {
  const size_t tile_count = this->count_tiles();
  const size_t empty_tile_count = this->get_hand_size() - tile_count;
  const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

  for (size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
    out << endl;
  }

  out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING
      << "Your Hand: " << endl
      << endl;

  // Draw top line
  out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE
      << BG_COLOR_NORMAL_SQUARE;
  print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
  out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

  // Draw middle 3 lines
  for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
    out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD
        << repeat(SPACE, HAND_LEFT_MARGIN);
    for (auto it = this->hand_tiles.cbegin(); it != this->hand_tiles.cend();
         ++it) {
      out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL
          << BG_COLOR_PLAYER_HAND;

      // Print letter
      if (line == 1) {
        out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter)
            << repeat(SPACE, 2);

        // Print score in bottom right
      } else if (line == SQUARE_INNER_HEIGHT - 1) {
        out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2)
            << setw(2) << it->points;

      } else {
        out << repeat(SPACE, SQUARE_INNER_WIDTH);
      }
    }
    if (this->count_tiles() > 0) {
      out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
      out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
    }
  }

  // Draw bottom line
  out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE
      << BG_COLOR_NORMAL_SQUARE;
  print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
  out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}

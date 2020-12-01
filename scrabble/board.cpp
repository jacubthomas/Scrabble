#include "board.h"
#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <fstream>
#include <iomanip>

using namespace std;

bool Board::Position::operator==(const Board::Position &other) const {
  return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position &other) const {
  return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const {
  return this->translate(direction, 1);
}

Board::Position Board::Position::translate(Direction direction,
                                           size_t distance) const {
  if (direction == Direction::DOWN) {
    return Board::Position(this->row + distance, this->column);
  } else {
    return Board::Position(this->row, this->column + distance);
  }
}

Board Board::read(const string &file_path) {
  ifstream file(file_path);
  if (!file) {
    throw FileException("cannot open board file!");
  }

  size_t rows;
  size_t columns;
  size_t starting_row;
  size_t starting_column;
  file >> rows >> columns >> starting_row >> starting_column;
  Board board(rows, columns, starting_row, starting_column);

  string schema;
  getline(file, schema);

  for (size_t i = 0; i < rows; ++i) {
    getline(file, schema);
    if ((i != rows - 1 && file.eof()) || schema.size() != columns) {
      throw FileException("invalid board file!");
    }

    board.squares.emplace_back();
    for (size_t j = 0; j < columns; ++j) {
      unsigned int letter_multiplier = 1;
      unsigned int word_multiplier = 1;
      switch (schema[j]) {
      case '.':
        break;
      case '2':
        letter_multiplier = 2;
        break;
      case '3':
        letter_multiplier = 3;
        break;
      case 'd':
        word_multiplier = 2;
        break;
      case 't':
        word_multiplier = 3;
        break;
      default:
        throw FileException("invalid board file!");
      }
      board.squares.at(i).emplace_back(letter_multiplier, word_multiplier);
    }
  }

  return board;
}

size_t Board::get_move_index() const { return this->move_index; }

PlaceResult Board::test_place(const Move &move) const {

  bool start_or_neighboring_tile = false;

  Board::Position cursor(move.row, move.column);

  // Move must be in bounds
  if (!this->is_in_bounds(cursor))
    return PlaceResult("Given starting placement must be in bounds");

  // Can't start on an existing tile.
  if (this->at(cursor).has_tile()) {
    return PlaceResult("cannot start a word on an already-placed tile");
  }

  // Go to start of word
  while (in_bounds_and_has_tile(cursor.translate(move.direction, -1))) {
    cursor = cursor.translate(move.direction, -1);
  }

  string word = "";
  unsigned int total_points = 0;

  vector<string> words;
  unsigned int word_multiplier = 1;
  unsigned int word_points = 0;

  // Check every consequent letter, or remaining letters of word
  for (size_t i = 0; i < move.tiles.size() || in_bounds_and_has_tile(cursor);) {
    // Check in bounds
    if (!this->is_in_bounds(cursor)) {
      return PlaceResult("word placement goes out of bounds");
    }

    const BoardSquare &square = this->at(cursor);

    start_or_neighboring_tile =
        start_or_neighboring_tile || square.has_tile() || cursor == start;

    TileKind tile =
        square.has_tile() ? square.get_tile_kind() : move.tiles[i++];

    word += tile.letter == TileKind::BLANK_LETTER ? tile.assigned : tile.letter;
    word_points += square.has_tile() ? tile.points
                                     : tile.points * square.letter_multiplier;
    word_multiplier *= square.has_tile() ? 1 : square.word_multiplier;

    // Get normal word if tile is newly placed.
    if (!square.has_tile()) {
      Board::Position normal_cursor(cursor);
      while (in_bounds_and_has_tile(
          normal_cursor.translate(!move.direction, -1))) {
        normal_cursor = normal_cursor.translate(!move.direction, -1);
      }
      if (normal_cursor != cursor ||
          in_bounds_and_has_tile(
              cursor.translate(!move.direction, 1))) { // there is a normal word
        start_or_neighboring_tile = true;
        string normal_word;
        unsigned int normal_word_points = 0;
        unsigned int normal_word_multiplier = 1;

        do {
          const BoardSquare &normal_square = this->at(normal_cursor);
          TileKind normal_tile =
              normal_square.has_tile() ? normal_square.get_tile_kind() : tile;
          normal_word += normal_tile.letter == TileKind::BLANK_LETTER
                             ? normal_tile.assigned
                             : normal_tile.letter;
          ;
          normal_word_points +=
              normal_square.has_tile()
                  ? normal_tile.points
                  : normal_tile.points * normal_square.letter_multiplier;
          normal_word_multiplier *=
              normal_square.has_tile() ? 1 : normal_square.word_multiplier;
          normal_cursor = normal_cursor.translate(!move.direction, 1);
        } while (normal_cursor == cursor ||
                 in_bounds_and_has_tile(normal_cursor));

        total_points += normal_word_points * normal_word_multiplier;
        words.push_back(normal_word);
      }
    }
    // advance to next square
    cursor = cursor.translate(move.direction);
  }
  if (word.size() > 1) {
    words.push_back(word);
    total_points += word_points * word_multiplier;
  }
  if (words.size() == 0) {
    return PlaceResult("No words formed.");
  }
  if (!start_or_neighboring_tile)
    return PlaceResult(
        "Words must neighbor placed tile or contain start square.");
  return PlaceResult(words, total_points);
}

PlaceResult Board::place(const Move &move) {
  PlaceResult result = this->test_place(move);
  if (result.valid) {
    Board::Position cursor(move.row, move.column);
    for (size_t i = 0; i < move.tiles.size();) {
      if (this->at(cursor).has_tile()) {
      } else {
        this->at(cursor).set_tile_kind(move.tiles[i++]);
      }
      cursor = cursor.translate(move.direction);
    }
  }
  return result;
}

BoardSquare &Board::at(const Board::Position &position) {
  return this->squares.at(position.row).at(position.column);
}

const BoardSquare &Board::at(const Board::Position &position) const {
  return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position &position) const {
  return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position &position) const {
  return is_in_bounds(position) && at(position).has_tile();
}

void Board::print(ostream &out) const {
  // Draw horizontal number labels
  for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
    out << std::endl;
  }
  out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
  const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
  const size_t left_number_space =
      (SQUARE_OUTER_WIDTH - 3) - right_number_space;
  for (size_t column = 0; column < this->columns; ++column) {
    out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1
        << repeat(SPACE, right_number_space);
  }
  out << std::endl;

  // Draw top line
  out << repeat(SPACE, BOARD_LEFT_MARGIN);
  print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
  out << endl;

  // Draw inner board
  for (size_t row = 0; row < this->rows; ++row) {
    if (row > 0) {
      out << repeat(SPACE, BOARD_LEFT_MARGIN);
      print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
      out << endl;
    }

    // Draw insides of squares
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
      out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

      // Output column number of left padding
      if (line == 1) {
        out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
        out << std::setw(2) << row + 1;
        out << SPACE;
      } else {
        out << repeat(SPACE, BOARD_LEFT_MARGIN);
      }

      // Iterate columns
      for (size_t column = 0; column < this->columns; ++column) {
        out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
        const BoardSquare &square = this->squares.at(row).at(column);
        bool is_start = this->start.row == row && this->start.column == column;

        // Figure out background color
        if (square.word_multiplier == 2) {
          out << BG_COLOR_WORD_MULTIPLIER_2X;
        } else if (square.word_multiplier == 3) {
          out << BG_COLOR_WORD_MULTIPLIER_3X;
        } else if (square.letter_multiplier == 2) {
          out << BG_COLOR_LETTER_MULTIPLIER_2X;
        } else if (square.letter_multiplier == 3) {
          out << BG_COLOR_LETTER_MULTIPLIER_3X;
        } else if (is_start) {
          out << BG_COLOR_START_SQUARE;
        }

        // Text
        if (line == 0 && is_start) {
          out << "  \u2605  ";
        } else if (line == 0 && square.word_multiplier > 1) {
          out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2)
              << 'W' << std::setw(1) << square.word_multiplier;
        } else if (line == 0 && square.letter_multiplier > 1) {
          out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2)
              << 'L' << std::setw(1) << square.letter_multiplier;
        } else if (line == 1 && square.has_tile()) {
          char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER
                       ? square.get_tile_kind().assigned
                       : ' ';
          out << repeat(SPACE, 2) << FG_COLOR_LETTER
              << square.get_tile_kind().letter << l << repeat(SPACE, 1);
        } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
          out << repeat(SPACE, SQUARE_INNER_WIDTH - 1) << FG_COLOR_SCORE
              << square.get_points();
        } else {
          out << repeat(SPACE, SQUARE_INNER_WIDTH);
        }
      }

      // Add vertical line
      out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL
          << BG_COLOR_OUTSIDE_BOARD << std::endl;
    }
  }

  // draw bottom line
  out << repeat(SPACE, BOARD_LEFT_MARGIN);
  print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
  out << endl << rang::style::reset << std::endl;
}

vector<Board::Anchor> Board::get_anchors() const {

  vector<Anchor> anchors;

  // if board is empty
  if (!at(this->start).has_tile()) {

    // create limits for anchor construction; limits look to squares opposite of
    // place direction.
    size_t limit1 = start.row;
    size_t limit2 = start.column;

    // construct anchors
    Anchor a1(start, Direction::DOWN, limit1);
    Anchor a2(start, Direction::ACROSS, limit2);

    // prep vector for return
    anchors.push_back(a1);
    anchors.push_back(a2);
    return anchors;
  }

  // otherwise, scan entire board from (0,0) for anchors
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < columns; j++) {

      // current boardsquare of scan
      Position temp(i, j);

      // if anchor, evaluate limits and push onto vector
      if (is_anchor_spot(temp)) {

        // look left and above to evaluate limits
        Position left(i, j - 1);
        Position above(i - 1, j);
        size_t limit_left = 0;
        size_t limit_right = 0;
        while (is_in_bounds(left) && !in_bounds_and_has_tile(left) &&
               !is_anchor_spot(left)) {
          limit_left++;
          left.column -= 1;
        }
        while (is_in_bounds(above) && !in_bounds_and_has_tile(above) &&
               !is_anchor_spot(above)) {
          limit_right++;
          above.row -= 1;
        }

        // construct anchors
        Anchor a_left(temp, Direction::ACROSS, limit_left);
        Anchor a_right(temp, Direction::DOWN, limit_right);

        // build vector for return
        anchors.push_back(a_left);
        anchors.push_back(a_right);
      }
    }
  }
  return anchors;
}

bool Board::is_anchor_spot(Position position) const {

  // an anchor cannot have a tile
  if (!in_bounds_and_has_tile(position)) {

    // creates adjacent positions
    Position left(position.row, position.column - 1);
    Position right(position.row, position.column + 1);
    Position above(position.row - 1, position.column);
    Position below(position.row + 1, position.column);

    // any neighboring squares have a tile, than position is an anchor spot
    if (in_bounds_and_has_tile(left))
      return true;
    if (in_bounds_and_has_tile(right))
      return true;
    if (in_bounds_and_has_tile(above))
      return true;
    if (in_bounds_and_has_tile(below))
      return true;
  }
  // not an anchor
  return false;
}

char Board::letter_at(Position p) const {
  BoardSquare loc_square = at(p);
  TileKind loc_tile = loc_square.get_tile_kind();
  if (loc_tile.letter == '?')
    return loc_tile.assigned;
  else
    return loc_tile.letter;
}


#include "computer_player.h"
#include <iostream>
#include <iterator>
#include <memory>
#include <string>

using namespace std;

void ComputerPlayer::left_part(Board::Position anchor_pos, string partial_word,
                               Move partial_move,
                               shared_ptr<Dictionary::TrieNode> node,
                               size_t limit, TileCollection &remaining_tiles,
                               vector<Move> &legal_moves, const Board &board,
                               const Dictionary &d) const {

  Move new_move = partial_move;

  // increment move to position w/ lesser limit
  if (partial_move.direction == Direction::ACROSS)
    new_move.column -= 1;
  else
    new_move.row -= 1;

  // each call to left_part calls extend_right
  extend_right(anchor_pos, partial_word, new_move, node, remaining_tiles,
               legal_moves, board, d);

  // base case
  if (limit == 0) {
    extend_right(anchor_pos, partial_word, partial_move, node, remaining_tiles,
                 legal_moves, board, d);
    return;
  }

  // check for blank
  bool has_blank = false;
  try {
    TileKind blank = remaining_tiles.lookup_tile('?');
    has_blank = true;
  } catch (out_of_range &oor) {
  }

  // iterate through all possible prefix combos available in trie
  map<char, shared_ptr<Dictionary::TrieNode>>::iterator it =
      node->nexts.begin();

  for (; it != node->nexts.end(); it++) {
    try {
      // Call for blank tile
      if (has_blank) {
        TileKind tk2 = remaining_tiles.lookup_tile('?');

        // if so, add tile to front of partial word & check for prefix
        string temp_string;
        temp_string.push_back(it->first);
        temp_string += partial_word;
        shared_ptr<Dictionary::TrieNode> trie_this = d.find_prefix(temp_string);
        if (trie_this == nullptr)
          return;

        // if prefix exists, update blank assigned & partial move
        tk2.assigned = it->first;
        partial_move.tiles.insert(partial_move.tiles.begin(), tk2);

        // sandwich recursion, since blank tile, unassign upon return
        remaining_tiles.remove_tile(tk2);
        left_part(anchor_pos, temp_string, partial_move, node, limit - 1,
                  remaining_tiles, legal_moves, board, d);
        tk2.assigned = '\0';
        remaining_tiles.add_tile(tk2);

        // backstep
        temp_string.erase(0, 1);
        partial_move.tiles.erase(partial_move.tiles.begin());
      }
      // check for non-blank tile in hand
      TileKind tk = remaining_tiles.lookup_tile(it->first);

      // if so, add tile to front of partial word & check for prefix
      string temp_string;
      temp_string.push_back(it->first);
      temp_string += partial_word;
      shared_ptr<Dictionary::TrieNode> trie_this = d.find_prefix(temp_string);
      if (trie_this == nullptr)
        return;

      // if prefix exists, update partial move
      partial_move.tiles.insert(partial_move.tiles.begin(), tk);

      // sandwich recursion
      remaining_tiles.remove_tile(tk);
      left_part(anchor_pos, temp_string, partial_move, node, limit - 1,
                remaining_tiles, legal_moves, board, d);
      remaining_tiles.add_tile(tk);

      // backstep
      temp_string.erase(0, 1);
      partial_move.tiles.erase(partial_move.tiles.begin());

    } catch (out_of_range &oor) {
      continue;
    } catch (exception &e) {
      continue;
    }
  }
  // increment move to position w/ lesser limit
  if (partial_move.direction == Direction::ACROSS)
    partial_move.column += 1;
  else
    partial_move.row += 1;

  // recursive call w/ lesser limit
  left_part(anchor_pos, partial_word, partial_move, node, limit - 1,
            remaining_tiles, legal_moves, board, d);
}

void ComputerPlayer::extend_right(Board::Position square, string partial_word,
                                  Move partial_move,
                                  shared_ptr<Dictionary::TrieNode> node,
                                  TileCollection &remaining_tiles,
                                  vector<Move> &legal_moves, const Board &board,
                                  const Dictionary &d) const {

  // if move passed is valid, include it!
  if (node->is_final) {
    legal_moves.push_back(partial_move);
  }

  // Base Case: if position is out-of-bounds
  if (!board.is_in_bounds(square)) {
    return;
  }
  if (board.in_bounds_and_has_tile(square)) {
    // capture square for word prefix lookup
    partial_word.push_back(board.letter_at(square));

    // dictionary check
    shared_ptr<Dictionary::TrieNode> trie_this = d.find_prefix(partial_word);
    if (trie_this == nullptr)
      return;

    // advance square for R.C.
    square.translate(partial_move.direction, 1);
    extend_right(square, partial_word, partial_move, trie_this, remaining_tiles,
                 legal_moves, board, d);
  } else {
    bool has_blank = false;
    try {
      TileKind blank = remaining_tiles.lookup_tile('?');
      has_blank = true;
    } catch (out_of_range &oor) {
    }
    // Check all possible combos
    map<char, shared_ptr<Dictionary::TrieNode>>::iterator it =
        node->nexts.begin();
    for (; it != node->nexts.end(); it++) {
      try {
        // Call for blank tile
        if (has_blank) {
          TileKind tk2 = remaining_tiles.lookup_tile('?');

          // update word w/ tile from hand/trie
          string temp_string(partial_word);
          temp_string.push_back(it->first);

          // build trie and verify, string is word
          shared_ptr<Dictionary::TrieNode> trie_this =
              d.find_prefix(temp_string);
          if (trie_this == nullptr)
            return;

          // update blank assigned
          tk2.assigned = it->first;
          partial_move.tiles.push_back(tk2);

          // sandwich R.C. to extend_right
          remaining_tiles.remove_tile(tk2);
          extend_right(square, temp_string, partial_move, trie_this,
                       remaining_tiles, legal_moves, board, d);
          tk2.assigned = '\0';
          remaining_tiles.add_tile(tk2);

          // backstep
          temp_string.pop_back();
          partial_move.tiles.pop_back();
        }
        // build call with non-blank tile to extend right
        TileKind tk = remaining_tiles.lookup_tile(it->first);

        // update word w/ tile from hand/trie
        string temp_string(partial_word);
        temp_string.push_back(it->first);

        // build trie and verify, string is word
        shared_ptr<Dictionary::TrieNode> trie_this = d.find_prefix(temp_string);
        if (trie_this == nullptr)
          return;

        // update move object
        partial_move.tiles.push_back(tk);
        square.translate(partial_move.direction, 1);

        // sandwich R.C. to extend_right
        remaining_tiles.remove_tile(tk);
        extend_right(square, temp_string, partial_move, trie_this,
                     remaining_tiles, legal_moves, board, d);
        remaining_tiles.add_tile(tk);

        // backstep
        temp_string.pop_back();
        partial_move.tiles.pop_back();

      } catch (out_of_range &oor) {
        continue;
      } catch (exception &e) {
        continue;
      }
    }
    // clear out tiles vector for next combo
    partial_move.tiles.clear();
  }
}

Move ComputerPlayer::get_move(const Board &board,
                              const Dictionary &dictionary) const {
  vector<Move> legal_moves;
  vector<Board::Anchor> anchors = board.get_anchors();
  vector<TileKind> temp_tiles;
  TileCollection tiles_copy = tiles;

  // computer makes first move
  if (!board.in_bounds_and_has_tile(board.start)) {
    shared_ptr<Dictionary::TrieNode> trie_this = dictionary.find_prefix("");
    Move part_move = Move(temp_tiles, anchors[0].position.row,
                          anchors[0].position.column, anchors[0].direction);
    extend_right(board.start, "", part_move, trie_this, tiles_copy, legal_moves,
                 board, dictionary);

  } else {
    // Search through all anchors, building all legal word combos to pass for
    // get_best_move
    for (size_t i = 0; i < anchors.size(); i++) {

      bool end_search = false;

      // position will shift in search
      Board::Position pos_traverse = anchors[i].position;
      Direction dir_traverse = anchors[i].direction;

      // position will be passed to move object for left/right calls
      Board::Position move_pos = pos_traverse;
      Move part_move =
          Move(temp_tiles, pos_traverse.row, pos_traverse.column, dir_traverse);

      // if limit is 0, no call to left part, but requires checking for existing
      // prefix to call extend_right
      if (anchors[i].limit == 0) {

        // look for tiles on board opposite place direction and add to prefix
        string prefix = "";
        while (!end_search) {
          if (board.in_bounds_and_has_tile(pos_traverse)) {
            prefix.push_back(board.letter_at(pos_traverse));
            pos_traverse.translate(dir_traverse, -1);
          } else {
            end_search = true;
          }
        }
        // call right w/ all letters prior to place considered
        shared_ptr<Dictionary::TrieNode> trie_this =
            dictionary.find_prefix(prefix);
        extend_right(anchors[i].position, prefix, part_move, trie_this,
                     tiles_copy, legal_moves, board, dictionary);
      }
      // call left for where prefixes may be built
      else if (anchors[i].limit > 0) {

        // look for tiles on board in the place direction and add to prefix
        string prefix = "";
        while (!end_search) {
          if (board.in_bounds_and_has_tile(pos_traverse)) {
            prefix.push_back(board.letter_at(pos_traverse));
            pos_traverse.translate(dir_traverse, 1);
          } else {
            end_search = true;
          }
        }
        // Move position will change with limit
        if (anchors[i].direction == Direction::ACROSS) {
          part_move =
              Move(temp_tiles, anchors[i].position.row,
                   anchors[i].position.column - anchors[i].limit, dir_traverse);
        } else {
          part_move =
              Move(temp_tiles, anchors[i].position.row - anchors[i].limit,
                   anchors[i].position.column, dir_traverse);
        }
        shared_ptr<Dictionary::TrieNode> trie_this = dictionary.get_root();
        left_part(anchors[i].position, prefix, part_move, trie_this,
                  anchors[i].limit, tiles_copy, legal_moves, board, dictionary);
      }
    }
  }
  return get_best_move(legal_moves, board, dictionary);
}

Move ComputerPlayer::get_best_move(vector<Move> legal_moves, const Board &board,
                                   const Dictionary &dictionary) const {
  Move best_move = Move(); // Pass if no move found
  // HW5: IMPLEMENT THIS
  if (legal_moves.empty()) {
    return best_move;
  }

  // base place result
  vector<string> words;
  PlaceResult best_place(words, 0);
  size_t best_idx;

  // consider all legal_moves
  for (size_t i = 0; i < legal_moves.size(); i++) {

    // use test_place to verify move is legal and gather its return points for
    // comparison
    PlaceResult temp_place = board.test_place(legal_moves[i]);
    try {
      if (!temp_place.valid) {
        continue;
      }

      // dictionary check on all words/subwords from place
      for (size_t i = 0; i < temp_place.words.size(); i++) {
        if (!dictionary.is_word(temp_place.words[i])) {
          throw MoveException("Word not in dictionary :/");
        }
      }

      // only the highest scoring move will be returned
      if (temp_place.points > best_place.points) {
        best_place = temp_place;
        best_idx = i;
      }
    } catch (MoveException &e) {
      continue;
    }
  }

  // error handling
  if (best_idx > legal_moves.size()) {
    return best_move;
  }

  // update best move & return
  best_move = legal_moves[best_idx];
  return best_move;
}

#include "scrabble.h"
#include "formatting.h"
#include <iostream>
#include <iomanip>
#include <map>

using namespace std;


Scrabble::Scrabble(const ScrabbleConfig& config)
    : hand_size(config.hand_size)
    , minimum_word_length(config.minimum_word_length)
    , tile_bag(TileBag::read(config.tile_bag_file_path, config.seed))
    , board(Board::read(config.board_file_path))
    , dictionary(Dictionary::read(config.dictionary_file_path)) {
        num_human_players = 0;
    }


void Scrabble::add_players() {
    cout << "Please enter number of players: ";
    size_t player_count;
    cin >> player_count;

    if (player_count < 1 || player_count > 8) {
        throw FileException("PLAYERS");
    }
    cout << player_count << " players confirmed." << endl;
    cin.ignore();

    for (size_t i = 1; i <= player_count; ++i) {
        cout << "Please enter name for player " << i << ": ";
        string player_name;
        getline(cin, player_name);
        cout << "Is this player a CPU? Enter 'y' or 'n' : ";
        string temp; char is_CPU = 'N';
        getline(cin, temp);
        for(size_t i=0; i< temp.size(); i++){
            if(temp[i] == 'Y'  || temp[i] == 'y' ) is_CPU = 'Y';
        }
        cout << "Player " << i << ", named \"" << player_name << "\", has been added." << endl;
        if(is_CPU == 'Y'){
            shared_ptr<Player> player = make_shared<ComputerPlayer>(player_name, hand_size);
            player->add_tiles(tile_bag.remove_random_tiles(this->hand_size));
            player->assign_human('N');
            players.push_back(player);
            return;
        }
        shared_ptr<Player> player = make_shared<HumanPlayer>(player_name, hand_size);
        player->add_tiles(tile_bag.remove_random_tiles(this->hand_size));
        player->assign_human('Y');
        update_num_humans();
		players.push_back(player);
    }
}

void Scrabble::game_loop() {
	size_t sequential_passes = 0;

	while (true) {
		for (auto player : this->players) {
            board.print(cout);
            Move move = player->get_move(board, dictionary);
            sequential_passes = move.kind == MoveKind::PASS ? sequential_passes + 1 : 0;

			if (move.kind == MoveKind::EXCHANGE) {
				for (auto tile : move.tiles) {
       				tile_bag.add_tile(tile);
    			}  
			}

            // Remove necessary tiles
			player->remove_tiles(move.tiles);

            // Commit place if necessary
            if (move.kind == MoveKind::PLACE) {
                PlaceResult result = board.place(move);
                player->add_points(result.points);
                cout << "You gained " << SCORE_COLOR << result.points << rang::style::reset << " points!" << endl;
                if (move.tiles.size() == hand_size){
                    player->add_points(EMPTY_HAND_BONUS);
                    cout << "Full hand used! +" << SCORE_COLOR << EMPTY_HAND_BONUS << rang::style::reset << " points!" << endl;
                }
            }

            // Replenish tiles
            player->add_tiles(this->tile_bag.remove_random_tiles(this->hand_size - player->count_tiles()));

            cout << "Your current score: " << SCORE_COLOR << player->get_points() << rang::style::reset << endl;

            // wait for player confirmation, spec requires this
            cout << endl << "Press [enter] to continue.";
            cin.ignore();

            // If all players pass for one whole loop, or someone ran out of tiles and could not get more, the game is over
            if (sequential_passes >= players.size() || player->count_tiles() == 0) {
                return;
            }
		}
	}
}

void Scrabble::final_subtraction(vector<shared_ptr<Player>> & plrs) {

	ssize_t empty_hand_idx = -1;
    // this player gets the bonus
	for(size_t player_num = 0; player_num < plrs.size(); ++player_num)
	{
		if(plrs[player_num]->count_tiles() == 0)
    	{
        	empty_hand_idx = player_num;
			break;
    	}
	}

	unsigned int total_remaining = 0;

	for(size_t player_num = 0; player_num < plrs.size(); ++player_num)
    {
        unsigned int remaining_points = plrs[player_num]->get_hand_value();
        total_remaining += remaining_points;
        plrs[player_num]->subtract_points(remaining_points);
    }

    // If someone finished with an empty hand they get a bonus
    if(empty_hand_idx != -1)
    {
        plrs[empty_hand_idx]->add_points(total_remaining);
    }
}

void Scrabble::print_result() {
	size_t max_points = 0;
	for (auto player : this->players) {
		if (player->get_points() > max_points) {
			max_points = player->get_points();
        }
	}

	// Determine the winner(s) indexes
	vector<shared_ptr<Player>> winners;
	for (auto player : this->players) {
		if (player->get_points() >= max_points) {
			winners.push_back(player);
        }
	}

    cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
	for (auto player : winners) {
		cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
	}
	cout << rang::style::reset << endl;

	// now print score table
    cout << "Scores: " << endl;
    cout << "---------------------------------" << endl;

	// Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

	for (auto player : this->players) {
		cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR << player->get_name() << rang::style::reset << endl;
	}
}


void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(players);
    print_result();
}

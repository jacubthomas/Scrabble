#include "player.h"

using namespace std;


void Player::add_points(size_t points) {
    this->points += points;
}

void Player::subtract_points(size_t points) {
	if(points >= this->points) {
		this->points = 0;
		return;
	}
    this->points -= points;
}

size_t Player::get_points() const {
    return points;
}

const std::string& Player::get_name() const {
    return this->name;
}

size_t Player::count_tiles() const {
    return tiles.count_tiles();
}

void Player::remove_tiles(const std::vector<TileKind>& tiles) {
    for (auto tile : tiles) {
       this->tiles.remove_tile(tile);
    }
}

void Player::add_tiles(const std::vector<TileKind>& tiles) {
    for (auto tile : tiles) {
       this->tiles.add_tile(tile);
    }
}

bool Player::has_tile(TileKind tile) {
	try {
		tiles.lookup_tile(tile.letter);
	} catch (const out_of_range e) {
		return false;
	}
	return true;
}

unsigned int Player::get_hand_value() const {
    return this->tiles.total_points();
}

size_t Player::get_hand_size() const {
    return this->hand_size;
}

void Player::assign_human(char human){
    if(human == 'Y') this->human = true;
    else this->human = false;
}


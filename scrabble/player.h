#ifndef PLAYER_H
#define PLAYER_H

#include "move.h"
#include "board.h"
#include "dictionary.h"
#include "tile_collection.h"
#include <string>
#include <vector>

class Player {
public:
    Player(const std::string& name, size_t hand_size) // Used for testing
        : name(name)
        , hand_size(hand_size)
        , points(0) {}
    virtual ~Player() {};

    void add_points(size_t points);
    void subtract_points(size_t points);
    size_t get_points() const; // Used for testing

    const std::string& get_name() const;

    virtual Move get_move(const Board& board, const Dictionary& d) const = 0;

	virtual bool is_human() const = 0;
    
    void assign_human(char human);

    size_t count_tiles() const; // Used for testing
    void remove_tiles(const std::vector<TileKind>& tiles); // Used for testing
    void add_tiles(const std::vector<TileKind>& tiles); // Used for testing
	bool has_tile(TileKind tile); // Used for testing 

    unsigned int get_hand_value() const; // Used for testing
    size_t get_hand_size() const;

protected:
    TileCollection tiles;

private:
    std::string name;
    size_t hand_size;
    size_t points;
    bool human;
};


#endif

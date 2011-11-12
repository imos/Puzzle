// Copyright (C) 2011 Kentaro Imajo. All rights reserved.
// Author: Kentaro Imajo (Twitter: imos)
//
// This program uses special ordered cell keys. From the upper-left-most cell
// to bottom-right-most cell, it traces every cell with diagonal lines from
// upper-right to bottom-left. For 4x4 table, we uses the order that is denoted
// in the following table.
//   00 01 03 06
//   02 04 07 10
//   05 08 11 13
//   09 12 14 15
//
// This program inputs some datasets from the standard input. Each dataset
// should have `width' and `height' in this order with a space delimiter in
// the first line, and each of the following height lines should have width
// integers. Zeroes represent blank cells, and one or more represent
// themselves. For the Number Link (the left figure), you should use the input
// (the right figure).
//
//   +---+---+---+---+  Standard Input:
//   | 1           2 |  4 3
//   +   +   +   +   +  1 0 0 2
//   |     3   1     |  0 3 1 0
//   +   +   +   +   +  3 2 0 0
//   | 3   2         |
//   +---+---+---+---+

#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <stack>
#include <vector>
using namespace std;

bool FLAGS_color = false;

class NumberLink {
 public:
	// The type of the number written in a cell
	typedef char CellNumber;
	// The type of the ID number of a cell
	typedef int CellKey;
	// The type of a coordinate represented by (y * width + x)
	typedef int CellPosition;
	// The type of a distance such as width, height, x and y
	typedef int Distance;
	// The size of the NumberLink task
	Distance width_;
	Distance height_;

	NumberLink(const Distance width, const Distance height):
	    width_(width), height_(height), size_((size_t)width * height),
	    cell_x_(size_), cell_y_(size_), table_(size_),
	    keys_(size_), mates_(size_), start_(size_ + 1),
	    connected_x_(size_), connected_y_(size_), memo_(size_) {}

	void Initialize() {
		// Initialize mates
		for (CellKey cell_key = 0; cell_key < size_; cell_key++) {
			mates_[cell_key] = cell_key;
		}

		// Generates references: (x, y) <=> CellKey
		Distance x = 0, y = 0;
		CellKey cell_key = 0;
		while (true) {
			CellPosition position = y * width_ + x;
			cell_x_[cell_key] = x;
			cell_y_[cell_key] = y;
			keys_[position] = cell_key;
			cell_key++;
			if (cell_key == size_) break;
			do {
				x--;
				y++;
				if (x < 0) {
					x = y;
					y = 0;
				}
			} while (x < 0 || width_ <= x || y < 0 || height_ <= y);
		}

		// Pre-compute CellKey to look back for every cell
		for (CellPosition i = 0; i < size_; i++) {
			Distance x = cell_x_[i], y = cell_y_[i];
			start_[i] = 0 < y ? GetCellKey(x, y - 1) :
			                    (0 < x ? GetCellKey(x - 1, y) : 0);
		}
		// For a guard
		start_[size_] = size_;
	}

	// Returns the reference of the number written in the cell (x,y).
	CellNumber& Cell(const Distance x, const Distance y)
	    { return table_[GetCellKey(x, y)]; }
	// Returns the key of the special order for the coordinate (x,y).
	CellKey GetCellKey(const Distance x, const Distance y) const
	    { return keys_[y * width_ + x]; }

	double Solve(const CellKey cell_key = 0) {
		if (cell_key == 0) solved_ = false;
		// See the newly fixed cells
		if (0 < cell_key) {
			for (CellKey hidden = start_[cell_key - 1];
			    hidden < start_[cell_key]; hidden++) {
				if (table_[hidden] == 0) {
					// Return if the empty cell has an end
					if (mates_[hidden] != -1 && mates_[hidden] != hidden) return 0.0;
				} else {
					// Return if the numbered cell has no line
					if (mates_[hidden] == hidden) return 0.0;
				}
			}
		}
		// If all the cells are filled
		if (cell_key == size_) {
			if (!solved_) {
				Print();
				solved_ = true;
			}
			return 1.0;
		}
		// If this set has been seen
		const vector<CellKey> mate_tuple(mates_.begin() + start_[cell_key],
		                                 mates_.begin() + cell_key);
		if (!memo_[cell_key].count(mate_tuple)) {
			memo_[cell_key][mate_tuple] = Connect(cell_key);
		}
		return memo_[cell_key][mate_tuple];
	}

	double Connect(const CellKey cell_key) {
		double solution_count = 0.0;
		Distance x = cell_x_[cell_key], y = cell_y_[cell_key];
		CellKey left_cell_key = -1, up_cell_key = -1;
		if (0 < x) left_cell_key = GetCellKey(x - 1, y);
		if (0 < y) up_cell_key = GetCellKey(x, y - 1);
		int revert_point = mate_stack_.size();
		// Connect the cell with nothing
		solution_count += Solve(cell_key + 1);
		// Connect the cell with the upper cell
		if (0 <= up_cell_key) {
			if (UniteMates(cell_key, up_cell_key)) {
				connected_y_[cell_key] = true;
				solution_count += Solve(cell_key + 1);
				connected_y_[cell_key] = false;
			}
			RevertMates(revert_point);
		}
		// Connect the cell with the left cell
		if (0 <= left_cell_key) {
			if (UniteMates(cell_key, left_cell_key)) {
				connected_x_[cell_key] = true;
				solution_count += Solve(cell_key + 1);
				// Connect the cell with the upper and the left cells
				if (0 <= up_cell_key) {
					if (UniteMates(cell_key, up_cell_key)) {
						connected_y_[cell_key] = true;
						solution_count += Solve(cell_key + 1);
						connected_y_[cell_key] = false;
					}
				}
				connected_x_[cell_key] = false;
			}
			RevertMates(revert_point);
		}
		return solution_count;
	}

	void Print() {
		for (Distance y = 0; y <= height_; y++) {
			for (Distance x = 0; x < width_; x++) {
				printf(FLAGS_color ? "\x1b[32m+\x1b[0m" : "+");
				printf((y % height_ == 0) ?
				       (FLAGS_color ? "\x1b[32m---\x1b[0m" : "---") :
				       (connected_y_[GetCellKey(x, y)] ? " # " : "   "));
			}
			puts(FLAGS_color ? "\x1b[32m+\x1b[0m" : "+");
			if (height_ <= y) break;
			for (Distance x = 0; x < width_; x++) {
				printf(x ? (connected_x_[GetCellKey(x, y)] ? "#" : " ") :
				       (FLAGS_color ? "\x1b[32m|\x1b[0m" : "|"));
				if (table_[GetCellKey(x, y)]) {
					printf("%03d", table_[GetCellKey(x, y)]);
				} else {
					printf(connected_x_[GetCellKey(x, y)] ? "#" : " ");
					printf(mates_[GetCellKey(x, y)] == GetCellKey(x, y) ? " " : "#");
					printf((x + 1 < width_ && connected_x_[GetCellKey(x + 1, y)])
					       ? "#" : " ");
				}
			}
			puts(FLAGS_color ? "\x1b[32m|\x1b[0m" : "|");
		}
	}

 private:
	CellPosition size_;
	stack< pair<CellKey, CellKey> > mate_stack_;
	vector< map<vector<CellKey>, double> > memo_;
	vector<Distance> cell_x_, cell_y_;
	vector<CellNumber> table_;
	vector<CellKey> keys_, mates_, start_;
	vector<bool> connected_x_, connected_y_;
	bool solved_;

	int ChangeMates(const CellKey cell_key, const CellKey cell_value) {
		int last_stack_size = mate_stack_.size();
		CellKey last_value = mates_[cell_key];
		if (last_value != cell_value) {
			mate_stack_.push(make_pair(cell_key, last_value));
			mates_[cell_key] = cell_value;
		}
		return last_stack_size;
	}

	int RevertMates(const int stack_size) {
		for (; stack_size < mate_stack_.size(); mate_stack_.pop())
		    mates_[mate_stack_.top().first] = mate_stack_.top().second;
	}

	bool UniteMates(const CellKey cell_key1, const CellKey cell_key2) {
		CellKey end1 = mates_[cell_key1], end2 = mates_[cell_key2];
		// Cannot connect any branch
		if (end1 == -1 || end2 == -1) return false;
		// Avoid making a cycle
		if (cell_key1 == end2 && cell_key2 == end1) return false;
		// Change states of mates to connect cell_key1 and cell_key2
		ChangeMates(cell_key1, -1);
		ChangeMates(cell_key2, -1);
		ChangeMates(end1, end2);
		ChangeMates(end2, end1);
		// Check three constraints:
		//   1. cell_key1 must not be a branch if cell_key1 is numbered,
		//   2. cell_key2 must not be a branch if cell_key2 is numbered,
		//   3. Different numbers cannot be connected.
		if (mates_[cell_key1] == -1 && 0 < table_[cell_key1]) return false;
		if (mates_[cell_key2] == -1 && 0 < table_[cell_key2]) return false;
		if (0 < table_[end1] && 0 < table_[end2] &&
		    table_[end1] != table_[end2]) return false;
		return true;
	}
};

void ParseArguments(int *argc, char **argv) {
	int arg_pos = 1;
	for (int i = 1; i < *argc; i++) {
		char *flag = argv[i];
		if (flag[0] == '-' && flag[1] == '-') {
			char *key = flag + 2, *value = strstr(key, "=");
			if (value) {
				*value = 0;
				value++;
			}
			if (strcmp(key, "color") == 0) {
				FLAGS_color = true;
			}
		} else {
			argv[arg_pos] = argv[i];
			arg_pos++;
		}
	}
	*argc = arg_pos;
}

int main(int argc, char **argv) {
	ParseArguments(&argc, argv);
	int width, height;
	while (2 == scanf("%d%d", &width, &height) && width && height) {
		NumberLink nl((int)width, (int)height);
		nl.Initialize();
		for (int y = 0; y < nl.height_; y++) {
			for (int x = 0; x < nl.width_; x++) {
				int value;
				scanf("%d", &value);
				nl.Cell(x, y) = value;
			}
		}
		double solution_count = nl.Solve();
		printf("# of solutions: ");
		if (solution_count < 1e13) {
			printf("%.0f\n", solution_count);
		} else {
			printf("%.13e\n", solution_count);
		}
	}
	return 0;
}

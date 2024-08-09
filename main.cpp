/*
* main.cpp - Main file for CSP program
* Author: Zeno Marquis
* HW4: Constraint Satisfaction Problems
* 3/31/2024
*/

#include "Square.hpp"

#include <vector>
#include <iostream>
#include <string>
#include <stack>
#include <unordered_map>

using namespace std;

int get_input();
string get_string();
vector<vector<Square>> set_board(string board_state);
void set_domains(vector<vector<Square>> *board);
void display_board(vector<vector<Square>> board);
int fill_board(vector<vector<Square>> *board, stack<string> *move_path, unordered_map<string, string> *all_moves);
bool check_full(vector<vector<Square>> *board);
int least_constraining_value(int row_index, int col_index, vector<vector<Square>> *board);
void backtrack(vector<vector<Square>> *board, stack<string> *move_path);
string get_board_state(vector<vector<Square>> *board);
bool check_valid_board(vector<vector<Square>> *board);

/**
     * Main function for CSP algorithm
     */
int main(){

    vector<vector<Square>> board;
    stack<string> move_path;
    unordered_map<string, string> all_moves;

    //set starting board based on user input
    int input = get_input();
    if (input == 1){
        board = set_board("608702100400010002025400000701080405080000070509060301000006750200090008006805203");
    } else if (input == 2) {
        board = set_board("070042000000008610390000007000004009003000700500100000800000076054800000000610050");
    } else {
        board = set_board(get_string());
    }

    cout << "Starting board:\n";
    display_board(board);

    int count = 0;

    while (fill_board(&board, &move_path, &all_moves) != 2){
        count++;
        if (count % 1000 == 0 && count != 0){
            cout << "Checked " << count << " board states\n";
        }
    }

    cout << "Solution found in " << count << " checks\n";

    cout << "Final board:\n";
    display_board(board);

    return 0;
}

/**
     * Gets input from user
     * 
     * @return an integer representing the user input
     */
int get_input(){

    bool success = false;
    string instring;
    int int_input;

    cout << "Enter 1 for the easy board, 2 for the harder board, or 3";
    cout << " to enter your own: ";

    while (!success){

        cin >> instring;

        try{
            int_input = stoi(instring);
        } catch(std::invalid_argument){
            int_input = 0;
        }  

        if (int_input >= 1 && int_input <= 3){
            return int_input;
        } else {
            cout << "Invalid input, please try again\n";
        }
    }

    return 1;

}

/**
     * Gets a board state from the user
     * 
     * @return a string representing the board state requested
     */
string get_string(){
    
    cout << "Enter a sudoku board by inputing the cell values from the"; 
    cout << " top left corner to the bottom right, going row by row. ";
    cout << " For example, the board\n";
    
    vector<vector<Square>> tempboard = set_board("608702100400010002025400000701080405080000070509060301000006750200090008006805203");

    display_board(tempboard);

    cout << "would be written 608702100400010002025400000701080405080000070509060301000006750200090008006805203.\n";
    cout << "Note: this program does not check for malformed inputs,";
    cout << " and may run out of memory on especially challenging problems\n";
    cout << "Type the whole sequence and hit enter: "; 
    
    string input;
    cin >> input;
    return input;
}

/**
     * Turns the input board state into a 2d vector of squares
     * 
     * @param board_state: string representing the board state 
     * @return a 2d vector storing the board as square classes
     */
vector<vector<Square>> set_board(string board_state){

    //board is represented in row, column order as vectors
    vector<vector<Square>> board;

    for (int i = 0; i < 9; i++){

        vector<Square> row;

        for (int j = 0; j < 9; j++){
            Square *sp = new Square(board_state.at(j + (9 * (i))) - 48);
            row.push_back(*sp);
        }

        board.push_back(row);

    }

    set_domains(&board);
    return board;

}

/**
     * Sets the domains of all squares in the board
     * 
     * @param board: a pointer to a 2d vector representing the board
     */
void set_domains(vector<vector<Square>> *board){

    for (int i = 0; i < 9; i++){
        for (int j = 0; j < 9; j++){
            if (board->at(i).at(j).get_value() == 0){
                //start domain at all possible values, check for values in all
                //squares in the same row, column, or 3x3 square
                set<int> domain({1, 2, 3, 4, 5, 6, 7, 8, 9});

                for (int k = 0; k < 9; k++){
                    if (k != i){
                        int val = board->at(k).at(j).get_value();
                        if(val != 0){
                            domain.erase(val);
                        }
                    }

                    if (k != j){
                        int val = board->at(i).at(k).get_value();
                        if(val != 0){
                            domain.erase(val);
                        }
                    }

                    int row_square = i/3;
                    int col_square = j/3;
                    
                    int row = 3*row_square + k%3;
                    int col = 3*col_square + k/3;

                    if (row != i || col != j){

                        int val = board->at(row).at(col).get_value();
                        if(val != 0){
                                domain.erase(val);
                        }

                    }
                }

                //set the domains of the node
                board->at(i).at(j).set_domain(domain);

            }
        }
    }
}

/**
     * Prints the board to the terminal
     * 
     * @param board: a 2d vector of squares representing the board
     */
void display_board(vector<vector<Square>> board){

    cout << "-------------------------------------" << endl;

    for (int i = 0; i < 9; i++){

        cout << "| ";

        for (int j = 0; j < 9; j++){
            cout << board.at(i).at(j).get_value() << " | ";
        }

        cout << endl;
        cout << "-------------------------------------" << endl;

    }

}

/**
     * Fills the board by either making the next move or backtracking
     * 
     * @param board: a 2d vector of squares representing the board
     * @param move_path: a stack storing the path to get to the current state
     * @param all_moves: a map storing all moves including ones backtracked
     */
int fill_board(vector<vector<Square>> *board, stack<string> *move_path,
unordered_map<string, string> *all_moves){

    //choose square with smallest domain
    int lowest = 10;
    int chosen_index[2];
    bool index_set = false;
    bool found_square = false;

    //map to keep track of squares tried
    unordered_map<int,int> tested_squares;
    int square_count = 0;

    while (!found_square && square_count < 81){

        for (int i = 0; i < 9; i++){
            for (int j = 0; j < 9; j++){

                int dom_size = board->at(i).at(j).get_domain_size();

                if (dom_size != 0){
                    if (dom_size < lowest){
                        if (tested_squares.find(9*i + j) == tested_squares.end()){
                            lowest = dom_size;
                            chosen_index[0] = i;
                            chosen_index[1] = j;
                            index_set = true;
                        }
                        
                    }
                }
            }
        }

        //if a square was found with a non-zero domain, try to fill it
        if (index_set){

            set<int> temp = board->at(chosen_index[0]).at(chosen_index[1]).get_domain();

            bool found_move = false;

            while (!found_move){

                if (temp.size() == 0){
                    //if all values have been tested, record the square and pick a new one
                    int cell_index = 9*chosen_index[0] + chosen_index[1];
                    pair<int, int> cell (cell_index, cell_index);
                    tested_squares.insert(cell);
                    square_count++;
                    break;
                }

                //find the least constraining available value in the domain
                int chosen_value = least_constraining_value(chosen_index[0], chosen_index[1], board);

                //mark it as being tried and make the move
                temp.erase(chosen_value);
                board->at(chosen_index[0]).at(chosen_index[1]).set_value(chosen_value);
                string new_board = get_board_state(board);

                //check to see if the move results in a board state that's
                //illegal or been seen before
                if (all_moves->find(new_board) == all_moves->end() &&
                check_valid_board(board)){

                    //record the move in the move stack and move record
                    pair<string, string> result (new_board, new_board);
                    all_moves->insert(result);
                    move_path->push(new_board);
                    found_move = true;
                    found_square = true;

                    //return 2 if the board is full, 1 otherwise
                    if (check_full(board)){
                        return 2;
                    } else {
                        return 1;
                    }
                } else {
                    //undo the move and remove it from the domain
                    board->at(chosen_index[0]).at(chosen_index[1]).set_value(0);
                    board->at(chosen_index[0]).at(chosen_index[1]).set_domain(temp);
                }
            }

        //if no square was found, backtrack and return 0
        } else {
            break;
        }

    }

    backtrack(board, move_path);
    return 0;

}

/**
     * Gets a board state from the user
     * 
     * @param board a 2d vector representing board state
     * 
     * @return a bool representing whether or not the board has been filled
     */
bool check_full(vector<vector<Square>> *board){

    for (int i = 0; i < 9; i++){
        for (int j = 0; j < 9; j++){
            if (board->at(i).at(j).get_value() == 0){
                return false;
            }
        }
    }

    return true;
}

/**
     * Gets the least constraining value for a given square
     * 
     * @param row_index the int row of the square
     * @param col_index the int column of the square
     * @param board a 2d vector representing the board
     * 
     * @return an int representing the value
     */
int least_constraining_value(int row_index, int col_index,
vector<vector<Square>> *board){

    set<int> curr_domain = board->at(row_index).at(col_index).get_domain();
    int final_val;
    int min_constraining = 81;

    //test each value in the domain and see how many other squares it impacts
    for (set<int>::iterator it=curr_domain.begin(); it!=curr_domain.end(); ++it)
    {
        int curr_val = *it;
        set<int> temp_domain;
        int constraing_count = 0;

        //count which squares the current value impacts
        for (int k = 0; k < 9; k++){
            if (k != row_index){

                temp_domain = board->at(k).at(col_index).get_domain();
                if (temp_domain.find(curr_val) != temp_domain.end()){
                    constraing_count++;
                }

            }

            if (k != col_index){

                temp_domain = board->at(row_index).at(k).get_domain();
                if (temp_domain.find(curr_val) != temp_domain.end()){
                    constraing_count++;
                }

            }

            int row_square = row_index/3;
            int col_square = col_index/3;
            
            int row = 3*row_square + k%3;
            int col = 3*col_square + k/3;

            if (row != row_index || col != col_index){
                temp_domain = board->at(row).at(col).get_domain();
                if (temp_domain.find(curr_val) != temp_domain.end()){
                    constraing_count++;
                }

            }

            if (constraing_count < min_constraining){
                min_constraining = constraing_count;
                final_val = curr_val;
            }
        }
    }

    return final_val;
}

/**
     * Backtracks the last move in the stack
     * 
     * @param board a 2d vector representing the board
     * @param move_path a stack of all of the moves
     */
void backtrack(vector<vector<Square>> *board, stack<string> *move_path){

    //get the last move in the stack and revert the board to that state
    string previous_state = move_path->top();
    move_path->pop();

    *board = set_board(previous_state);

}

/**
     * Turns the board state into a string representation (the reverse of
     * set_board)
     * 
     * @param board a 2d vector representing the board
     * 
     * @return a string representing the board state
     */
string get_board_state(vector<vector<Square>> *board){

    string board_state = "";

    for (int i = 0; i < 9; i++){
        for (int j = 0; j < 9; j++){
            board_state += char(board->at(i).at(j).get_value() + 48);
        }
    }

    return board_state;

}

/**
     * Checks if the current board state is valid or invalid
     * 
     * @param board a 2d vector representing the board
     * 
     * @return the validity of the board as a bool
     */
bool check_valid_board(vector<vector<Square>> *board){

    for (int i = 0; i < 9; i++){
        for (int j = 0; j < 9; j++){

            int curr_val = board->at(i).at(j).get_value();

            if (curr_val != 0){
                
                //checks for any row, columnm or 3x3 square conflicts
                for (int k = 0; k < 9; k++){
                    if (k != i){
                        int val = board->at(k).at(j).get_value();
                        if(val == curr_val){
                            return false;
                        }
                    }

                    if (k != j){
                        int val = board->at(i).at(k).get_value();
                        if(val == curr_val){
                            return false;
                        }
                    }

                    int row_square = i/3;
                    int col_square = j/3;
                    
                    int row = 3*row_square + k%3;
                    int col = 3*col_square + k/3;


                    if (row != i || col != j){

                        int val = board->at(row).at(col).get_value();
                        if(val == curr_val){
                                return false;
                        }
                    }              
                }
            }
        }
    }

    return true;

}

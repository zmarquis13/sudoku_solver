/*
* Square.hpp - Definition of the square class
* Author: Zeno Marquis
* HW4: Constraint Satisfaction Problems
* 3/31/2024
*/

#include <vector>
#include <set>

//square class definition
class Square{
    
public:
    //instantiates square using a given number
    Square(int input_value){
        
        this->value = input_value;

    }

    //getters and setters for domain and value
    void set_domain(std::set<int> input_domain){

        this->domain = input_domain;

    }

    std::set<int> get_domain(){
        return domain;
    }

    void set_value(int input_value){

        value = input_value;
        domain.clear();
    }

    int get_value(){
        return this->value;
    }

    int get_domain_size(){
        return domain.size();
    }

private:

    int value;
    std::set<int> domain;

};

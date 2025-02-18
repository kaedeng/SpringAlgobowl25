#pragma once

enum class Type {
    TREE,
    TENT,
    NONE
};

class Tile{
    private:

    Type type;
    char tentDir = 'X';
        
    public:

        Tile(Type tileType){
            type = tileType;
        }    

        Type getType(){ return type; };
        void setType(Type type){ this->type = type; };

        char getDir(){ return tentDir; };
        void setDir(char tentDir){ this->tentDir = tentDir; }

};

#include "ttsolver.h"
#include "board.h"

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>

// A single iteration of the solving function
void TTSolver::iterate(){

    // Current generation becomes the parent generation (need to deep copy it)
    std::vector<Board> parentGeneration = std::vector<Board>(currentGeneration);

    #pragma omp parallel for
    for(size_t i = 0; i < generationSize; i+=2){
        std::mt19937 localGen(std::random_device{}());

        // Clone and select parents so it doesn't get changed
        std::vector<Board> parents = selection(parentGeneration, localGen);
        std::vector<Board> children = crossover(parents, localGen);
        for(auto& child : children) {mutation(child, localGen);}

        currentGeneration[i] = children[0];

        // If population is odd
        if (i != generationSize - 1) {
            currentGeneration[i + 1] = children[1];
        }
    }

}

/*
////////////////////////////////////////////////////
Runs on every iteration
////////////////////////////////////////////////////
*/

// Sorting algo for remembering: 
// std::sort(currentGeneration.begin(), currentGeneration.end(), [](const Board& a, const Board& b){return a.getViolations() < b.getViolations();});
std::vector<Board> TTSolver::selection(std::vector<Board> parentPopulation, std::mt19937 &gen){
    std::vector<Board> parents;

    // Tune-able! Lambda function can be based on more than just the violations
    std::sort(parentPopulation.begin(), parentPopulation.end(), 
        [](const Board& a, const Board& b){
            return a.getViolations() < b.getViolations();
        }
    );

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < 2; i++) {
        double r = dist(gen);
        // Raise to a power > 1 to bias toward 0.
        // Can alternatively use the selection factor as r
        int index = static_cast<int>(parentPopulation.size() * std::pow(r, 2));
        parents.push_back(parentPopulation[index]);
    }

    return parents;
}

std::vector<Board> TTSolver::crossover(std::vector<Board> parents, std::mt19937 &gen){
    std::vector<Board> children;

    // Crossover Point
    std::uniform_int_distribution<size_t> dist(1, numTiles - 1);
    size_t crossoverPoint = dist(gen);

    // Create two children from the parents using single point crossover (Bad but im lazy)
    Board parent1 = parents[0];
    Board parent2 = parents[1];

    // Create the children
    Board child1 = parent1;
    Board child2 = parent2;

    
    // Swap the tiles
    size_t currentTileNum = 0;
    for (size_t j = 0; j < numRows; j++){
        for (size_t k = 0; k < numCols; k++){
            
            if(child1.getTile(j, k).getType() != parent2.getTile(j, k).getType() && currentTileNum < crossoverPoint){
                child1.setTile(parent2.getTile(j, k));
            }
            else if(child2.getTile(j, k).getType() != parent1.getTile(j, k).getType() && currentTileNum >= crossoverPoint){
                child2.setTile(parent1.getTile(j, k));
            }

            currentTileNum++;
        }
    }

    // Add the children to the population
    children.push_back(child1);
    children.push_back(child2);

    return children;
}

void TTSolver::mutation(Board& child, std::mt19937 &gen) {
    std::uniform_int_distribution<int> chanceDist(0, 100);
    int randValue = chanceDist(gen);

    if (randValue <= mutationChance) {
        std::uniform_int_distribution<> mutationTypeDist(0, 2);
        int mutationType = mutationTypeDist(gen);

        if (mutationType == 0) {
            if (!child.addTent(gen))
                child.removeTent(gen);
        }
        else if (mutationType == 1) {
            if (!child.removeTent(gen))
                child.addTent(gen);
        }
        else {
            child.moveTent(gen);
        }
    }

    // Generate a new random value for the second (double) mutation.
    int randValue2 = chanceDist(gen);
    if (randValue2 <= mutationChance / 2) {
        std::uniform_int_distribution<> mutationTypeDist(0, 2);
        int mutationType = mutationTypeDist(gen);

        if (mutationType == 0) {
            if (!child.addTent(gen))
                child.removeTent(gen);
        }
        else if (mutationType == 1) {
            if (!child.removeTent(gen))
                child.addTent(gen);
        }
        else {
            child.moveTent(gen);
        }
    }
}

void TTSolver::initialize(){
    numRows = startingBoard.getNumRows();
    numCols = startingBoard.getNumCols();
    numTiles = numRows * numCols;
    
    // Create a random number generator.
    std::mt19937 gen(std::random_device{}());
    
    int count = 1;
    const double k = 1/currentGeneration.size();
    for (auto &board : currentGeneration) {
        for (int i = 0; i < (int)(count * k * board.getNumTiles()); ++i) {
            mutation(board, gen);
        }
        count++;
    }
}

/*
////////////////////////////////////////////////////
Running and Output
////////////////////////////////////////////////////
*/

// Solve function should be the only one you run
void TTSolver::solve(){
    numTiles = numRows * numCols;
    initialize();
    // Loop for a given number of runs
    for(size_t i = 0; i < maxGenerations; i++){
        iterate();
        //mutationChance *= coolingRate;
        std::cout << "iteration: " << i << std::endl;
        std::sort(currentGeneration.begin(), currentGeneration.end(), 
            [](const Board& a, const Board& b) {
                return a.getViolations() < b.getViolations();
            }
        );

        std::cout << currentGeneration[0].getViolations() << std::endl;
    }

    createOutput();
}

bool TTSolver::createOutput(){
    std::sort(currentGeneration.begin(), currentGeneration.end(), 
        [](const Board& a, const Board& b) {
            return a.getViolations() < b.getViolations();
        }
    );
    currentGeneration[0].drawBoard();
    currentGeneration[0].debugPrintViolations();
    return true;
};

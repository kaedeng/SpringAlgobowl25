#include "ttsolver.h"
#include "board.h"
#include <omp.h>

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>

// A single iteration of the solving function
void TTSolver::iterate() {
    // 1) Copy current generation to parentGeneration
    std::vector<Board> parentGeneration = currentGeneration;

    // 2) Sort parentGeneration by fitness (violations). 
    //    The best (fewest violations) will be at index 0,1,2,...
    std::partial_sort(
        parentGeneration.begin(),
        parentGeneration.begin() + elitismNum,
        parentGeneration.end(),
        [](const Board &a, const Board &b) {
            return a.getViolations() < b.getViolations();
        }
    );

    // 3) Elitism: Copy the top 'elitismNum' boards from parentGeneration into currentGeneration.
    for (size_t i = 0; i < elitismNum; i++) {
        currentGeneration[i] = parentGeneration[i];
    }

    // 4) Parallel for the *rest* of the population
    unsigned baseSeed = std::random_device{}();

    #pragma omp parallel
    {
        unsigned seed = baseSeed + omp_get_thread_num();
        std::mt19937 localGen(seed);

        // Fill from 'elitismNum' onward
        #pragma omp for
        for (size_t i = elitismNum; i < generationSize; i += 2) {
            // Perform selection, crossover, and mutation 
            std::vector<Board> parents = selection(parentGeneration, localGen);
            std::vector<Board> children = crossover(parents, localGen);
            for (auto &child : children) {
                mutation(child, localGen);
            }

            currentGeneration[i] = children[0];
            if (i + 1 < generationSize) {
                currentGeneration[i + 1] = children[1];
            }
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

    // std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_int_distribution<size_t> dist(0, parentPopulation.size() - 1);
    for (int i = 0; i < 2; i++) {
        // Num of people in tournament is j < 2
        Board tournamentWinner = parentPopulation[0];
        for (int j = 0; j < 2; j++){
            int index = dist(gen);
            if(j == 0) 
                // First one is default the winner
                tournamentWinner = parentPopulation[index];
            else if(parentPopulation[index].getViolations() < tournamentWinner.getViolations())
                // If the new one is better
                tournamentWinner = parentPopulation[index];
            
        }
        parents.push_back(tournamentWinner);

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
    
    for (int i = 0; i < std::max((int)numTiles/32, 1); i++){

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

    }

}

void TTSolver::initialize(){
    numRows = startingBoard.getNumRows();
    numCols = startingBoard.getNumCols();
    numTiles = numRows * numCols;
    
    // Create a random number generator.
    std::mt19937 gen(std::random_device{}());
    
    // int count = 1;
    // const double k = 1.0/currentGeneration.size();
    // for (auto &board : currentGeneration) {
    //     for (int i = 0; i < std::max((int)(count * k * 32), (int)(numTiles*0.75)); ++i) {
    //         mutation(board, gen);
    //     }
    //     count++;
    // }
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
    
    //if(currentGeneration[0].getViolations() < bestBoard.getViolations()) bestBoard = currentGeneration[0];

    currentGeneration[0].drawBoard();
    currentGeneration[0].debugPrintViolations();
    return true;
};

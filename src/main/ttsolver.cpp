#include "ttsolver.h"
#include "board.h"

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>

void TTSolver::solve(){
    
    numCols = startingBoard.getNumCols();
    numRows = startingBoard.getNumRows();
    numTiles = numRows * numCols;

    for(int i = 0; i < maxGenerations; i++){
        iterate();
    }
    createOutput();
}

void TTSolver::iterate(){
    selection();
    crossover();
    mutation();
    int bestViolations = currentGeneration[0].getViolations();
    bestViolationsHistory.push_back(bestViolations);
}

void TTSolver::selection(){

    size_t totalViolations = 0;
    for(const auto &currentBoard : currentGeneration){
        currentBoard.getViolations();
    }

    // Sort the population by fitness
    std::sort(currentGeneration.begin(), currentGeneration.end(), [](const Board& a, const Board& b){return a.getViolations() < b.getViolations();});

    // Remove the worst half of the population
    currentGeneration.erase(currentGeneration.begin() + generationSize/2, currentGeneration.end());

}

void TTSolver::crossover(){

    // Choose two parents from the population not counting the same children number
    std::shuffle(currentGeneration.begin() + sameChildrenNum, currentGeneration.end(), gen);

    // Pick a random crossover point between 1 and numTiles - 1.
    std::uniform_int_distribution<size_t> dist(1, numTiles - 1);

    // Create two children from the parents using single point crossover (Bad but im lazy)
    for (size_t i = 0; i < generationSize/2; i += 2){
        Board& parent1 = currentGeneration[i];
        Board& parent2 = currentGeneration[i+1];

        size_t crossoverPoint = dist(gen);

        // Create the children
        Board child1 = parent1;
        Board child2 = parent2;

        size_t currentTileNum = 0;
        
        // Swap the tiles
        for (size_t j = 0; j < numRows; j++){
            for (size_t k = 0; k < numCols; k++){
                
                if(child1.getTile(j, k).getType() != parent2.getTile(j, k).getType() && currentTileNum < crossoverPoint){
                    child1.setTile(parent2.getTile(j, k));
                }else if(child2.getTile(j, k).getType() != parent1.getTile(j, k).getType() && currentTileNum >= crossoverPoint){
                    child2.setTile(parent1.getTile(j, k));
                }

                currentTileNum++;
            }
        }

        // Add the children to the population
        currentGeneration.push_back(child1);
        currentGeneration.push_back(child2);

    }

}

void TTSolver::mutation(){

    std::uniform_int_distribution<> random(0, 2);

    // Randomly mutate each child
    for (int i = 0; i < currentGeneration.size(); i++){
        for (int j = 0; j < 1; j++){
            int mutationType = random(gen);
            if (mutationType == 0 && !currentGeneration[i].addTent()){
                currentGeneration[i].removeTent();
            }
            else if (mutationType == 1 && !currentGeneration[i].removeTent()){
                currentGeneration[i].addTent();
            }
            else{
                currentGeneration[i].moveTent();
            }
        }
    }

}

bool TTSolver::createOutput(){

    selection();
    currentGeneration[0].drawBoard();
    currentGeneration[0].debugPrintViolations();

    return true;
};
#include "ttsolver.h"
#include "board.h"
#include "tilesSet.h"
#include <omp.h>

#include <random>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

// A single iteration of the solving function
void TTSolver::iterate() {

    //    The best (fewest violations) will be at index 0,1,2,...
    std::partial_sort(
        parentGeneration.begin(),
        parentGeneration.begin() + elitismNum,
        parentGeneration.end(),
        [](const Board &a, const Board &b) {
            return a.getViolations() < b.getViolations();
        }
    );

    // elitism copies best boards from parentGeneration to currentGeneration
    for (size_t i = 0; i < elitismNum; i++) {
        currentGeneration[i] = std::move(parentGeneration[i]);
    }

    unsigned baseSeed = std::random_device{}();

    #pragma omp parallel
    {
        unsigned seed = baseSeed + omp_get_thread_num();
        std::mt19937 localGen(seed);

        // Fill from 'elitismNum' onward
        #pragma omp for
        for (size_t i = elitismNum; i < generationSize; i += 2) {
            // Perform selection, crossover, and mutation 
            std::pair<size_t, size_t> parents = selection(localGen);
            std::pair<Board, Board> children = crossover(parents, localGen);
            mutation(children, localGen);


            currentGeneration[i] = std::move(children.first);
            if (i + 1 < generationSize) {
                currentGeneration[i + 1] = std::move(children.second);
            }
        }
    }

    std::swap(currentGeneration, parentGeneration);

}

/*
////////////////////////////////////////////////////
Runs on every iteration
////////////////////////////////////////////////////
*/

// Sorting algo for remembering: 
// std::sort(currentGeneration.begin(), currentGeneration.end(), [](const Board& a, const Board& b){return a.getViolations() < b.getViolations();});
std::pair<size_t, size_t> TTSolver::selection(std::mt19937 &gen) {
    std::pair<size_t, size_t> parents1;
    std::pair<size_t, size_t> parents2;
    std::uniform_int_distribution<size_t> dist(0, parentGeneration.size() - 1);

    // First tournament to form the first candidate pair.
    for (int i = 0; i < 2; i++) {
        size_t bestIndex = dist(gen);
        // Run a mini tournament of size 2 (adjust tournament size as needed)
        for (int j = 0; j < 2; j++){
            size_t index = dist(gen);
            if (parentGeneration[index].getViolations() < parentGeneration[bestIndex].getViolations())
                bestIndex = index;
        }
        if (i == 0)
            parents1.first = bestIndex;
        else
            parents1.second = bestIndex;
    }

    // Second tournament to form the second candidate pair.
    for (int i = 0; i < 2; i++) {
        size_t bestIndex = dist(gen);
        for (int j = 0; j < 2; j++){
            size_t index = dist(gen);
            if (parentGeneration[index].getViolations() < parentGeneration[bestIndex].getViolations())
                bestIndex = index;
        }
        if (i == 0)
            parents2.first = bestIndex;
        else
            parents2.second = bestIndex;
    }

    double score1 = weightedPairScore(parentGeneration[parents1.first], parentGeneration[parents1.second]);
    double score2 = weightedPairScore(parentGeneration[parents2.first], parentGeneration[parents2.second]);

    // Lower score indicates a better pair.
    if (score1 < score2)
        return parents1;
    else
        return parents2;
}


double TTSolver::weightedPairScore(const Board &a,const Board &b) {
    // Average the violations of both boards (lower is better)
    double avgViolations = (a.getViolations() + b.getViolations()) / 2.0;
    // Get the Hamming distance (diversity) between the two boards
    double diversity = a.countXorBits(b.getBitBoard());
    // Lower score is better, so subtract diversity-weight
    return (avgViolations/static_cast<double>(numTiles)) - diversityWeight * diversity;
}

std::pair<Board, Board> TTSolver::crossover(std::pair<size_t, size_t>& parents, std::mt19937 &gen) {

    std::pair<Board, Board> childrenBoards = std::make_pair(parentGeneration[parents.first], parentGeneration[parents.second]);

    // Choose a crossover point
    std::uniform_int_distribution<size_t> dist(0, numTiles);
    size_t crossoverPoint = dist(gen);

    size_t tileCount = 0;
    for (size_t r = 0; r < numRows; r++) {
        for (size_t c = 0; c < numCols; c++) {
            if (tileCount >= crossoverPoint) {

                Tile p1Tile = parentGeneration[parents.first].getTile(r, c);
                Tile p2Tile = parentGeneration[parents.second].getTile(r, c);

                if (childrenBoards.first.getTile(r, c).getType() != p2Tile.getType() || ((childrenBoards.first.getTile(r, c).getType() == Type::TENT && p2Tile.getType() == Type::TENT) && childrenBoards.first.getTile(r, c).getDir() != p2Tile.getDir())) {
                    childrenBoards.first.setTile(p2Tile, gen);
                }

                if (childrenBoards.second.getTile(r, c).getType() != p1Tile.getType() || ((childrenBoards.second.getTile(r, c).getType() == Type::TENT && p1Tile.getType() == Type::TENT) && childrenBoards.second.getTile(r, c).getDir() != p1Tile.getDir())) {
                    childrenBoards.second.setTile(p1Tile, gen);
                }

            }
            tileCount++;

        }
    }

    return childrenBoards;
}

void TTSolver::mutation(std::pair<Board, Board>& children, std::mt19937 &gen) {
    std::uniform_int_distribution<int> chanceDist(0, 100);

    auto& board1 = children.first;
    auto& board2 = children.second; 
    
    for (int i = 0; i < std::max((int)initalEmptyTiles/32, 1); i++){

        int randValue = chanceDist(gen);

        if (randValue <= mutationChance) {
            std::uniform_int_distribution<> mutationTypeDist(0, 2);
            int mutationType = mutationTypeDist(gen);

            if (mutationType == 0) {
                if (!board1.addTent(gen))
                    board1.removeTent(gen);
            }
            else if (mutationType == 1) {
                if (!board1.removeTent(gen))
                board1.addTent(gen);
            }
            else {
                board1.moveTent(gen);
            }
        }

    }

    for (int i = 0; i < std::max((int)initalEmptyTiles/32, 1); i++){

        int randValue = chanceDist(gen);

        if (randValue <= mutationChance) {
            std::uniform_int_distribution<> mutationTypeDist(0, 2);
            int mutationType = mutationTypeDist(gen);

            if (mutationType == 0) {
                if (!board2.addTent(gen))
                    board2.removeTent(gen);
            }
            else if (mutationType == 1) {
                if (!board2.removeTent(gen))
                    board2.addTent(gen);
            }
            else {
                board2.moveTent(gen);
            }
        }

    }

}

void TTSolver::initialize(){
    numRows = startingBoard.getNumRows();
    numCols = startingBoard.getNumCols();
    numTiles = numRows * numCols;
    std::uniform_int_distribution<int> dist(0, static_cast<int>(numTiles)/2);
    // #pragma omp parallel
    // {
    //     // Create a random number generator.
    //     std::mt19937 gen(std::random_device{}());

    //     #pragma omp for
    //     for (auto &board : currentGeneration) {
    //         int randomNumberOfTents = dist(gen);  // Random number of tents between 0% to 50% of numTiles
    //         for (int i = 0; i < randomNumberOfTents; ++i) {
    //             board.addTent(gen);
    //         }
    //     }
    // }

}

/*
////////////////////////////////////////////////////
Running and Output
////////////////////////////////////////////////////
*/

// Solve function should be the only one you run
size_t TTSolver::solve(){

    numTiles = numRows * numCols;
    initialize();
    size_t minViolations = startingBoard.getViolations();
    size_t counter = 0;
    // Loop for a given number of runs
    int j = 1;
    for(size_t i = 0; i < 1000000; i++){
        iterate();
        //mutationChance *= coolingRate;

        currentGeneration[0].drawBoard();

        std::cout << "iteration: " << j++ << std::endl;

        if(currentGeneration[0].getViolations() < minViolations){
            minViolations = currentGeneration[0].getViolations(); 
            counter = 0;
        }
        std::cout << minViolations << std::endl;
        if(minViolations == 0 || counter >= maxGenerationsNoImprovement){
            // std::cout << "enter c to continue or p for continue and make output" << std::endl;
            // std::string a;
            // std::cin >> a;
            // if(a == "c"){
            //     counter = 0;
            // }
            // else if (a == "p"){
            //     counter = 0;
            //     createOutput();
            //     return minViolations;
            // }
            // else{
            //     createOutput();
            //     return minViolations;
            // }
            createOutput();
            return minViolations;

        }
        counter++;
    }
}

bool TTSolver::createOutput() {

    std::filesystem::path inputPath(filePath);
    std::string baseName = inputPath.stem().string();
    std::filesystem::path directory = inputPath.parent_path();

    // get output folder name
    std::string outputFolderName = baseName + "_output";
    std::filesystem::path outputFolderPath = directory / outputFolderName;

    // Make new output folder
    if (!std::filesystem::exists(outputFolderPath)) {
        if (!std::filesystem::create_directory(outputFolderPath)) {
            std::cerr << "Error creating output directory: " << outputFolderPath << std::endl;
            return false;
        }
    }

    // random number for name
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 999999);
    int randomIndex = dis(gen);

    // Construct the output file name with the random index.
    std::filesystem::path outFilePath = outputFolderPath / (baseName + '_' + std::to_string(randomIndex) + ".out");
    std::ofstream outFile(outFilePath);
    if (!outFile) {
        std::cerr << "Error opening file for writing: " << outFilePath << std::endl;
        return false;
    }

    // Sort the current generation
    std::sort(currentGeneration.begin(), currentGeneration.end(),
        [](const Board &a, const Board &b) {
            return a.getViolations() < b.getViolations();
        }
    );
    const Board &bestBoard = currentGeneration[0];

    // Write the total number of violations
    outFile << bestBoard.getViolations() << "\n";

    TilesSet bestTentTiles = bestBoard.getTentTilesData();

    // Second line number of tents added
    outFile << bestTentTiles.size() << "\n";

    // tentCount lines of row col dir
    for (int i = 0; i < bestTentTiles.size(); i++) {

        int row = bestTentTiles.getTileAtIndex(i).value().getRow();
        int col = bestTentTiles.getTileAtIndex(i).value().getCol();

        outFile << row + 1 << " " << col + 1 << " " << bestBoard.getBoard()[row][col].getDir() << std::endl;

    }

    outFile.close();
    return true;
}

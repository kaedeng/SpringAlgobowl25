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
            std::vector<Board> parents = selection(parentGeneration, localGen);
            std::vector<Board> children = crossover(parents, localGen);
            for (auto &child : children) {
                mutation(child, localGen);
            }

            currentGeneration[i] = std::move(children[0]);
            if (i + 1 < generationSize) {
                currentGeneration[i + 1] = std::move(children[1]);
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
std::vector<Board> TTSolver::selection(const std::vector<Board>& parentPopulation, std::mt19937 &gen){
    std::vector<Board> parents1;
    std::vector<Board> parents2;
    std::uniform_int_distribution<size_t> dist(0, parentPopulation.size() - 1);

    // First tournament to form the first candidate pair.
    for (int i = 0; i < 2; i++) {
        Board tournamentWinner = parentPopulation[dist(gen)];
        // Run a mini tournament of size 2 (you can increase the tournament size as needed)
        for (int j = 0; j < 2; j++){
            int index = dist(gen);
            if (parentPopulation[index].getViolations() < tournamentWinner.getViolations())
                tournamentWinner = parentPopulation[index];
        }
        parents1.push_back(tournamentWinner);
    }

    // Second tournament to form the second candidate pair.
    for (int i = 0; i < 2; i++) {
        Board tournamentWinner = parentPopulation[dist(gen)];
        for (int j = 0; j < 2; j++){
            int index = dist(gen);
            if (parentPopulation[index].getViolations() < tournamentWinner.getViolations())
                tournamentWinner = parentPopulation[index];
        }
        parents2.push_back(tournamentWinner);
    }

    double score1 = weightedPairScore(parents1[0], parents1[1]);
    double score2 = weightedPairScore(parents2[0], parents2[1]);

    // Lower score better pair
    if (score1 < score2)
        return parents1;
    else
        return parents2;
}

double TTSolver::weightedPairScore(Board &a, Board &b) {
    // Average the violations of both boards (lower is better)
    double avgViolations = (a.getViolations() + b.getViolations()) / 2.0;
    // Get the Hamming distance (diversity) between the two boards
    double diversity = a.countXorBits(b.getBitBoard());
    // Lower score is better, so subtract diversity-weight
    return (avgViolations/static_cast<double>(numTiles)) - diversityWeight * diversity;
}

std::vector<Board> TTSolver::crossover(const std::vector<Board>& parents, std::mt19937 &gen){
    std::vector<Board> children;
    children.reserve(2);

    Board parent1 = parents[0];
    Board parent2 = parents[1];

    // Make child boards as copies of the parents
    Board child1 = parent1;
    Board child2 = parent2;

    // Choose a crossover point
    std::uniform_int_distribution<size_t> dist(0, numTiles);
    size_t crossoverPoint = dist(gen);

    size_t tileCount = 0;
    for (size_t r = 0; r < numRows; r++) {
        for (size_t c = 0; c < numCols; c++) {
            if (tileCount >= crossoverPoint) {

                Tile p1Tile = parent1.getTile(r, c);
                Tile p2Tile = parent2.getTile(r, c);

                if (child1.getTile(r, c).getType() != p2Tile.getType() || ((child1.getTile(r, c).getType() == Type::TENT && p2Tile.getType() == Type::TENT) && child1.getTile(r, c).getDir() != p2Tile.getDir())) {
                    child1.setTile(p2Tile, gen);
                }
                if (child2.getTile(r, c).getType() != p1Tile.getType() || ((child2.getTile(r, c).getType() == Type::TENT && p1Tile.getType() == Type::TENT) && child2.getTile(r, c).getDir() != p1Tile.getDir())) {
                    child2.setTile(p1Tile, gen);
                }
            }
            tileCount++;
        }
    }

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
    std::uniform_int_distribution<int> dist(0, static_cast<int>(numTiles)/2);
    #pragma omp parallel
    {
        // Create a random number generator.
        std::mt19937 gen(std::random_device{}());

        #pragma omp for
        for (auto &board : currentGeneration) {
            int randomNumberOfTents = dist(gen);  // Random number of tents between 0% to 50% of numTiles
            for (int i = 0; i < randomNumberOfTents; ++i) {
                board.addTent(gen);
            }
            board.drawBoard();
        }
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
            createOutput();
            return;
        }
        counter++;
    }
    createOutput();
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
    std::filesystem::path outFilePath = outputFolderPath / (baseName + std::to_string(randomIndex) + ".out");
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

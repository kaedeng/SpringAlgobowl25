#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include "input.h"
#include "ttsolver.h"

void test(char* filePath, Board board);

struct ParamSet {
    int generationSize;
    int maxGenerationsNoImprovement;
    int mutationChance;
    int selectionFactor;
    double coolingRate;
    int elitism;
    double diversity;
};

struct Result {
    ParamSet params;
    double score;
};

void clFlags(Input* input, const std::string& clArg) {
    if (clArg == "--parse") {
        input->testOutput();
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file1> [file2 ...] [options]" << std::endl;
        return 1;
    }

    // Separate file paths and command-line options (those starting with "--")
    for (int i = 0; i < 10000; ++i){
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg.rfind("--", 0) == 0) {
                
            } else {
                Input inputData;
                Board board = inputData.inputFromFile(argv[i]);
                TTSolver solver(argv[i], 100, 50, board, 1, 0, 0, 13, 40);
                solver.solve();
            }
        }
    }


    //test(argv[1], board);

    return 0;
}

void test(char* filePath, Board board) {

    // Define parameter ranges for the grid search.
    std::vector<int> generationSizes = {100};
    std::vector<int> mutationChances = {1};
    std::vector<int> elitisms = {13};
    std::vector<double> diversities = {50};

    // Fixed parameters for those we are not varying.
    const int fixedMaxGenerations = 200;
    const int fixedSelectionFactor = 0;
    const double fixedCoolingRate = 0.0;

    std::vector<Result> results;

    // For each combination, run a few trials (e.g., 3) and take the average score.
    const int trialCount = 3;
    for (int genSize : generationSizes) {
        for (int mutationChance : mutationChances) {
            for (int elitism : elitisms) {
                for (double diversity : diversities) {
                    ParamSet currentParams;
                    currentParams.generationSize = genSize;
                    currentParams.maxGenerationsNoImprovement = fixedMaxGenerations;
                    currentParams.mutationChance = mutationChance;
                    currentParams.selectionFactor = fixedSelectionFactor;
                    currentParams.coolingRate = fixedCoolingRate;
                    currentParams.elitism = elitism;
                    currentParams.diversity = diversity;
                    
                    double totalScore = 0.0;
                    for (int trial = 0; trial < trialCount; ++trial) {
                        // Use the filePath passed to test instead of a hardcoded path.
                        TTSolver solver(filePath,
                                        currentParams.generationSize,
                                        currentParams.maxGenerationsNoImprovement,
                                        board,
                                        currentParams.mutationChance,
                                        currentParams.selectionFactor,
                                        currentParams.coolingRate,
                                        currentParams.elitism,
                                        currentParams.diversity);
                        totalScore += solver.solve();
                    }
                    double avgScore = totalScore / trialCount;

                    // Save the result.
                    Result res;
                    res.params = currentParams;
                    res.score = avgScore;
                    results.push_back(res);
                }
            }
        }
    }

    // Sort results by score (ascending: best score first)
    std::sort(results.begin(), results.end(), [](const Result& a, const Result& b) {
        return a.score < b.score;
    });

    // Output all results in order.
    std::cout << "Results sorted by score (best first):\n";
    for (const auto& res : results) {
        std::cout << "Score: " << res.score
                  << " | Generation Size: " << res.params.generationSize
                  << ", Max Generations: " << res.params.maxGenerationsNoImprovement
                  << ", Mutation Chance: " << res.params.mutationChance << "%"
                  << ", Selection Factor: " << res.params.selectionFactor
                  << ", Cooling Rate: " << res.params.coolingRate
                  << ", Elitism: " << res.params.elitism
                  << ", Diversity: " << res.params.diversity
                  << std::endl;
    }
}

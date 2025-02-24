#include "ttsolver.h"
#include "board.h"
#include <omp.h>
#include <random>
#include <algorithm>
#include <vector>
#include <iostream>
#include <bitset>
#include <limits>

static constexpr std::size_t MAX_BOARD_SIZE = 250*400;

// ======================= NSGA-II Helper Structures & Functions =======================

// Returns true if 'a' dominates 'b' (minimization for both objectives).
bool dominates(const TTSolver::NSGAIndividual &a, const TTSolver::NSGAIndividual &b) {
    return ((a.f1 <= b.f1 && a.f2 <= b.f2) && (a.f1 < b.f1 || a.f2 < b.f2));
}

// Compute NSGA-II ranking and crowding distances for the given population.
// We use the same diversity (niche count) concept as before.
std::vector<TTSolver::NSGAIndividual> nsga2Ranking(const std::vector<Board>& population) {
    size_t popSize = population.size();
    std::vector<TTSolver::NSGAIndividual> individuals(popSize);
    // Precompute each board's bitBoard for computing diversity.
    std::vector<std::bitset<MAX_BOARD_SIZE>> bitBoards(popSize);
    for (size_t i = 0; i < popSize; i++) {
        bitBoards[i] = population[i].getBitBoard();
        individuals[i].board = &population[i];
        individuals[i].f1 = population[i].getViolations();
        individuals[i].f2 = 0.0; // will accumulate niche count
    }
    const double sigma = 10.0; // Similarity threshold (tweak as needed)
    // Compute niche counts over unique pairs.
    for (size_t i = 0; i < popSize; i++) {
        for (size_t j = i + 1; j < popSize; j++) {
            double distance = population[i].countXorBits(bitBoards[j]);
            if (distance < sigma) {
                double share = 1.0 - distance / sigma;
                individuals[i].f2 += share;
                individuals[j].f2 += share;
            }
        }
    }
    
    // ---------------- Non-Dominated Sorting ----------------
    std::vector<std::vector<int>> S(popSize); // For each individual, set of individuals it dominates.
    std::vector<int> n(popSize, 0);           // For each individual, count of individuals that dominate it.
    std::vector<int> rank(popSize, 0);          // Rank (front number) for each individual.
    std::vector<std::vector<int>> fronts;
    fronts.push_back(std::vector<int>());      // Front 1
    
    for (size_t p = 0; p < popSize; p++) {
        S[p].clear();
        n[p] = 0;
        for (size_t q = 0; q < popSize; q++) {
            if (p == q) continue;
            if (dominates(individuals[p], individuals[q])) {
                S[p].push_back(q);
            } else if (dominates(individuals[q], individuals[p])) {
                n[p]++;
            }
        }
        if (n[p] == 0) {
            rank[p] = 1;
            fronts[0].push_back(p);
        }
    }
    
    int currentRank = 1;
    while (!fronts[currentRank - 1].empty()) {
        std::vector<int> nextFront;
        for (int p : fronts[currentRank - 1]) {
            for (int q : S[p]) {
                n[q]--;
                if (n[q] == 0) {
                    rank[q] = currentRank + 1;
                    nextFront.push_back(q);
                }
            }
        }
        if (!nextFront.empty())
            fronts.push_back(nextFront);
        currentRank++;
    }
    
    // Assign computed rank to each individual.
    for (size_t i = 0; i < popSize; i++) {
        individuals[i].rank = rank[i];
    }
    
    // ---------------- Crowding Distance Calculation ----------------
    // For each front, compute crowding distances.
    for (const auto &front : fronts) {
        int frontSize = front.size();
        if (frontSize == 0) continue;
        // Initialize crowding distances.
        for (int idx : front) {
            individuals[idx].crowdingDistance = 0.0;
        }
        // For each objective (f1 and f2).
        for (int m = 0; m < 2; m++) {
            auto compareObjective = [&](int a, int b) -> bool {
                return (m == 0) ? (individuals[a].f1 < individuals[b].f1)
                                : (individuals[a].f2 < individuals[b].f2);
            };
            std::vector<int> sortedFront = front;
            std::sort(sortedFront.begin(), sortedFront.end(), compareObjective);
            // Set boundary individuals to infinite distance.
            individuals[sortedFront.front()].crowdingDistance = std::numeric_limits<double>::infinity();
            individuals[sortedFront.back()].crowdingDistance = std::numeric_limits<double>::infinity();
            
            double objMin, objMax;
            if (m == 0) {
                objMin = individuals[sortedFront.front()].f1;
                objMax = individuals[sortedFront.back()].f1;
            } else {
                objMin = individuals[sortedFront.front()].f2;
                objMax = individuals[sortedFront.back()].f2;
            }
            double range = objMax - objMin;
            if (range == 0) range = 1.0;
            // For interior individuals, sum the normalized differences.
            for (size_t k = 1; k < sortedFront.size() - 1; k++) {
                double prev, next;
                if (m == 0) {
                    prev = individuals[sortedFront[k - 1]].f1;
                    next = individuals[sortedFront[k + 1]].f1;
                } else {
                    prev = individuals[sortedFront[k - 1]].f2;
                    next = individuals[sortedFront[k + 1]].f2;
                }
                individuals[sortedFront[k]].crowdingDistance += (next - prev) / range;
            }
        }
    }
    
    return individuals;
}

// In tournament selection for NSGA-II, we compare two individuals by rank first
// (lower rank is better) and, if equal, by crowding distance (higher is better).
bool compareNSGA(const TTSolver::NSGAIndividual &a, const TTSolver::NSGAIndividual &b) {
    if (a.rank < b.rank)
        return true;
    if (a.rank == b.rank && a.crowdingDistance > b.crowdingDistance)
        return true;
    return false;
}

// ======================= NSGA-II Based Selection Function =======================

// NSGA-II based selection that now takes a precomputed ranking
std::vector<Board> TTSolver::selectionNSGA(
    const std::vector<Board>& parentPopulation,
    const std::vector<NSGAIndividual>& nsgaIndividuals,
    std::mt19937 &gen) 
{
    const size_t popSize = parentPopulation.size();
    const size_t tournamentSize = 5; // adjust as needed
    std::uniform_int_distribution<size_t> indexDist(0, popSize - 1);
    
    auto tournamentSelect = [&]() -> const Board& {
        size_t bestIndex = indexDist(gen);
        for (size_t i = 1; i < tournamentSize; i++) {
            size_t candidateIndex = indexDist(gen);
            if (compareNSGA(nsgaIndividuals[candidateIndex], nsgaIndividuals[bestIndex])) {
                bestIndex = candidateIndex;
            }
        }
        return parentPopulation[bestIndex];
    };
    
    Board parent1 = tournamentSelect();
    Board parent2 = tournamentSelect();
    return { parent1, parent2 };
}

// ======================= Integration into the Main Loop =======================

// In iterate(), compute NSGA ranking once and pass it to selectionNSGA:
void TTSolver::iterate() {
    // 1) Copy current generation to parentGeneration.
    std::vector<Board> parentGeneration = currentGeneration;

    // Elitism (using raw violations sort or you can use NSGA ranking for elitism as well)
    std::partial_sort(
        parentGeneration.begin(),
        parentGeneration.begin() + elitismNum,
        parentGeneration.end(),
        [](const Board &a, const Board &b) {
            return a.getViolations() < b.getViolations();
        }
    );
    for (size_t i = 0; i < elitismNum; i++) {
        currentGeneration[i] = parentGeneration[i];
    }
    
    // Compute NSGA-II ranking only once in serial.
    std::vector<NSGAIndividual> nsgaIndividuals = nsga2Ranking(parentGeneration);
    
    unsigned baseSeed = std::random_device{}();
    #pragma omp parallel
    {
        unsigned seed = baseSeed + omp_get_thread_num();
        std::mt19937 localGen(seed);
        #pragma omp for
        for (size_t i = elitismNum; i < generationSize; i += 2) {
            // Use precomputed NSGA ranking for selection.
            std::vector<Board> parents = selectionNSGA(parentGeneration, nsgaIndividuals, localGen);
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

std::vector<Board> TTSolver::crossover(const std::vector<Board>& parents, std::mt19937 &gen) {
    std::vector<Board> children;
    children.reserve(2);

    Board parent1 = parents[0];
    Board parent2 = parents[1];

    // Make child boards as copies of the parents.
    Board child1 = parent1;
    Board child2 = parent2;

    // Choose a crossover point.
    std::uniform_int_distribution<size_t> dist(0, numTiles);
    size_t crossoverPoint = dist(gen);

    size_t tileCount = 0;
    for (size_t r = 0; r < numRows; r++) {
        for (size_t c = 0; c < numCols; c++) {
            if (tileCount >= crossoverPoint) {
                Tile p1Tile = parent1.getTile(r, c);
                Tile p2Tile = parent2.getTile(r, c);
                if (child1.getTile(r, c).getType() != p2Tile.getType() ||
                    ((child1.getTile(r, c).getType() == Type::TENT &&
                      p2Tile.getType() == Type::TENT) &&
                     child1.getTile(r, c).getDir() != p2Tile.getDir())) {
                    child1.setTile(p2Tile);
                }
                if (child2.getTile(r, c).getType() != p1Tile.getType() ||
                    ((child2.getTile(r, c).getType() == Type::TENT &&
                      p1Tile.getType() == Type::TENT) &&
                     child2.getTile(r, c).getDir() != p1Tile.getDir())) {
                    child2.setTile(p1Tile);
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
    
    for (int i = 0; i < std::max((int)numTiles/32, 1); i++) {
        int randValue = chanceDist(gen);
        if (randValue <= mutationChance) {
            std::uniform_int_distribution<> mutationTypeDist(0, 2);
            int mutationType = mutationTypeDist(gen);
            if (mutationType == 0) {
                if (!child.addTent(gen))
                    child.removeTent(gen);
            } else if (mutationType == 1) {
                if (!child.removeTent(gen))
                    child.addTent(gen);
            } else {
                child.moveTent(gen);
            }
        }
    }
}

void TTSolver::initialize() {
    numRows = startingBoard.getNumRows();
    numCols = startingBoard.getNumCols();
    numTiles = numRows * numCols;
    
    // Create a random number generator.
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(numTiles)/2);
    
    for (auto &board : currentGeneration) {
        int randomNumberOfTents = dist(gen);  // Random number of tents between 0% to 50% of numTiles
        for (int i = 0; i < randomNumberOfTents; ++i) {
            board.addTent(gen);
        }
        board.drawBoard();
    }
}

/*
////////////////////////////////////////////////////
Running and Output
////////////////////////////////////////////////////
*/

void TTSolver::solve() {
    numTiles = numRows * numCols;
    initialize();
    // Loop for a given number of runs.
    for (size_t i = 0; i < maxGenerations; i++) {
        iterate();
        // mutationChance *= coolingRate;

        currentGeneration[0].drawBoard();
        std::cout << "iteration: " << i << std::endl;
        std::cout << currentGeneration[0].getViolations() << std::endl;
    }
    createOutput();
}

bool TTSolver::createOutput() {
    std::sort(currentGeneration.begin(), currentGeneration.end(), 
        [](const Board& a, const Board& b) {
            return a.getViolations() < b.getViolations();
        }
    );
    currentGeneration[0].drawBoard();
    currentGeneration[0].debugPrintViolations();
    return true;
}

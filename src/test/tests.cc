#include <gtest/gtest.h>
#include "../main/board.h"
#include "../main/tile.h"
#include "../main/input.h"

/**
 * @brief Testing if board construction properly works
 * @test getTile()
 * @test Board()
 */
TEST(BasicTest, BoardUnitTests){

  std::vector<size_t> rowTentNum = {1, 2, 3};
  std::vector<size_t> colTentNum = {4, 3, 2, 1};

  std::vector<std::vector<Tile>> boardMap = {
    {Tile(Type::NONE, 0, 0),Tile(Type::NONE, 0, 1),Tile(Type::NONE, 0, 2),Tile(Type::TREE, 0, 3)},
    {Tile(Type::NONE, 1, 0),Tile(Type::TREE, 1, 1),Tile(Type::NONE, 1, 2),Tile(Type::NONE, 1, 3)},
    {Tile(Type::NONE, 2, 0),Tile(Type::NONE, 2, 1),Tile(Type::NONE, 2, 2),Tile(Type::NONE, 2, 3)},
  };

  Board board = Board(3, 4, rowTentNum, colTentNum, boardMap, 2);

  // Check some tiles on the board
  EXPECT_EQ(board.getTile(0, 0).getType(), Type::NONE);
  EXPECT_EQ(board.getTile(0, 3).getType(), Type::TREE);
  EXPECT_EQ(board.getTile(1, 1).getType(), Type::TREE);
  EXPECT_EQ(board.getTile(2, 2).getType(), Type::NONE);
}

/**
 * @brief Tests violation tracking with prebuilt tent/tree board
 * @test Board()
 * @test getViolations()
 */
TEST(ViolationsTest, BoardUnitTests){
  std::vector<size_t> rowTentNum = {0, 0, 3};
  std::vector<size_t> colTentNum = {1, 1, 2, 1};

  std::vector<std::vector<Tile>> boardMap = {
    {Tile(Type::NONE, 0, 0),Tile(Type::TREE, 0, 1),Tile(Type::NONE, 0, 2),Tile(Type::NONE, 0, 3)},
    {Tile(Type::TREE, 1, 0),Tile(Type::NONE, 1, 1),Tile(Type::TREE, 1, 2),Tile(Type::NONE, 1, 3)},
    {Tile(Type::NONE, 2, 0),Tile(Type::NONE, 2, 1),Tile(Type::NONE, 2, 2),Tile(Type::TREE, 2, 3)},
  };

  Board board = Board(3, 4, rowTentNum, colTentNum, boardMap, 4);
  EXPECT_EQ(12, board.getViolations());

  Tile tent1 = Tile(Type::TENT, 0, 0);
  tent1.setDir('R');
  Tile tent2 = Tile(Type::TENT, 0, 3);
  tent2.setDir('X');
  Tile tent3 = Tile(Type::TENT, 1, 1);
  tent3.setDir('L');
  Tile tent4 = Tile(Type::TENT, 2, 2);
  tent4.setDir('U');
  
  boardMap = {
    {tent1,Tile(Type::TREE, 0, 1),Tile(Type::NONE, 0, 2),tent2},
    {Tile(Type::TREE, 1, 0),tent3,Tile(Type::TREE, 1, 2),tent3},
    {Tile(Type::NONE, 2, 0),Tile(Type::NONE, 2, 1),tent4,Tile(Type::TREE, 2, 3)},
  };

  board = Board(3, 4, rowTentNum, colTentNum, boardMap, 4);
  EXPECT_EQ(11, board.getViolations());

  tent1 = Tile(Type::TENT, 0, 0);
  tent1.setDir('R');
  tent2 = Tile(Type::TENT, 1, 1);
  tent2.setDir('L');
  tent3 = Tile(Type::TENT, 1, 3);
  tent3.setDir('D');
  tent4 = Tile(Type::TENT, 2, 2);
  tent4.setDir('U');
  
  boardMap = {
    {tent1,Tile(Type::TREE, 0, 1),Tile(Type::NONE, 0, 2),Tile(Type::NONE, 0, 3)},
    {Tile(Type::TREE, 1, 0),tent2,Tile(Type::TREE, 1, 2),tent3},
    {Tile(Type::NONE, 2, 0),Tile(Type::NONE, 2, 1),tent4,Tile(Type::TREE, 2, 3)},
  };

  board = Board(3, 4, rowTentNum, colTentNum, boardMap, 4);

  EXPECT_EQ(10, board.getViolations());
}

TEST(DebugOutputTest, IOTest){
  std::string filePath = "../tests/one.test";

  Input input;
  Board board = input.inputFromFile(filePath);

  // Redirect std::cout to capture the output of testOutput().
  std::stringstream buffer;
  std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
  input.testOutput();
  std::cout.rdbuf(old); // Restore std::cout

  std::string output = buffer.str();
  std::string expected;
  expected += "3 4\n";
  expected += "0 0 3 \n";
  expected += "1 1 2 1 \n";
  expected += "2022\n";
  expected += "0202\n";
  expected += "2220\n";

  EXPECT_EQ(output, expected);
}

// Helper function to compare two Board objects.
bool boardsAreEqual(const Board& boardA, const Board& boardB, size_t rows, size_t cols,
  const std::vector<size_t>& expectedRowTents,
  const std::vector<size_t>& expectedColTents,
  size_t expectedNumTrees) {

  // Compare board dimensions and each tile type.
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      if (boardA.getTile(i, j).getType() != boardB.getTile(i, j).getType()) {
        return false;
      }
    }
  }
  return true;
}

TEST(BoardEquivalence, IOTest){
  std::string filePath = "../tests/one.test";

  Input input;
  Board board = input.inputFromFile(filePath);

  // Redirect std::cout to capture the output of testOutput().
  std::stringstream buffer;
  std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
  input.testOutput();
  std::cout.rdbuf(old); // Restore std::cout

  std::string output = buffer.str();
  std::string expected;
  expected += "3 4\n";
  expected += "0 0 3 \n";
  expected += "1 1 2 1 \n";
  expected += "2022\n";
  expected += "0202\n";
  expected += "2220\n";

  EXPECT_EQ(output, expected);
}

TEST(BoardEquivalence, CompareBoardObjects) {
  std::string filePath = "../tests/one.test";
  Input input;
  Board generatedBoard = input.inputFromFile(filePath);

  // Board object with the same data.
  const size_t rows = 3;
  const size_t cols = 4;
  std::vector<size_t> rowTents = {0, 0, 3};
  std::vector<size_t> colTents = {1, 1, 2, 1};
  size_t numTrees = 4;

  std::vector<std::vector<Tile>> prebuiltTiles;
  prebuiltTiles.push_back(  { Tile(Type::NONE, 0, 0), Tile(Type::TREE, 0, 1), Tile(Type::NONE, 0, 2), Tile(Type::NONE, 0, 3) }  );
  prebuiltTiles.push_back(  { Tile(Type::TREE, 1, 0), Tile(Type::NONE, 1, 1), Tile(Type::TREE, 1, 2), Tile(Type::NONE, 1, 3) }  );
  prebuiltTiles.push_back(  { Tile(Type::NONE, 2, 0), Tile(Type::NONE, 2, 1), Tile(Type::NONE, 2, 2), Tile(Type::TREE, 2, 3) }  );
  
  Board prebuiltBoard(rows, cols, rowTents, colTents, prebuiltTiles, numTrees);

  // Compare the two board objects
  for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
          EXPECT_EQ(prebuiltBoard.getTile(i, j).getType(), generatedBoard.getTile(i, j).getType())
              << "Mismatch at tile (" << i << ", " << j << ")";
      }
  }
}

TEST(BoardPrint, PrintBoard) {

  srand(1);

  std::string filePath = "../tests/one.test";
  Input input;
  Board generatedBoard = input.inputFromFile(filePath);

  std::stringstream buffer;
  std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
  generatedBoard.drawBoard();
  std::cout.rdbuf(old);

  std::string output = buffer.str();
  std::string expected;
  expected += "     1  1  2  1 \n";
  expected += "   +------------+\n";
  expected += " 0 | .  T  .  . |\n";
  expected += " 0 | T  .  T  . |\n";
  expected += " 3 | .  .  .  T |\n";
  expected += "   +------------+\n";

  EXPECT_EQ(output, expected);

  generatedBoard.setTile(Tile(Type::TREE, 0, 0));

  buffer.str("");
  old = std::cout.rdbuf(buffer.rdbuf());
  generatedBoard.drawBoard();
  std::cout.rdbuf(old);

  output = buffer.str();
  expected = "";
  expected += "     1  1  2  1 \n";
  expected += "   +------------+\n";
  expected += " 0 | T  T  .  . |\n";
  expected += " 0 | T  .  T  . |\n";
  expected += " 3 | .  .  .  T |\n";
  expected += "   +------------+\n";

  EXPECT_EQ(output, expected);

}

TEST(AddTent, TentMoves) {
  std::string filePath = "../tests/onetile.test";
  Input input;
  Board generatedBoard = input.inputFromFile(filePath);

  generatedBoard.addTent();

  EXPECT_EQ(generatedBoard.getTile(0, 0).getType(), Type::TENT);

  filePath = "../tests/ninetiletree.test";
  generatedBoard = input.inputFromFile(filePath);

  generatedBoard.addTent();
  generatedBoard.addTent();
  generatedBoard.addTent();
  generatedBoard.addTent();

  EXPECT_EQ(generatedBoard.addTent(), false);

  generatedBoard.removeTent();
  generatedBoard.removeTent();
  generatedBoard.removeTent();
  generatedBoard.removeTent();

  EXPECT_EQ(generatedBoard.moveTent(), false);


}

TEST(RemoveTent, TentMoves){
  std::string filePath = "../tests/onetile.test";
  Input input;
  Board generatedBoard = input.inputFromFile(filePath);

  generatedBoard.setTile(Tile(Type::TENT, 0, 0));

  generatedBoard.removeTent();

  EXPECT_EQ(generatedBoard.getTile(0, 0).getType(), Type::NONE);

  EXPECT_EQ(generatedBoard.removeTent(), false);

}

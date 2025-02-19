#include <gtest/gtest.h>
#include "../main/board.h"
#include "../main/tile.h"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

//
TEST(BasicTest, BoardUnitTests){

  std::vector<size_t> rowTentNum = {1, 2, 3};
  std::vector<size_t> colTentNum = {4, 3, 2, 1};

  std::vector<std::vector<Tile>> boardMap = {
    {Tile(Type::NONE, 0, 0),Tile(Type::NONE, 0, 1),Tile(Type::NONE, 0, 2),Tile(Type::TREE, 0, 3)},
    {Tile(Type::NONE, 1, 0),Tile(Type::TREE, 1, 1),Tile(Type::NONE, 1, 2),Tile(Type::NONE, 1, 3)},
    {Tile(Type::NONE, 2, 0),Tile(Type::NONE, 2, 1),Tile(Type::NONE, 2, 2),Tile(Type::NONE, 2, 3)},
  };

  Board board = Board(3, 4, rowTentNum, colTentNum, boardMap, 2);

  // EXPECT_EQ(board[0][0].getType(), Type::NONE);
  // EXPECT_EQ(board[1][1].getType(), Type::TREE);

}

TEST(ViolationsTest, BoardUnitTests){
  std::vector<size_t> rowTentNum = {0, 0, 3};
  std::vector<size_t> colTentNum = {1, 1, 2, 1};

  std::vector<std::vector<Tile>> boardMap = {
    {Tile(Type::NONE, 0, 0),Tile(Type::TREE, 0, 1),Tile(Type::NONE, 0, 2),Tile(Type::NONE, 0, 3)},
    {Tile(Type::TREE, 1, 0),Tile(Type::NONE, 1, 1),Tile(Type::TREE, 1, 2),Tile(Type::NONE, 1, 3)},
    {Tile(Type::NONE, 2, 0),Tile(Type::NONE, 2, 1),Tile(Type::NONE, 2, 2),Tile(Type::TREE, 2, 3)},
  };

  Board board = Board(3, 4, rowTentNum, colTentNum, boardMap, 4);

  EXPECT_EQ(12, board.checkViolations());

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

  EXPECT_EQ(11, board.checkViolations());

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

  EXPECT_EQ(10, board.checkViolations());
}
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
    {Tile(Type::NONE),Tile(Type::NONE),Tile(Type::NONE),Tile(Type::TREE)},
    {Tile(Type::NONE),Tile(Type::TREE),Tile(Type::NONE),Tile(Type::NONE)},
    {Tile(Type::NONE),Tile(Type::NONE),Tile(Type::NONE),Tile(Type::NONE)},
  };

  Board board = Board(3, 4, rowTentNum, colTentNum, boardMap);

  // EXPECT_EQ(board[0][0].getType(), Type::NONE);
  // EXPECT_EQ(board[1][1].getType(), Type::TREE);

}
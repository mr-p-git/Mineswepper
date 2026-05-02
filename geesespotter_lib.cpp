#include <cstdlib>
#include <iostream>
#include <cctype>
#include <ctime>      // for time(NULL)
#include "geesespotter_lib.h"
#include "geesespotter.h"

int main()
{
  srand(time(NULL));
  game();
  return 0;
}

bool game()
{
  std::size_t xdim {0};
  std::size_t ydim {0};
  unsigned int numgeese {0};
  char * gameBoard {NULL};

  startGame(gameBoard, xdim, ydim, numgeese);

  char currAction {0};
  while(currAction != 'Q')
  {
    switch (currAction)
    {
      case 'S' :  // show
      {
        actionShow(gameBoard, xdim, ydim, numgeese);
        break;
      }
      case 'M' :  // mark
      {
        actionMark(gameBoard, xdim, ydim);
        break;
      }
      case 'R' :  // restart
      {
        std::cout << "Restarting the game." << std::endl;
        startGame(gameBoard, xdim, ydim, numgeese);
        break;
      }
    }

    printBoard(gameBoard, xdim, ydim);
    if (isGameWon(gameBoard, xdim, ydim))
    {
      std::cout << "You have revealed all the fields without disturbing a goose!" << std::endl;
      std::cout << "YOU WON!!!" << std::endl;
      for (std::size_t reveal_row {0}; reveal_row < ydim; reveal_row++)
      {
        for (std::size_t reveal_col {0}; reveal_col < xdim; reveal_col++)
        {
          gameBoard[reveal_row*xdim + reveal_col] = (gameBoard[reveal_row*xdim + reveal_col] & valueMask());
        }
      }
      printBoard(gameBoard, xdim, ydim);
      std::cout << "Resetting the game board." << std::endl;
      startGame(gameBoard, xdim, ydim, numgeese);
    }
    currAction = getAction();
  }

  cleanBoard(gameBoard);
  return true;
}

void startGame(char * & board, std::size_t & xdim, std::size_t & ydim, unsigned int & numgeese)
{
  std::cout << "Welcome to GeeseSpotter!" << std::endl;
  std::cout << "Set the board size and number of geese to begin." << std::endl;
  std::cout << "During the game, use S to show, M to mark, R to restart, and Q to quit." << std::endl;
  do
  {
    do {
      std::cout << "Please enter the x dimension (2-" << xdim_max() << "): ";
      xdim = readSizeT();
      if (xdim < 2 || xdim > xdim_max())
      {
        std::cout << "Number entered is invalid." << std::endl;
      }
    } while(xdim < 2 || xdim > xdim_max());
    do {
      std::cout << "Please enter the y dimension (2-" << ydim_max() << "): ";
      ydim = readSizeT();
      if (ydim < 2 || ydim > ydim_max())
      {
        std::cout << "Number entered is invalid." << std::endl;
      }
    } while(ydim < 2 || ydim > ydim_max());
  } while (xdim * ydim < 4);

  std::size_t maxGeese {xdim * ydim - 1};
  std::size_t geeseInput {0};
  std::cout << "Please enter the number of geese (1-" << maxGeese << "): ";
  geeseInput = readSizeT();
  while (geeseInput < 1 || geeseInput > maxGeese)
  {
    std::cout << "Number entered is invalid." << std::endl;
    std::cout << "Please enter the number of geese (1-" << maxGeese << "): ";
    geeseInput = readSizeT();
  }
  numgeese = geeseInput;

  cleanBoard(board);
  board = createBoard(xdim, ydim);
  spreadGeese(board, xdim, ydim, numgeese);
  computeNeighbors(board, xdim, ydim);
  hideBoard(board, xdim, ydim);
}

char getAction()
{
  char action {0};

  std::cout << "Please enter the action ([S]how, [M]ark, [R]estart, [Q]uit): ";
  if (!(std::cin >> action))
  {
    return 'Q';
  }

  if (islower(action))
    action = toupper(action);

  return action;
}

void actionShow(char * & board, std::size_t & xdim, std::size_t & ydim, unsigned int & numgeese)
{
  std::size_t reveal_x {0};
  std::size_t reveal_y {0};
  std::cout << "Please enter the x location to show: ";
  reveal_x = readSizeT();
  std::cout << "Please enter the y location to show: ";
  reveal_y = readSizeT();

  if (reveal_x >= xdim || reveal_y >= ydim)
  {
    std::cout << "Location entered is not on the board." << std::endl;
  }
  else if (board[xdim*reveal_y + reveal_x] & markedBit())
  {
    std::cout << "Location is marked, and therefore cannot be revealed." << std::endl;
    std::cout << "Use Mark on location to unmark." << std::endl;
  }
  else if (reveal(board, xdim, ydim, reveal_x, reveal_y) == 9)
  {
    std::cout << "You disturbed a goose! Your game has ended." << std::endl;
    printBoard(board, xdim, ydim);
    std::cout << "Starting a new game." << std::endl;
    startGame(board, xdim, ydim, numgeese);
  }
}

void actionMark(char * board, std::size_t xdim, std::size_t ydim)
{
  std::size_t mark_x {0};
  std::size_t mark_y {0};
  std::cout << "Please enter the x location to mark: ";
  mark_x = readSizeT();
  std::cout << "Please enter the y location to mark: ";
  mark_y = readSizeT();

  if (mark_x >= xdim || mark_y >= ydim)
  {
    std::cout << "Location entered is not on the board." << std::endl;
  }
  else if (mark(board, xdim, ydim, mark_x, mark_y) == 2)
  {
    std::cout << "Position already revealed, so cannot be marked." << std::endl;
  }
}

std::size_t readSizeT()
{
  std::size_t value {0};

  while (!(std::cin >> value))
  {
    if (std::cin.eof())
    {
      std::cout << std::endl << "Input ended. Exiting game." << std::endl;
      std::exit(0);
    }

    std::cout << "Number entered is invalid." << std::endl;
    std::cout << "Please enter a number: ";
    std::cin.clear();
    std::cin.ignore(10000, '\n');
  }

  return value;
}

std::size_t xdim_max()
{
  return 60;
}

std::size_t ydim_max()
{
  return 60;
}

char markedBit()
{
  return 0x10;
}

char hiddenBit()
{
  return 0x20;
}

char valueMask()
{
  return 0x0F;
}

void spreadGeese(char * board, std::size_t xdim, std::size_t ydim, unsigned int numgeese)
{
  if (board != NULL)
  {
    for (unsigned int gen_goose {0}; gen_goose < numgeese; gen_goose++)
    {
      std::size_t try_position {0};
      do {
         try_position = rand() % (xdim * ydim);
      } while (board[try_position] != 0);

      board[try_position] = 9;
    }
  }
}

//change 

#include "geesespotter_lib.h"

char* createBoard(std::size_t xdim, std::size_t ydim){
	std::size_t capacity = xdim * ydim;

	char* newBoard = new char [capacity];
	for (std::size_t k = 0; k < capacity; ++k) {
		newBoard[k] = 0;
	}
	return newBoard;
}

void computeNeighborsHelper(char* board, std::size_t xdim, std::size_t ydim, std::size_t xloc, std::size_t yloc) {
	int counter = 0; 

	if ((board[yloc * xdim + xloc] & valueMask()) == 0x09) {
		return; 
	}

	std::size_t start_x = xloc;
	std::size_t end_x = xloc;
	std::size_t start_y = yloc;
	std::size_t end_y = yloc;

	if (xloc > 0) {
		start_x = xloc - 1;
	}
	if (xloc + 1 < xdim) {
		end_x = xloc + 1;
	}
	if (yloc > 0) {
		start_y = yloc - 1;
	}
	if (yloc + 1 < ydim) {
		end_y = yloc + 1;
	}

	for (std::size_t y = start_y; y <= end_y; ++y) {
		for (std::size_t x = start_x; x <= end_x; ++x) {
			if ((board[y * xdim + x] & valueMask()) == 0x09) {
				++counter;
			}
		}
	}
	board[yloc * xdim + xloc] += counter; 
	return; 
}

void computeNeighbors(char* board, std::size_t xdim, std::size_t ydim) {
	for (std::size_t index = 0; index < xdim * ydim; ++index) {
		computeNeighborsHelper(board, xdim, ydim, index % xdim, index / xdim);
	}
	return; 
}

void hideBoard(char* board, std::size_t xdim, std::size_t ydim) {
	for (std::size_t k = 0; k < xdim * ydim; ++k) {  
		board[k] |= hiddenBit();
	}
	return; 
}

void cleanBoard(char* board) {

	delete[] board;
	board = nullptr;
	
}

void printBoard(char* board, std::size_t xdim, std::size_t ydim) {
	for (std::size_t k = 0; k < ydim * xdim; k = k+xdim) {
		for (std::size_t i = 0; i < xdim; ++i) {
			if ((board[k + i] & markedBit()) == markedBit()) {
			std::cout << "M";
			}
			 else if ((board[k + i] & hiddenBit()) == hiddenBit()) {
				std::cout << "*";
			}
			 else {
				std::cout << (int)board[k + i]; 
			}
		}
		std::cout <<std::endl ;
	}
}

int reveal(char* board, std::size_t xdim, std::size_t ydim, std::size_t xloc, std::size_t yloc) {

	if ((board[yloc * xdim + xloc] & markedBit()) == markedBit()) {
		return 1;
	}

	else if ((board[yloc * xdim + xloc] & valueMask()) == board[yloc * xdim + xloc]) {
		return 2;
	}

	else if ((board[yloc * xdim + xloc] & valueMask()) == 0x09) {
		board[yloc * xdim + xloc] &= valueMask();
		return 9;
	}
	//only when adj value is zero
	
	else {
		board[yloc * xdim + xloc] &= valueMask();

		if ((board[yloc * xdim + xloc] & valueMask()) == 0x00) {
			std::size_t start_x = xloc;
			std::size_t end_x = xloc;
			std::size_t start_y = yloc;
			std::size_t end_y = yloc;

			if (xloc > 0) {
				start_x = xloc - 1;
			}
			if (xloc + 1 < xdim) {
				end_x = xloc + 1;
			}
			if (yloc > 0) {
				start_y = yloc - 1;
			}
			if (yloc + 1 < ydim) {
				end_y = yloc + 1;
			}

			for (std::size_t rev_y = start_y; rev_y <= end_y; ++rev_y) {
				for (std::size_t rev_x = start_x; rev_x <= end_x; ++rev_x) {
					if ((board[rev_y * xdim + rev_x] & markedBit()) != markedBit()) {
						board[rev_y * xdim + rev_x] &= valueMask();
					}				
				}
			}
		}
	}
	

	return 0;

}

int mark(char* board, std::size_t xdim, std::size_t ydim, std::size_t xloc, std::size_t yloc) { 



	if (xloc >= xdim || yloc >= ydim) {
		return 1;
	}

	if ((board[yloc * xdim + xloc] & valueMask()) == board[yloc * xdim + xloc]) {
		return 2;
	}
	else if ((board[yloc * xdim + xloc] & markedBit()) == markedBit()) {
		board[yloc * xdim + xloc] ^= markedBit(); 
		return 0 ;
	}

	board[yloc * xdim + xloc] |= markedBit();

	return 0;
}

bool isGameWon(char* board, std::size_t xdim, std::size_t ydim) {
	bool allSafeCellsRevealed = true;
	bool allGeeseMarked = true;
	bool hasGoose = false;

	for (std::size_t i = 0; i < xdim * ydim; ++i) {
		bool isHidden = (board[i] & hiddenBit()) == hiddenBit();
		bool isMarked = (board[i] & markedBit()) == markedBit();
		bool isGoose = (board[i] & valueMask()) == 0x09;

		if (isGoose) {
			hasGoose = true;
		}

		if (isHidden && !isGoose) {
			allSafeCellsRevealed = false;
		}

		if ((isGoose && !isMarked) || (!isGoose && isMarked)) {
			allGeeseMarked = false;
		}
	}

	return allSafeCellsRevealed || (hasGoose && allGeeseMarked);		
}

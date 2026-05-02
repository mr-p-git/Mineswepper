#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>
#include "geesespotter.h"

const int ID_XDIM = 101;
const int ID_YDIM = 102;
const int ID_GEESE = 103;
const int ID_START = 104;
const int ANIMATION_TIMER = 201;

const int TOP_SPACE = 205;
const int LEFT_SPACE = 28;
const int CELL_SIZE = 30;
const int MIN_WINDOW_WIDTH = 650;
const int DEFAULT_WINDOW_HEIGHT = 720;

char* board = NULL;
std::size_t xdim = 8;
std::size_t ydim = 8;
unsigned int numgeese = 10;
bool gameOver = false;
int scrollX = 0;
int scrollY = 0;
std::string statusText = "Enter X, Y, and Geese, then click Start Game.";
std::string setupText = "Customize X, Y, and Geese, then press Start Game.";
std::string helpLine1 = "Left click: reveal a cell.";
std::string helpLine2 = "Right click: mark or unmark a cell.";
std::string helpLine3 = "Blank revealed cell: 0 nearby geese.";
std::string helpLine4 = "Close the window to quit.";
int animationType = 0;
int animationStep = 0;

HWND xdimBox = NULL;
HWND ydimBox = NULL;
HWND geeseBox = NULL;
HFONT titleFont = NULL;
HFONT textFont = NULL;
HFONT cellFont = NULL;

int largerInt(int a, int b)
{
  if (a > b)
  {
    return a;
  }
  return b;
}

int smallerInt(int a, int b)
{
  if (a < b)
  {
    return a;
  }
  return b;
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

void spreadGeese(char* board, std::size_t xdim, std::size_t ydim, unsigned int numgeese)
{
  if (board != NULL)
  {
    for (unsigned int gen_goose = 0; gen_goose < numgeese; ++gen_goose)
    {
      std::size_t try_position = 0;
      do {
        try_position = rand() % (xdim * ydim);
      } while (board[try_position] != 0);

      board[try_position] = 9;
    }
  }
}

int readBoxNumber(HWND box)
{
  char text[32];
  GetWindowTextA(box, text, 32);
  return atoi(text);
}

void revealBoard()
{
  for (std::size_t row = 0; row < ydim; ++row)
  {
    for (std::size_t col = 0; col < xdim; ++col)
    {
      board[row * xdim + col] = board[row * xdim + col] & valueMask();
    }
  }
}

void updateScrollBars(HWND window)
{
  RECT client;
  GetClientRect(window, &client);

  int boardWidth = (int)xdim * CELL_SIZE;
  int boardHeight = (int)ydim * CELL_SIZE;
  int pageWidth = largerInt(1, client.right - LEFT_SPACE);
  int pageHeight = largerInt(1, client.bottom - TOP_SPACE);
  int maxScrollX = largerInt(0, boardWidth - pageWidth);
  int maxScrollY = largerInt(0, boardHeight - pageHeight);

  scrollX = smallerInt(scrollX, maxScrollX);
  scrollY = smallerInt(scrollY, maxScrollY);

  SCROLLINFO horizontal = {};
  horizontal.cbSize = sizeof(SCROLLINFO);
  horizontal.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  horizontal.nMin = 0;
  horizontal.nMax = boardWidth;
  horizontal.nPage = pageWidth;
  horizontal.nPos = scrollX;
  SetScrollInfo(window, SB_HORZ, &horizontal, TRUE);

  SCROLLINFO vertical = {};
  vertical.cbSize = sizeof(SCROLLINFO);
  vertical.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  vertical.nMin = 0;
  vertical.nMax = boardHeight;
  vertical.nPage = pageHeight;
  vertical.nPos = scrollY;
  SetScrollInfo(window, SB_VERT, &vertical, TRUE);
}

void scrollWindow(HWND window, int bar, WPARAM wParam)
{
  RECT client;
  GetClientRect(window, &client);

  int boardSize = 0;
  int pageSize = 0;
  int* scrollValue = NULL;

  if (bar == SB_HORZ)
  {
    boardSize = (int)xdim * CELL_SIZE;
    pageSize = largerInt(1, client.right - LEFT_SPACE);
    scrollValue = &scrollX;
  }
  else
  {
    boardSize = (int)ydim * CELL_SIZE;
    pageSize = largerInt(1, client.bottom - TOP_SPACE);
    scrollValue = &scrollY;
  }

  int maxScroll = largerInt(0, boardSize - pageSize);
  int newPosition = *scrollValue;

  switch (LOWORD(wParam))
  {
    case SB_LINELEFT:
      newPosition -= CELL_SIZE;
      break;
    case SB_LINERIGHT:
      newPosition += CELL_SIZE;
      break;
    case SB_PAGELEFT:
      newPosition -= pageSize;
      break;
    case SB_PAGERIGHT:
      newPosition += pageSize;
      break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
    {
      SCROLLINFO info = {};
      info.cbSize = sizeof(SCROLLINFO);
      info.fMask = SIF_TRACKPOS;
      GetScrollInfo(window, bar, &info);
      newPosition = info.nTrackPos;
      break;
    }
  }

  if (newPosition < 0)
  {
    newPosition = 0;
  }
  if (newPosition > maxScroll)
  {
    newPosition = maxScroll;
  }

  *scrollValue = newPosition;
  updateScrollBars(window);
  InvalidateRect(window, NULL, FALSE);
}

void startNewGame(HWND window)
{
  int newX = readBoxNumber(xdimBox);
  int newY = readBoxNumber(ydimBox);
  int newGeese = readBoxNumber(geeseBox);

  if (newX < 2 || newX > 60 || newY < 2 || newY > 60)
  {
    statusText = "Invalid board size. X and Y must both be 2-60.";
    InvalidateRect(window, NULL, TRUE);
    return;
  }

  if (newGeese < 1 || newGeese >= newX * newY)
  {
    statusText = "Invalid goose count. Use 1 through one less than the number of cells.";
    InvalidateRect(window, NULL, TRUE);
    return;
  }

  cleanBoard(board);
  xdim = newX;
  ydim = newY;
  numgeese = newGeese;
  board = createBoard(xdim, ydim);
  spreadGeese(board, xdim, ydim, numgeese);
  computeNeighbors(board, xdim, ydim);
  hideBoard(board, xdim, ydim);
  gameOver = false;
  scrollX = 0;
  scrollY = 0;
  animationType = 0;
  animationStep = 0;
  KillTimer(window, ANIMATION_TIMER);
  statusText = "Game started. Left click to reveal. Right click to mark.";

  SetWindowPos(window, NULL, 0, 0, MIN_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SWP_NOMOVE | SWP_NOZORDER);
  updateScrollBars(window);
  InvalidateRect(window, NULL, FALSE);
}

void startAnimation(HWND window, int type)
{
  animationType = type;
  animationStep = 0;
  SetTimer(window, ANIMATION_TIMER, 120, NULL);
}

void fillRect(HDC hdc, RECT rect, COLORREF color)
{
  HBRUSH brush = CreateSolidBrush(color);
  FillRect(hdc, &rect, brush);
  DeleteObject(brush);
}

void drawPage(HDC hdc)
{
  RECT background = {0, 0, 5000, 5000};
  RECT header = {16, 12, MIN_WINDOW_WIDTH - 18, 122};
  RECT instructions = {16, 130, MIN_WINDOW_WIDTH - 18, 190};

  fillRect(hdc, background, RGB(239, 245, 241));
  fillRect(hdc, header, RGB(222, 235, 226));
  fillRect(hdc, instructions, RGB(255, 252, 242));

  HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(180, 196, 184));
  HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
  Rectangle(hdc, header.left, header.top, header.right, header.bottom);
  Rectangle(hdc, instructions.left, instructions.top, instructions.right, instructions.bottom);
  SelectObject(hdc, oldBrush);
  SelectObject(hdc, oldPen);
  DeleteObject(borderPen);

  SelectObject(hdc, titleFont);
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, RGB(45, 78, 64));
  TextOutA(hdc, 28, 24, "GeeseSpotter", 12);

  SelectObject(hdc, textFont);
  SetTextColor(hdc, RGB(60, 60, 60));
  TextOutA(hdc, 28, 56, statusText.c_str(), statusText.length());
  TextOutA(hdc, 28, 78, setupText.c_str(), setupText.length());

  SetTextColor(hdc, RGB(70, 80, 80));
  TextOutA(hdc, 28, 140, helpLine1.c_str(), helpLine1.length());
  TextOutA(hdc, 28, 154, helpLine2.c_str(), helpLine2.length());
  TextOutA(hdc, 28, 168, helpLine3.c_str(), helpLine3.length());
  TextOutA(hdc, 300, 168, helpLine4.c_str(), helpLine4.length());
}

void drawBoard(HDC hdc)
{
  if (board == NULL)
  {
    return;
  }

  HBRUSH hiddenBrush = CreateSolidBrush(RGB(83, 145, 161));
  HBRUSH openBrush = CreateSolidBrush(RGB(250, 249, 239));
  HBRUSH markBrush = CreateSolidBrush(RGB(242, 184, 75));
  HBRUSH gooseBrush = CreateSolidBrush(RGB(211, 92, 86));
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hiddenBrush);
  HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(24, 55, 63));
  HPEN oldPen = (HPEN)SelectObject(hdc, gridPen);

  SetBkMode(hdc, TRANSPARENT);
  SelectObject(hdc, cellFont);

  for (std::size_t row = 0; row < ydim; ++row)
  {
    for (std::size_t col = 0; col < xdim; ++col)
    {
      int left = LEFT_SPACE + col * CELL_SIZE - scrollX;
      int top = TOP_SPACE + row * CELL_SIZE - scrollY;
      RECT cellRect = {left, top, left + CELL_SIZE, top + CELL_SIZE};

      if (cellRect.right < LEFT_SPACE || cellRect.bottom < TOP_SPACE)
      {
        continue;
      }
      char cell = board[row * xdim + col];
      bool isMarked = (cell & markedBit()) == markedBit();
      bool isHidden = (cell & hiddenBit()) == hiddenBit();
      int value = cell & valueMask();

      if (isMarked)
      {
        SelectObject(hdc, markBrush);
      }
      else if (isHidden)
      {
        SelectObject(hdc, hiddenBrush);
      }
      else if (value == 9)
      {
        SelectObject(hdc, gooseBrush);
      }
      else
      {
        SelectObject(hdc, openBrush);
      }

      RoundRect(hdc, cellRect.left, cellRect.top, cellRect.right, cellRect.bottom, 5, 5);

      if (isMarked)
      {
        DrawTextA(hdc, "M", -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      }
      else if (!isHidden)
      {
        if (value == 9)
        {
          DrawTextA(hdc, "G", -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        else if (value > 0)
        {
          std::ostringstream text;
          text << value;
          DrawTextA(hdc, text.str().c_str(), -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
      }
    }
  }

  SelectObject(hdc, oldBrush);
  SelectObject(hdc, oldPen);
  DeleteObject(hiddenBrush);
  DeleteObject(openBrush);
  DeleteObject(markBrush);
  DeleteObject(gooseBrush);
  DeleteObject(gridPen);
}

void drawWinAnimation(HDC hdc, int left, int top)
{
  HBRUSH darkBrush = CreateSolidBrush(RGB(80, 80, 80));
  HBRUSH redBrush = CreateSolidBrush(RGB(220, 70, 70));
  HBRUSH yellowBrush = CreateSolidBrush(RGB(245, 190, 60));
  HBRUSH blueBrush = CreateSolidBrush(RGB(70, 130, 220));
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, darkBrush);

  Rectangle(hdc, left, top + 55, left + 70, top + 80);
  Rectangle(hdc, left + 55, top + 35, left + 115, top + 55);
  Ellipse(hdc, left + 10, top + 72, left + 30, top + 92);
  Ellipse(hdc, left + 80, top + 72, left + 100, top + 92);

  int burst = 15 + (animationStep % 6) * 5;
  SelectObject(hdc, redBrush);
  Ellipse(hdc, left + 125 - burst, top + 35 - burst, left + 125 + burst, top + 35 + burst);
  SelectObject(hdc, yellowBrush);
  Ellipse(hdc, left + 150 - burst / 2, top + 20 - burst / 2, left + 150 + burst / 2, top + 20 + burst / 2);
  SelectObject(hdc, blueBrush);
  Ellipse(hdc, left + 145 - burst / 2, top + 60 - burst / 2, left + 145 + burst / 2, top + 60 + burst / 2);

  TextOutA(hdc, left, top, "You won!", 8);

  SelectObject(hdc, oldBrush);
  DeleteObject(darkBrush);
  DeleteObject(redBrush);
  DeleteObject(yellowBrush);
  DeleteObject(blueBrush);
}

void drawLoseAnimation(HDC hdc, int left, int top)
{
  HBRUSH bodyBrush = CreateSolidBrush(RGB(245, 245, 245));
  HBRUSH beakBrush = CreateSolidBrush(RGB(245, 170, 60));
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bodyBrush);

  Ellipse(hdc, left + 20, top + 35, left + 100, top + 95);
  Ellipse(hdc, left + 80, top + 15, left + 135, top + 65);
  SelectObject(hdc, beakBrush);
  POINT beak[3] = {
    {left + 130, top + 35},
    {left + 160, top + 45},
    {left + 130, top + 55}
  };
  Polygon(hdc, beak, 3);

  SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  Ellipse(hdc, left + 110, top + 32, left + 118, top + 40);

  TextOutA(hdc, left, top, "Try again!", 10);

  SelectObject(hdc, oldBrush);
  DeleteObject(bodyBrush);
  DeleteObject(beakBrush);
}

void drawAnimation(HDC hdc)
{
  if (animationType == 0)
  {
    return;
  }

  int left = MIN_WINDOW_WIDTH - 210;
  int top = TOP_SPACE + 20;

  if (animationType == 1)
  {
    drawWinAnimation(hdc, left, top);
  }
  else if (animationType == 2)
  {
    drawLoseAnimation(hdc, left, top);
  }
}

void handleCellClick(HWND window, LPARAM lParam, bool rightClick)
{
  if (board == NULL || gameOver)
  {
    return;
  }

  int mouseX = LOWORD(lParam);
  int mouseY = HIWORD(lParam);
  int col = (mouseX + scrollX - LEFT_SPACE) / CELL_SIZE;
  int row = (mouseY + scrollY - TOP_SPACE) / CELL_SIZE;

  if (mouseX < LEFT_SPACE || mouseY < TOP_SPACE || col < 0 || row < 0)
  {
    return;
  }

  if (col >= (int)xdim || row >= (int)ydim)
  {
    return;
  }

  if (rightClick)
  {
    mark(board, xdim, ydim, col, row);
  }
  else
  {
    int result = reveal(board, xdim, ydim, col, row);
    if (result == 9)
    {
      revealBoard();
      gameOver = true;
      statusText = "You clicked a goose. Press Start Game to try again.";
      startAnimation(window, 2);
    }
  }

  if (!gameOver && isGameWon(board, xdim, ydim))
  {
    revealBoard();
    gameOver = true;
    statusText = "You won! Press Start Game to play again.";
    startAnimation(window, 1);
  }

  InvalidateRect(window, NULL, FALSE);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_CREATE:
      titleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
      textFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
      cellFont = CreateFontA(18, 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");

      CreateWindowA("STATIC", "X:", WS_VISIBLE | WS_CHILD, 350, 24, 20, 20, window, NULL, NULL, NULL);
      xdimBox = CreateWindowA("EDIT", "8", WS_VISIBLE | WS_CHILD | WS_BORDER, 372, 20, 45, 26, window, (HMENU)ID_XDIM, NULL, NULL);
      CreateWindowA("STATIC", "Y:", WS_VISIBLE | WS_CHILD, 430, 24, 20, 20, window, NULL, NULL, NULL);
      ydimBox = CreateWindowA("EDIT", "8", WS_VISIBLE | WS_CHILD | WS_BORDER, 452, 20, 45, 26, window, (HMENU)ID_YDIM, NULL, NULL);
      CreateWindowA("STATIC", "Geese:", WS_VISIBLE | WS_CHILD, 350, 60, 48, 20, window, NULL, NULL, NULL);
      geeseBox = CreateWindowA("EDIT", "10", WS_VISIBLE | WS_CHILD | WS_BORDER, 402, 56, 55, 26, window, (HMENU)ID_GEESE, NULL, NULL);
      CreateWindowA("BUTTON", "Start Game", WS_VISIBLE | WS_CHILD, 472, 54, 125, 32, window, (HMENU)ID_START, NULL, NULL);
      startNewGame(window);
      return 0;

    case WM_COMMAND:
      if (LOWORD(wParam) == ID_START)
      {
        startNewGame(window);
      }
      return 0;

    case WM_LBUTTONDOWN:
      handleCellClick(window, lParam, false);
      return 0;

    case WM_RBUTTONDOWN:
      handleCellClick(window, lParam, true);
      return 0;

    case WM_SIZE:
      updateScrollBars(window);
      InvalidateRect(window, NULL, FALSE);
      return 0;

    case WM_HSCROLL:
      scrollWindow(window, SB_HORZ, wParam);
      return 0;

    case WM_VSCROLL:
      scrollWindow(window, SB_VERT, wParam);
      return 0;

    case WM_ERASEBKGND:
      return 1;

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(window, &ps);
      RECT client;
      GetClientRect(window, &client);

      HDC buffer = CreateCompatibleDC(hdc);
      HBITMAP bitmap = CreateCompatibleBitmap(hdc, client.right, client.bottom);
      HBITMAP oldBitmap = (HBITMAP)SelectObject(buffer, bitmap);

      drawPage(buffer);
      drawBoard(buffer);
      drawAnimation(buffer);

      BitBlt(hdc, 0, 0, client.right, client.bottom, buffer, 0, 0, SRCCOPY);

      SelectObject(buffer, oldBitmap);
      DeleteObject(bitmap);
      DeleteDC(buffer);
      EndPaint(window, &ps);
      return 0;
    }

    case WM_TIMER:
      if (wParam == ANIMATION_TIMER)
      {
        ++animationStep;
        if (animationStep > 30)
        {
          KillTimer(window, ANIMATION_TIMER);
        }
        InvalidateRect(window, NULL, FALSE);
      }
      return 0;

    case WM_DESTROY:
      KillTimer(window, ANIMATION_TIMER);
      DeleteObject(titleFont);
      DeleteObject(textFont);
      DeleteObject(cellFont);
      cleanBoard(board);
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(window, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand)
{
  srand(time(NULL));

  WNDCLASSA wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = instance;
  wc.lpszClassName = "GeeseSpotterWindow";
  wc.hbrBackground = CreateSolidBrush(RGB(239, 245, 241));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);

  RegisterClassA(&wc);

  HWND window = CreateWindowA(
    "GeeseSpotterWindow",
    "GeeseSpotter",
    WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    MIN_WINDOW_WIDTH,
    DEFAULT_WINDOW_HEIGHT,
    NULL,
    NULL,
    instance,
    NULL
  );

  ShowWindow(window, showCommand);

  MSG message = {};
  while (GetMessage(&message, NULL, 0, 0))
  {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  return 0;
}

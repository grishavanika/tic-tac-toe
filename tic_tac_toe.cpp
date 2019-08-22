// See original code from
// "Procedural vs Functional" (Profun):
//  - https://github.com/tsoding/profun
//  - https://www.youtube.com/playlist?list=PLguYJK7ydFE7KBfUplAk_qLp--xG6gw3R

#include <cassert>
#include <cstdlib>

constexpr unsigned N = 3;

[[noreturn]] inline void Unreachable()
{
    std::abort();
}

enum class Cell
{
    Empty,
    O,
    X,
};

using Board = Cell [N][N];

enum class Player
{
    O,
    X,
};

enum class State
{
    Running,
    Quit,
    WinPlayerO,
    WinPlayerX,
    Tie,
};

struct Game
{
    Player player_{Player::O};
    State state_{State::Running};
    Board board_{};

    void player_turn(unsigned row, unsigned column);
    void reset();

private:
    static Cell PlayerToCell(Player p);
    static State PlayerToWinState(Player p);

    Player next_player(Player p) const;
    State check_game_over(Player p) const;
    bool check_tie() const;
    bool check_player_won(Player p) const;
};

inline void Game::player_turn(unsigned row, unsigned column)
{
    assert(row < N);
    assert(column < N);
    assert(state_ == State::Running);

    if (board_[row][column] == Cell::Empty)
    {
        const Player old_player = player_;
        player_ = next_player(old_player);
        board_[row][column] = PlayerToCell(old_player);
        state_ = check_game_over(old_player);
    }
}

/*static*/ inline Cell Game::PlayerToCell(Player p)
{
    switch (p)
    {
    case Player::O: return Cell::O;
    case Player::X: return Cell::X;
    }
    Unreachable();
}

/*static*/ inline State Game::PlayerToWinState(Player p)
{
    switch (p)
    {
    case Player::O: return State::WinPlayerO;
    case Player::X: return State::WinPlayerX;
    }
    Unreachable();
}

inline Player Game::next_player(Player p) const
{
    switch (p)
    {
    case Player::O: return Player::X;
    case Player::X: return Player::O;
    }
    Unreachable();
}

inline State Game::check_game_over(Player p) const
{
    if (check_player_won(p))
    {
        return PlayerToWinState(p);
    }
    else if (check_tie())
    {
        return State::Tie;
    }
    return State::Running;
}

inline bool Game::check_player_won(Player p) const
{
    const Cell win_cell = PlayerToCell(p);
    unsigned row_count = 0;
    unsigned column_count = 0;
    unsigned diag1_count = 0;
    unsigned diag2_count = 0;

    // Expects board to have same number of rows and columns
    for (unsigned i = 0; i < N; ++i)
    {
        for (unsigned j = 0; j < N; ++j)
        {
            if (board_[i][j] == win_cell)
            {
                ++row_count;
            }

            if (board_[j][i] == win_cell)
            {
                ++column_count;
            }
        }

        if ((row_count >= N) || (column_count >= N))
        {
            return true;
        }

        row_count = 0;
        column_count = 0;

        if (board_[i][i] == win_cell)
        {
            ++diag1_count;
        }
        if (board_[i][N - i - 1] == win_cell)
        {
            ++diag2_count;
        }
    }

    return ((diag1_count >= N) || (diag2_count >= N));
}

inline bool Game::check_tie() const
{
    for (const auto& row : board_)
    {
        for (const Cell cell : row)
        {
            if (cell == Cell::Empty)
            {
                return false;
            }
        }
    }
    return true;
}

inline void Game::reset()
{
    state_ = State::Running;
    player_ = Player::O;
    for (auto& row : board_)
    {
        for (Cell& cell : row)
        {
            cell = Cell::Empty;
        }
    }
}

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include <algorithm>

constexpr int k_ScreenWidth  = 640;
constexpr int k_ScreenHeight = 480;
constexpr int k_CellHeight   = (k_ScreenHeight / N);
constexpr int k_CellWidth    = (k_ScreenWidth  / N);
const SDL_Color k_GridColor   {0xff, 0xff, 0xff, 0xff}; // white
const SDL_Color k_PlayerXColor{0xff, 0x32, 0x32, 0xff}; // red
const SDL_Color k_PlayerOColor{0x32, 0x64, 0xff, 0xff}; // blue
const SDL_Color k_TieColor    {0x64, 0x64, 0x64, 0xff}; // gray

void HandleWindowClick(Game& game, int y, int x)
{
    if (game.state_ == State::Running)
    {
        game.player_turn(y / k_CellHeight, x / k_CellWidth);
    }
    else
    {
        game.reset();
    }
}

void RenderGrid(SDL_Renderer* renderer, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xff);

    for (unsigned i = 1; i < N; ++i)
    {
        SDL_RenderDrawLine(renderer
            , i * k_CellWidth
            , 0
            , i * k_CellWidth
            , k_ScreenHeight);
        SDL_RenderDrawLine(renderer
            , 0
            , i * k_CellHeight
            , k_ScreenWidth
            , i * k_CellHeight);
    }
}

void RenderX(SDL_Renderer* renderer
    , unsigned row
    , unsigned column
    , const SDL_Color& color)
{
    const float half_box_side = std::min(k_CellWidth, k_CellHeight) * 0.25f;
    const float center_x = k_CellWidth * 0.5f + column * k_CellWidth;
    const float center_y = k_CellHeight * 0.5f + row * k_CellHeight;

    thickLineRGBA(renderer
        , static_cast<Sint16>(center_x - half_box_side)
        , static_cast<Sint16>(center_y - half_box_side)
        , static_cast<Sint16>(center_x + half_box_side)
        , static_cast<Sint16>(center_y + half_box_side)
        , 10 // line width
        , color.r
        , color.g
        , color.b
        , 0xff);
    thickLineRGBA(renderer
        , static_cast<Sint16>(center_x + half_box_side)
        , static_cast<Sint16>(center_y - half_box_side)
        , static_cast<Sint16>(center_x - half_box_side)
        , static_cast<Sint16>(center_y + half_box_side)
        , 10 // line width
        , color.r
        , color.g
        , color.b
        , 0xff);
}

void RenderO(SDL_Renderer* renderer
    , int row
    , int column
    , const SDL_Color& color)
{
    const float half_box_side = std::min(k_CellWidth, k_CellHeight) * 0.25f;
    const float center_x = k_CellWidth * 0.5f + column * k_CellWidth;
    const float center_y = k_CellHeight * 0.5f + row * k_CellHeight;

    filledCircleRGBA(renderer
        , static_cast<Sint16>(center_x)
        , static_cast<Sint16>(center_y)
        , static_cast<Sint16>(half_box_side + 5)
        , color.r, color.g, color.b, 0xff);
    filledCircleRGBA(renderer
        , static_cast<Sint16>(center_x)
        , static_cast<Sint16>(center_y)
        , static_cast<Sint16>(half_box_side - 5)
        , 0, 0, 0, 0xff);
}

void RenderBoard(SDL_Renderer* renderer
    , const Board& board
    , const SDL_Color& player_x_color
    , const SDL_Color& player_o_color)
{
    for (unsigned i = 0; i < N; ++i)
    {
        for (unsigned j = 0; j < N; ++j)
        {
            switch (board[i][j])
            {
            case Cell::X:
                RenderX(renderer, i, j, player_x_color);
                break;
            case Cell::O:
                RenderO(renderer, i, j, player_o_color);
                break;
            case Cell::Empty:
                break;
            }
        }
    }
}

void RenderRunningState(SDL_Renderer* renderer, const Game& game)
{
    RenderGrid(renderer, k_GridColor);
    RenderBoard(renderer
        , game.board_
        , k_PlayerXColor
        , k_PlayerOColor);
}

void RenderGameOverState(SDL_Renderer* renderer
    , const Game& game
    , const SDL_Color& color)
{
    RenderGrid(renderer, color);
    RenderBoard(renderer
        , game.board_
        , color
        , color);
}

void RenderGame(SDL_Renderer* renderer, const Game& game)
{
    switch (game.state_)
    {
    case State::Running:
        RenderRunningState(renderer, game);
        break;
    case State::WinPlayerX:
        RenderGameOverState(renderer, game, k_PlayerXColor);
        break;
    case State::WinPlayerO:
        RenderGameOverState(renderer, game, k_PlayerOColor);
        break;
    case State::Tie:
        RenderGameOverState(renderer, game, k_TieColor);
        break;
    case State::Quit:
        break;
    }
}

void AbortOnSDLError(int code)
{
    if (code != 0)
    {
        assert(false && "SDL call failed");
        abort();
    }
}

void AbortOnSDLError(const void* resource)
{
    if (!resource)
    {
        assert(false && "SDL call (resource) failed");
        abort();
    }
}

struct TickData
{
    Game& game;
    SDL_Renderer* renderer;
};

void MainTick(void* data_ptr)
{
    TickData* data = static_cast<TickData*>(data_ptr);
    Game& game = data->game;
    SDL_Renderer* renderer = data->renderer;

    SDL_Event e{};
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            game.state_ = State::Quit;
            break;
        case SDL_MOUSEBUTTONDOWN:
            HandleWindowClick(game, e.button.y, e.button.x);
            break;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);

    RenderGame(renderer, game);

    SDL_RenderPresent(renderer);
}

#if (__EMSCRIPTEN__)
#include <emscripten/emscripten.h>

void MainLoop(TickData& data)
{
    emscripten_set_main_loop_arg(&MainTick
        , &data
        , -1  // use whatever FPS browser needs
        , 1); // simulate infinite loop. Don't destroy objects on stack (?)
}

int main(int, char**)
{

#else

#include <Windows.h>
#include <tchar.h>

void MainLoop(TickData& data)
{
    while (data.game.state_ != State::Quit)
    {
        MainTick(&data);
    }
}

int WINAPI _tWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPTSTR, _In_ int)
{
    SDL_SetMainReady();
#endif

    // Initialize SDL. Ignore any errors and leak resources
    AbortOnSDLError(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window* window = SDL_CreateWindow(
        "Tic-Tac-Toe" // title
        , SDL_WINDOWPOS_CENTERED // x position
        , SDL_WINDOWPOS_CENTERED // y position
        , k_ScreenWidth
        , k_ScreenHeight
        , SDL_WINDOW_SHOWN);
    AbortOnSDLError(window);
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window
        , -1 // first supporting renderer
        , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    AbortOnSDLError(renderer);

    Game game{Player::O, State::Running
        , { {Cell::Empty, Cell::Empty, Cell::Empty}
          , {Cell::Empty, Cell::Empty, Cell::Empty}
          , {Cell::Empty, Cell::Empty, Cell::Empty}
        }};

    TickData data{game, renderer};
    MainLoop(data);

    return 0;
}

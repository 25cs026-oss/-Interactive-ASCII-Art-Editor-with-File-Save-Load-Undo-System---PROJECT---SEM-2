#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stack>
#include <memory>
#include <conio.h>   // for _getch()
using namespace std;

// ======================= Canvas Class =======================
class Canvas {
    vector<string> grid;
    int rows, cols;
    int cursorRow, cursorCol;

public:
    Canvas(int r = 10, int c = 20) : rows(r), cols(c), cursorRow(0), cursorCol(0) {
        grid.assign(rows, string(cols, ' '));
    }

    // Operator << for displaying
    friend ostream& operator<<(ostream& os, const Canvas& canvas) {
        os << "\n--- ASCII Canvas ---\n";
        for (int i = 0; i < canvas.rows; i++) {
            for (int j = 0; j < canvas.cols; j++) {
                if (i == canvas.cursorRow && j == canvas.cursorCol)
                    os << "_"; // cursor
                else
                    os << canvas.grid[i][j];
            }
            os << "\n";
        }
        os << "--------------------\n";
        os << "Use arrow keys to move, type a character to draw, 'u' to undo, 's' to save, 'l' to load, 'q' to quit.\n";
        return os;
    }

    // Operator () for drawing
    void operator()(int r, int c, char ch) {
        ensureSize(r + 1, c + 1);
        grid[r][c] = ch;
    }

    // Operator -- for undo (prefix)
    void operator--() {
        // handled by UndoManager externally
    }

    void moveCursor(int dr, int dc) {
        cursorRow += dr;
        cursorCol += dc;
        ensureSize(cursorRow + 1, cursorCol + 1);
        cursorRow = max(0, min(rows - 1, cursorRow));
        cursorCol = max(0, min(cols - 1, cursorCol));
    }

    void placeChar(char ch) {
        ensureSize(cursorRow + 1, cursorCol + 1);
        grid[cursorRow][cursorCol] = ch;
    }

    void saveToFile(const string& filename) const {
        ofstream fout(filename);
        if (!fout) throw runtime_error("File could not be opened!");
        for (const auto& row : grid) fout << row << "\n";
    }

    void loadFromFile(const string& filename) {
        ifstream fin(filename);
        if (!fin) throw runtime_error("File could not be opened!");
        grid.clear();
        string line;
        while (getline(fin, line)) grid.push_back(line);
        rows = grid.size();
        cols = (rows > 0 ? grid[0].size() : 0);
        cursorRow = 0; cursorCol = 0;
    }

    vector<string> getState() const { return grid; }
    void restoreState(const vector<string>& state) {
        grid = state;
        rows = grid.size();
        cols = (rows > 0 ? grid[0].size() : 0);
    }

private:
    void ensureSize(int newRows, int newCols) {
        if (newRows > rows) {
            for (int i = rows; i < newRows; i++) {
                grid.push_back(string(cols, ' '));
            }
            rows = newRows;
        }
        if (newCols > cols) {
            for (auto& row : grid) {
                row.resize(newCols, ' ');
            }
            cols = newCols;
        }
    }
};

// ======================= Undo Manager =======================
class UndoManager {
    stack<vector<string>> history;

public:
    void saveState(const Canvas& canvas) {
        history.push(canvas.getState());
    }

    bool undo(Canvas& canvas) {
        if (history.empty()) return false;
        canvas.restoreState(history.top());
        history.pop();
        return true;
    }
};

// ======================= Polymorphic Tools =======================
class Tool {
public:
    virtual void apply(Canvas& canvas) = 0;
    virtual ~Tool() {}
};

class DrawTool : public Tool {
    char ch;
public:
    DrawTool(char character) : ch(character) {}
    void apply(Canvas& canvas) override {
        canvas.placeChar(ch);
    }
};

// ======================= Main Program =======================
int main() {
    Canvas canvas(10, 20);
    UndoManager undoManager;
    bool running = true;

    while (running) {
        system("cls"); // clear screen
        cout << canvas; // operator<<

        int key = _getch();

        try {
            if (key == 'q' || key == 'Q') {
                running = false;
            } else if (key == 'u' || key == 'U') {
                if (undoManager.undo(canvas))
                    cout << "Undo successful!\n";
                else
                    cout << "Nothing to undo!\n";
                _getch(); // pause
            } else if (key == 's' || key == 'S') {
                string filename;
                cout << "Enter filename to save: ";
                cin >> filename;
                canvas.saveToFile(filename);
                cout << "Canvas saved!\n";
                _getch();
            } else if (key == 'l' || key == 'L') {
                string filename;
                cout << "Enter filename to load: ";
                cin >> filename;
                undoManager.saveState(canvas);
                canvas.loadFromFile(filename);
                cout << "Canvas loaded!\n";
                _getch();
            } else if (key == 224) { // arrow keys
                int arrow = _getch();
                switch (arrow) {
                    case 72: canvas.moveCursor(-1, 0); break; // up
                    case 80: canvas.moveCursor(1, 0); break;  // down
                    case 75: canvas.moveCursor(0, -1); break; // left
                    case 77: canvas.moveCursor(0, 1); break;  // right
                }
            } else {
                if (isprint(key)) {
                    undoManager.saveState(canvas);
                    unique_ptr<Tool> tool = make_unique<DrawTool>((char)key);
                    tool->apply(canvas); // polymorphism
                }
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
            _getch();
        }
    }

    cout << "Exiting interactive editor...\n";
    return 0;
}

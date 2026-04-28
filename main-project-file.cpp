#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stack>
#include <memory>
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

// ======================= Helper: Read Key =======================
// Reads a single key from stdin, including arrow keys as escape sequences
int readKey() {
    char ch;
    cin.get(ch);
    if (ch == '\033') { // ESC sequence for arrows
        char next1, next2;
        if (cin.get(next1) && cin.get(next2)) {
            if (next1 == '[') {
                switch (next2) {
                    case 'A': return 1000; // up
                    case 'B': return 1001; // down
                    case 'C': return 1002; // right
                    case 'D': return 1003; // left
                }
            }
        }
    }
    return ch; // normal key
}

// ======================= Main Program =======================
int main() {
    Canvas canvas(10, 20);
    UndoManager undoManager;
    bool running = true;

    while (running) {
        system("clear"); // use "cls" on Windows, "clear" on Linux/macOS
        cout << canvas;

        int key = readKey();

        try {
            if (key == 'q' || key == 'Q') {
                running = false;
            } else if (key == 'u' || key == 'U') {
                if (undoManager.undo(canvas))
                    cout << "Undo successful!\n";
                else
                    cout << "Nothing to undo!\n";
                cin.get(); // pause
            } else if (key == 's' || key == 'S') {
                string filename;
                cout << "Enter filename to save: ";
                cin >> filename;
                canvas.saveToFile(filename);
                cout << "Canvas saved!\n";
                cin.ignore();
                cin.get();
            } else if (key == 'l' || key == 'L') {
                string filename;
                cout << "Enter filename to load: ";
                cin >> filename;
                undoManager.saveState(canvas);
                canvas.loadFromFile(filename);
                cout << "Canvas loaded!\n";
                cin.ignore();
                cin.get();
            } else if (key == 1000) { // up
                canvas.moveCursor(-1, 0);
            } else if (key == 1001) { // down
                canvas.moveCursor(1, 0);
            } else if (key == 1002) { // right
                canvas.moveCursor(0, 1);
            } else if (key == 1003) { // left
                canvas.moveCursor(0, -1);
            } else {
                if (isprint(key)) {
                    undoManager.saveState(canvas);
                    unique_ptr<Tool> tool = make_unique<DrawTool>((char)key);
                    tool->apply(canvas);
                }
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
            cin.get();
        }
    }

    cout << "Exiting interactive editor...\n";
    return 0;
}

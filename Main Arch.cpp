#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

// --- Data Structures ---
struct Point {
    int x, y;
};

// --- 1. File Menu Functions ---
struct Shape {
    string type;
    int color;
};

// --- Global Application Data ---
vector<Shape> drawnShapes;
string windowBackground = "Default (Black)";
int currentShapeColor = 0;

// --- 1. File Menu Implementation ---
void clearScreen() {
    drawnShapes.clear();
    cout << "Screen cleared from shapes." << endl;
}

void saveData(const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        for (const auto& shape : drawnShapes) {
            file << shape.type << " " << shape.color << endl;
        }
        file.close();
        cout << "All data drawn on the screen saved to " << filename << "." << endl;
    } else {
        cout << "Error: Could not open file for saving." << endl;
    }
}

void loadData(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        drawnShapes.clear();
        string shapeType;
        int color;
        while (file >> shapeType >> color) {
            drawnShapes.push_back({shapeType, color});
        }
        file.close();
        cout << "Data loaded successfully from " << filename << "." << endl;
    } else {
        cout << "Error: Could not open file for loading." << endl;
    }
}

// --- 2. Preferences Menu Functions ---
void changeBackgroundToWhite() {
    windowBackground = "White";
    cout << "Changed the background of window to be white." << endl;
}

void changeMouseShape() {
    // Logic to change window mouse cursor (e.g., using a graphics library like OpenGL)
    cout << "Changed the shape of your window mouse." << endl;
}

void setShapeColor(int colorValue) {
    currentShapeColor = colorValue;
    cout << "Shape color selected (Code: " << currentShapeColor << ")." << endl;
}

// --- 3. Lines Menu Functions ---
void drawLineDDA() {
    // TODO: Implement DDA line algorithm
}

void drawLineMidpoint() {
    // TODO: Implement Midpoint line algorithm
}

void drawLineParametric() {
    // TODO: Implement Parametric line algorithm
}

// --- 4. Circles Menu Functions ---
void drawCircleDirect() {
    // TODO: Implement Direct circle algorithm
}

void drawCirclePolar() {
    // TODO: Implement Polar circle algorithm
}

void drawCircleIterativePolar() {
    // TODO: Implement iterative Polar circle algorithm
}

void drawCircleMidpoint() {
    // TODO: Implement Midpoint circle algorithm
}

void drawCircleModifiedMidpoint() {
    // TODO: Implement modified Midpoint circle algorithm
}

// --- 5. Ellipse Menu Functions ---
void drawEllipseDirect() {
    // TODO: Implement Direct ellipse algorithm
}

void drawEllipsePolar() {
    // TODO: Implement Polar ellipse algorithm
}

void drawEllipseMidpoint() {
    // TODO: Implement Midpoint ellipse algorithm
}

// --- 6. Curves Menu Functions ---
void drawCardinalSpline() {
    // TODO: Implement Cardinal Spline Curve
}

// --- 7. Filling Menu Functions ---
void fillCircleWithLines(int quarter) {
    // TODO: Implement filling Circle with lines after taking filling quarter from user
}

void fillCircleWithCircles(int quarter) {
    // TODO: Implement filling Circle with other circles after taking filling quarter from user
}

void fillSquareWithHermit() {
    // TODO: Implement filling Square with Hermit Curve (Vertical)
}

void fillRectangleWithBezier() {
    // TODO: Implement filling Rectangle with Bezier Curve (Horizontal)
}

void fillConvex() {
    // TODO: Implement Convex filling algorithm
}

void fillNonConvex() {
    // TODO: Implement Non-Convex filling algorithm
}

void fillFloodRecursive() {
    // TODO: Implement Recursive Flood Fill
}

void fillFloodNonRecursive() {
    // TODO: Implement Non-Recursive Flood Fill
}

// --- 8. Clipping Menu Functions ---
void clipRectanglePoint() {
    // TODO: Clipping algorithm using Rectangle as Clipping Window (Point)
}

void clipRectangleLine() {
    // TODO: Clipping algorithm using Rectangle as Clipping Window (Line)
}

void clipRectanglePolygon() {
    // TODO: Clipping algorithm using Rectangle as Clipping Window (Polygon)
}

void clipSquarePoint() {
    // TODO: Clipping algorithm using Square as Clipping Window (Point)
}

void clipSquareLine() {
    // TODO: Clipping algorithm using Square as Clipping Window (Line)
}

// --- Bonus Menu Functions ---
void clipCirclePoint() {
    // TODO: Clipping algorithm using circle as a Clipping Window (Point)
}

void clipCircleLine() {
    // TODO: Clipping algorithm using circle as a Clipping Window (Line)
}

void drawHappyFace() {
    // TODO: Draw happy smiley face
}

void drawSadFace() {
    // TODO: Draw sad smiley face
}

// --- User Interaction Functions ---
void handleMouseInteraction() {
    cout << "Mouse interaction tracking initialized. You must interact with the window using the mouse only to draw." << endl;
}

void displayConsoleLogs() {
    cout << "===========================================" << endl;
    cout << "Computer Graphics Project - Main Console" << endl;
    cout << "===========================================" << endl;
    cout << "1. File Menu" << endl;
    cout << "2. Preferences Menu" << endl;
    cout << "3. Open Lines/Shapes Menu" << endl;
    cout << "-------------------------------------------" << endl;
    cout << "Select a menu option: ";
}

// --- Main Execution ---
int main() {
    displayConsoleLogs();
    
    int option;
    cin >> option;

    switch (option) {
        case 1: {
            int subOption;
            cout << "\n--- File Menu ---" << endl;
            cout << "1. Clear screen from shapes" << endl;
            cout << "2. Save data to a file" << endl;
            cout << "3. Load saved data from file" << endl;
            cout << "Choice: ";
            cin >> subOption;

            if (subOption == 1) {
                clearScreen();
            } else if (subOption == 2) {
                saveData("shapes_data.txt");
            } else if (subOption == 3) {
                loadData("shapes_data.txt");
            }
            break;
        }
        case 2: {
            int subOption;
            cout << "\n--- Preferences Menu ---" << endl;
            cout << "1. Change background to white" << endl;
            cout << "2. Change the shape of the window mouse" << endl;
            cout << "3. Choose shape color" << endl;
            cout << "Choice: ";
            cin >> subOption;

            if (subOption == 1) {
                changeBackgroundToWhite();
            } else if (subOption == 2) {
                changeMouseShape();
            } else if (subOption == 3) {
                int c;
                cout << "Enter color code/number: ";
                cin >> c;
                setShapeColor(c);
            }
            break;
        }
        case 3:
            handleMouseInteraction();
            break;
        default:
            cout << "Invalid Option. Exiting." << endl;
            break;
    }

    return 0;
    
    return 0;
}
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <windows.h>
#include <math.h>
using namespace std;

// --- Data Structures ---
struct Point {
    int x, y;
};
// --- helper ---
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c)
{
    SetPixel(hdc, xc + x, yc + y, c);
    SetPixel(hdc, xc - x, yc + y, c);
    SetPixel(hdc, xc + x, yc - y, c);
    SetPixel(hdc, xc - x, yc - y, c);

    SetPixel(hdc, xc + y, yc + x, c);
    SetPixel(hdc, xc - y, yc + x, c);
    SetPixel(hdc, xc + y, yc - x, c);
    SetPixel(hdc, xc - y, yc - x, c);
}

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
void drawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    double xinc = (double)dx / steps;
    double yinc = (double)dy / steps;

    double x = x1;
    double y = y1;

    for (int i = 0; i <= steps; i++)
    {
        SetPixel(hdc, round(x), round(y), c);
        x += xinc;
        y += yinc;
    }
}

void drawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
      int dx = x2 - x1;
    int dy = y2 - y1;

    int d = dx - 2 * dy;
    int x = x1;
    int y = y1;

    SetPixel(hdc, x, y, c);

    while (x < x2)
    {
        if (d <= 0)
        {
            d += 2 * (dx - dy);
            y++;
        }
        else
            d -= 2 * dy;

        x++;
        SetPixel(hdc, x, y, c); 
    }
}

void drawLineParametric(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy)); 
    double dt = 1.0 / steps;           

    for (double t = 0; t <= 1; t += dt)
    {
        int x = x1 + dx * t;
        int y = y1 + dy * t;
        SetPixel(hdc, x, y, c);
    }
}
// --- 4. Circles Menu Functions ---
void drawCircleDirect(HDC hdc, int xc, int yc, int R, COLORREF c) {
   int x = 0;
    int y = R;

    while (x <= y)
    {
        y = sqrt((double)R * R - x * x);
        Draw8Points(hdc, xc, yc, x, y, c);
        x++;
    }
}

void drawCirclePolar(HDC hdc, int xc, int yc, int R, COLORREF c) {
        double theta = 0;
    double dTheta = 1.0 / R;

    while (theta <= 6.28) // 2 * pi
    {
        int x = R * cos(theta);
        int y = R * sin(theta);

        SetPixel(hdc, xc + x, yc + y, c);
        theta += dTheta;
    }
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
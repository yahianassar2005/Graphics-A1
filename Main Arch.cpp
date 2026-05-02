#include <iostream>
#include <vector>
#include <string>

using namespace std;

// --- Data Structures ---
struct Point {
    int x, y;
};

// --- 1. File Menu Functions ---
void clearScreen() {
    // TODO: Implement function to clear screen from shapes
}

void saveData(const string& filename) {
    // TODO: Implement function to save all data drawn on the screen to a file
}

void loadData(const string& filename) {
    // TODO: Implement function to load saved data from files
}

// --- 2. Preferences Menu Functions ---
void changeBackgroundToWhite() {
    // TODO: Implement function to change the background of window to be white
}

void changeMouseShape() {
    // TODO: Implement function to change the shape of your window mouse
}

void setShapeColor(int color) {
    // TODO: Give the user option to choose shape color before drawing from menu
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
    // TODO: User must interact with the window using mouse only to draw the shapes
}

void displayConsoleLogs() {
    // TODO: Use the console to display logs/information + take an input option from the user
}

// --- Main Execution ---
int main() {
    cout << "Cairo University" << endl;
    cout << "Faculty of Computers and AI" << endl;
    cout << "Information Technology Department - Computer Graphics Term Project 2025-2026" << endl;
    
    // Display project instructions to the user/console
    cout << "--------------------------------------------------------" << endl;
    cout << "Requirements: 2D drawing package using menus/drop downs." << endl;
    cout << "Groups: 3 to 5 students." << endl;
    cout << "Submission in Lab during deadline week (Wednesday 13/5 - Thursday 14/5)." << endl;
    cout << "--------------------------------------------------------" << endl;

    // Run display
    displayConsoleLogs();
    
    // Initialize graphics and event loop here
    
    return 0;
}

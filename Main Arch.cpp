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
struct Vector2
{
    double x,y;
    Vector2(double a=0,double b=0)
    {
        x=a; y=b;
    }
};
class Vector4
{
    double v[4];
public:
    Vector4(double a=0,double b=0,double c=0,double d=0)
    {
        v[0]=a; v[1]=b; v[2]=c; v[3]=d;
    }

    Vector4(double a[])
    {
        memcpy(v,a,4*sizeof(double));
    }
    double& operator[](int i)
    {
        return v[i];
    }
};
class Matrix4
{
    Vector4 M[4];
public:
    Matrix4(double A[])
    {
        memcpy(M,A,16*sizeof(double));
    }
    Vector4& operator[](int i)
    {
        return M[i];
    }
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
double DotProduct(Vector4& a,Vector4& b)
{
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];
}
Vector4 operator*(Matrix4 M,Vector4& b)
{
    Vector4 res;
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            res[i]+=M[i][j]*b[j];

    return res;
}
Vector4 GetHermiteCoeff(double x0,double s0,double x1,double s1)
{
    static double H[16]={2,1,-2,1,-3,-2,3,-1,0,1,0,0,1,0,0,0};
    static Matrix4 basis(H);
    Vector4 v(x0,s0,x1,s1);
    return basis*v;
}
void DrawHermiteCurve (HDC hdc,Vector2& P0,Vector2& T0,Vector2& P1,Vector2& T1 ,int
numpoints)
{
    Vector4 xcoeff=GetHermiteCoeff(P0.x,T0.x,P1.x,T1.x);
    Vector4 ycoeff=GetHermiteCoeff(P0.y,T0.y,P1.y,T1.y);
    if(numpoints<2)return;
    double dt=1.0/(numpoints-1);
    for(double t=0;t<=1;t+=dt)
    {
        Vector4 vt;
        vt[3]=1;
        for(int i=2;i>=0;i--)vt[i]=vt[i+1]*t;
        int x=round(DotProduct(xcoeff,vt));
        int y=round(DotProduct(ycoeff,vt));
        if(t==0)MoveToEx(hdc,x,y,NULL);else LineTo(hdc,x,y);
    }
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

void drawCircleIterativePolar(HDC hdc, int xc, int yc, int R, COLORREF c) {
    double x = 0, y = R;
double dTheta = 1.0 / R;

double cosd = cos(dTheta);
double sind = sin(dTheta);

while (x < y)
{
    Draw8Points(hdc, xc, yc, round(x), round(y), c);

    double x1 = x * cosd - y * sind;
    y = x * sind + y * cosd;
    x = x1;
}
}

void drawCircleMidpoint(HDC hdc, int xc, int yc, int R, COLORREF c) {
     int x = 0;
    int y = R;

    int d = 1 - R;

    Draw8Points(hdc, xc, yc, x, y, c);

    while (x < y)
    {
        if (d < 0)
            d += 2 * x + 3;
        else
        {
            d += 2 * (x - y) + 5;
            y--;
        }

        x++;
        Draw8Points(hdc, xc, yc, x, y, c);
    }
}

void drawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R, COLORREF c) {
    int x = 0;
    int y = R;

    int d = 1 - R;
    int ch1 = 3; //  2 * x + 3
    int ch2 = 5 - 2 * R; //  2 * (x - y) + 5

    while (x <= y)
    {
        Draw8Points(hdc, xc, yc, x, y, c);

        if (d < 0)
        {
            d += ch1;
            ch2 += 2;
        }
        else
        {
            d += ch2;
            ch2 += 4;
            y--;
        }

        ch1 += 2;
        x++;
    }

}

// --- 5. Ellipse Menu Functions ---
void drawEllipseDirect(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    for (int x = 0; x <= a; x++) {
        int y = round(b * sqrt(1.0 - (double)(x*x) / (a*a)));
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
    }
    for (int y = 0; y <= b; y++) {
        int x = round(a * sqrt(1.0 - (double)(y*y) / (b*b)));
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
    }
}

void drawEllipsePolar(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    float dTheta = 1.0 / max(a, b);
    for (float theta = 0; theta <= M_PI / 2; theta += dTheta) {
        int x = round(a * cos(theta));
        int y = round(b * sin(theta));
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
    }
}

void drawEllipseMidpoint(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    long long a2 = (long long)a * a;
    long long b2 = (long long)b * b;

    auto plot4 = [&](int x, int y) {
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
    };
    int x = 0, y = b;
    long long d1 = b2 - a2 * b + a2 / 4;
    while (b2 * x < a2 * y) {
        plot4(x, y);
        if (d1 < 0)
            d1 += b2 * (2 * x + 3);
        else {
            d1 += b2 * (2 * x + 3) + a2 * (-2 * y + 2);
            y--;
        }
        x++;
    }
    long long d2 = b2 * (x + 0.5) * (x + 0.5) + a2 * (y - 1) * (y - 1) - a2 * b2;
    while (y >= 0) {
        plot4(x, y);
        if (d2 > 0)
            d2 += a2 * (-2 * y + 3);
        else {
            d2 += b2 * (2 * x + 2) + a2 * (-2 * y + 3);
            x++;
        }
        y--;
    }
}

// --- 6. Curves Menu Functions ---
void DrawCardinalSpline(HDC hdc,Vector2 P[],int n,double c,int numpix)
{

    double c1=1-c;
    Vector2 T0(c1*(P[2].x-P[0].x),c1*(P[2].y-P[0].y));
    for(int i=2;i<n-1;i++)
    {
        Vector2 T1(c1*(P[i+1].x-P[i-1].x),c1*(P[i+1].y-P[i-1].y));
        DrawHermiteCurve(hdc,P[i-1],T0,P[i],T1,numpix);
        T0=T1;
    }

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
    // cout << "4. Circles Menu" << endl;
    cout << "5. Ellipse Menu" << endl;
    cout << "6. Curves Menu" << endl;
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
        case 5: {
            HDC hdc = GetDC(GetConsoleWindow());
            int subOption, xc, yc, a, b;
            COLORREF color = RGB(255, 255, 255);

            cout << "\n--- Ellipse Menu ---" << endl;
            cout << "1. Direct" << endl;
            cout << "2. Polar" << endl;
            cout << "3. Midpoint" << endl;
            cout << "Choice: ";
            cin >> subOption;

            cout << "Enter center (xc yc): ";
            cin >> xc >> yc;
            cout << "Enter radii (a b): ";
            cin >> a >> b;

            switch (subOption) {
                case 1: drawEllipseDirect(hdc, xc, yc, a, b, color); break;
                case 2: drawEllipsePolar(hdc, xc, yc, a, b, color);  break;
                case 3: drawEllipseMidpoint(hdc, xc, yc, a, b, color); break;
                default: cout << "Invalid choice." << endl;
            }

            ReleaseDC(GetConsoleWindow(), hdc);
            break;
        }
        case 6: {
            HDC hdc = GetDC(GetConsoleWindow());

            cout << "\n--- Curves Menu ---" << endl;
            cout << "1. Cardinal Spline" << endl;
            cout << "Choice: ";
            int subOption;
            cin >> subOption;

            if (subOption == 1) {
                int n;
                cout << "Enter number of control points (min 4): ";
                cin >> n;

                Vector2* P = new Vector2[n];
                cout << "Enter points (x y):" << endl;
                for (int i = 0; i < n; i++) {
                    cout << "P[" << i << "]: ";
                    cin >> P[i].x >> P[i].y;
                }

                double tension;
                cout << "Enter tension (0=Catmull-Rom, closer to 1=tighter): ";
                cin >> tension;

                DrawCardinalSpline(hdc, P, n, tension, 1000);
                delete[] P;
            } else {
                cout << "Invalid choice" << endl;
            }

            ReleaseDC(GetConsoleWindow(), hdc);
            break;
        }
        default:
            cout << "Invalid Option. Exiting." << endl;
            break;
    }

    return 0;
    
    return 0;
}
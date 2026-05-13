#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <stack>
#include <algorithm>
#include <climits>
#include <cstring>
#include <sstream>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace std;

// Data Structures
struct Point
{
    int x, y;
};
struct Vector2
{
    double x, y;
    Vector2(double a = 0, double b = 0)
    {
        x = a;
        y = b;
    }
};
class Vector4
{
    double v[4];

public:
    Vector4(double a = 0, double b = 0, double c = 0, double d = 0)
    {
        v[0] = a;
        v[1] = b;
        v[2] = c;
        v[3] = d;
    }

    Vector4(double a[])
    {
        memcpy(v, a, 4 * sizeof(double));
    }
    double &operator[](int i)
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
        memcpy(M, A, 16 * sizeof(double));
    }
    Vector4 &operator[](int i)
    {
        return M[i];
    }
};
// helper
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
double DotProduct(Vector4 &a, Vector4 &b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}
Vector4 operator*(Matrix4 M, Vector4 &b)
{
    Vector4 res;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            res[i] += M[i][j] * b[j];

    return res;
}
Vector4 GetHermiteCoeff(double x0, double s0, double x1, double s1)
{
    static double H[16] = {2, 1, -2, 1, -3, -2, 3, -1, 0, 1, 0, 0, 1, 0, 0, 0};
    static Matrix4 basis(H);
    Vector4 v(x0, s0, x1, s1);
    return basis * v;
}
void DrawHermiteCurve(HDC hdc, Vector2 &P0, Vector2 &T0, Vector2 &P1, Vector2 &T1, int numpoints)
{
    Vector4 xcoeff = GetHermiteCoeff(P0.x, T0.x, P1.x, T1.x);
    Vector4 ycoeff = GetHermiteCoeff(P0.y, T0.y, P1.y, T1.y);
    if (numpoints < 2)
        return;
    double dt = 1.0 / (numpoints - 1);
    for (double t = 0; t <= 1; t += dt)
    {
        Vector4 vt;
        vt[3] = 1;
        for (int i = 2; i >= 0; i--)
            vt[i] = vt[i + 1] * t;
        int x = round(DotProduct(xcoeff, vt));
        int y = round(DotProduct(ycoeff, vt));
        if (t == 0)
            MoveToEx(hdc, x, y, NULL);
        else
            LineTo(hdc, x, y);
    }
}
// Canvas + stroke log (minimal)
HWND g_canvas = nullptr;
COLORREF g_bgColor = RGB(30, 30, 38);
vector<string> g_drawLog;

static int g_waitN = 0;
static vector<POINT> g_collect;

static LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void ensureCanvas();
static void waitClicks(unsigned n, vector<POINT> &out);
static void replayOne(HDC hdc, const string &s);
static void replayAll(HDC hdc);

static void rgbAppend(ostringstream &o, COLORREF c)
{
    o << (int)GetRValue(c) << " " << (int)GetGValue(c) << " " << (int)GetBValue(c);
}

int currentShapeColor = 0;
string windowBackground = "Default (Black)";

COLORREF getDrawColor()
{
    static const COLORREF pal[] = {
        RGB(255, 255, 255),
        RGB(255, 80, 80),
        RGB(80, 255, 120),
        RGB(90, 180, 255),
        RGB(255, 220, 60),
        RGB(255, 120, 255),
        RGB(200, 200, 200),
        RGB(255, 160, 40),
    };
    return pal[currentShapeColor % 8];
}

void recordStroke(const string &line)
{
    if (!g_canvas)
        return;
    g_drawLog.push_back(line);
    InvalidateRect(g_canvas, nullptr, FALSE);
}

static string colorFields(COLORREF c)
{
    ostringstream o;
    rgbAppend(o, c);
    return o.str();
}

// 1. File Menu Implementation
void clearScreen()
{
    g_drawLog.clear();
    if (g_canvas)
        InvalidateRect(g_canvas, nullptr, TRUE);
    cout << "Screen cleared from shapes." << endl;
}

void saveData(const string &filename)
{
    ofstream file(filename);
    if (!file.is_open())
    {
        cout << "Error: Could not open file for saving." << endl;
        return;
    }
    file << "V2\n";
    file << "BG " << (int)GetRValue(g_bgColor) << " " << (int)GetGValue(g_bgColor) << " " << (int)GetBValue(g_bgColor) << "\n";
    for (const auto &line : g_drawLog)
        file << line << "\n";
    file.close();
    cout << "Saved " << g_drawLog.size() << " stroke(s) to " << filename << "." << endl;
}

void loadData(const string &filename)
{
    ifstream in(filename);
    if (!in.is_open())
    {
        cout << "Error: Could not open file for loading." << endl;
        return;
    }
    string head;
    if (!getline(in, head) || head != "V2")
    {
        cout << "Unsupported file (need V2 header)." << endl;
        return;
    }
    string bgLine;
    if (!getline(in, bgLine))
    {
        cout << "Invalid file." << endl;
        return;
    }
    g_drawLog.clear();
    istringstream ibg(bgLine);
    string bgt;
    int br, bgg, bb;
    if (ibg >> bgt >> br >> bgg >> bb && bgt == "BG")
        g_bgColor = RGB(br, bgg, bb);
    string line;
    while (getline(in, line))
    {
        if (!line.empty())
            g_drawLog.push_back(line);
    }
    if (g_canvas)
        InvalidateRect(g_canvas, nullptr, TRUE);
    cout << "Loaded " << g_drawLog.size() << " stroke(s)." << endl;
}

// 2. Preferences Menu Functions
void changeBackgroundToWhite()
{
    windowBackground = "White";
    g_bgColor = RGB(255, 255, 255);
    if (g_canvas)
        InvalidateRect(g_canvas, nullptr, TRUE);
    cout << "Background set white (canvas)." << endl;
}

void changeMouseShape()
{
    static int curIx = 0;
    if (!g_canvas)
    {
        cout << "Canvas not ready." << endl;
        return;
    }
    curIx = (curIx + 1) % 3;
    HCURSOR h = LoadCursor(nullptr, curIx == 0 ? IDC_ARROW : curIx == 1 ? IDC_CROSS
                                                                        : IDC_HAND);
    SetClassLongPtr(g_canvas, GCLP_HCURSOR, (LONG_PTR)h);
    SetCursor(h);
    cout << "Cursor style cycled on drawing window." << endl;
}

void setShapeColor(int colorValue)
{
    currentShapeColor = colorValue;
    cout << "Shape color selected (Code: " << currentShapeColor << ")." << endl;
}

// 3. Lines Menu Functions
void drawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));
    if (steps == 0)
    {
        SetPixel(hdc, x1, y1, c);
        return;
    }

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

void drawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int x = x1, y = y1;
    int DX = abs(x2 - x1), SX = x1 < x2 ? 1 : -1;
    int DY = -abs(y2 - y1), SY = y1 < y2 ? 1 : -1;
    int err = DX + DY;
    for (;;)
    {
        SetPixel(hdc, x, y, c);
        if (x == x2 && y == y2)
            break;
        int e2 = 2 * err;
        if (e2 >= DY)
        {
            err += DY;
            x += SX;
        }
        if (e2 <= DX)
        {
            err += DX;
            y += SY;
        }
    }
}

void drawLineParametric(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));
    if (steps == 0)
    {
        SetPixel(hdc, x1, y1, c);
        return;
    }
    double dt = 1.0 / steps;

    for (double t = 0; t <= 1; t += dt)
    {
        int x = x1 + dx * t;
        int y = y1 + dy * t;
        SetPixel(hdc, x, y, c);
    }
}
// 4. Circles Menu Functions
void drawCircleDirect(HDC hdc, int xc, int yc, int R, COLORREF c)
{
    int x = 0;
    int y = R;

    while (x <= y)
    {
        y = sqrt((double)R * R - x * x);
        Draw8Points(hdc, xc, yc, x, y, c);
        x++;
    }
}

void drawCirclePolar(HDC hdc, int xc, int yc, int R, COLORREF c)
{
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

void drawCircleIterativePolar(HDC hdc, int xc, int yc, int R, COLORREF c)
{
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

void drawCircleMidpoint(HDC hdc, int xc, int yc, int R, COLORREF c)
{
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

void drawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R, COLORREF c)
{
    int x = 0;
    int y = R;

    int d = 1 - R;
    int ch1 = 3;         //  2 * x + 3
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

// 5. Ellipse Menu Functions
void drawEllipseDirect(HDC hdc, int xc, int yc, int a, int b, COLORREF c)
{
    for (int x = 0; x <= a; x++)
    {
        int y = round(b * sqrt(1.0 - (double)(x * x) / (a * a)));
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
    }
    for (int y = 0; y <= b; y++)
    {
        int x = round(a * sqrt(1.0 - (double)(y * y) / (b * b)));
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
    }
}

void drawEllipsePolar(HDC hdc, int xc, int yc, int a, int b, COLORREF c)
{
    float dTheta = 1.0 / max(a, b);
    for (float theta = 0; theta <= M_PI / 2; theta += dTheta)
    {
        int x = round(a * cos(theta));
        int y = round(b * sin(theta));
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
    }
}

void drawEllipseMidpoint(HDC hdc, int xc, int yc, int a, int b, COLORREF c)
{
    long long a2 = (long long)a * a;
    long long b2 = (long long)b * b;

    auto plot4 = [&](int x, int y)
    {
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc - x, yc - y, c);
    };
    int x = 0, y = b;
    long long d1 = b2 - a2 * b + a2 / 4;
    while (b2 * x < a2 * y)
    {
        plot4(x, y);
        if (d1 < 0)
            d1 += b2 * (2 * x + 3);
        else
        {
            d1 += b2 * (2 * x + 3) + a2 * (-2 * y + 2);
            y--;
        }
        x++;
    }
    long long d2 = b2 * (x + 0.5) * (x + 0.5) + a2 * (y - 1) * (y - 1) - a2 * b2;
    while (y >= 0)
    {
        plot4(x, y);
        if (d2 > 0)
            d2 += a2 * (-2 * y + 3);
        else
        {
            d2 += b2 * (2 * x + 2) + a2 * (-2 * y + 3);
            x++;
        }
        y--;
    }
}

// 6. Curves Menu Functions
void DrawCardinalSpline(HDC hdc, Vector2 P[], int n, double c, int numpix)
{

    double c1 = 1 - c;
    Vector2 T0(c1 * (P[2].x - P[0].x), c1 * (P[2].y - P[0].y));
    for (int i = 2; i < n - 1; i++)
    {
        Vector2 T1(c1 * (P[i + 1].x - P[i - 1].x), c1 * (P[i + 1].y - P[i - 1].y));
        DrawHermiteCurve(hdc, P[i - 1], T0, P[i], T1, numpix);
        T0 = T1;
    }
}

// 7. Filling Menu (Student 4)
struct FillEdgeTableEntry
{
    int xmin, xmax;
    FillEdgeTableEntry() : xmin(INT_MAX), xmax(INT_MIN) {}
};

static void InitFillEntries(vector<FillEdgeTableEntry> &table)
{
    for (auto &e : table)
    {
        e.xmin = INT_MAX;
        e.xmax = INT_MIN;
    }
}

static void ScanEdgeForFill(int x1, int y1, int x2, int y2, vector<FillEdgeTableEntry> &table)
{
    if (y1 == y2)
        return;
    if (y1 > y2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    double x = x1;
    const double slopeInv = static_cast<double>(x2 - x1) / (y2 - y1);
    const int height = static_cast<int>(table.size());
    for (int y = y1; y <= y2; y++)
    {
        if (y >= 0 && y < height)
        {
            int xi = static_cast<int>(round(x));
            table[y].xmin = std::min(table[y].xmin, xi);
            table[y].xmax = std::max(table[y].xmax, xi);
        }
        x += slopeInv;
    }
}

void ConvexFill(const vector<POINT> &poly, vector<FillEdgeTableEntry> &table, HDC hdc, COLORREF c)
{
    InitFillEntries(table);
    for (int i = 0; i < (int)poly.size(); i++)
    {
        ScanEdgeForFill(poly[i].x, poly[i].y, poly[(i + 1) % poly.size()].x, poly[(i + 1) % poly.size()].y, table);
    }
    for (int y = 0; y < (int)table.size(); y++)
    {
        if (table[y].xmin <= table[y].xmax)
        {
            for (int x = table[y].xmin; x <= table[y].xmax; x++)
                SetPixel(hdc, x, y, c);
        }
    }
}

void fillC(int xc, int yc, int r, int q, HDC hdc, COLORREF c)
{
    for (int y = -r; y <= r; y++)
    {
        for (int x = -r; x <= r; x++)
        {
            if (x * x + y * y <= r * r)
            {
                bool test = false;
                if (q == 1 && x >= 0 && y >= 0)
                    test = true;
                if (q == 2 && x <= 0 && y >= 0)
                    test = true;
                if (q == 3 && x <= 0 && y <= 0)
                    test = true;
                if (q == 4 && x >= 0 && y <= 0)
                    test = true;
                if (test)
                    SetPixel(hdc, xc + x, yc + y, c);
            }
        }
    }
}

void fillS(int xmin, int xmax, int ymin, int ymax, HDC hdc, COLORREF c)
{
    for (double t = 0; t <= 1; t += 0.01)
    {
        double h1 = 2 * t * t * t - 3 * t * t + 1;
        double h2 = -2 * t * t * t + 3 * t * t;
        int y = static_cast<int>(h1 * ymin + h2 * ymax);
        for (int x = xmin; x <= xmax; x++)
            SetPixel(hdc, x, y, c);
    }
}

void fillR(int xmin, int xmax, int ymin, int ymax, HDC hdc, COLORREF c)
{
    for (double t = 0; t <= 1; t += 0.01)
    {
        int x = static_cast<int>((1 - t) * xmin + t * xmax);
        for (int y = ymin; y <= ymax; y++)
            SetPixel(hdc, x, y, c);
    }
}

void ffR(int x, int y, COLORREF oc, COLORREF nc, HDC hdc)
{
    if (GetPixel(hdc, x, y) != oc)
        return;
    SetPixel(hdc, x, y, nc);
    ffR(x + 1, y, oc, nc, hdc);
    ffR(x - 1, y, oc, nc, hdc);
    ffR(x, y + 1, oc, nc, hdc);
    ffR(x, y - 1, oc, nc, hdc);
}

void ffI(int x, int y, COLORREF oc, COLORREF nc, HDC hdc)
{
    stack<POINT> s;
    s.push({x, y});
    while (!s.empty())
    {
        POINT p = s.top();
        s.pop();
        if (GetPixel(hdc, p.x, p.y) == oc)
        {
            SetPixel(hdc, p.x, p.y, nc);
            s.push({p.x + 1, p.y});
            s.push({p.x - 1, p.y});
            s.push({p.x, p.y + 1});
            s.push({p.x, p.y - 1});
        }
    }
}

void fillP(vector<POINT> v, HDC hdc, COLORREF c)
{
    int ymin = v[0].y, ymax = v[0].y;
    for (auto p : v)
    {
        ymin = std::min(ymin, (int)p.y);
        ymax = std::max(ymax, (int)p.y);
    }
    for (int y = ymin; y <= ymax; y++)
    {
        vector<int> n;
        for (int i = 0; i < (int)v.size(); i++)
        {
            int j = (i + 1) % v.size();
            if ((v[i].y < y && v[j].y >= y) || (v[j].y < y && v[i].y >= y))
            {
                n.push_back(v[i].x + (y - v[i].y) * (v[j].x - v[i].x) / (v[j].y - v[i].y));
            }
        }
        sort(n.begin(), n.end());
        for (int i = 0; i < (int)n.size(); i += 2)
        {
            if (i + 1 < (int)n.size())
            {
                for (int x = n[i]; x <= n[i + 1]; x++)
                    SetPixel(hdc, x, y, c);
            }
        }
    }
}

static int fillPolyMaxY(const vector<POINT> &poly)
{
    int m = 0;
    for (const auto &p : poly)
        m = std::max(m, (int)p.y);
    return m;
}

static void FillCircleQuarterWithLines(HDC hdc, int xc, int yc, int r, int q, COLORREF c)
{
    HPEN pen = CreatePen(PS_SOLID, 1, c);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    for (int yr = -r; yr <= r; yr++)
    {
        long long disc = (long long)r * r - (long long)yr * yr;
        if (disc < 0)
            continue;
        int xr = (int)round(sqrt((double)disc));
        int yPix = yc + yr;
        if (q == 1 && yr >= 0)
        {
            MoveToEx(hdc, xc, yPix, nullptr);
            LineTo(hdc, xc + xr, yPix);
        }
        else if (q == 2 && yr >= 0)
        {
            MoveToEx(hdc, xc - xr, yPix, nullptr);
            LineTo(hdc, xc, yPix);
        }
        else if (q == 3 && yr <= 0)
        {
            MoveToEx(hdc, xc - xr, yPix, nullptr);
            LineTo(hdc, xc, yPix);
        }
        else if (q == 4 && yr <= 0)
        {
            MoveToEx(hdc, xc, yPix, nullptr);
            LineTo(hdc, xc + xr, yPix);
        }
    }
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

static void FillCircleQuarterWithCircles(HDC hdc, int xc, int yc, int r, int q, COLORREF c)
{
    for (int rk = 1; rk <= r; rk++)
        drawCircleMidpoint(hdc, xc, yc, rk, c);
    fillC(xc, yc, r, q, hdc, c);
}

void fillCircleWithLines(HDC hdc, int xc, int yc, int r, int quarter, COLORREF c)
{
    FillCircleQuarterWithLines(hdc, xc, yc, r, quarter, c);
}

void fillCircleWithCircles(HDC hdc, int xc, int yc, int r, int quarter, COLORREF c)
{
    FillCircleQuarterWithCircles(hdc, xc, yc, r, quarter, c);
}

void fillSquareWithHermit(HDC hdc, int xmin, int xmax, int ymin, int ymax, COLORREF c)
{
    fillS(xmin, xmax, ymin, ymax, hdc, c);
}

void fillRectangleWithBezier(HDC hdc, int xmin, int xmax, int ymin, int ymax, COLORREF c)
{
    fillR(xmin, xmax, ymin, ymax, hdc, c);
}

void fillConvex(HDC hdc, const vector<POINT> &poly, COLORREF c)
{
    if (poly.size() < 3)
        return;
    int h = std::max(fillPolyMaxY(poly) + 2, 1200);
    vector<FillEdgeTableEntry> table(h);
    ConvexFill(poly, table, hdc, c);
}

void fillNonConvex(HDC hdc, vector<POINT> poly, COLORREF c)
{
    fillP(std::move(poly), hdc, c);
}

void fillFloodRecursive(HDC hdc, int x, int y, COLORREF oldC, COLORREF newC)
{
    ffR(x, y, oldC, newC, hdc);
}

void fillFloodNonRecursive(HDC hdc, int x, int y, COLORREF oldC, COLORREF newC)
{
    ffI(x, y, oldC, newC, hdc);
}

// 8. Clipping & Bonus (Student 5)
enum CSOutCode : int
{
    CS_INSIDE = 0,
    CS_LEFT = 1,
    CS_RIGHT = 2,
    CS_BOTTOM = 4,
    CS_TOP = 8
};

static int computeCSOutCode(double x, double y, double xmin, double xmax, double ymin, double ymax)
{
    int code = CS_INSIDE;
    if (x < xmin)
        code |= CS_LEFT;
    else if (x > xmax)
        code |= CS_RIGHT;
    if (y < ymin)
        code |= CS_TOP;
    else if (y > ymax)
        code |= CS_BOTTOM;
    return code;
}

static void normalizeAABox(double &xmin, double &xmax, double &ymin, double &ymax)
{
    if (xmin > xmax)
        std::swap(xmin, xmax);
    if (ymin > ymax)
        std::swap(ymin, ymax);
}

static bool intersectCSBoundary(double x0, double y0, double x1, double y1, int edge,
                                double xmin, double xmax, double ymin, double ymax, double &xi, double &yi)
{
    double dx = x1 - x0;
    double dy = y1 - y0;
    if (edge == CS_LEFT)
    {
        xi = xmin;
        if (fabs(dx) < 1e-12)
            return false;
        yi = y0 + dy * ((xmin - x0) / dx);
        return true;
    }
    if (edge == CS_RIGHT)
    {
        xi = xmax;
        if (fabs(dx) < 1e-12)
            return false;
        yi = y0 + dy * ((xmax - x0) / dx);
        return true;
    }
    if (edge == CS_TOP)
    {
        yi = ymin;
        if (fabs(dy) < 1e-12)
            return false;
        xi = x0 + dx * ((ymin - y0) / dy);
        return true;
    }
    yi = ymax;
    if (fabs(dy) < 1e-12)
        return false;
    xi = x0 + dx * ((ymax - y0) / dy);
    return true;
}

static bool cohenSutherlandClip(double xmin, double xmax, double ymin, double ymax,
                                double &x0, double &y0, double &x1, double &y1)
{
    normalizeAABox(xmin, xmax, ymin, ymax);
    int out0 = computeCSOutCode(x0, y0, xmin, xmax, ymin, ymax);
    int out1 = computeCSOutCode(x1, y1, xmin, xmax, ymin, ymax);

    while (true)
    {
        if (!(out0 | out1))
            return true;
        if (out0 & out1)
            return false;

        int out = out0 ? out0 : out1;
        double xi = 0, yi = 0;
        bool ok = false;
        if (out & CS_BOTTOM)
            ok = intersectCSBoundary(x0, y0, x1, y1, CS_BOTTOM, xmin, xmax, ymin, ymax, xi, yi);
        else if (out & CS_TOP)
            ok = intersectCSBoundary(x0, y0, x1, y1, CS_TOP, xmin, xmax, ymin, ymax, xi, yi);
        else if (out & CS_RIGHT)
            ok = intersectCSBoundary(x0, y0, x1, y1, CS_RIGHT, xmin, xmax, ymin, ymax, xi, yi);
        else if (out & CS_LEFT)
            ok = intersectCSBoundary(x0, y0, x1, y1, CS_LEFT, xmin, xmax, ymin, ymax, xi, yi);
        if (!ok)
            return false;

        if (out == out0)
        {
            x0 = xi;
            y0 = yi;
            out0 = computeCSOutCode(x0, y0, xmin, xmax, ymin, ymax);
        }
        else
        {
            x1 = xi;
            y1 = yi;
            out1 = computeCSOutCode(x1, y1, xmin, xmax, ymin, ymax);
        }
    }
}

static POINT intersectVerticalLine(long xBoundary, POINT S, POINT E)
{
    if (E.x == S.x)
        return {xBoundary, S.y};
    double t = (double)(xBoundary - S.x) / (double)(E.x - S.x);
    return {xBoundary, S.y + (LONG)std::round(t * (double)(E.y - S.y))};
}

static POINT intersectHorizontalLine(long yBoundary, POINT S, POINT E)
{
    if (E.y == S.y)
        return {S.x, yBoundary};
    double t = (double)(yBoundary - S.y) / (double)(E.y - S.y);
    return {S.x + (LONG)std::round(t * (double)(E.x - S.x)), yBoundary};
}

static vector<POINT> shClipLeft(const vector<POINT> &in, long xmin)
{
    vector<POINT> out;
    if (in.empty())
        return out;
    POINT S = in.back();
    for (POINT E : in)
    {
        bool S_in = S.x >= xmin;
        bool E_in = E.x >= xmin;
        if (E_in)
        {
            if (!S_in)
                out.push_back(intersectVerticalLine(xmin, S, E));
            out.push_back(E);
        }
        else if (S_in)
            out.push_back(intersectVerticalLine(xmin, S, E));
        S = E;
    }
    return out;
}

static vector<POINT> shClipRight(const vector<POINT> &in, long xmax)
{
    vector<POINT> out;
    if (in.empty())
        return out;
    POINT S = in.back();
    for (POINT E : in)
    {
        bool S_in = S.x <= xmax;
        bool E_in = E.x <= xmax;
        if (E_in)
        {
            if (!S_in)
                out.push_back(intersectVerticalLine(xmax, S, E));
            out.push_back(E);
        }
        else if (S_in)
            out.push_back(intersectVerticalLine(xmax, S, E));
        S = E;
    }
    return out;
}

static vector<POINT> shClipBottom(const vector<POINT> &in, long ymax)
{
    vector<POINT> out;
    if (in.empty())
        return out;
    POINT S = in.back();
    for (POINT E : in)
    {
        bool S_in = S.y <= ymax;
        bool E_in = E.y <= ymax;
        if (E_in)
        {
            if (!S_in)
                out.push_back(intersectHorizontalLine(ymax, S, E));
            out.push_back(E);
        }
        else if (S_in)
            out.push_back(intersectHorizontalLine(ymax, S, E));
        S = E;
    }
    return out;
}

static vector<POINT> shClipTop(const vector<POINT> &in, long ymin)
{
    vector<POINT> out;
    if (in.empty())
        return out;
    POINT S = in.back();
    for (POINT E : in)
    {
        bool S_in = S.y >= ymin;
        bool E_in = E.y >= ymin;
        if (E_in)
        {
            if (!S_in)
                out.push_back(intersectHorizontalLine(ymin, S, E));
            out.push_back(E);
        }
        else if (S_in)
            out.push_back(intersectHorizontalLine(ymin, S, E));
        S = E;
    }
    return out;
}

static vector<POINT> clipPolygonRectangle(vector<POINT> poly, double xmin, double xmax, double ymin, double ymax)
{
    normalizeAABox(xmin, xmax, ymin, ymax);
    poly = shClipLeft(poly, (long)xmin);
    poly = shClipRight(poly, (long)xmax);
    poly = shClipBottom(poly, (long)ymax);
    poly = shClipTop(poly, (long)ymin);
    return poly;
}

static void drawAABoxOutline(HDC hdc, double xmin, double xmax, double ymin, double ymax, COLORREF border)
{
    HPEN pen = CreatePen(PS_DOT, 1, border);
    HGDIOBJ old = SelectObject(hdc, pen);
    MoveToEx(hdc, (int)std::lround(xmin), (int)std::lround(ymin), nullptr);
    LineTo(hdc, (int)std::lround(xmax), (int)std::lround(ymin));
    LineTo(hdc, (int)std::lround(xmax), (int)std::lround(ymax));
    LineTo(hdc, (int)std::lround(xmin), (int)std::lround(ymax));
    LineTo(hdc, (int)std::lround(xmin), (int)std::lround(ymin));
    SelectObject(hdc, old);
    DeleteObject(pen);
}

static void drawPolygonOutline(HDC hdc, const vector<POINT> &poly, COLORREF c)
{
    size_t n = poly.size();
    if (n < 2)
        return;
    for (size_t i = 0; i < n; ++i)
    {
        POINT a = poly[i], b = poly[(i + 1) % n];
        drawLineDDA(hdc, (int)a.x, (int)a.y, (int)b.x, (int)b.y, c);
    }
}

static bool pointInsideDisk(long long px, long long py, long long xc, long long yc, long long R2)
{
    long long dx = px - xc, dy = py - yc;
    return dx * dx + dy * dy <= R2;
}

static long long sqlen(long dx, long dy)
{
    return 1LL * dx * dx + 1LL * dy * dy;
}

static bool clipLineAgainstCircle(int x1, int y1, int x2, int y2, double xc, double yc, double R,
                                  double &ox1, double &oy1, double &ox2, double &oy2)
{
    const double Rx = xc, Ry = yc;
    const double xs1 = x1, ys1 = y1;
    const double xd = x2 - x1, yd = y2 - y1;
    const double Rsq = R * R;

    auto inside = [&](double px, double py)
    {
        double dx = px - Rx, dy = py - Ry;
        return dx * dx + dy * dy <= Rsq + 1e-9;
    };

    if (inside(xs1, ys1) && inside(xs1 + xd, ys1 + yd))
    {
        ox1 = xs1;
        oy1 = ys1;
        ox2 = xs1 + xd;
        oy2 = ys1 + yd;
        return true;
    }

    double fx = xs1 - Rx;
    double fy = ys1 - Ry;
    double a = xd * xd + yd * yd;
    if (a < 1e-15)
        return inside(xs1, ys1);

    double b = 2 * (fx * xd + fy * yd);
    double cc = fx * fx + fy * fy - Rsq;
    double disc = b * b - 4 * a * cc;
    if (disc < 0)
    {
        return inside(xs1, ys1) && inside(xs1 + xd, ys1 + yd);
    }

    double sdisc = sqrt(std::max(0.0, disc));
    double t0 = (-b - sdisc) / (2 * a);
    double t1 = (-b + sdisc) / (2 * a);
    if (t0 > t1)
        std::swap(t0, t1);

    double s = std::max(0.0, t0);
    double e = std::min(1.0, t1);
    if (s <= e + 1e-9)
    {
        ox1 = xs1 + s * xd;
        oy1 = ys1 + s * yd;
        ox2 = xs1 + e * xd;
        oy2 = ys1 + e * yd;
        return true;
    }

    if (inside(xs1, ys1) && inside(xs1 + xd, ys1 + yd))
    {
        ox1 = xs1;
        oy1 = ys1;
        ox2 = xs1 + xd;
        oy2 = ys1 + yd;
        return true;
    }
    return false;
}

static void drawCircleOutlineDotted(HDC hdc, double xc, double yc, double R, COLORREF c, int segments = 288)
{
    if (segments < 16)
        segments = 16;
    HPEN pen = CreatePen(PS_DOT, 1, c);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    MoveToEx(hdc, (int)std::lround(xc + R), (int)std::lround(yc), nullptr);
    for (int i = 1; i <= segments; ++i)
    {
        double th = (2.0 * M_PI * i) / segments;
        LineTo(hdc, (int)std::lround(xc + R * cos(th)), (int)std::lround(yc + R * sin(th)));
    }
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

static void strokedHermite(HDC hdc, Vector2 &P0, Vector2 &T0, Vector2 &P1, Vector2 &T1, int nPts, COLORREF c)
{
    HPEN pen = CreatePen(PS_SOLID, 1, c);
    HGDIOBJ old = SelectObject(hdc, pen);
    DrawHermiteCurve(hdc, P0, T0, P1, T1, nPts);
    SelectObject(hdc, old);
    DeleteObject(pen);
}

void clipRectanglePoint(HDC hdc, double xmin, double xmax, double ymin, double ymax,
                        int px, int py, COLORREF clipOutline, COLORREF acceptColor, COLORREF rejectColor)
{
    normalizeAABox(xmin, xmax, ymin, ymax);
    drawAABoxOutline(hdc, xmin, xmax, ymin, ymax, clipOutline);
    double x = px, y = py;
    if (computeCSOutCode(x, y, xmin, xmax, ymin, ymax) == CS_INSIDE)
    {
        cout << "[Rectangle clip POINT] ACCEPT." << endl;
        SetPixel(hdc, px, py, acceptColor);
    }
    else
    {
        cout << "[Rectangle clip POINT] REJECT." << endl;
        SetPixel(hdc, px, py, rejectColor);
    }
}

void clipRectangleLine(HDC hdc, double xmin, double xmax, double ymin, double ymax,
                       int x1, int y1, int x2, int y2, COLORREF clipOutline, COLORREF clippedColor, COLORREF originalColor)
{
    normalizeAABox(xmin, xmax, ymin, ymax);
    drawAABoxOutline(hdc, xmin, xmax, ymin, ymax, clipOutline);
    drawLineDDA(hdc, x1, y1, x2, y2, originalColor);

    double cx0 = x1, cy0 = y1, cx1 = x2, cy1 = y2;
    if (cohenSutherlandClip(xmin, xmax, ymin, ymax, cx0, cy0, cx1, cy1))
    {
        int ix0 = (int)std::lround(cx0), iy0 = (int)std::lround(cy0);
        int ix1 = (int)std::lround(cx1), iy1 = (int)std::lround(cy1);
        cout << "[Rectangle clip LINE] ACCEPT (" << ix0 << "," << iy0 << ") -> (" << ix1 << "," << iy1 << ")." << endl;
        drawLineDDA(hdc, ix0, iy0, ix1, iy1, clippedColor);
    }
    else
    {
        cout << "[Rectangle clip LINE] REJECT." << endl;
    }
}

void clipRectanglePolygon(HDC hdc, double xmin, double xmax, double ymin, double ymax,
                          const vector<POINT> &polyIn, COLORREF clipOutline, COLORREF clippedColor, COLORREF originalColor)
{
    if (polyIn.size() < 3)
    {
        cout << "[Rectangle clip POLYGON] Need >= 3 vertices." << endl;
        return;
    }
    normalizeAABox(xmin, xmax, ymin, ymax);
    drawAABoxOutline(hdc, xmin, xmax, ymin, ymax, clipOutline);
    drawPolygonOutline(hdc, polyIn, originalColor);

    vector<POINT> clipped = clipPolygonRectangle(polyIn, xmin, xmax, ymin, ymax);
    if (clipped.size() < 2)
    {
        cout << "[Rectangle clip POLYGON] REJECT / empty polygon." << endl;
        return;
    }
    cout << "[Rectangle clip POLYGON] Clipped vertex count: " << clipped.size() << endl;
    drawPolygonOutline(hdc, clipped, clippedColor);
}

void clipSquarePoint(HDC hdc, int left, int top, int side,
                     int px, int py, COLORREF clipOutline, COLORREF acceptColor, COLORREF rejectColor)
{
    if (side <= 0)
    {
        cout << "[Square clip POINT] Invalid side." << endl;
        return;
    }
    clipRectanglePoint(hdc, left, left + side, top, top + side, px, py, clipOutline, acceptColor, rejectColor);
}

void clipSquareLine(HDC hdc, int left, int top, int side,
                    int x1, int y1, int x2, int y2, COLORREF clipOutline, COLORREF clippedColor, COLORREF originalColor)
{
    if (side <= 0)
    {
        cout << "[Square clip LINE] Invalid side." << endl;
        return;
    }
    clipRectangleLine(hdc, left, left + side, top, top + side, x1, y1, x2, y2, clipOutline, clippedColor, originalColor);
}

void clipCirclePoint(HDC hdc, int xc, int yc, int R, int px, int py,
                     COLORREF clipOutline, COLORREF acceptColor, COLORREF rejectColor)
{
    if (R <= 0)
    {
        cout << "[Circle clip POINT] Invalid radius." << endl;
        return;
    }
    drawCircleOutlineDotted(hdc, xc, yc, R, clipOutline);
    long long R2 = 1LL * R * R;
    if (pointInsideDisk(px, py, xc, yc, R2))
    {
        cout << "[Circle clip POINT] ACCEPT." << endl;
        SetPixel(hdc, px, py, acceptColor);
    }
    else
    {
        cout << "[Circle clip POINT] REJECT." << endl;
        SetPixel(hdc, px, py, rejectColor);
    }
}

void clipCircleLine(HDC hdc, int xc, int yc, int R,
                    int x1, int y1, int x2, int y2, COLORREF clipOutline, COLORREF clippedColor, COLORREF originalColor)
{
    if (R <= 0)
    {
        cout << "[Circle clip LINE] Invalid radius." << endl;
        return;
    }
    drawCircleOutlineDotted(hdc, xc, yc, R, clipOutline);
    drawLineDDA(hdc, x1, y1, x2, y2, originalColor);
    double ox1, oy1, ox2, oy2;
    if (clipLineAgainstCircle(x1, y1, x2, y2, xc, yc, (double)R, ox1, oy1, ox2, oy2))
    {
        int ix0 = (int)std::lround(ox1), iy0 = (int)std::lround(oy1);
        int ix1 = (int)std::lround(ox2), iy1 = (int)std::lround(oy2);
        if (sqlen(ix1 - ix0, iy1 - iy0) <= 9)
        {
            cout << "[Circle clip LINE] ACCEPT (near-degenerate)." << endl;
            SetPixel(hdc, ix0, iy0, clippedColor);
        }
        else
        {
            cout << "[Circle clip LINE] ACCEPT." << endl;
            drawLineDDA(hdc, ix0, iy0, ix1, iy1, clippedColor);
        }
    }
    else
    {
        cout << "[Circle clip LINE] REJECT." << endl;
    }
}

void drawHappyFace(HDC hdc, int cx, int cy, int R, COLORREF c)
{
    if (R < 12)
        return;
    drawCircleMidpoint(hdc, cx, cy, R, c);
    int eyeR = std::max(2, R / 10);
    drawCircleMidpoint(hdc, cx - R / 3, cy - R / 5, eyeR, c);
    drawCircleMidpoint(hdc, cx + R / 3, cy - R / 5, eyeR, c);
    drawLineDDA(hdc, cx, cy - R / 15, cx, cy + R / 7, c);

    double mw = R / 2.2;
    double my = cy + R / 4.2;
    Vector2 P0(cx - mw, my);
    Vector2 P1(cx + mw, my);
    Vector2 T0(mw * 1.05, +R / 5.8);
    Vector2 T1(-mw * 1.05, +R / 5.8);
    strokedHermite(hdc, P0, T0, P1, T1, 140, c);
}

void drawSadFace(HDC hdc, int cx, int cy, int R, COLORREF c)
{
    if (R < 12)
        return;
    drawCircleMidpoint(hdc, cx, cy, R, c);
    int eyeR = std::max(2, R / 10);
    drawCircleMidpoint(hdc, cx - R / 3, cy - R / 5, eyeR, c);
    drawCircleMidpoint(hdc, cx + R / 3, cy - R / 5, eyeR, c);
    drawLineDDA(hdc, cx, cy - R / 15, cx, cy + R / 7, c);

    double mw = R / 2.2;
    double my = cy + R / 3.5;
    Vector2 P0(cx - mw, my);
    Vector2 P1(cx + mw, my);
    Vector2 T0(mw * 1.05, -R / 5.8);
    Vector2 T1(-mw * 1.05, -R / 5.8);
    strokedHermite(hdc, P0, T0, P1, T1, 140, c);
}

static void replayAll(HDC hdc)
{
    for (const auto &s : g_drawLog)
        replayOne(hdc, s);
}

static void replayOne(HDC hdc, const string &s)
{
    istringstream iss(s);
    string tok;
    iss >> tok;
    int x1, y1, x2, y2, xc, yc, r, q, n, a, b;
    double tens;
    int rr, gg, bb;
    auto readC = [&](istringstream &is) -> bool
    { return !!(is >> rr >> gg >> bb); };

    if (tok == "LINE_DDA" && iss >> x1 >> y1 >> x2 >> y2 && readC(iss))
        drawLineDDA(hdc, x1, y1, x2, y2, RGB(rr, gg, bb));
    else if (tok == "LINE_MP" && iss >> x1 >> y1 >> x2 >> y2 && readC(iss))
        drawLineMidpoint(hdc, x1, y1, x2, y2, RGB(rr, gg, bb));
    else if (tok == "LINE_PAR" && iss >> x1 >> y1 >> x2 >> y2 && readC(iss))
        drawLineParametric(hdc, x1, y1, x2, y2, RGB(rr, gg, bb));
    else if (tok == "C_DIR" && iss >> xc >> yc >> r && readC(iss))
        drawCircleDirect(hdc, xc, yc, r, RGB(rr, gg, bb));
    else if (tok == "C_POL" && iss >> xc >> yc >> r && readC(iss))
        drawCirclePolar(hdc, xc, yc, r, RGB(rr, gg, bb));
    else if (tok == "C_IPOL" && iss >> xc >> yc >> r && readC(iss))
        drawCircleIterativePolar(hdc, xc, yc, r, RGB(rr, gg, bb));
    else if (tok == "C_MID" && iss >> xc >> yc >> r && readC(iss))
        drawCircleMidpoint(hdc, xc, yc, r, RGB(rr, gg, bb));
    else if (tok == "C_MMOD" && iss >> xc >> yc >> r && readC(iss))
        drawCircleModifiedMidpoint(hdc, xc, yc, r, RGB(rr, gg, bb));
    else if (tok == "E_DIR" && iss >> xc >> yc >> a >> b && readC(iss))
        drawEllipseDirect(hdc, xc, yc, a, b, RGB(rr, gg, bb));
    else if (tok == "E_POL" && iss >> xc >> yc >> a >> b && readC(iss))
        drawEllipsePolar(hdc, xc, yc, a, b, RGB(rr, gg, bb));
    else if (tok == "E_MID" && iss >> xc >> yc >> a >> b && readC(iss))
        drawEllipseMidpoint(hdc, xc, yc, a, b, RGB(rr, gg, bb));
    else if (tok == "SPL")
    {
        if (iss >> n >> tens >> rr >> gg >> bb && n >= 4)
        {
            Vector2 *P = new Vector2[n];
            bool ok = true;
            for (int i = 0; i < n; ++i)
            {
                double px, py;
                if (!(iss >> px >> py))
                {
                    ok = false;
                    break;
                }
                P[i].x = px;
                P[i].y = py;
            }
            if (ok)
                DrawCardinalSpline(hdc, P, n, tens, 1000);
            delete[] P;
        }
    }
    else if (tok == "F_CWL" && iss >> xc >> yc >> r >> q && readC(iss))
        fillCircleWithLines(hdc, xc, yc, r, q, RGB(rr, gg, bb));
    else if (tok == "F_CWC" && iss >> xc >> yc >> r >> q && readC(iss))
        fillCircleWithCircles(hdc, xc, yc, r, q, RGB(rr, gg, bb));
    else if (tok == "F_SHER" && iss >> x1 >> x2 >> y1 >> y2 && readC(iss))
        fillSquareWithHermit(hdc, x1, x2, y1, y2, RGB(rr, gg, bb));
    else if (tok == "F_RECTB" && iss >> x1 >> x2 >> y1 >> y2 && readC(iss))
        fillRectangleWithBezier(hdc, x1, x2, y1, y2, RGB(rr, gg, bb));
    else if (tok == "F_CVEX" && iss >> n)
    {
        vector<POINT> poly(n);
        bool ok = true;
        for (int i = 0; i < n; ++i)
        {
            if (!(iss >> poly[i].x >> poly[i].y))
                ok = false;
        }
        if (ok && readC(iss))
            fillConvex(hdc, poly, RGB(rr, gg, bb));
    }
    else if (tok == "F_NCX" && iss >> n)
    {
        vector<POINT> poly(n);
        bool ok = true;
        for (int i = 0; i < n; ++i)
        {
            if (!(iss >> poly[i].x >> poly[i].y))
                ok = false;
        }
        if (ok && readC(iss))
            fillNonConvex(hdc, std::move(poly), RGB(rr, gg, bb));
    }
    else if (tok == "FLOOD_R")
    {
        int xr, yr, obr, obg, obb, nx, ny, nz;
        if (iss >> xr >> yr >> obr >> obg >> obb >> nx >> ny >> nz)
            fillFloodRecursive(hdc, xr, yr, RGB(obr, obg, obb), RGB(nx, ny, nz));
    }
    else if (tok == "FLOOD_I")
    {
        int xr, yr, obr, obg, obb, nx, ny, nz;
        if (iss >> xr >> yr >> obr >> obg >> obb >> nx >> ny >> nz)
            fillFloodNonRecursive(hdc, xr, yr, RGB(obr, obg, obb), RGB(nx, ny, nz));
    }
    else if (tok == "HAPPY" && iss >> xc >> yc >> r && readC(iss))
        drawHappyFace(hdc, xc, yc, r, RGB(rr, gg, bb));
    else if (tok == "SAD" && iss >> xc >> yc >> r && readC(iss))
        drawSadFace(hdc, xc, yc, r, RGB(rr, gg, bb));
}

static LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH hb = CreateSolidBrush(g_bgColor);
        FillRect(hdc, &rc, hb);
        DeleteObject(hb);
        replayAll(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN:
        if (g_waitN > 0 && (int)g_collect.size() < g_waitN)
        {
            int x = (int)(short)LOWORD(lParam);
            int y = (int)(short)HIWORD(lParam);
            g_collect.push_back({x, y});
        }
        return 0;
    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

static void ensureCanvas()
{
    if (g_canvas)
        return;
    static bool registered = false;
    WNDCLASSA wc{};
    wc.lpfnWndProc = CanvasProc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "GAPrjCanvasCls";
    wc.hCursor = LoadCursor(nullptr, IDC_CROSS);
    wc.hbrBackground = nullptr;
    if (!registered)
    {
        RegisterClassA(&wc);
        registered = true;
    }
    g_canvas = CreateWindowExA(WS_EX_LEFT, "GAPrjCanvasCls", "Drawing Canvas (use mouse here)",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT, CW_USEDEFAULT, 860, 640,
                               nullptr, nullptr, GetModuleHandleA(nullptr), nullptr);
    if (g_canvas)
    {
        ShowWindow(g_canvas, SW_SHOW);
        UpdateWindow(g_canvas);
    }
}

static void waitClicks(unsigned n, vector<POINT> &out)
{
    out.clear();
    if (!g_canvas || n == 0)
        return;
    g_collect.clear();
    g_waitN = (int)n;
    SetForegroundWindow(g_canvas);
    SetFocus(g_canvas);
    cout << "... click " << n << " point(s) on the Drawing Canvas window.\n";
    while ((unsigned)g_collect.size() < n)
    {
        MSG m;
        while (PeekMessageA(&m, nullptr, 0, 0, PM_REMOVE))
        {
            if (m.message == WM_QUIT)
                break;
            TranslateMessage(&m);
            DispatchMessageA(&m);
        }
        Sleep(10);
    }
    out.assign(g_collect.begin(), g_collect.begin() + (ptrdiff_t)n);
    g_waitN = 0;
}

static HDC hdcCanvas()
{
    return g_canvas ? GetDC(g_canvas) : nullptr;
}

// User Interaction Functions
void handleMouseInteraction()
{
    cout << "Use the Drawing Canvas window for mouse input when the menu asks for clicks.\n";
}

void displayConsoleLogs()
{
    cout << "Computer Graphics" << endl;
    cout << "0. Exit" << endl;
    cout << "1. File Menu" << endl;
    cout << "2. Preferences Menu" << endl;
    cout << "3. Lines Menu (mouse: two points)" << endl;
    cout << "4. Circles Menu (mouse: center then rim)" << endl;
    cout << "5. Ellipse Menu (mouse: center then axis corner)" << endl;
    cout << "6. Curves Menu" << endl;
    cout << "7. Filling Menu" << endl;
    cout << "8. Clipping and Bonus Menu" << endl;
    cout << "-------------------------------------------" << endl;
    cout << "Select option: ";
}

// Main Execution
int main()
{
    ensureCanvas();
    if (!g_canvas)
    {
        cerr << "Failed to create drawing window.\n";
        return 1;
    }
    cout << "Drawing Canvas window opened. Use it whenever the menu asks for mouse clicks.\n";

    while (true)
    {
        displayConsoleLogs();
        int option;
        cin >> option;

        switch (option)
        {
        case 0:
            cout << "Goodbye.\n";
            return 0;
        case 1:
        {
            int subOption;
            cout << "\n--- File Menu" << endl;
            cout << "1. Clear screen from shapes" << endl;
            cout << "2. Save data to a file" << endl;
            cout << "3. Load saved data from file" << endl;
            cout << "Choice: ";
            cin >> subOption;

            if (subOption == 1)
            {
                clearScreen();
            }
            else if (subOption == 2)
            {
                saveData("shapes_data.txt");
            }
            else if (subOption == 3)
            {
                loadData("shapes_data.txt");
            }
            break;
        }
        case 2:
        {
            int subOption;
            cout << "\n--- Preferences Menu" << endl;
            cout << "1. Change background to white" << endl;
            cout << "2. Change the shape of the window mouse" << endl;
            cout << "3. Choose shape color" << endl;
            cout << "Choice: ";
            cin >> subOption;

            if (subOption == 1)
            {
                changeBackgroundToWhite();
            }
            else if (subOption == 2)
            {
                changeMouseShape();
            }
            else if (subOption == 3)
            {
                int c;
                cout << "Enter color code/number: ";
                cin >> c;
                setShapeColor(c);
            }
            break;
        }
        case 3:
        {
            HDC hdc = GetDC(g_canvas);
            COLORREF color = getDrawColor();
            cout << "\n--- Lines Menu" << endl;
            cout << "1. DDA  2. Midpoint  3. Parametric" << endl;
            cout << "Choice: ";
            int sm;
            cin >> sm;
            vector<POINT> pt;
            waitClicks(2, pt);
            if ((int)pt.size() >= 2 && hdc)
            {
                int x1 = pt[0].x, y1 = pt[0].y, x2 = pt[1].x, y2 = pt[1].y;
                ostringstream ln;
                if (sm == 1)
                {
                    drawLineDDA(hdc, x1, y1, x2, y2, color);
                    ln << "LINE_DDA " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << colorFields(color);
                    recordStroke(ln.str());
                }
                else if (sm == 2)
                {
                    drawLineMidpoint(hdc, x1, y1, x2, y2, color);
                    ln << "LINE_MP " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << colorFields(color);
                    recordStroke(ln.str());
                }
                else if (sm == 3)
                {
                    drawLineParametric(hdc, x1, y1, x2, y2, color);
                    ln << "LINE_PAR " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << colorFields(color);
                    recordStroke(ln.str());
                }
                else
                {
                    cout << "Invalid lines choice." << endl;
                }
            }
            ReleaseDC(g_canvas, hdc);
            break;
        }
        case 4:
        {
            HDC hdc = GetDC(g_canvas);
            COLORREF color = getDrawColor();
            cout << "\n--- Circles Menu" << endl;
            cout << "1.Direct  2.Polar  3.Iter.Polar  4.Midpoint  5.Mod.Midpoint" << endl;
            cout << "Choice: ";
            int sm;
            cin >> sm;
            vector<POINT> pt;
            waitClicks(2, pt);
            if ((int)pt.size() >= 2 && hdc)
            {
                int xc = pt[0].x, yc = pt[0].y;
                long long dx = pt[1].x - xc, dy = pt[1].y - yc;
                int R = (int)(sqrt(double(dx * dx + dy * dy)) + 0.5);
                if (R < 1)
                    R = 1;
                ostringstream ln;
                if (sm == 1)
                {
                    drawCircleDirect(hdc, xc, yc, R, color);
                    ln << "C_DIR " << xc << " " << yc << " " << R << " " << colorFields(color);
                }
                else if (sm == 2)
                {
                    drawCirclePolar(hdc, xc, yc, R, color);
                    ln << "C_POL " << xc << " " << yc << " " << R << " " << colorFields(color);
                }
                else if (sm == 3)
                {
                    drawCircleIterativePolar(hdc, xc, yc, R, color);
                    ln << "C_IPOL " << xc << " " << yc << " " << R << " " << colorFields(color);
                }
                else if (sm == 4)
                {
                    drawCircleMidpoint(hdc, xc, yc, R, color);
                    ln << "C_MID " << xc << " " << yc << " " << R << " " << colorFields(color);
                }
                else if (sm == 5)
                {
                    drawCircleModifiedMidpoint(hdc, xc, yc, R, color);
                    ln << "C_MMOD " << xc << " " << yc << " " << R << " " << colorFields(color);
                }
                else
                {
                    cout << "Invalid circles choice.\n";
                    ReleaseDC(g_canvas, hdc);
                    break;
                }
                if (sm >= 1 && sm <= 5)
                    recordStroke(ln.str());
            }
            ReleaseDC(g_canvas, hdc);
            break;
        }
        case 5:
        {
            HDC hdc = GetDC(g_canvas);
            COLORREF color = getDrawColor();
            cout << "\n--- Ellipse Menu" << endl;
            cout << "1. Direct  2. Polar  3. Midpoint" << endl;
            cout << "Choice: ";
            int subOption;
            cin >> subOption;
            cout << "(Mouse) ellipse center then a corner defining axis-aligned a,b.\n";
            vector<POINT> pt;
            waitClicks(2, pt);
            int xc = 0, yc = 0, a = 1, b = 1;
            if ((int)pt.size() >= 2)
            {
                xc = pt[0].x;
                yc = pt[0].y;
                a = std::abs(pt[1].x - xc);
                b = std::abs(pt[1].y - yc);
                if (a < 1)
                    a = 1;
                if (b < 1)
                    b = 1;
                switch (subOption)
                {
                case 1:
                    if (hdc)
                        drawEllipseDirect(hdc, xc, yc, a, b, color);
                    break;
                case 2:
                    if (hdc)
                        drawEllipsePolar(hdc, xc, yc, a, b, color);
                    break;
                case 3:
                    if (hdc)
                        drawEllipseMidpoint(hdc, xc, yc, a, b, color);
                    break;
                default:
                    cout << "Invalid choice." << endl;
                }
                if (subOption >= 1 && subOption <= 3 && hdc)
                {
                    string tag = subOption == 1 ? "E_DIR" : subOption == 2 ? "E_POL"
                                                                           : "E_MID";
                    ostringstream oe;
                    oe << tag << " " << xc << " " << yc << " " << a << " " << b << " " << colorFields(color);
                    recordStroke(oe.str());
                }
            }
            else
                cout << "Need two mouse clicks.\n";
            ReleaseDC(g_canvas, hdc);
            break;
        }
        case 6:
        {
            HDC hdc = GetDC(g_canvas);
            COLORREF color = getDrawColor();

            cout << "\n--- Curves Menu" << endl;
            cout << "1. Cardinal Spline" << endl;
            cout << "Choice: ";
            int subOption;
            cin >> subOption;

            if (subOption == 1 && hdc)
            {
                int n;
                cout << "Enter number of control points (min 4): ";
                cin >> n;

                Vector2 *P = new Vector2[n];
                cout << "Enter points (x y):" << endl;
                for (int i = 0; i < n; i++)
                {
                    cout << "P[" << i << "]: ";
                    cin >> P[i].x >> P[i].y;
                }

                double tension;
                cout << "Enter tension (0=Catmull-Rom, closer to 1=tighter): ";
                cin >> tension;

                DrawCardinalSpline(hdc, P, n, tension, 1000);
                ostringstream os;
                os << "SPL " << n << " " << tension << " ";
                rgbAppend(os, color);
                for (int i = 0; i < n; ++i)
                    os << " " << P[i].x << " " << P[i].y;
                recordStroke(os.str());
                delete[] P;
            }
            else
            {
                cout << "Invalid choice" << endl;
            }

            ReleaseDC(g_canvas, hdc);
            break;
        }
        case 7:
        {
            HDC hdc = GetDC(g_canvas);
            COLORREF color = getDrawColor();

            cout << "\n--- Filling Menu" << endl;
            cout << "1. Circle fill (horizontal lines, quarter 1-4)" << endl;
            cout << "2. Circle fill (concentric circles + quarter interior)" << endl;
            cout << "3. Square fill (vertical Hermite blend between ymin/ymax)" << endl;
            cout << "4. Rectangle fill (horizontal sweep, linear blend xmin-xmax)" << endl;
            cout << "5. Convex polygon (scan-fill; use screen y >= 0)" << endl;
            cout << "6. Non-convex polygon" << endl;
            cout << "7. Recursive flood fill (RGB in console + seed mouse click)" << endl;
            cout << "8. Non-recursive flood fill (same)" << endl;
            cout << "Choice: ";
            int sub;
            cin >> sub;

            if (sub == 1 || sub == 2)
            {
                int q;
                cout << "Quarter (1..4): ";
                cin >> q;
                cout << "(Mouse) circle center then a point on the rim.\n";
                vector<POINT> pt;
                waitClicks(2, pt);
                if ((int)pt.size() >= 2 && hdc && q >= 1 && q <= 4)
                {
                    int xc = pt[0].x, yc = pt[0].y;
                    long long dx = pt[1].x - xc, dy = pt[1].y - yc;
                    int rr = (int)(sqrt(double(dx * dx + dy * dy)) + 0.5);
                    if (rr < 1)
                        rr = 1;
                    if (sub == 1)
                    {
                        fillCircleWithLines(hdc, xc, yc, rr, q, color);
                        ostringstream of;
                        of << "F_CWL " << xc << " " << yc << " " << rr << " " << q << " " << colorFields(color);
                        recordStroke(of.str());
                    }
                    else
                    {
                        fillCircleWithCircles(hdc, xc, yc, rr, q, color);
                        ostringstream of;
                        of << "F_CWC " << xc << " " << yc << " " << rr << " " << q << " " << colorFields(color);
                        recordStroke(of.str());
                    }
                }
                else if (q < 1 || q > 4)
                    cout << "Invalid quarter.\n";
            }
            else if (sub == 3)
            {
                int xmin, xmax, ymin, ymax;
                cout << "Enter xmin xmax ymin ymax: ";
                cin >> xmin >> xmax >> ymin >> ymax;
                if (hdc)
                    fillSquareWithHermit(hdc, xmin, xmax, ymin, ymax, color);
                ostringstream of;
                of << "F_SHER " << xmin << " " << xmax << " " << ymin << " " << ymax << " " << colorFields(color);
                recordStroke(of.str());
            }
            else if (sub == 4)
            {
                int xmin, xmax, ymin, ymax;
                cout << "Enter xmin xmax ymin ymax: ";
                cin >> xmin >> xmax >> ymin >> ymax;
                if (hdc)
                    fillRectangleWithBezier(hdc, xmin, xmax, ymin, ymax, color);
                ostringstream of;
                of << "F_RECTB " << xmin << " " << xmax << " " << ymin << " " << ymax << " " << colorFields(color);
                recordStroke(of.str());
            }
            else if (sub == 5)
            {
                int n;
                cout << "Vertex count (>=3): ";
                cin >> n;
                if (n < 3)
                {
                    cout << "Need at least 3 vertices." << endl;
                }
                else
                {
                    vector<POINT> poly(n);
                    for (int i = 0; i < n; i++)
                    {
                        cout << "P" << i << " (x y): ";
                        cin >> poly[i].x >> poly[i].y;
                    }
                    if (hdc)
                        fillConvex(hdc, poly, color);
                    ostringstream fc;
                    fc << "F_CVEX " << n;
                    for (int i = 0; i < n; ++i)
                        fc << " " << poly[i].x << " " << poly[i].y;
                    fc << " " << colorFields(color);
                    recordStroke(fc.str());
                }
            }
            else if (sub == 6)
            {
                int n;
                cout << "Vertex count (>=3): ";
                cin >> n;
                if (n < 3)
                {
                    cout << "Need at least 3 vertices." << endl;
                }
                else
                {
                    vector<POINT> poly(n);
                    for (int i = 0; i < n; i++)
                    {
                        cout << "P" << i << " (x y): ";
                        cin >> poly[i].x >> poly[i].y;
                    }
                    if (hdc)
                        fillNonConvex(hdc, poly, color);
                    ostringstream fc;
                    fc << "F_NCX " << n;
                    for (int i = 0; i < n; ++i){
                        fc << " " << poly[i].x << " " << poly[i].y;
                    }
                    fc << " " << colorFields(color);
                    recordStroke(fc.str());
                }
            }
            else if (sub == 7 || sub == 8)
            {
                int obr, obg, obb, nbr, nbg, nbb;
                cout << "Enter OLD rgb then NEW rgb (6 ints): ";
                cin >> obr >> obg >> obb >> nbr >> nbg >> nbb;
                cout << "(Mouse) click seed pixel.\n";
                vector<POINT> pt;
                waitClicks(1, pt);
                if ((int)pt.size() >= 1 && hdc)
                {
                    int x = pt[0].x, y = pt[0].y;
                    COLORREF oldC = RGB(obr, obg, obb);
                    COLORREF newC = RGB(nbr, nbg, nbb);
                    if (sub == 7)
                        fillFloodRecursive(hdc, x, y, oldC, newC);
                    else
                        fillFloodNonRecursive(hdc, x, y, oldC, newC);
                    ostringstream ff;
                    ff << (sub == 7 ? "FLOOD_R " : "FLOOD_I ") << x << " " << y << " " << obr << " " << obg << " " << obb << " ";
                    ff << nbr << " " << nbg << " " << nbb;
                    recordStroke(ff.str());
                }
            }
            else
            {
                cout << "Invalid filling option." << endl;
            }

            ReleaseDC(g_canvas, hdc);
            break;
        }
        case 8:
        {
            HDC hdc = GetDC(g_canvas);
            const COLORREF outline = RGB(0, 220, 90);
            const COLORREF clipped = RGB(255, 255, 255);
            const COLORREF original = RGB(120, 120, 120);
            const COLORREF reject = RGB(255, 40, 40);

            cout << "\n--- Clipping and Bonus Menu" << endl;
            cout << "1. Rectangle clip: point" << endl;
            cout << "2. Rectangle clip: line (Cohen-Sutherland)" << endl;
            cout << "3. Rectangle clip: polygon (Sutherland-Hodgman)" << endl;
            cout << "4. Square clip: point" << endl;
            cout << "5. Square clip: line" << endl;
            cout << "6. Circle clip: point" << endl;
            cout << "7. Circle clip: line (parametric quadratic)" << endl;
            cout << "8. Happy face (circles + line + Hermite mouth)" << endl;
            cout << "9. Sad face" << endl;
            cout << "Choice: ";
            int sub;
            cin >> sub;

            if (sub == 1)
            {
                double xa, xb, ya, yb;
                int px, py;
                cout << "Clip rectangle: xmin xmax ymin ymax, then point px py: ";
                cin >> xa >> xb >> ya >> yb >> px >> py;
                clipRectanglePoint(hdc, xa, xb, ya, yb, px, py, outline, clipped, reject);
            }
            else if (sub == 2)
            {
                double xa, xb, ya, yb;
                int x1, y1, x2, y2;
                cout << "Rectangle xmin xmax ymin ymax, line x1 y1 x2 y2: ";
                cin >> xa >> xb >> ya >> yb >> x1 >> y1 >> x2 >> y2;
                clipRectangleLine(hdc, xa, xb, ya, yb, x1, y1, x2, y2, outline, clipped, original);
            }
            else if (sub == 3)
            {
                double xa, xb, ya, yb;
                int n;
                cout << "Rectangle xmin xmax ymin ymax, then vertex count n: ";
                cin >> xa >> xb >> ya >> yb >> n;
                if (n < 3)
                {
                    cout << "Need at least 3 polygon vertices." << endl;
                }
                else
                {
                    vector<POINT> poly(n);
                    for (int i = 0; i < n; ++i)
                    {
                        cout << "Vertex " << i << " (x y): ";
                        cin >> poly[i].x >> poly[i].y;
                    }
                    clipRectanglePolygon(hdc, xa, xb, ya, yb, poly, outline, clipped, original);
                }
            }
            else if (sub == 4)
            {
                int left, top, side, px, py;
                cout << "Square: left top side, then point px py: ";
                cin >> left >> top >> side >> px >> py;
                clipSquarePoint(hdc, left, top, side, px, py, outline, clipped, reject);
            }
            else if (sub == 5)
            {
                int left, top, side, x1, y1, x2, y2;
                cout << "Square: left top side, then line x1 y1 x2 y2: ";
                cin >> left >> top >> side >> x1 >> y1 >> x2 >> y2;
                clipSquareLine(hdc, left, top, side, x1, y1, x2, y2, outline, clipped, original);
            }
            else if (sub == 6)
            {
                int xc, yc, R, px, py;
                cout << "Circle: xc yc R, then point px py: ";
                cin >> xc >> yc >> R >> px >> py;
                clipCirclePoint(hdc, xc, yc, R, px, py, outline, clipped, reject);
            }
            else if (sub == 7)
            {
                int xc, yc, R, x1, y1, x2, y2;
                cout << "Circle: xc yc R, then line x1 y1 x2 y2: ";
                cin >> xc >> yc >> R >> x1 >> y1 >> x2 >> y2;
                clipCircleLine(hdc, xc, yc, R, x1, y1, x2, y2, outline, clipped, original);
            }
            else if (sub == 8)
            {
                COLORREF fc = getDrawColor();
                cout << "(Mouse) Happy face: center then rim for radius.\n";
                vector<POINT> pt;
                waitClicks(2, pt);
                if ((int)pt.size() >= 2 && hdc)
                {
                    int cx = pt[0].x, cy = pt[0].y;
                    long long dx = pt[1].x - cx, dy = pt[1].y - cy;
                    int R = (int)(sqrt(double(dx * dx + dy * dy)) + 0.5);
                    if (R < 12)
                        R = 12;
                    drawHappyFace(hdc, cx, cy, R, fc);
                    ostringstream oh;
                    oh << "HAPPY " << cx << " " << cy << " " << R << " " << colorFields(fc);
                    recordStroke(oh.str());
                    cout << "Drew happy face." << endl;
                }
            }
            else if (sub == 9)
            {
                COLORREF fc = getDrawColor();
                cout << "(Mouse) Sad face: center then rim for radius.\n";
                vector<POINT> pt;
                waitClicks(2, pt);
                if ((int)pt.size() >= 2 && hdc)
                {
                    int cx = pt[0].x, cy = pt[0].y;
                    long long dx = pt[1].x - cx, dy = pt[1].y - cy;
                    int R = (int)(sqrt(double(dx * dx + dy * dy)) + 0.5);
                    if (R < 12)
                        R = 12;
                    drawSadFace(hdc, cx, cy, R, fc);
                    ostringstream osad;
                    osad << "SAD " << cx << " " << cy << " " << R << " " << colorFields(fc);
                    recordStroke(osad.str());
                    cout << "Drew sad face." << endl;
                }
            }
            else
            {
                cout << "Invalid clipping/bonus option." << endl;
            }

            ReleaseDC(g_canvas, hdc);
            break;
        }
        default:
            cout << "Invalid menu option.\n";
            break;
        }
    }
    return 0;
}
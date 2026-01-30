#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

struct Point
{
    float x;
    float y;
    float z;
};
struct Point2D
{
    int x;
    int y;
};

class Cube{

    Point vertices[8] = {
        {-1.0f, -1.0f, -1.0f},     // Vertex 0
        {-1.0f, 1.0f, -1.0f},    // Vertex 1
        {1.0f, 1.0f, -1.0f},    // Vertex 2
        {1.0f, -1.0f, -1.0f},   // Vertex 3
        {1.0f, 1.0f, 1.0f},    // Vertex 4
        {1.0f, -1.0f, 1.0f},   // Vertex 5
        {-1.0f, -1.0f, 1.0f},   // Vertex 6
        {-1.0f, 1.0f, 1.0f}   // Vertex 7
    };

    uint8_t triangles[12][3] = {
        {0, 1, 2}, {0, 2, 3}, // Front face
        {3, 2, 4}, {3, 4, 5}, // Right face
        {5, 4, 7}, {5, 7, 6}, // Back face
        {6, 7, 1}, {6, 1, 0}, // Left face
        {6, 0, 3}, {6, 3, 5}, // Top face
        {1, 7, 4}, {1, 4, 2}  // Bottom face
    };

    uint8_t symbols[12] = {
        '$', '$', '*', '*', '+', '+', '-', '-', '@', '@', '=', '='
    };

    // Viewport for 3D projection
    static const int VP_COL = 200;
    static const int VP_ROW = 50;
    uint8_t viewport[VP_ROW][VP_COL] = {' '};
    // Camera for back-face culling
    Point camera = { 0, 0, 1};

    
    void ClearViewport()
    {
        for(int r = 0; r < VP_ROW; r++)
        {
            for(int c = 0; c < VP_COL; c++)
            {
                viewport[r][c] = ' ';
            }
        }
    }

    // Function for back-face culling
    float product(Point a, Point b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Point2D project(Point p)
    {
        Point2D projected;
        projected.x = std::round(p.x / p.z + VP_COL / 2);
        projected.y = std::round(p.y / p.z + VP_ROW / 2);
        return projected;
    }
/* Drawing Methods */
    void DrawTriangle(Point2D p1, Point2D p2, Point2D p3, uint8_t symbol)
    {
        //sort by asc y
        if(p1.y > p2.y) std::swap(p1, p2);
        if(p2.y > p3.y) std::swap(p2, p3);
        if(p1.y > p2.y) std::swap(p1, p2);

        if(p2.y == p3.y)
        {
            DrawFlatBottomTriangle(p1, p2, p3, symbol);
        }
        else if(p1.y == p2.y)
        {
            DrawFlatTopTriangle(p1, p2, p3, symbol);
        }
        else
        {
            //split triangle
            Point2D mid;
            mid.y = p2.y;
            mid.x = p1.x + (float)(p3.x - p1.x) * (float)(p2.y - p1.y) / (float)(p3.y - p1.y);
            DrawFlatBottomTriangle(p1, p2, mid, symbol);
            DrawFlatTopTriangle(p2, mid, p3, symbol);
        }
    }

    void DrawFlatTopTriangle(Point2D t0, Point2D t1, Point2D b, uint8_t symbol)
    {
        // ignoring left and right
        int x_b = t0.x;
        int x_e = t1.x;
        for(int y = t0.y; y <= b.y; y++)
        {
            DrawScanLine(y, x_b, x_e, symbol);
            x_b += (b.x - t0.x) / (b.y - t0.y);
            x_e += (b.x - t1.x) / (b.y - t1.y);
        }
    }
    void DrawFlatBottomTriangle(Point2D t, Point2D b1, Point2D b2, uint8_t symbol)
    {
        // ignoring left and right
        int x_b = t.x;
        int x_e = t.x;
        for(int y = t.y; y <= b1.y; y++)
        {
            DrawScanLine(y, x_b, x_e, symbol);
            x_b -= (t.x - b1.x) / (b1.y - t.y);
            x_e -= (t.x - b2.x) / (b2.y - t.y);
        }
    }
    void DrawScanLine(int y, int xStart, int xEnd, uint8_t symbol)
    {
        if(xStart > xEnd)
            std::swap(xStart, xEnd);

        for(int x = xStart; x <= xEnd; x++)
        {
            if(x >= 0 && x < VP_COL && y >= 0 && y < VP_ROW)
                viewport[y][x] = symbol;
        }
    }
    /* Rotation Methods */
    void rotateAroundX(Point &p, float radians)
    {
        float cosA = std::cos(radians);
        float sinA = std::sin(radians);
        float y = p.y * cosA - p.z * sinA;
        float z = p.y * sinA + p.z * cosA;
        p.y = y;
        p.z = z;
    }
    void rotateAroundY(Point &p, float radians)
    {
        float cosA = std::cos(radians);
        float sinA = std::sin(radians);
        float x = cosA * p.x + sinA * p.z;
        float z = p.z * cosA - p.x * sinA;
        p.x = x;
        p.z = z;
    }
    void rotateAroundZ(Point &p, float radians)
    {
        float cosA = std::cos(radians);
        float sinA = std::sin(radians);
        float x = p.x * cosA - p.y * sinA;
        float y = p.x * sinA + p.y * cosA;
        p.x = x;
        p.y = y;
    }

    public:

        Cube() {}

        void Draw(float rx, float ry, float rz)
        {
            ClearViewport();
            int symbolIndex = 0;

            for(auto triangle : triangles)
            {
                Point transformed[3];
                for(int i = 0; i < 3; i++)
                {
                    transformed[i] = vertices[triangle[i]];
                    // Rotate
                    rotateAroundX(transformed[i], rx);
                    rotateAroundY(transformed[i], ry);
                    rotateAroundZ(transformed[i], rz);
                    // Push it away from camera
                    transformed[i].z += 8.0f;
                    
                    // Scale it
                    float scale = 60.0f;
                    transformed[i].x *= scale * 3;
                    transformed[i].y *= scale;
                }

                // back-face culling
                // Compute normal 
                Point line1, line2, normal;
                // first line
                line1.x = transformed[1].x - transformed[0].x;
                line1.y = transformed[1].y - transformed[0].y;
                line1.z = transformed[1].z - transformed[0].z;
                // second line
                line2.x = transformed[2].x - transformed[0].x;
                line2.y = transformed[2].y - transformed[0].y;
                line2.z = transformed[2].z - transformed[0].z;
                // Cross product
                normal.x = line1.y * line2.z - line1.z * line2.y;
                normal.y = line1.z * line2.x - line1.x * line2.z;
                normal.z = line1.x * line2.y - line1.y * line2.x;
            
                // Vector from camera to triangle
                float res = product(camera, normal);
                if(res >= 0)
                    continue;

                // Project to 2D
                Point2D projected[3];
                for(int i = 0; i < 3; i++)
                {
                    projected[i] = project(transformed[i]);
                }

                // Draw the triangle
                DrawTriangle(projected[0], projected[1], projected[2], symbols[symbolIndex]);
                symbolIndex = (symbolIndex + 1) % 12;
            }
        }

        void Render()
        {
            system("clear"); // For Windows use "cls"

            for(int r = 0; r < VP_ROW; r++)
            {
                for(int c = 0; c < VP_COL; c++)
                {
                    std::cout << viewport[r][c];
                }
                std::cout << std::endl;
            }
        }
};

int main() {
    Cube myCube;
    float radians[3] = {0.0f, 0.0f, 0.0f};
    while(true)
    {
        myCube.Draw(radians[0], radians[1], radians[2]);
        myCube.Render();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        radians[0] += 0.05f;
        radians[1] += 0.03f;
        radians[2] += 0.02f;
    }
    return 0;
}

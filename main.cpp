#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int ScreenWidth = 1280;
constexpr int ScreenHeight = 800;
constexpr int PanelWidth = 380;

enum class ObjectType {
    Human,
    Tree,
    House,
    Table,
    Rock,
    Lamp
};

struct SceneObject {
    ObjectType type = ObjectType::Human;
    std::string name;
    Vector3 position{0.0f, 0.0f, 0.0f};
    float scale = 1.0f;
    Color color{100, 170, 230, 255};
};

struct Project {
    std::string name = "MyGame";
    std::vector<SceneObject> objects;
    int selected = -1;
    std::filesystem::path filePath;
};

const char* typeName(ObjectType type)
{
    switch (type) {
    case ObjectType::Human: return "Human";
    case ObjectType::Tree: return "Tree";
    case ObjectType::House: return "House";
    case ObjectType::Table: return "Table";
    case ObjectType::Rock: return "Rock";
    case ObjectType::Lamp: return "Lamp";
    }
    return "Object";
}

const char* typeId(ObjectType type)
{
    switch (type) {
    case ObjectType::Human: return "HUMAN";
    case ObjectType::Tree: return "TREE";
    case ObjectType::House: return "HOUSE";
    case ObjectType::Table: return "TABLE";
    case ObjectType::Rock: return "ROCK";
    case ObjectType::Lamp: return "LAMP";
    }
    return "HUMAN";
}

bool parseType(const std::string& value, ObjectType& result)
{
    for (ObjectType type : {ObjectType::Human, ObjectType::Tree, ObjectType::House,
                            ObjectType::Table, ObjectType::Rock, ObjectType::Lamp}) {
        if (value == typeId(type)) {
            result = type;
            return true;
        }
    }
    return false;
}

Color defaultColor(ObjectType type)
{
    switch (type) {
    case ObjectType::Human: return {100, 170, 230, 255};
    case ObjectType::Tree: return {70, 160, 80, 255};
    case ObjectType::House: return {210, 130, 85, 255};
    case ObjectType::Table: return {155, 105, 60, 255};
    case ObjectType::Rock: return {125, 130, 140, 255};
    case ObjectType::Lamp: return {240, 205, 80, 255};
    }
    return WHITE;
}

std::string safeFileStem(const std::string& value)
{
    std::string result;
    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_') result.push_back(static_cast<char>(c));
        else if (c == ' ' && !result.empty() && result.back() != '_') result.push_back('_');
    }
    while (!result.empty() && result.back() == '_') result.pop_back();
    return result.empty() ? "MyGame" : result;
}

std::string escapeCppString(const std::string& value)
{
    std::string result;
    for (char c : value) {
        if (c == '\\' || c == '"') result.push_back('\\');
        if (c == '\n') {
            result += "\\n";
        } else {
            result.push_back(c);
        }
    }
    return result;
}

void drawCube(Vector3 center, Vector3 size, Color color)
{
    DrawCubeV(center, size, color);
}

void drawObject(const SceneObject& object)
{
    const Vector3 p = object.position;
    const float s = object.scale;
    const Color c = object.color;
    const Color dark = {
        static_cast<unsigned char>(c.r * 0.72f),
        static_cast<unsigned char>(c.g * 0.72f),
        static_cast<unsigned char>(c.b * 0.72f),
        c.a
    };

    switch (object.type) {
    case ObjectType::Human:
        drawCube({p.x, p.y + 0.95f * s, p.z}, {0.52f * s, 0.72f * s, 0.30f * s}, c);
        DrawSphere({p.x, p.y + 1.52f * s, p.z}, 0.25f * s, {225, 185, 145, 255});
        drawCube({p.x - 0.38f * s, p.y + 0.91f * s, p.z}, {0.22f * s, 0.65f * s, 0.22f * s}, dark);
        drawCube({p.x + 0.38f * s, p.y + 0.91f * s, p.z}, {0.22f * s, 0.65f * s, 0.22f * s}, dark);
        drawCube({p.x - 0.16f * s, p.y + 0.30f * s, p.z}, {0.22f * s, 0.58f * s, 0.24f * s}, {45, 55, 85, 255});
        drawCube({p.x + 0.16f * s, p.y + 0.30f * s, p.z}, {0.22f * s, 0.58f * s, 0.24f * s}, {45, 55, 85, 255});
        break;
    case ObjectType::Tree:
        DrawCylinder({p.x, p.y + 0.75f * s, p.z}, 0.18f * s, 0.24f * s, 1.5f * s, 8, {110, 75, 45, 255});
        DrawSphere({p.x, p.y + 1.8f * s, p.z}, 0.78f * s, c);
        DrawSphere({p.x - 0.42f * s, p.y + 1.48f * s, p.z + 0.08f * s}, 0.53f * s, dark);
        DrawSphere({p.x + 0.42f * s, p.y + 1.48f * s, p.z - 0.08f * s}, 0.53f * s, c);
        break;
    case ObjectType::House:
        drawCube({p.x, p.y + 0.8f * s, p.z}, {2.0f * s, 1.6f * s, 1.7f * s}, c);
        drawCube({p.x, p.y + 1.72f * s, p.z}, {2.3f * s, 0.25f * s, 2.0f * s}, {130, 65, 55, 255});
        drawCube({p.x, p.y + 0.48f * s, p.z + 0.87f * s}, {0.42f * s, 0.95f * s, 0.08f * s}, {100, 70, 45, 255});
        drawCube({p.x - 0.58f * s, p.y + 1.12f * s, p.z + 0.87f * s}, {0.38f * s, 0.38f * s, 0.08f * s}, SKYBLUE);
        drawCube({p.x + 0.58f * s, p.y + 1.12f * s, p.z + 0.87f * s}, {0.38f * s, 0.38f * s, 0.08f * s}, SKYBLUE);
        break;
    case ObjectType::Table:
        drawCube({p.x, p.y + 0.95f * s, p.z}, {1.8f * s, 0.18f * s, 1.2f * s}, c);
        for (float x : {-0.72f, 0.72f})
            for (float z : {-0.43f, 0.43f})
                drawCube({p.x + x * s, p.y + 0.45f * s, p.z + z * s},
                         {0.14f * s, 0.9f * s, 0.14f * s}, dark);
        break;
    case ObjectType::Rock:
        DrawSphere({p.x, p.y + 0.42f * s, p.z}, 0.65f * s, c);
        DrawSphere({p.x - 0.35f * s, p.y + 0.32f * s, p.z + 0.18f * s}, 0.38f * s, dark);
        break;
    case ObjectType::Lamp:
        DrawCylinder({p.x, p.y + 1.0f * s, p.z}, 0.08f * s, 0.08f * s, 2.0f * s, 8, dark);
        DrawSphere({p.x, p.y + 2.05f * s, p.z}, 0.32f * s, c);
        break;
    }
}

std::string generatedGameCode(const Project& project)
{
    std::ostringstream out;
    out << "#include \"raylib.h\"\n"
           "#include <vector>\n\n"
           "enum class Kind { Human, Tree, House, Table, Rock, Lamp };\n"
           "struct PlacedObject { Kind kind; Vector3 position; float scale; Color color; };\n\n"
           "static void cube(Vector3 p, Vector3 size, Color color) { DrawCubeV(p, size, color); }\n"
           "static void drawObject(const PlacedObject& o) {\n"
           "    const Vector3 p = o.position;\n"
           "    const float s = o.scale;\n"
           "    const Color c = o.color;\n"
           "    switch (o.kind) {\n"
           "    case Kind::Human:\n"
           "        cube({p.x,p.y+.95f*s,p.z},{.52f*s,.72f*s,.30f*s},c);\n"
           "        DrawSphere({p.x,p.y+1.52f*s,p.z},.25f*s,{225,185,145,255});\n"
           "        cube({p.x-.38f*s,p.y+.91f*s,p.z},{.22f*s,.65f*s,.22f*s},c);\n"
           "        cube({p.x+.38f*s,p.y+.91f*s,p.z},{.22f*s,.65f*s,.22f*s},c);\n"
           "        cube({p.x-.16f*s,p.y+.30f*s,p.z},{.22f*s,.58f*s,.24f*s},{45,55,85,255});\n"
           "        cube({p.x+.16f*s,p.y+.30f*s,p.z},{.22f*s,.58f*s,.24f*s},{45,55,85,255}); break;\n"
           "    case Kind::Tree:\n"
           "        DrawCylinder({p.x,p.y+.75f*s,p.z},.18f*s,.24f*s,1.5f*s,8,{110,75,45,255});\n"
           "        DrawSphere({p.x,p.y+1.8f*s,p.z},.78f*s,c);\n"
           "        DrawSphere({p.x-.42f*s,p.y+1.48f*s,p.z},.53f*s,c);\n"
           "        DrawSphere({p.x+.42f*s,p.y+1.48f*s,p.z},.53f*s,c); break;\n"
           "    case Kind::House:\n"
           "        cube({p.x,p.y+.8f*s,p.z},{2*s,1.6f*s,1.7f*s},c);\n"
           "        cube({p.x,p.y+1.72f*s,p.z},{2.3f*s,.25f*s,2*s},{130,65,55,255});\n"
           "        cube({p.x,p.y+.48f*s,p.z+.87f*s},{.42f*s,.95f*s,.08f*s},{110,75,45,255}); break;\n"
           "    case Kind::Table:\n"
           "        cube({p.x,p.y+.95f*s,p.z},{1.8f*s,.18f*s,1.2f*s},c);\n"
           "        for (float x : {-.72f,.72f}) for (float z : {-.43f,.43f})\n"
           "            cube({p.x+x*s,p.y+.45f*s,p.z+z*s},{.14f*s,.9f*s,.14f*s},{110,75,45,255}); break;\n"
           "    case Kind::Rock: DrawSphere({p.x,p.y+.42f*s,p.z},.65f*s,c); break;\n"
           "    case Kind::Lamp:\n"
           "        DrawCylinder({p.x,p.y+1*s,p.z},.08f*s,.08f*s,2*s,8,{70,70,75,255});\n"
           "        DrawSphere({p.x,p.y+2.05f*s,p.z},.32f*s,c); break;\n"
           "    }\n"
           "}\n\n"
           "int main() {\n"
           "    InitWindow(1280, 800, \"" << escapeCppString(project.name) << "\");\n"
           "    SetTargetFPS(60);\n"
           "    Camera3D camera = {{12, 12, 16}, {0, 0, 0}, {0, 1, 0}, 45, CAMERA_PERSPECTIVE};\n"
           "    std::vector<PlacedObject> objects = {\n";

    out << std::fixed << std::setprecision(2);
    for (const SceneObject& object : project.objects) {
        const char* kind = typeName(object.type);
        std::string enumName = kind;
        if (enumName == "Human") enumName = "Human";
        else if (enumName == "Tree") enumName = "Tree";
        else if (enumName == "House") enumName = "House";
        else if (enumName == "Table") enumName = "Table";
        else if (enumName == "Rock") enumName = "Rock";
        else enumName = "Lamp";
        out << "        {Kind::" << enumName << ", {"
            << object.position.x << "f, " << object.position.y << "f, " << object.position.z
            << "f}, " << object.scale << "f, {" << static_cast<int>(object.color.r) << ", "
            << static_cast<int>(object.color.g) << ", " << static_cast<int>(object.color.b)
            << ", 255}},\n";
    }

    out << "    };\n"
           "    while (!WindowShouldClose()) {\n"
           "        BeginDrawing(); ClearBackground(SKYBLUE);\n"
           "        BeginMode3D(camera);\n"
           "        DrawPlane({0,0,0}, {30,30}, {90,145,95,255}); DrawGrid(30, 1.0f);\n"
           "        for (const auto& object : objects) drawObject(object);\n"
           "        EndMode3D();\n"
           "        DrawText(\"Generated with GameBuilder\", 20, 20, 20, DARKGRAY);\n"
           "        EndDrawing();\n"
           "    }\n"
           "    CloseWindow();\n"
           "    return 0;\n"
           "}\n";
    return out.str();
}

bool saveProject(Project& project, std::string& message)
{
    const std::string stem = safeFileStem(project.name);
    const std::filesystem::path path = stem + ".gbproj";
    std::ofstream file(path);
    if (!file) {
        message = "Could not write " + path.string();
        return false;
    }

    file << "GAMEBUILDER_PROJECT 1\n";
    file << "NAME " << std::quoted(project.name) << '\n';
    file << project.objects.size() << '\n';
    for (const SceneObject& object : project.objects) {
        file << typeId(object.type) << ' ' << std::quoted(object.name) << ' '
             << object.position.x << ' ' << object.position.y << ' ' << object.position.z << ' '
             << object.scale << ' ' << static_cast<int>(object.color.r) << ' '
             << static_cast<int>(object.color.g) << ' ' << static_cast<int>(object.color.b) << '\n';
    }
    if (!file) {
        message = "An error occurred while writing the project.";
        return false;
    }
    project.filePath = path;
    message = "Saved " + path.string();
    return true;
}

bool loadProject(Project& project, std::string& message)
{
    const std::filesystem::path path = safeFileStem(project.name) + ".gbproj";
    std::ifstream file(path);
    if (!file) {
        message = "Could not open " + path.string();
        return false;
    }

    std::string magic;
    int version = 0;
    std::string name;
    std::size_t count = 0;
    if (!(file >> magic >> version) || magic != "GAMEBUILDER_PROJECT" || version != 1 ||
        !(file >> magic >> std::quoted(name)) || magic != "NAME" ||
        !(file >> count) || count > 10000) {
        message = "Invalid or unsupported project file.";
        return false;
    }

    std::vector<SceneObject> objects;
    objects.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        std::string encodedType;
        SceneObject object;
        int r = 0, g = 0, b = 0;
        if (!(file >> encodedType >> std::quoted(object.name)
              >> object.position.x >> object.position.y >> object.position.z
              >> object.scale >> r >> g >> b) ||
            !parseType(encodedType, object.type) ||
            !std::isfinite(object.position.x) || !std::isfinite(object.position.y) ||
            !std::isfinite(object.position.z) || !std::isfinite(object.scale) ||
            object.scale <= 0.0f || r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            message = "Project contains an invalid object.";
            return false;
        }
        object.color = {static_cast<unsigned char>(r), static_cast<unsigned char>(g),
                        static_cast<unsigned char>(b), 255};
        objects.push_back(object);
    }

    project.name = name;
    project.objects = std::move(objects);
    project.selected = project.objects.empty() ? -1 : 0;
    project.filePath = path;
    message = "Loaded " + path.string();
    return true;
}

bool exportCode(const Project& project, std::string& code, std::string& message)
{
    code = generatedGameCode(project);
    const std::filesystem::path path = safeFileStem(project.name) + "_game.cpp";
    std::ofstream file(path);
    if (!file) {
        message = "Could not write " + path.string();
        return false;
    }
    file << code;
    if (!file) {
        message = "An error occurred while writing generated code.";
        return false;
    }
    std::cout << code << std::flush;
    message = "Code printed to console and saved as " + path.string();
    return true;
}

bool button(Rectangle bounds, const char* label, Vector2 mouse, bool pressed)
{
    const bool hovered = CheckCollisionPointRec(mouse, bounds);
    DrawRectangleRec(bounds, hovered ? Color{65, 83, 105, 255} : Color{43, 55, 72, 255});
    DrawRectangleLinesEx(bounds, 1.0f, Color{95, 116, 139, 255});
    const int width = MeasureText(label, 16);
    DrawText(label, static_cast<int>(bounds.x + (bounds.width - width) / 2),
             static_cast<int>(bounds.y + (bounds.height - 16) / 2), 16, RAYWHITE);
    return hovered && pressed;
}

void addObject(Project& project, ObjectType type)
{
    SceneObject object;
    object.type = type;
    object.name = std::string(typeName(type)) + " " + std::to_string(project.objects.size() + 1);
    object.color = defaultColor(type);
    const int index = static_cast<int>(project.objects.size());
    object.position = {static_cast<float>((index % 5) - 2) * 3.0f, 0.0f,
                       static_cast<float>(index / 5) * 3.0f - 1.5f};
    project.objects.push_back(object);
    project.selected = static_cast<int>(project.objects.size()) - 1;
}

} // namespace

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(ScreenWidth, ScreenHeight, "GameBuilder - 3D Project Editor");
    SetTargetFPS(60);

    Project project;
    std::string status = "Add a group object to start building.";
    std::string generatedCode;
    bool editingName = false;
    bool showCode = false;
    float codeScroll = 0.0f;
    Camera3D camera = {{12.0f, 12.0f, 16.0f}, {0.0f, 0.0f, 0.0f},
                       {0.0f, 1.0f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE};
    const Color panel = {25, 33, 44, 255};

    while (!WindowShouldClose()) {
        const int screenWidth = GetScreenWidth();
        const int screenHeight = GetScreenHeight();
        const int panelX = std::max(0, screenWidth - PanelWidth);
        const Vector2 mouse = GetMousePosition();
        const bool pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (!showCode && mouse.x < panelX) UpdateCamera(&camera, CAMERA_ORBITAL);

        if (editingName && !showCode) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 126 && project.name.size() < 40)
                    project.name.push_back(static_cast<char>(key));
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !project.name.empty()) project.name.pop_back();
            if (IsKeyPressed(KEY_ENTER)) editingName = false;
        }

        if (IsKeyPressed(KEY_DELETE) && !showCode && project.selected >= 0 &&
            project.selected < static_cast<int>(project.objects.size())) {
            project.objects.erase(project.objects.begin() + project.selected);
            project.selected = project.objects.empty()
                ? -1
                : std::min(project.selected, static_cast<int>(project.objects.size()) - 1);
        }

        if (showCode && IsKeyPressed(KEY_ESCAPE)) showCode = false;

        BeginDrawing();
        ClearBackground({18, 24, 32, 255});

        BeginScissorMode(0, 0, panelX, screenHeight);
        ClearBackground({30, 42, 52, 255});
        BeginMode3D(camera);
        DrawPlane({0.0f, -0.03f, 0.0f}, {40.0f, 40.0f}, {77, 119, 83, 255});
        DrawGrid(40, 1.0f);
        for (const SceneObject& object : project.objects) drawObject(object);
        EndMode3D();
        DrawRectangle(16, 16, 270, 56, Color{22, 31, 42, 220});
        DrawText("3D SCENE", 30, 24, 15, SKYBLUE);
        DrawText("Drag to orbit  |  Wheel to zoom", 30, 45, 13, RAYWHITE);
        EndScissorMode();

        DrawRectangle(panelX, 0, screenWidth - panelX, screenHeight, panel);
        DrawLine(panelX, 0, panelX, screenHeight, {82, 100, 121, 255});

        const int x = panelX + 18;
        const int usable = screenWidth - x - 18;
        DrawText("GAMEBUILDER", x, 16, 24, RAYWHITE);
        DrawText("Create a world, then export playable code.", x, 45, 13, LIGHTGRAY);

        DrawText("PROJECT NAME", x, 76, 12, SKYBLUE);
        const Rectangle nameBox{static_cast<float>(x), 94.0f,
                                static_cast<float>(usable), 34.0f};
        DrawRectangleRec(nameBox, editingName ? Color{48, 65, 83, 255} : Color{35, 45, 58, 255});
        DrawRectangleLinesEx(nameBox, 1.0f, editingName ? SKYBLUE : GRAY);
        DrawText(project.name.c_str(), x + 9, 103, 16, RAYWHITE);
        if (pressed && CheckCollisionPointRec(mouse, nameBox)) editingName = true;
        if (editingName) DrawText("Press Enter when done", x, 130, 12, LIGHTGRAY);

        const int actionY = 136;
        const float actionW = (usable - 8.0f) / 3.0f;
        if (button({static_cast<float>(x), static_cast<float>(actionY), actionW, 32},
                   "New", mouse, pressed)) {
            project.objects.clear();
            project.selected = -1;
            project.filePath.clear();
            status = "New project ready.";
        }
        if (button({x + actionW + 4, static_cast<float>(actionY), actionW, 32},
                   "Save", mouse, pressed)) saveProject(project, status);
        if (button({x + 2 * (actionW + 4), static_cast<float>(actionY), actionW, 32},
                   "Load", mouse, pressed)) loadProject(project, status);

        DrawText("ADD GROUP OBJECT", x, 180, 13, SKYBLUE);
        const float addW = (usable - 8.0f) / 3.0f;
        const char* labels[] = {"Human", "Tree", "House", "Table", "Rock", "Lamp"};
        const ObjectType types[] = {ObjectType::Human, ObjectType::Tree, ObjectType::House,
                                    ObjectType::Table, ObjectType::Rock, ObjectType::Lamp};
        for (int i = 0; i < 6; ++i) {
            const int row = i / 3;
            const int col = i % 3;
            const Rectangle bounds{x + col * (addW + 4), 200.0f + row * 37.0f, addW, 32};
            if (button(bounds, labels[i], mouse, pressed)) addObject(project, types[i]);
        }

        DrawText(TextFormat("OBJECTS (%i)", static_cast<int>(project.objects.size())),
                 x, 284, 13, SKYBLUE);
        const Rectangle listBounds{static_cast<float>(x), 303.0f,
                                   static_cast<float>(usable), 146.0f};
        DrawRectangleRec(listBounds, {31, 41, 54, 255});
        const int visibleCount = 5;
        int first = 0;
        if (project.selected >= visibleCount) first = project.selected - visibleCount + 1;
        for (int row = 0; row < visibleCount; ++row) {
            const int index = first + row;
            if (index >= static_cast<int>(project.objects.size())) break;
            const Rectangle rowBounds{static_cast<float>(x + 4), 307.0f + row * 27.0f,
                                      static_cast<float>(usable - 8), 24.0f};
            const bool selected = index == project.selected;
            if (selected) DrawRectangleRec(rowBounds, {59, 86, 112, 255});
            const SceneObject& object = project.objects[index];
            DrawCircle(x + 14, 319 + row * 27, 5, object.color);
            DrawText(TextFormat("%s  -  %s", object.name.c_str(), typeName(object.type)),
                     x + 27, 311 + row * 27, 14, RAYWHITE);
            if (pressed && CheckCollisionPointRec(mouse, rowBounds)) project.selected = index;
        }
        DrawText(TextFormat("%i object(s)", static_cast<int>(project.objects.size())),
                 x, 453, 12, LIGHTGRAY);

        DrawText("SELECTED OBJECT", x, 473, 13, SKYBLUE);
        if (project.selected >= 0 && project.selected < static_cast<int>(project.objects.size())) {
            SceneObject& object = project.objects[project.selected];
            DrawText(TextFormat("%s  |  Scale %.1f", object.name.c_str(), object.scale),
                     x, 493, 14, RAYWHITE);
            DrawText(TextFormat("Position  X %.1f   Y %.1f   Z %.1f",
                                object.position.x, object.position.y, object.position.z),
                     x, 512, 13, LIGHTGRAY);

            const float moveW = (usable - 8.0f) / 3.0f;
            if (button({static_cast<float>(x), 535, moveW, 30}, "X -", mouse, pressed))
                object.position.x -= 0.5f;
            if (button({x + moveW + 4, 535, moveW, 30}, "X +", mouse, pressed))
                object.position.x += 0.5f;
            if (button({x + 2 * (moveW + 4), 535, moveW, 30}, "Z -", mouse, pressed))
                object.position.z -= 0.5f;
            if (button({static_cast<float>(x), 570, moveW, 30}, "Z +", mouse, pressed))
                object.position.z += 0.5f;
            if (button({x + moveW + 4, 570, moveW, 30}, "Scale -", mouse, pressed))
                object.scale = std::max(0.25f, object.scale - 0.25f);
            if (button({x + 2 * (moveW + 4), 570, moveW, 30}, "Scale +", mouse, pressed))
                object.scale = std::min(5.0f, object.scale + 0.25f);

            if (button({static_cast<float>(x), 605, static_cast<float>(usable), 28},
                       "Change color", mouse, pressed)) {
                const Color colors[] = {{100, 170, 230, 255}, {70, 160, 80, 255},
                                        {220, 115, 75, 255}, {235, 195, 70, 255},
                                        {150, 105, 180, 255}};
                auto current = std::find_if(std::begin(colors), std::end(colors),
                    [&object](Color color) {
                        return color.r == object.color.r && color.g == object.color.g &&
                               color.b == object.color.b;
                    });
                object.color = current == std::end(colors) || current + 1 == std::end(colors)
                    ? colors[0] : *(current + 1);
            }
            if (button({static_cast<float>(x), 638, static_cast<float>(usable), 28},
                       "Delete selected object", mouse, pressed)) {
                project.objects.erase(project.objects.begin() + project.selected);
                project.selected = project.objects.empty()
                    ? -1
                    : std::min(project.selected, static_cast<int>(project.objects.size()) - 1);
            }
        } else {
            DrawText("Choose an object or add a group.", x, 496, 13, LIGHTGRAY);
        }

        const int codeY = std::max(680, screenHeight - 82);
        if (button({static_cast<float>(x), static_cast<float>(codeY),
                    static_cast<float>(usable), 38}, "PRINT / GENERATE GAME CODE", mouse, pressed)) {
            if (exportCode(project, generatedCode, status)) {
                codeScroll = 0.0f;
                showCode = true;
            }
        }
        DrawText(status.c_str(), x, codeY - 21, 11, LIGHTGRAY);
        DrawText("Generated game requires Raylib.", x, screenHeight - 20, 11, GRAY);

        if (showCode) {
            DrawRectangle(0, 0, screenWidth, screenHeight, {0, 0, 0, 190});
            const int modalX = 30;
            const int modalY = 30;
            const int modalW = screenWidth - 60;
            const int modalH = screenHeight - 60;
            DrawRectangle(modalX, modalY, modalW, modalH, {24, 31, 41, 255});
            DrawRectangleLines(modalX, modalY, modalW, modalH, SKYBLUE);
            DrawText("GENERATED GAME CODE", modalX + 18, modalY + 14, 20, RAYWHITE);
            DrawText("Saved to the project folder. Scroll to inspect; press Esc to close.",
                     modalX + 18, modalY + 42, 13, LIGHTGRAY);
            const Rectangle codeArea{static_cast<float>(modalX + 18),
                                     static_cast<float>(modalY + 70),
                                     static_cast<float>(modalW - 36),
                                     static_cast<float>(modalH - 88)};
            if (CheckCollisionPointRec(mouse, codeArea)) codeScroll -= GetMouseWheelMove() * 3.0f;

            std::vector<std::string> lines;
            std::istringstream codeInput(generatedCode);
            std::string line;
            while (std::getline(codeInput, line)) lines.push_back(line);
            const int lineHeight = 16;
            const int visibleLines = static_cast<int>(codeArea.height) / lineHeight;
            const int maxScroll = std::max(0, static_cast<int>(lines.size()) - visibleLines);
            codeScroll = std::clamp(codeScroll, 0.0f, static_cast<float>(maxScroll));

            BeginScissorMode(static_cast<int>(codeArea.x), static_cast<int>(codeArea.y),
                             static_cast<int>(codeArea.width), static_cast<int>(codeArea.height));
            for (int i = 0; i < visibleLines; ++i) {
                const int lineIndex = static_cast<int>(codeScroll) + i;
                if (lineIndex >= static_cast<int>(lines.size())) break;
                DrawText(lines[lineIndex].c_str(), static_cast<int>(codeArea.x),
                         static_cast<int>(codeArea.y) + i * lineHeight, 12,
                         {190, 215, 235, 255});
            }
            EndScissorMode();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

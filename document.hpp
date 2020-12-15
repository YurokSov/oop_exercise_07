#pragma once

#include <vector>
#include <stack>
#include <fstream>
#include <variant>

#include "factory.hpp"
#include "figure.hpp"

class Document {
public:
    Document();
    ~Document() = default;

    bool Create(std::string name = "unnamed");
    void Save(std::string name);
    void Load(std::string name);

    void Add(std::unique_ptr<Figure>&& figure);
    void Remove(uint32_t index);
    void Print();

    void Undo();
    void Redo();

private:
    std::string name;
    std::vector<std::unique_ptr<Figure>> container;
    std::fstream file;

    Factory<Figure>& factory = Factory<Figure>::GetInstance();

    using vertices = std::pair<Point2D, Point2D>;
    using figure_cmd = std::pair<std::string, vertices>;
    using cmd_and_index = std::pair<figure_cmd, uint32_t>;
    using command = std::pair<uint32_t, cmd_and_index>;

    std::stack<command> undo_history;
    std::stack<command> redo_history;

    command ComposeCommand(uint32_t option, uint32_t index, Figure* figure);
};
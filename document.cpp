#include "document.hpp"

#include <sstream>

Document::Document() {
    if (!factory.IsRegistered("triangle"))
        factory.RegisterClass("triangle", FigureCreator<Triangle>::Create);
    if (!factory.IsRegistered("square"))
        factory.RegisterClass("square", FigureCreator<Square>::Create);
    if (!factory.IsRegistered("rectangle"))
        factory.RegisterClass("rectangle", FigureCreator<Rectangle>::Create);
}

bool Document::Create(std::string name) {
    std::cout << "This unsaved document will be lost, are you sure about that? (yes/no)" << std::endl;
    std::string ans;
    std::cin >> ans;
    if (ans != "yes") {
        return false;
    }
    this->name = name;
    container.clear();
    std::cout << "Document created" << std::endl;
    return true;
}

void Document::Save(std::string name) {
    file.open(name, std::fstream::out);
    if (file.is_open()) {
        for (auto& elem : container) {
            file << elem->GetName() << ' ';
            auto center = elem->GetCentroid();
            file << center.x << ' ' << center.y << ' ';
            auto vertices = elem->GetVertices();
            file << vertices[0].x << ' ' << vertices[0].y << ' ';
            file << '\n';
        }
        file.close();
    }
    else {
        throw std::runtime_error("File couldn't be opened!");
    }
}

void Document::Load(std::string name) {
    file.open(name, std::fstream::in);
    if (file.is_open()) {
        while (!file.eof()) {
            std::string figureType;
            file >> figureType;
            if (figureType.length() == 0)
                break;
            container.emplace_back(factory.Create(figureType));
            file >> *(container.back());
        }
        file.close();
    }
    else {
        throw std::runtime_error("File couldn't be opened!");
    }
}

void Document::Add(std::unique_ptr<Figure>&& figure) {
    try {
        auto figureName = figure->GetName();
        container.emplace_back(std::move(figure));
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
    undo_history.emplace(ComposeCommand(1, container.size() - 1, container.back().get()));
}

void Document::Remove(uint32_t index) {
    if (index >= container.size()) {
        throw std::out_of_range("Bad index provided!");
    }
    auto figure = container.at(index).get();
    undo_history.emplace(ComposeCommand(2, index, figure));
    container.erase(container.begin() + index);
}

void Document::Print() {
    //double totalArea = 0;
    std::cout << "Print:\n";
    for (size_t i = 0; i < container.size(); ++i) {
        std::cout << '\t' << *(container[i]) << std::endl;
        //std::cout << "\tGetCentroid() = " << std::fixed << container[i]->GetCentroid() << std::endl;
        //double area = container[i]->GetArea();
        //totalArea += area;
        //std::cout << "\tGetArea() = " << area << std::endl;
    }

    // std::cout << "Total Area: " << totalArea << std::endl;
}

void Document::Undo() {
    if (undo_history.size() == 0) {
        throw std::runtime_error("Nothing to undo!");
    }
    command cmd = undo_history.top();
    undo_history.pop();
    if (cmd.first == 2) {
        auto figure = factory.Create(cmd.second.first.first);
        std::stringstream ss;
        ss
            << cmd.second.first.second.first.x << ' ' << cmd.second.first.second.first.y << ' '
            << cmd.second.first.second.second.x << ' ' << cmd.second.first.second.second.y;
        ss >> *(figure.get());
        container.insert(container.begin() + cmd.second.second, std::move(figure));
    }
    else if (cmd.first == 1) {
        container.erase(container.begin() + cmd.second.second);
    }
    else {
        throw std::runtime_error("Corrupted history!");
    }
    redo_history.push(cmd);
}

void Document::Redo() {
    if (redo_history.size() == 0) {
        throw std::runtime_error("Nothing to redo!");
    }
    command cmd = redo_history.top();
    redo_history.pop();
    if (cmd.first == 1) {
        auto figure = factory.Create(cmd.second.first.first);
        std::stringstream ss;
        ss
            << cmd.second.first.second.first.x << ' ' << cmd.second.first.second.first.y << ' '
            << cmd.second.first.second.second.x << ' ' << cmd.second.first.second.second.y;
        ss >> *(figure.get());
        container.insert(container.begin() + cmd.second.second, std::move(figure));
    }
    else if (cmd.first == 2) {
        container.erase(container.begin() + cmd.second.second);
    }
    else {
        throw std::runtime_error("Corrupted history!");
    }
    undo_history.push(cmd);
}

Document::command Document::ComposeCommand(uint32_t option, uint32_t index, Figure* figure) {
    return command(option, cmd_and_index(figure_cmd(figure->GetName(), vertices(figure->GetCentroid(), figure->GetVertices().at(0))), index));
}
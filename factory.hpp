#pragma once

#include <memory>
#include <functional>
#include <map>
#include <string>

#include "figure.hpp"


template <typename T>
class Factory {
public:

    bool IsRegistered(std::string typeRepr) {
        return (registered.find(typeRepr) != registered.end());
    }

    void RegisterClass(std::string typeRepr, std::function<T* ()> typeCreator) {
        if (registered.find(typeRepr) != registered.end()) {
            throw std::runtime_error("Already registered!");
        }
        registered.insert(std::make_pair(typeRepr, typeCreator));
    }

    std::unique_ptr<T> Create(std::string typeRepr) {
        for (auto& pair : registered) {
            if (pair.first == typeRepr) {
                auto typeCreator = registered.at(typeRepr);
                return std::unique_ptr<T>(typeCreator());
            }
        }
        throw std::runtime_error("No such creator!");
    }

    static Factory& GetInstance() {
        static Factory<T> factory;
        return factory;
    }

private:
    Factory() = default;

    std::map < std::string, std::function<T* ()>> registered;
};
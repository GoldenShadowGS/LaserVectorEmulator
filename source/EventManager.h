#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

class EventManager
{
public:
    // Map event name -> list of callbacks
    std::unordered_map<std::string, std::vector<std::function<void()>>> listeners;

    // Subscribe to an event
    void Subscribe(const std::string& name, std::function<void()> callback)
    {
        listeners[name].push_back(std::move(callback));
    }

    // Emit/Trigger an event
    void Emit(const std::string& name)
    {
        if (listeners.contains(name))
        {
            for (auto& callback : listeners[name])
            {
                callback();
            }
        }
    }
};

#pragma once

#include <functional>
#include <utility>

#include <RE/T/TESCellFullyLoadedEvent.h>

namespace ScriptsWithoutESP {

    class OnCellFullyLoadedEventSink : public RE::BSTEventSink<RE::TESCellFullyLoadedEvent> {
        std::function<void(const RE::TESCellFullyLoadedEvent* event)> _eventCallback;

    public:
        explicit OnCellFullyLoadedEventSink(std::function<void(const RE::TESCellFullyLoadedEvent* event)> eventCallback) : _eventCallback(std::move(eventCallback)) {}

        RE::BSEventNotifyControl ProcessEvent(const RE::TESCellFullyLoadedEvent* event, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*) override {
            _eventCallback(event);
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}
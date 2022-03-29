#pragma once

#include <functional>
#include <utility>

#include <RE/T/TESObjectLoadedEvent.h>

namespace ScriptsWithoutESP {

    class OnObjectLoadedEventSink : public RE::BSTEventSink<RE::TESObjectLoadedEvent> {
        std::function<void(const RE::TESObjectLoadedEvent* event)> _eventCallback;

    public:
        explicit OnObjectLoadedEventSink(std::function<void(const RE::TESObjectLoadedEvent* event)> eventCallback) : _eventCallback(std::move(eventCallback)) {}

        RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override {
            _eventCallback(event);
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}
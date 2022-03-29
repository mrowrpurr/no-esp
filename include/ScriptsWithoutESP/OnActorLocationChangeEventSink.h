#pragma once

#include <functional>
#include <utility>

#include <RE/T/TESActorLocationChangeEvent.h>

namespace ScriptsWithoutESP {

    class OnActorLocationChangeEventSink : public RE::BSTEventSink<RE::TESActorLocationChangeEvent> {
        std::function<void(const RE::TESActorLocationChangeEvent* event)> _eventCallback;

    public:
        explicit OnActorLocationChangeEventSink(std::function<void(const RE::TESActorLocationChangeEvent* event)> eventCallback) : _eventCallback(std::move(eventCallback)) {}

        RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*) override {
            _eventCallback(event);
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}
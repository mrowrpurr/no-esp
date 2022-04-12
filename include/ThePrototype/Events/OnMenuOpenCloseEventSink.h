#pragma once

#include <functional>
#include <utility>

#include <RE/M/MenuOpenCloseEvent.h>

namespace ThePrototype {

    class OnMenuOpenCloseEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
        std::function<void(const RE::MenuOpenCloseEvent* event)> _eventCallback;

    public:
        explicit OnMenuOpenCloseEventSink(std::function<void(const RE::MenuOpenCloseEvent* event)> eventCallback) : _eventCallback(std::move(eventCallback)) {}

        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
            _eventCallback(event);
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}
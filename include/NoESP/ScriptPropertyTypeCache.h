#pragma once

using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace NoESP {

    class ScriptPropertyTypeCache {
        // Cache types of properties which we need to manually set on scripts
        std::unordered_map<std::string, std::unordered_map<std::string, RE::BSScript::TypeInfo::RawType>> _scriptPropertyTypes;

        ScriptPropertyTypeCache() = default;

    public:
        ScriptPropertyTypeCache(const ScriptPropertyTypeCache&) = delete;
        ScriptPropertyTypeCache& operator=(const ScriptPropertyTypeCache&) = delete;

        static ScriptPropertyTypeCache& GetSingleton() {
            static ScriptPropertyTypeCache cache;
            return cache;
        }

        std::optional<RE::BSScript::TypeInfo::RawType> GetOrLookupScriptPropertyType(const std::string& scriptName, const std::string& propertyName) {
            auto lowerScriptName = Utilities::ToLowerCase(scriptName);

            if (! _scriptPropertyTypes.contains(lowerScriptName)) {
                std::unordered_map<std::string, RE::BSScript::TypeInfo::RawType> propertyTypes;
                try {
                    auto* vm = VirtualMachine::GetSingleton();
                    RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> objectTypeInfoPtr;
                    vm->GetScriptObjectType(scriptName, objectTypeInfoPtr);
                    auto* properties = objectTypeInfoPtr->GetPropertyIter();
                    if (properties) {
                        for (uint32_t i = 0; i < objectTypeInfoPtr->propertyCount; i++) {
                            auto thisPropertyName = properties[i].name;
                            try {
                                auto thisPropertyType = properties[i].info.type.GetRawType();
                                propertyTypes.insert_or_assign(Utilities::ToLowerCase(thisPropertyName.c_str()), thisPropertyType);
                            } catch (...) {
                                Log("properties[i].info.type.GetRawType() failed for {}", scriptName);
                            }
                        }
                    }
                } catch (...) {
                    Log("Failed to lookup/cache script property types for {} {}", scriptName, propertyName);
                }
                _scriptPropertyTypes.insert_or_assign(lowerScriptName, propertyTypes);
            }

            if (_scriptPropertyTypes.contains(lowerScriptName)) {
                auto lowerPropertyName = Utilities::ToLowerCase(propertyName);
                if (_scriptPropertyTypes[lowerScriptName].contains(lowerPropertyName)) {
                    return _scriptPropertyTypes[lowerScriptName][lowerPropertyName];
                }
            }
        }
    };
}

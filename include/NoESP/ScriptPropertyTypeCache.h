#pragma once

using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace NoESP {

    struct ScriptPropertyType {
        std::optional<TypeInfo::RawType> RawType = TypeInfo::RawType::kNone;
        std::string PropertyScriptName;
    };

    class ScriptPropertyTypeCache {

        // Cache types of properties which we need to manually set on scripts
        std::unordered_map<std::string, std::unordered_map<std::string, ScriptPropertyType>> _scriptPropertyTypes;

        ScriptPropertyTypeCache() = default;

    public:
        ScriptPropertyTypeCache(const ScriptPropertyTypeCache&) = delete;
        ScriptPropertyTypeCache& operator=(const ScriptPropertyTypeCache&) = delete;

        static ScriptPropertyTypeCache& GetSingleton() {
            static ScriptPropertyTypeCache cache;
            return cache;
        }

        std::optional<ScriptPropertyType> GetOrLookupScriptPropertyType(const std::string& scriptName, const std::string& propertyName) {
            auto lowerScriptName = Utilities::ToLowerCase(scriptName);
            Log("Get or Lookup... script:{} property:{}", scriptName, propertyName);

            if (! _scriptPropertyTypes.contains(lowerScriptName)) {
                std::unordered_map<std::string, ScriptPropertyType> propertyTypes;
                Log("Caching all properties for script {}", scriptName);
                try {
                    auto* vm = VirtualMachine::GetSingleton();
                    RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> objectTypeInfoPtr;
                    vm->GetScriptObjectType(scriptName, objectTypeInfoPtr);
                    auto* properties = objectTypeInfoPtr->GetPropertyIter();
                    if (properties) {
                        for (uint32_t i = 0; i < objectTypeInfoPtr->propertyCount; i++) {
                            auto thisPropertyName = properties[i].name;
                            Log("This Property Name: {}", thisPropertyName.c_str());
                            try {
                                ScriptPropertyType type;
                                Log("Getting raw type...");
                                auto rawType = properties[i].info.type.GetRawType();
                                type.RawType = rawType;
                                Log("Got raw type.");
                                try {
                                    Log("Getting type info...");
                                    auto *typeInfo = properties[i].info.type.GetTypeInfo();
                                    Log("Got type info.");
                                    if (typeInfo) {
                                        auto rawTypeId = (size_t) rawType;
                                        if (rawType == TypeInfo::RawType::kArraysEnd || (rawTypeId >= 0 && rawTypeId <= 15)) {
                                            Log("Not getting name for {} - rawType is {}", thisPropertyName.c_str(), rawTypeId);
                                            // ...
                                        } else {
                                            Log("Getting name...");
                                            type.PropertyScriptName = typeInfo->GetName();
                                            Log("Got name: '{}'.", type.PropertyScriptName);
                                        }
                                    }
                                } catch (...) {
                                    Log("Could not lookup type info for property {}", propertyName);
                                }
                                propertyTypes.insert_or_assign(Utilities::ToLowerCase(thisPropertyName.c_str()), type);
                            } catch (...) {
                                Log("No PropertyScriptName could be looked up for {}", propertyName);
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

            std::optional<ScriptPropertyType> type;
            return type;
        }
    };
}

#pragma once

using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace ThePrototype {

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
                            try {
                                ScriptPropertyType type;
                                auto rawType = properties[i].info.type.GetRawType();
                                type.RawType = rawType;
                                try {
                                    auto *typeInfo = properties[i].info.type.GetTypeInfo();
                                    if (typeInfo) {
                                        auto rawTypeId = (size_t) rawType;
                                        if (rawType == TypeInfo::RawType::kArraysEnd || (rawTypeId >= 0 && rawTypeId <= 15)) {
                                            // ...
                                        } else {
                                            type.PropertyScriptName = typeInfo->GetName();
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

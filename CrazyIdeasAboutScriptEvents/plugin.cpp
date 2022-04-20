#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

// 1. Invoke global Papyrus function (non-native)
// 2. Invoke global Papyrus function (native from other plugin)
// 3. Invoke one on an event and watch it work and smile.

class CallbackFunctorImpl : public RE::BSScript::IStackCallbackFunctor {
public:
    CallbackFunctorImpl() {}
    void operator()(RE::BSScript::Variable a_result) { /* do something! */ }
    bool CanSave() { return false; }
    void SetObject(const RE::BSTSmartPointer<RE::BSScript::Object>&) { /* ??? */ }
};

void CrazyIdeasNativeFunctions_CallThePapyrusFunction(RE::StaticFunctionTag*) {
    auto* console = RE::ConsoleLog::GetSingleton();
    console->Print("Well, hi there. The function was at least called!");

    // TODO - call a global function!
    RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> typeInfo;
    auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    vm->GetScriptObjectType("CrazyIdeasExample", typeInfo);
    if (typeInfo) {
        console->Print("TYPE *FOUND!*");
        size_t globalFunctionCount = typeInfo.get()->GetNumGlobalFuncs();
        auto* globalFunctions = typeInfo.get()->GetGlobalFuncIter();
        for (uint32_t i = 0; i < globalFunctionCount; i++) {
            auto fn = globalFunctions[i].func;
            if (fn) {
                auto fnName = fn.get()->GetName();
                if (fnName == "CallThisFunction") {
                    console->Print(std::format("MATCH fn name {}", fnName.c_str()).c_str());
                    auto callback = new CallbackFunctorImpl();
                    auto callbackPtr = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>(callback);
                    auto args = RE::BSScript::ZeroFunctionArguments();
                    vm->DispatchStaticCall("CrazyIdeasExample", "CallThisFunction", &args, callbackPtr);
                    console->Print(std::format("CALLED fn name {}", fnName.c_str()).c_str());
                } else {
                    console->Print(std::format("not a matching fn name {}", fnName.c_str()).c_str());
                }
            }
        }
    } else {
        console->Print("TYPE NOT FOUND!");
    }
}

bool RegisterPapyrusFunctions(RE::BSScript::Internal::VirtualMachine* vm) {
    vm->RegisterFunction("CallThePapyrusFunction", "CrazyIdeasNativeFunctions", CrazyIdeasNativeFunctions_CallThePapyrusFunction);
    return true;
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::GetPapyrusInterface()->Register(RegisterPapyrusFunctions);
    return true;
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* info) {
    info->infoVersion = SKSE::PluginInfo::kVersion;
    info->name = "HelloSkse";
    info->version = 1;
    return true;
}

extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = [](){
    SKSE::PluginVersionData version;
    version.PluginName("HelloSkse");
    version.PluginVersion({ 0, 0, 1 });
    version.CompatibleVersions({ SKSE::RUNTIME_LATEST });
    return version;
}();

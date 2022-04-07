#include "../specHelper.h"

#include <NoESP/AutoBindingsParser.h>
#include <NoESP/BindingDefinition.h>

go_bandit([](){
    describe("Parsing AutoBindings lines", [](){
        it("ScriptName", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("MyScript");
            AssertThat(def.ScriptName, Equals("Foo"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
            AssertThat(def.FormID, Equals(20)); // The player
            AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::None));
        });
        xit("ScriptName SomeEditorId (exact)", [&](){ });
        xit("ScriptName *SomeEditorId (ends with)", [&](){ });
        xit("ScriptName *SomeEditorId* (contains)", [&](){ });
        xit("ScriptName SomeEditorId* (starts with)", [&](){ });
        xit("ScriptName /.*Sweet.*Roll.*/ (regular expression)", [&](){ });
        xit("ScriptName 0x123", [&](){ });
        xit("ScriptName 0x123 SomePlugin.esp", [&](){ });
        xit("!ScriptName (don't auto fill)", [&](){ });
        xit("ScriptName Prop=TextValue", [&](){ });
        xit("ScriptName Prop=\"Quoted text value\"", [&](){ });
        xit("ScriptName *Editor* A=1 B=\"hi there\" C=true", [&](){ });
    });
});

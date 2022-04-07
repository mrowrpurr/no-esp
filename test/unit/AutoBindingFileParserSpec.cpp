#include "../specHelper.h"

#include <NoESP/AutoBindingsParser.h>
#include <NoESP/BindingDefinition.h>

go_bandit([](){
    describe("Parsing AutoBindings lines", [](){
        it("ScriptName", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("MyScript");

            AssertThat(def.ScriptName, Equals("MyScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
            AssertThat(def.FormID, Equals(20)); // The player
            AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::None));
        });
        it("ScriptName SomeEditorId (exact)", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("MyScript SomeEditorId");

            AssertThat(def.ScriptName, Equals("MyScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
            AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::Exact));
            AssertThat(def.EditorIdMatcher.Text, Equals("someeditorid")); // Storage is in lowercase
        });
        it("ScriptName *SomeEditorId (ends with)", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("MyScript *SomeEditorId");

            AssertThat(def.ScriptName, Equals("MyScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
            AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::SuffixMatch));
            AssertThat(def.EditorIdMatcher.Text, Equals("someeditorid")); // Storage is in lowercase
        });
        it("ScriptName *SomeEditorId* (contains)", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("MyScript *SomeEditorId*");

            AssertThat(def.ScriptName, Equals("MyScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
            AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::PrefixAndSuffixMatch));
            AssertThat(def.EditorIdMatcher.Text, Equals("someeditorid")); // Storage is in lowercase
        });
        it("ScriptName SomeEditorId* (starts with)", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("MyScript SomeEditorId*");

            AssertThat(def.ScriptName, Equals("MyScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
            AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::PrefixMatch));
            AssertThat(def.EditorIdMatcher.Text, Equals("someeditorid")); // Storage is in lowercase
        });
        it("ScriptName /.*Sweet.*Roll.*/ (regular expression)", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("MyScript /.*Sweet.*Roll.*/");

            AssertThat(def.ScriptName, Equals("MyScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
            AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::RegularExpression));

            AssertThat(std::regex_match("cool sweet the roll haha", def.EditorIdMatcher.RegularExpression), IsTrue());
            AssertThat(std::regex_match("cool sWeeT the rOLl haha", def.EditorIdMatcher.RegularExpression), IsTrue());
            AssertThat(std::regex_match("cool not the roll haha", def.EditorIdMatcher.RegularExpression), IsFalse());
        });
        xit("ScriptName 0x123", [&](){ });
        xit("ScriptName 0x123 SomePlugin.esp", [&](){ });
        xit("!ScriptName (don't auto fill)", [&](){ });
        xit("ScriptName Prop=TextValue", [&](){ });
        xit("ScriptName Prop=\"Quoted text value\"", [&](){ });
        xit("ScriptName *Editor* A=1 B=\"hi there\" C=true", [&](){ });
    });
});

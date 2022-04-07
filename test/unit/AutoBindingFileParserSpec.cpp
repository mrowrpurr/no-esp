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
        it("ScriptName 0x123", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("PlayerScript 0x14");

            AssertThat(def.ScriptName, Equals("PlayerScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
            AssertThat(def.FormID, Equals(20)); // 0x14
            AssertThat(def.Plugin, IsEmpty());
        });
        it("ScriptName 0x123 SomePlugin.esp", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("PlayerScript 0x14 SomePlugin.esp");

            AssertThat(def.ScriptName, Equals("PlayerScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
            AssertThat(def.FormID, Equals(20)); // 0x14
            AssertThat(def.Plugin, Equals("SomePlugin.esp"));
        });
        xit("!ScriptName (don't auto fill)", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("!MyScript");

            AssertThat(def.ScriptName, Equals("MyScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
            AssertThat(def.FormID, Equals(20)); // The player
//            AssertThat() ...
        });
        it("ScriptName Prop=TextValue", [&](){
            auto def = NoESP::AutoBindingsFile::ParseLine("MyScript Prop=TextValue");

            AssertThat(def.ScriptName, Equals("MyScript"));
            AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
            AssertThat(def.FormID, Equals(20)); // The player
            AssertThat(def.PropertyValues.size(), Equals(1));
            AssertThat(def.PropertyValues.contains("prop"), IsTrue());
            AssertThat(def.PropertyValues["prop"].PropertyName, Equals("prop"));
            AssertThat(def.PropertyValues["prop"].PropertyValueText, Equals("TextValue"));
            AssertThat(def.PropertyValues["prop"].PropertyTypeHasBeenLoaded, IsFalse());
        });
        xit("ScriptName Prop=\"Quoted text value\"", [&](){ });
        xit("ScriptName *Editor* A=1 B=\"hi there\" C=true", [&](){ });
    });
});

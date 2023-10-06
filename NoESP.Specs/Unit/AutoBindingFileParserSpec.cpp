#include "SpecHelper.h"
//

#include <NoESP/AutoBindingsParser.h>
#include <NoESP/BindingDefinition.h>

Describe("Parsing AutoBindings lines") {
    it("ScriptName", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("MyScript");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // The player
        AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::None));
    });
    it("ScriptName SomeEditorId (exact)", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("MyScript SomeEditorId");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
        AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::Exact));
        AssertThat(def.EditorIdMatcher.Text, Equals("someeditorid"));  // Storage is in lowercase
    });
    it("ScriptName *SomeEditorId (ends with)", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("MyScript *SomeEditorId");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
        AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::SuffixMatch));
        AssertThat(def.EditorIdMatcher.Text, Equals("someeditorid"));  // Storage is in lowercase
    });
    it("ScriptName *SomeEditorId* (contains)", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("MyScript *SomeEditorId*");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
        AssertThat(def.EditorIdMatcher.Type,
                   Equals(NoESP::EditorIdMatcherType::PrefixAndSuffixMatch));
        AssertThat(def.EditorIdMatcher.Text, Equals("someeditorid"));  // Storage is in lowercase
    });
    it("ScriptName SomeEditorId* (starts with)", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("MyScript SomeEditorId*");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
        AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::PrefixMatch));
        AssertThat(def.EditorIdMatcher.Text, Equals("someeditorid"));  // Storage is in lowercase
    });
    it("ScriptName /.*Sweet.*Roll.*/ (regular expression)", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("MyScript /.*Sweet.*Roll.*/");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::EditorID));
        AssertThat(def.EditorIdMatcher.Type, Equals(NoESP::EditorIdMatcherType::RegularExpression));

        AssertThat(
            std::regex_match("cool sweet the roll haha", def.EditorIdMatcher.RegularExpression),
            IsTrue());
        AssertThat(
            std::regex_match("cool sWeeT the rOLl haha", def.EditorIdMatcher.RegularExpression),
            IsTrue());
        AssertThat(
            std::regex_match("cool not the roll haha", def.EditorIdMatcher.RegularExpression),
            IsFalse());
    });
    it("ScriptName 0x123", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("PlayerScript 0x14");

        AssertThat(def.ScriptName, Equals("PlayerScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // 0x14
        AssertThat(def.Plugin, IsEmpty());
    });
    it("ScriptName 0x123 SomePlugin.esp", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("PlayerScript 0x14 SomePlugin.esp");

        AssertThat(def.ScriptName, Equals("PlayerScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // 0x14
        AssertThat(def.Plugin, Equals("SomePlugin.esp"));
    });
    // xit("!ScriptName (don't auto fill)", [&]() {
    //     auto def = NoESP::AutoBindingsFile::ParseLine("!MyScript");

    //     AssertThat(def.ScriptName, Equals("MyScript"));
    //     AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
    //     AssertThat(def.FormID, Equals(20));  // The player
    //                                          //            AssertThat() ...
    // });
    it("ScriptName Prop=TextValue", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine("MyScript Prop=TextValue");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // The player
        AssertThat(def.PropertyValues.size(), Equals(1));
        AssertThat(def.PropertyValues.contains("prop"), IsTrue());
        AssertThat(def.PropertyValues["prop"].PropertyName, Equals("prop"));
        AssertThat(def.PropertyValues["prop"].PropertyValueText, Equals("TextValue"));
        AssertThat(def.PropertyValues["prop"].PropertyTypeHasBeenLoaded, IsFalse());
    });
    it("ScriptName Prop=\"Quoted text value\"", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine(R"(MyScript Prop="Quoted text value")");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // The player
        AssertThat(def.PropertyValues.size(), Equals(1));
        AssertThat(def.PropertyValues.contains("prop"), IsTrue());
        AssertThat(def.PropertyValues["prop"].PropertyName, Equals("prop"));
        AssertThat(def.PropertyValues["prop"].PropertyValueText, Equals("Quoted text value"));
        AssertThat(def.PropertyValues["prop"].PropertyTypeHasBeenLoaded, IsFalse());
    });
    it("ScriptName *Editor* A=1 B=\"hi there\" C=true z=\"cool with an = sign too\"", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine(
            R"(MyScript A=1 B="Hi There" C=true D=69 multiline="\nthis\t has special\n characters\n" z=" cool with an = sign too ")");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // The player
        AssertThat(def.PropertyValues.size(), Equals(6));
        AssertThat(def.PropertyValues["a"].PropertyValueText, Equals("1"));
        AssertThat(def.PropertyValues["b"].PropertyValueText, Equals("Hi There"));
        AssertThat(def.PropertyValues["c"].PropertyValueText, Equals("true"));
        AssertThat(def.PropertyValues["d"].PropertyValueText, Equals("69"));
        AssertThat(def.PropertyValues["z"].PropertyValueText, Equals(" cool with an = sign too "));
        AssertThat(def.PropertyValues["multiline"].PropertyValueText,
                   Equals("\nthis\t has special\n characters\n"));
    });
    // xit("ScriptName Ints=[1, 2, 3]", [&]() {});
    it("ScriptName [BOOK]", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine(R"(MyScript [BOOK])");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // The player
        AssertThat(def.FormTypes.size(), Equals(1));

        std::vector<RE::FormType> formTypes;
        formTypes.assign(def.FormTypes.begin(), def.FormTypes.end());

        AssertThat(formTypes.size(), Equals(1));
        AssertThat(formTypes[0], Equals(RE::FormType::Book));
    });
    it("ScriptName [weapon|armor]", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine(R"(MyScript [weapon|armor])");

        AssertThat(def.ScriptName, Equals("MyScript"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // The player
        AssertThat(def.FormTypes.size(), Equals(2));
        AssertThat(def.FormTypes, Contains(RE::FormType::Weapon));
        AssertThat(def.FormTypes, Contains(RE::FormType::Armor));
    });
    it("Kaboom [Book] TheExplosion=ExplosionShockMass01", [&]() {
        auto def = NoESP::AutoBindingsFile::ParseLine(
            R"(Kaboom [Book] TheExplosion=ExplosionShockMass01)");

        AssertThat(def.ScriptName, Equals("Kaboom"));
        AssertThat(def.Type, Equals(NoESP::BindingDefinitionType::FormID));
        AssertThat(def.FormID, Equals(20));  // The player
        AssertThat(def.FormTypes.size(), Equals(2));
        AssertThat(def.FormTypes, Contains(RE::FormType::Weapon));
        AssertThat(def.FormTypes, Contains(RE::FormType::Armor));
    });
}
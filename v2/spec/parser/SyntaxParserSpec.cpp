#include "../SpecHelper.h"

#include <NoESP/Parser.h>

go_bandit([](){
    describe("NoESP Syntax Parser", [](){
        it("ScriptName", [&](){
            auto expressions = NoESP::Parser::Parse("ScriptName");
            AssertThat(expressions.size(), Equals(1));

            auto expression = expressions[0];
            AssertThat(expression.Type, Equals(NoESP::ExpressionType::BindScript));
            AssertThat(expression.BindScriptInfo.ScriptName, Equals("ScriptName"));

            expressions = NoESP::Parser::Parse("DifferentScriptName");

            AssertThat(expression.Type, Equals(NoESP::ExpressionType::BindScript));
            AssertThat(expression.ScriptName, Equals("DifferentScriptName"));
            AssertThat(expression.FunctionName, IsEmpty());
        });
        it("ScriptName *selector*", [&](){

        });
        xit("ScriptName [type]:*selector*", [&](){ });
        xit("ScriptName FunctionName", [&](){ });
        xit("ScriptName *selector*", [&](){ });
        xit("OnHit Source=[Spell]:*Fire* ScriptName.FunctionName Arg1=69", [&](){ });
        xit("AddItem Item=FoodSweetRoll Count=69 ScriptName.FunctionName Arg1=69", [&](){ });
    });
});
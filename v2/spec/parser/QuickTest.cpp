#include "../SpecHelper.h"

go_bandit([](){
    describe("NoESP Syntax Parser", [](){
        it("Passes", [&](){
            AssertThat(69, Equals(69));
        });
        it("Fails", [&](){
            AssertThat(69, Equals(420));
        });
    });
});

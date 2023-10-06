#include "SpecHelperInternal.h"

#include <functional>
#include <vector>

namespace HacksPendingSpecsCppLibraryCompletion {
    std::vector<std::function<void()>> SpecDefinitionBodies;

    CollectSpecDefinitionBody::CollectSpecDefinitionBody(std::function<void()> body) {
        SpecDefinitionBodies.push_back(body);
    }
}

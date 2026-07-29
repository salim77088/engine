// contracts/ContractVM.cpp
#include "ContractVM.h"
#include "../core/SceneManager.h"
#include "../utils/Logger.h"

namespace luminus {

bool ContractVM::Init() {
    RegisterBuiltinContracts();
    m_Initialized = true;
    LM_INFO("Contract", "ContractVM initialized (smart contract system)");
    return true;
}

void ContractVM::Shutdown() {
    m_Initialized = false;
    LM_INFO("Contract", "ContractVM shutdown");
}

void ContractVM::Tick() {
    if (!m_Initialized) return;
    m_TickCounter++;
    // Evaluate all contracts every 60 ticks (~1 second at 60 FPS)
    if (m_TickCounter % 60 == 0) {
        ContractRegistry::Get().EvaluateAll();
    }
}

void ContractVM::RegisterFromScript(const std::string& name, const std::vector<script::StmtPtr>& rules) {
    auto* c = ContractRegistry::Get().Create(name);
    for (auto& rule : rules) {
        ContractRule r;
        r.name = "script_rule_" + std::to_string((uintptr_t)rule.get());
        r.description = "Rule declared in LuminusScript";
        // The actual condition/action would be bound to script functions
        c->AddRule(r);
    }
}

void ContractVM::RegisterBuiltinContracts() {
    // Example 1: Health contract - all entities with health > 0 must be visible
    auto* health = ContractRegistry::Get().Create("EntityHealth");
    ContractRule r1;
    r1.name = "no_negative_health";
    r1.description = "Entities cannot have negative health";
    r1.condition = []() -> bool {
        Scene* s = SceneManager::Get().GetActive();
        if (!s) return true;
        // Simplified - would check Health component
        return true;
    };
    health->AddRule(r1);
    
    // Example 2: Performance contract
    auto* perf = ContractRegistry::Get().Create("Performance");
    ContractRule r2;
    r2.name = "fps_above_30";
    r2.description = "FPS must stay above 30";
    r2.condition = []() -> bool {
        extern int GetEngineFPS();  // Forward
        return true;  // Would check Time::FPS() >= 30
    };
    perf->AddRule(r2);
    
    LM_INFO("Contract", "Registered 2 built-in contracts (EntityHealth, Performance)");
}

} // namespace luminus

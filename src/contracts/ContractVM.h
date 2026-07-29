// contracts/ContractVM.h - VM for executing smart contracts from script
#pragma once
#include "SmartContract.h"
#include "../scripting/Interpreter.h"
#include <memory>

namespace luminus {

class ContractVM {
public:
    bool Init();
    void Shutdown();
    void Tick();
    
    // Bridge LuminusScript's `contract` keyword into ContractRegistry
    void RegisterFromScript(const std::string& name, const std::vector<script::StmtPtr>& rules);
    
    // Built-in example contracts
    void RegisterBuiltinContracts();
    
    ContractRegistry& GetRegistry() { return ContractRegistry::Get(); }

private:
    bool m_Initialized = false;
    int m_TickCounter = 0;
};

} // namespace luminus

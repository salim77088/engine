// contracts/SmartContract.h - Smart contract system for verifiable game logic
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <chrono>

namespace luminus {

struct ContractRule {
    std::string name;
    std::string description;
    std::function<bool()> condition;
    std::function<void()> action;
    bool enabled = true;
    int violations = 0;
    int executions = 0;
    std::chrono::system_clock::time_point lastTriggered;
};

class SmartContract {
public:
    SmartContract(const std::string& name);
    
    void AddRule(const ContractRule& rule);
    bool Evaluate();  // Run all rules, return true if all pass
    void Execute();    // Execute all action() callbacks
    
    const std::string& GetName() const { return m_Name; }
    const std::vector<ContractRule>& GetRules() const { return m_Rules; }
    
    void EnableRule(const std::string& name);
    void DisableRule(const std::string& name);
    
    int GetTotalViolations() const;
    int GetTotalExecutions() const;
    
    // Serialization for persistence/verification
    std::string Serialize() const;
    bool Deserialize(const std::string& data);

private:
    std::string m_Name;
    std::vector<ContractRule> m_Rules;
};

// Contract registry - global contract store
class ContractRegistry {
public:
    static ContractRegistry& Get();
    
    SmartContract* Create(const std::string& name);
    SmartContract* Get(const std::string& name);
    bool Remove(const std::string& name);
    
    void EvaluateAll();
    void ExecuteAll();
    
    const std::unordered_map<std::string, std::shared_ptr<SmartContract>>& GetAll() const { return m_Contracts; }
    
    // Snapshot for verification
    std::string CreateSnapshot() const;
    bool VerifySnapshot(const std::string& snapshot) const;

private:
    ContractRegistry() = default;
    std::unordered_map<std::string, std::shared_ptr<SmartContract>> m_Contracts;
};

} // namespace luminus

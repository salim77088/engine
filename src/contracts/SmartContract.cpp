// contracts/SmartContract.cpp
#include "SmartContract.h"
#include "../utils/Logger.h"
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace luminus {

SmartContract::SmartContract(const std::string& name) : m_Name(name) {}

void SmartContract::AddRule(const ContractRule& rule) {
    m_Rules.push_back(rule);
    LM_INFO("Contract", "[%s] Rule added: %s", m_Name.c_str(), rule.name.c_str());
}

bool SmartContract::Evaluate() {
    bool allPassed = true;
    for (auto& rule : m_Rules) {
        if (!rule.enabled) continue;
        
        bool passed = true;
        try {
            passed = rule.condition ? rule.condition() : true;
        } catch (const std::exception& e) {
            LM_ERROR("Contract", "[%s] Rule '%s' threw: %s", m_Name.c_str(), rule.name.c_str(), e.what());
            passed = false;
        }
        
        if (!passed) {
            rule.violations++;
            allPassed = false;
            LM_WARN("Contract", "[%s] Rule violated: %s", m_Name.c_str(), rule.name.c_str());
        }
    }
    return allPassed;
}

void SmartContract::Execute() {
    for (auto& rule : m_Rules) {
        if (!rule.enabled) continue;
        try {
            if (rule.condition && rule.condition() && rule.action) {
                rule.action();
                rule.executions++;
                rule.lastTriggered = std::chrono::system_clock::now();
            }
        } catch (const std::exception& e) {
            LM_ERROR("Contract", "[%s] Action threw: %s", m_Name.c_str(), e.what());
        }
    }
}

void SmartContract::EnableRule(const std::string& name) {
    for (auto& r : m_Rules) if (r.name == name) r.enabled = true;
}

void SmartContract::DisableRule(const std::string& name) {
    for (auto& r : m_Rules) if (r.name == name) r.enabled = false;
}

int SmartContract::GetTotalViolations() const {
    int total = 0;
    for (auto& r : m_Rules) total += r.violations;
    return total;
}

int SmartContract::GetTotalExecutions() const {
    int total = 0;
    for (auto& r : m_Rules) total += r.executions;
    return total;
}

std::string SmartContract::Serialize() const {
    json j;
    j["name"] = m_Name;
    j["rules"] = json::array();
    for (auto& r : m_Rules) {
        j["rules"].push_back({
            {"name", r.name},
            {"description", r.description},
            {"enabled", r.enabled},
            {"violations", r.violations},
            {"executions", r.executions}
        });
    }
    return j.dump(2);
}

bool SmartContract::Deserialize(const std::string& data) {
    try {
        json j = json::parse(data);
        m_Name = j.value("name", m_Name);
        if (j.contains("rules")) {
            for (auto& rj : j["rules"]) {
                ContractRule r;
                r.name = rj.value("name", "");
                r.description = rj.value("description", "");
                r.enabled = rj.value("enabled", true);
                r.violations = rj.value("violations", 0);
                r.executions = rj.value("executions", 0);
                m_Rules.push_back(r);
            }
        }
        return true;
    } catch (const std::exception& e) {
        LM_ERROR("Contract", "Deserialize error: %s", e.what());
        return false;
    }
}

// ============================================================================
// ContractRegistry
// ============================================================================
ContractRegistry& ContractRegistry::Get() {
    static ContractRegistry instance;
    return instance;
}

SmartContract* ContractRegistry::Create(const std::string& name) {
    auto c = std::make_shared<SmartContract>(name);
    m_Contracts[name] = c;
    LM_INFO("Contract", "Created contract: %s", name.c_str());
    return c.get();
}

SmartContract* ContractRegistry::Get(const std::string& name) {
    auto it = m_Contracts.find(name);
    if (it == m_Contracts.end()) return nullptr;
    return it->second.get();
}

bool ContractRegistry::Remove(const std::string& name) {
    return m_Contracts.erase(name) > 0;
}

void ContractRegistry::EvaluateAll() {
    for (auto& p : m_Contracts) {
        p.second->Evaluate();
    }
}

void ContractRegistry::ExecuteAll() {
    for (auto& p : m_Contracts) {
        p.second->Execute();
    }
}

std::string ContractRegistry::CreateSnapshot() const {
    json j;
    j["contracts"] = json::array();
    for (auto& p : m_Contracts) {
        json cj = json::parse(p.second->Serialize());
        j["contracts"].push_back(cj);
    }
    return j.dump(2);
}

bool ContractRegistry::VerifySnapshot(const std::string& snapshot) const {
    try {
        json snap = json::parse(snapshot);
        if (!snap.contains("contracts")) return false;
        for (auto& cj : snap["contracts"]) {
            std::string name = cj.value("name", "");
            auto it = m_Contracts.find(name);
            if (it == m_Contracts.end()) return false;
            // Compare state
            // (Full comparison would check all rule stats)
        }
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace luminus

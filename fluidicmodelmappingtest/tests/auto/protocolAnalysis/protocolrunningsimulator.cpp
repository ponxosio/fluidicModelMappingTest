#include "protocolrunningsimulator.h"

ProtocolRunningStringSimulator::ProtocolRunningStringSimulator(
        std::shared_ptr<ProtocolGraph> protocol,
        std::shared_ptr<LogicBlocksManager> logicBlocks,
        ContainerCharacteristicsExecutor* executor)
{
    this->protocol = protocol;
    this->logicBlocks = logicBlocks;
    this->executor = executor;
}

ProtocolRunningStringSimulator::~ProtocolRunningStringSimulator() {

}

void ProtocolRunningStringSimulator::simulateExecution() throw(std::runtime_error) {
    std::vector<int> nodes2process = {protocol->getStart()->getContainerId()};
    resetTemporalValues();

    while(!nodes2process.empty()) {
        bool simulateWhileFlag = false;
        bool simulateIfFlag = false;

        int nextId = nodes2process.back();
        nodes2process.pop_back();

        if (protocol->isCpuOperation(nextId)) {
            stream << protocol->getCpuOperation(nextId)->toText();
            protocol->getCpuOperation(nextId)->execute();
        } else if (protocol->isActuatorOperation(nextId)) {
            stream << protocol->getActuatorOperation(nextId)->toText();
            protocol->getActuatorOperation(nextId)->execute(executor);
        } else if (protocol->isControlOperations(nextId)) {
            stream << protocol->getControlNode(nextId)->toText();
            if(logicBlocks->isPhysicalLogicBlock(nextId)) {
                if(logicBlocks->isStartLogicIfBlock(nextId)) {
                    simulateIfFlag = true;
                } else if (logicBlocks->isStartLogicWhileBlock(nextId)) {
                    simulateWhileFlag = true;
                }
            }
        }

        if (simulateIfFlag) {
            simulateIf(nextId, nodes2process);
        } else if (simulateWhileFlag) {
            simulateWhile(nextId, nodes2process);
        } else {
            ProtocolGraph::ProtocolEdgeVectorPtr leaving = protocol->getProjectingEdges(nextId);
            for(const ProtocolGraph::ProtocolEdgePtr & edge: *leaving.get()) {
                if (edge->conditionMet()) {
                    int nextop = edge->getIdTarget();
                    if (find(nodes2process.begin(),nodes2process.end(), nextop) == nodes2process.end()) {
                        nodes2process.push_back(nextop);
                    }
                }
            }
        }
    }
}

void ProtocolRunningStringSimulator::resetTemporalValues() {
    whilesExecuted.clear();

    ifBranchesExecuted.clear();
    ifInitStateMap.clear();
    ifMaxDurationStateMap.clear();
}

void ProtocolRunningStringSimulator::simulateIf(int nodeId, std::vector<int> & nodes2process) {
    auto finded = ifBranchesExecuted.find(nodeId);
    if (finded == ifBranchesExecuted.end()) {
        startNewIfSimulation(nodeId);
    }

    const std::vector<std::shared_ptr<VariableEntry>> & endVariables = logicBlocks->getIfEndVars(nodeId);
    if (hasBeenWritten(endVariables)) {
        int branchNumber = finded->second;
        const std::vector<std::shared_ptr<VariableEntry>> & triggerBranches = logicBlocks->getBranchesTriggeredVars(nodeId);

        updateIfDuration(nodeId);

        if (branchNumber < triggerBranches.size()) {
            //restore to if's init state
            const IfState & initState = ifInitStateMap[nodeId];
            protocol->restoreVariableTableState(*initState.varTableState);
            executor->restoreMachineFlowState(*initState.machineFlowState);

            // set trigerred branch i time
            setToActualTime(triggerBranches[branchNumber]);
            finded->second = branchNumber + 1;

            //clear end variables to wait for branch to finish
            clearHasBeenWritten(endVariables);
        } else {
            finishIfSimulation(nodeId);
        }
    }

    std::shared_ptr<ControlNode> nodePtr = protocol->getControlNode(nodeId);
    const std::vector<int> & endBlocks = nodePtr->getEndBlockId();
    nodes2process.insert(nodes2process.end(), endBlocks.begin(), endBlocks.end());
}

void ProtocolRunningStringSimulator::startNewIfSimulation(int nodeId) {
    //save if init state
    IfState iniState;

    std::shared_ptr<VariableEntry> varTime = protocol->getTimeVariable();
    iniState.time = varTime->getValue();

    std::shared_ptr<Memento<VariableTable>> varTableCopy = protocol->makeVariableTableStateCopy();
    iniState.varTableState = varTableCopy;

    std::shared_ptr<Memento<MachineFlowStringAdapter>> machineFlowCopy = executor->createMachineFlowStateCopy();
    iniState.machineFlowState = machineFlowCopy;

    ifInitStateMap[nodeId] = iniState;

    //clear triggered
    std::shared_ptr<VariableEntry> trigered = logicBlocks->getIfExecutingFlagVar(nodeId);
    trigered->setValue(0.0);
    trigered->blockVariable();

    //clear has been written and block end variables
    const std::vector<std::shared_ptr<VariableEntry>> & endVariables = logicBlocks->getIfEndVars(nodeId);
    blockVariables(endVariables);
    clearHasBeenWritten(endVariables);

    //start first branch
    ifBranchesExecuted.insert(std::make_pair(nodeId, 1));
    const std::vector<std::shared_ptr<VariableEntry>> & triggerBranches = logicBlocks->getBranchesTriggeredVars(nodeId);
    setToActualTime(triggerBranches[0]);
}

void ProtocolRunningStringSimulator::finishIfSimulation(int nodeId) {
    //set longest execution end var table state
    const IfState & maxDurationState = ifMaxDurationStateMap[nodeId];
    protocol->restoreVariableTableState(*maxDurationState.varTableState);
    executor->restoreMachineFlowState(*maxDurationState.machineFlowState);

    //unblock end variables
    const std::vector<std::shared_ptr<VariableEntry>> & endVariables = logicBlocks->getIfEndVars(nodeId);
    unBlockVariables(endVariables);
    setToActualTime(endVariables);

    //set trigerred
    std::shared_ptr<VariableEntry> trigered = logicBlocks->getIfExecutingFlagVar(nodeId);
    trigered->unblockVariable();
    trigered->setValue(1.0);

    //release maps
    ifBranchesExecuted.erase(nodeId);
    ifInitStateMap.erase(nodeId);
    ifMaxDurationStateMap.erase(nodeId);
}

void ProtocolRunningStringSimulator::updateIfDuration(int nodeId) {
    double actualTime = protocol->getTimeVariable()->getValue();
    double initTime = ifInitStateMap[nodeId].time;
    double duration = actualTime - initTime;

    auto it = ifMaxDurationStateMap.find(nodeId);
    if (it != ifMaxDurationStateMap.end()) {
        IfState & maxDurationState = it->second;
        if (duration > maxDurationState.time) {
            //update max duration
            maxDurationState.time = duration;

            //update branch end var table state
            std::shared_ptr<Memento<VariableTable>> stateCopy = protocol->makeVariableTableStateCopy();
            maxDurationState.varTableState = stateCopy;

            std::shared_ptr<Memento<MachineFlowStringAdapter>> machineFlowState = executor->createMachineFlowStateCopy();
            maxDurationState.machineFlowState = machineFlowState;
        }
    } else {
        IfState newMaxState;
        newMaxState.time = duration;

        std::shared_ptr<Memento<VariableTable>> stateCopy = protocol->makeVariableTableStateCopy();
        newMaxState.varTableState = stateCopy;

        std::shared_ptr<Memento<MachineFlowStringAdapter>> machineFlowState = executor->createMachineFlowStateCopy();
        newMaxState.machineFlowState = machineFlowState;

        ifMaxDurationStateMap.insert(std::make_pair(nodeId, newMaxState));
    }
}

void ProtocolRunningStringSimulator::simulateWhile(int nodeId, std::vector<int> & nodes2process) {
    auto finded = whilesExecuted.find(nodeId);
    if (finded == whilesExecuted.end()) {
        startNewWhileSimulation(nodeId);
    }

    std::shared_ptr<VariableEntry> executingWhileFlag = logicBlocks->getWhileExecutingFlagVar(nodeId);
    if (executingWhileFlag->hasBeenWritten()) {
        finishWhileSimulation(nodeId);
    }

    std::shared_ptr<ControlNode> nodePtr = protocol->getControlNode(nodeId);
    const std::vector<int> & endBlocks = nodePtr->getEndBlockId();
    nodes2process.insert(nodes2process.end(), endBlocks.begin(), endBlocks.end());
}

void ProtocolRunningStringSimulator::startNewWhileSimulation(int nodeId) {
    //block end variables
    whilesExecuted.insert(nodeId);
    blockVariables(logicBlocks->getWhilesEndVars(nodeId));

    //clear haswritten of trigered variable to wait for be set again
    std::shared_ptr<VariableEntry> executingWhileFlag = logicBlocks->getWhileExecutingFlagVar(nodeId);
    executingWhileFlag->clearHasBeenWritten();

    //trigger the while
    std::shared_ptr<VariableEntry> trigger = logicBlocks->getWhileTrigeredVar(nodeId);
    setToActualTime(trigger);
}

void ProtocolRunningStringSimulator::finishWhileSimulation(int nodeId) {
    const std::vector<std::shared_ptr<VariableEntry>> & endVariables = logicBlocks->getWhilesEndVars(nodeId);
    unBlockVariables(endVariables);
    setToActualTime(endVariables);
}

void ProtocolRunningStringSimulator::setToActualTime(std::shared_ptr<VariableEntry> varEntry) {
    std::shared_ptr<VariableEntry> timeVar = protocol->getTimeVariable();
    varEntry->setValue(timeVar->getValue());
}

void ProtocolRunningStringSimulator::setToActualTime(const std::vector<std::shared_ptr<VariableEntry>> & varEntry) {
    std::shared_ptr<VariableEntry> timeVar = protocol->getTimeVariable();
    for(std::shared_ptr<VariableEntry> var: varEntry) {
        var->setValue(timeVar->getValue());
    }
}

void ProtocolRunningStringSimulator::blockVariables(const std::vector<std::shared_ptr<VariableEntry>> & varEntry) {
    for(std::shared_ptr<VariableEntry> var: varEntry) {
        var->blockVariable();
    }
}

void ProtocolRunningStringSimulator::unBlockVariables(const std::vector<std::shared_ptr<VariableEntry>> & varEntry) {
    for(std::shared_ptr<VariableEntry> var: varEntry) {
        var->unblockVariable();
    }
}

bool ProtocolRunningStringSimulator::hasBeenWritten(const std::vector<std::shared_ptr<VariableEntry>> & varEntry) {
    bool hasBeenWritten = false;
    for(auto it = varEntry.begin(); !hasBeenWritten && it != varEntry.end(); ++it) {
        hasBeenWritten = (*it)->hasBeenWritten();
    }
    return hasBeenWritten;
}

void ProtocolRunningStringSimulator::clearHasBeenWritten(const std::vector<std::shared_ptr<VariableEntry>> & varEntry) {
    for(std::shared_ptr<VariableEntry> var: varEntry) {
        var->clearHasBeenWritten();
    }
}
























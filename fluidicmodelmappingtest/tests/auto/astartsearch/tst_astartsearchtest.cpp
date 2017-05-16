#include <QString>
#include <QtTest>

#include <algorithm>

#include <commonmodel/functions/measureodfunction.h>
#include <commonmodel/functions/pumppluginfunction.h>
#include <commonmodel/functions/valvepluginroutefunction.h>
#include <constraintengine/prologtranslationstack.h>

#include <fluidicmachinemodel/fluidicmachinemodel.h>
#include <fluidicmachinemodel/machinegraph.h>

#include <fluidicmodelmapping/heuristic/containercharacteristics.h>
#include <fluidicmodelmapping/heuristic/topologyheuristic.h>
#include <fluidicmodelmapping/protocolAnalysis/machineflowstringadapter.h>
#include <fluidicmodelmapping/searchalgorithms/astarsearch.h>

class AstartsearchTest : public QObject
{
    Q_OBJECT

public:
    AstartsearchTest();

private:
    std::shared_ptr<FluidicMachineModel> makeMachineModel();
    void makeTurbidostatAnalysis(std::vector<ContainerCharacteristics> & containerCharacteristics,
                                 std::vector<MachineFlowStringAdapter::FlowsVector> & flowsintime);


private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void turbidostat_simpleMachine();
};

AstartsearchTest::AstartsearchTest()
{
}

void AstartsearchTest::initTestCase() {
    PrologExecutor::createEngine(std::string(QTest::currentAppName()));
}

void AstartsearchTest::cleanupTestCase() {
    PrologExecutor::destoryEngine();
}

void AstartsearchTest::turbidostat_simpleMachine()
{
    try{
        std::shared_ptr<FluidicMachineModel> model = makeMachineModel();

        std::vector<ContainerCharacteristics> protocolContainersCharacts;
        std::vector<MachineFlowStringAdapter::FlowsVector> flowsinTime;
        makeTurbidostatAnalysis(protocolContainersCharacts, flowsinTime);

        std::sort(protocolContainersCharacts.begin(), protocolContainersCharacts.end(), ContainerCharacteristics::ContainerCharacteristicsComparator());

        std::shared_ptr<HeuristicInterface> topologyH = std::make_shared<TopologyHeuristic>(model->getMachineGraph(), protocolContainersCharacts);
        AStarSearch aSearch(model, topologyH, protocolContainersCharacts, flowsinTime);

        QVERIFY2(aSearch.startSearch(), "search fail");

        const SearchInterface::RelationTable & solution = aSearch.getRelationTable().back();
        for(const auto & itTuple: solution) {
            const std::string & name = itTuple.first;
            int machineId = itTuple.second;

            qDebug() << name.c_str() << "->" << machineId;
        }

        QVERIFY2(solution.at("cell") == 2, "cell is not 2 check debug values");

        bool option1 = ((solution.at("media") == 0 || solution.at("media") == 1) && solution.at("waste") == 3);
        bool option2 = ((solution.at("waste") == 0 || solution.at("waste") == 1) && solution.at("media") == 3);
        QVERIFY2(option1 || option2,"media and waste are not correct, check debug values");


    } catch(std::exception & e) {
        QFAIL(std::string("Execpetion occured, message: " + std::string(e.what())).c_str());
    }
}

/*
 *
 *                    +--------+----------+---------+
 *                    |0:close | 1:c0 >c2 | 2:c1 >c2|
 *   +---+            +--------+--------------------+
 *   |C_1+--------+   |       3: (c0 & c1)> c2      |
 *   +---+        |   +-----------------------------+
 *              +-v-+      +---+    +---+    +---+
 *              |V_5+----> |C_2+--> |P_4+--> |C_3|
 *   +---+      +-^-+      +---+    +---+    +---+
 *   |C_0|        |
 *   +------------+
 *
 *
 *  C_0, C_1, C_3 : open container,
 *  V_5: valve,
 *  P_4: bidirectional pump,
 *  C_2: close container,
 */
std::shared_ptr<FluidicMachineModel> AstartsearchTest::makeMachineModel() {

    std::shared_ptr<MachineGraph> mGraph = std::make_shared<MachineGraph>();
    PluginConfiguration config;
    std::shared_ptr<PluginAbstractFactory> factory = nullptr;

    std::shared_ptr<Function> pumpf = std::make_shared<PumpPluginFunction>(factory, config);
    std::shared_ptr<Function> routef = std::make_shared<ValvePluginRouteFunction>(factory, config);
    std::shared_ptr<Function> measureOd = std::make_shared<MeasureOdFunction>(factory, config, 100);

    int c0 = mGraph->emplaceContainer(2, ContainerNode::open, 100.0);
    int c1 = mGraph->emplaceContainer(2, ContainerNode::open, 100.0);

    int c2 = mGraph->emplaceContainer(2, ContainerNode::close, 100.0);
    mGraph->getContainer(c2)->addOperation(measureOd);

    int c3 = mGraph->emplaceContainer(2, ContainerNode::open, 100.0);

    int p = mGraph->emplacePump(2, PumpNode::bidirectional, pumpf);

    ValveNode::TruthTable table;
    std::vector<std::unordered_set<int>> empty;
    table.insert(std::make_pair(0, empty));
    std::vector<std::unordered_set<int>> pos1 = {{0,2}};
    table.insert(std::make_pair(1, pos1));
    std::vector<std::unordered_set<int>> pos2 = {{1,2}};
    table.insert(std::make_pair(2, pos2));
    std::vector<std::unordered_set<int>> pos3 = {{0,1,2}};
    table.insert(std::make_pair(3, pos3));

    int v= mGraph->emplaceValve(3, table, routef);

    mGraph->connectNodes(c0,v,1,0);
    mGraph->connectNodes(c1,v,1,1);
    mGraph->connectNodes(v,c2,2,0);
    mGraph->connectNodes(c2,p,1,0);
    mGraph->connectNodes(p,c3,1,0);


    std::shared_ptr<TranslationStack> transStack = std::make_shared<PrologTranslationStack>();
    std::shared_ptr<FluidicMachineModel> modelPtr = std::make_shared<FluidicMachineModel>(mGraph, transStack);
    return modelPtr;
}

void AstartsearchTest::makeTurbidostatAnalysis(std::vector<ContainerCharacteristics> & containerCharacteristics,
                                               std::vector<MachineFlowStringAdapter::FlowsVector> & flowsintime)
{
    MachineFlowStringAdapter machineFlow;
    machineFlow.addFlow("media","cell", 300 * units::ml/units::hr);
    machineFlow.addFlow("cell","waste", 300 * units::ml/units::hr);
    const MachineFlowStringAdapter::FlowsVector & newFlow = machineFlow.updateFlows();
    flowsintime.push_back(newFlow);

    ContainerCharacteristics cmedia("media");
    cmedia.setLeavingConnections(1);
    cmedia.setType(ContainerNode::open);
    containerCharacteristics.push_back(cmedia);

    ContainerCharacteristics ccell("cell");
    ccell.setArrivingConnections(1);
    ccell.setLeavingConnections(1);
    ccell.setType(ContainerNode::close);
    ccell.addFunctions(FunctionSet::FUNCTIONS_FLAG_MAP.at(Function::measure_od));
    containerCharacteristics.push_back(ccell);

    ContainerCharacteristics cwaste("waste");
    cwaste.setArrivingConnections(1);
    cwaste.setType(ContainerNode::open);
    containerCharacteristics.push_back(cwaste);
}

QTEST_APPLESS_MAIN(AstartsearchTest)

#include "tst_astartsearchtest.moc"

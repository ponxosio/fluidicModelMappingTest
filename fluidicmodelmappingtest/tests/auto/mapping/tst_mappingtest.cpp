#include <QString>
#include <QtTest>

#include <bioblocksExecution/bioblocksSimulation/bioblocksrunningsimulator.h>

#include <bioblocksTranslation/bioblockstranslator.h>
#include <bioblocksTranslation/logicblocksmanager.h>

#include <commonmodel/functions/measureodfunction.h>
#include <commonmodel/functions/pumppluginfunction.h>
#include <commonmodel/functions/valvepluginroutefunction.h>
#include <constraintengine/prologtranslationstack.h>

#include <fluidicmachinemodel/fluidicmachinemodel.h>
#include <fluidicmachinemodel/machinegraph.h>

#include <fluidicmodelmapping/fluidicmodelmapping.h>

class MappingTest : public QObject
{
    Q_OBJECT

public:
    MappingTest();

private:
    std::shared_ptr<MachineGraph> makeMachineGraph();
    std::shared_ptr<MachineGraph> makeMultipathWashMachineGraph();

    std::shared_ptr<FluidicMachineModel> makeModel(std::shared_ptr<MachineGraph> machine);

    void copyResourceFile(const QString & resourcePath, QTemporaryFile* tempFile) throw(std::invalid_argument);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void mappingTurbidostatSimpleMachine();
    void mappingTurbidostatComplexMachine();

    void mappingSwitchingSimpleMachine();
    void mappingSwitchingComplexMachine();
};

MappingTest::MappingTest()
{
}

void MappingTest::initTestCase() {
    PrologExecutor::createEngine(std::string(QTest::currentAppName()));
}

void MappingTest::cleanupTestCase() {
    PrologExecutor::destoryEngine();
}

/*
 * rate = 300;
 * od = 0;
 * while[0s:-]( od < 600) {
 *  od = measureOD[x:2s](cell,650nm);
 *  rate = rate - (rate *(od - 600));
 *  setContinuosFlow[x:30s]([media,cell,waste], rate ml/hr);
 * }
 */
void MappingTest::mappingTurbidostatSimpleMachine()
{
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/trubidostat.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<FluidicMachineModel> model = makeModel(makeMachineGraph());
            std::shared_ptr<FluidicModelMapping> mapping = std::make_shared<FluidicModelMapping>(model);

            std::shared_ptr<ProtocolSimulatorInterface> simulator =
                    std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            std::string errorMsg;
            bool solution = mapping->findRelation(simulator, errorMsg);

            qDebug() << errorMsg.c_str();

            QVERIFY2(solution, "Impossible to find relation");

            int mappedMedia = mapping->getMappedComponent("media");
            int mappedCell = mapping->getMappedComponent("cell");
            int mappedWaste = mapping->getMappedComponent("waste");

            qDebug() << "media" << mappedMedia;
            qDebug() << "cell" << mappedCell;
            qDebug() << "waste" << mappedWaste;

            QVERIFY2(mappedCell == 2, "Mapped cell is not 2");
            QVERIFY2(((mappedMedia == 0 || mappedMedia == 1) && mappedWaste == 3) ||
                     ((mappedWaste == 0 || mappedWaste == 1) && mappedMedia == 3),
                     "mapping combinations not met, allowed combinations are:"
                     "media ={0,1} and waste = 3 or media = 3 and waste={0,1}");

        } catch(std::exception & e) {
            QFAIL(e.what());
        }
    } else {
        QFAIL("imposible to create temporary file");
    }
}

/*
 * rate = 300;
 * od = 0;
 * while[0s:-]( od < 600) {
 *  od = measureOD[x:2s](cell,650nm);
 *  rate = rate - (rate *(od - 600));
 *  setContinuosFlow[x:30s]([media,cell,waste], rate ml/hr);
 * }
 */
void MappingTest::mappingTurbidostatComplexMachine() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/trubidostat.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<FluidicMachineModel> model = makeModel(makeMultipathWashMachineGraph());
            std::shared_ptr<FluidicModelMapping> mapping = std::make_shared<FluidicModelMapping>(model);

            std::shared_ptr<ProtocolSimulatorInterface> simulator =
                    std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            std::string errorMsg;
            bool solution = mapping->findRelation(simulator, errorMsg);

            qDebug() << errorMsg.c_str();

            QVERIFY2(solution, "Impossible to find relation");

            int mappedMedia = mapping->getMappedComponent("media");
            int mappedCell = mapping->getMappedComponent("cell");
            int mappedWaste = mapping->getMappedComponent("waste");

            qDebug() << "media" << mappedMedia;
            qDebug() << "cell" << mappedCell;
            qDebug() << "waste" << mappedWaste;

            QVERIFY2(mappedCell == 7, "Mapped cell is not 7");
            QVERIFY2(mappedMedia == 1, "Mapped media is not 1");
            QVERIFY2((mappedWaste == 0) || (mappedWaste == 2), "Mapped waste is not 0 or 2");

        } catch(std::exception & e) {
            QFAIL(e.what());
        }
    } else {
        QFAIL("imposible to create temporary file");
    }
}

/*
 * countre = 0;
 * while[0s:x](counter < 10) {
 *  setContinuousflow[x:30min](media1,cell,waste,200ml/hr);
 *  setContinuousflow[x:30min](media2,cell,waste,200ml/hr);
 *  counter = counter + 1;
 * }
 */
void MappingTest::mappingSwitchingSimpleMachine() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/switchingProtocol.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::minute, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<FluidicMachineModel> model = makeModel(makeMachineGraph());
            std::shared_ptr<FluidicModelMapping> mapping = std::make_shared<FluidicModelMapping>(model);

            std::shared_ptr<ProtocolSimulatorInterface> simulator =
                    std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            std::string errorMsg;
            bool solution = mapping->findRelation(simulator, errorMsg);

            qDebug() << errorMsg.c_str();

            QVERIFY2(solution, "Impossible to find relation");

            int mappedMedia1 = mapping->getMappedComponent("media1");
            int mappedMedia2 = mapping->getMappedComponent("media2");
            int mappedCell = mapping->getMappedComponent("cell");
            int mappedWaste = mapping->getMappedComponent("waste");

            qDebug() << "media1" << mappedMedia1;
            qDebug() << "media2" << mappedMedia2;
            qDebug() << "cell" << mappedCell;
            qDebug() << "waste" << mappedWaste;

            QVERIFY2(mappedCell == 2, "Mapped cell is not 2");
            QVERIFY2((mappedMedia1 == 1 && mappedMedia2 == 0) ||
                     (mappedMedia1 == 0 && mappedMedia2 == 1)
                     , "media container are not mapped correctly, possibilites are: media1 = 0 and media2 = 1 or media1 = 1 and media2 = 0");
            QVERIFY2(mappedWaste == 3, "Mapped waste is not 0 or 2");

        } catch(std::exception & e) {
            QFAIL(e.what());
        }
    } else {
        QFAIL("imposible to create temporary file");
    }
}

/*
 * countre = 0;
 * while[0s:x](counter < 10) {
 *  setContinuousflow[x:30min](media1,cell,waste,200ml/hr);
 *  setContinuousflow[x:30min](media2,cell,waste,200ml/hr);
 *  counter = counter + 1;
 * }
 */
void MappingTest::mappingSwitchingComplexMachine() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/switchingProtocol.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::minute, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<FluidicMachineModel> model = makeModel(makeMultipathWashMachineGraph());
            std::shared_ptr<FluidicModelMapping> mapping = std::make_shared<FluidicModelMapping>(model);

            std::shared_ptr<ProtocolSimulatorInterface> simulator =
                    std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            std::string errorMsg;
            bool solution = mapping->findRelation(simulator, errorMsg);

            qDebug() << errorMsg.c_str();

            QVERIFY2(solution, "Impossible to find relation");

            int mappedMedia1 = mapping->getMappedComponent("media1");
            int mappedMedia2 = mapping->getMappedComponent("media2");
            int mappedCell = mapping->getMappedComponent("cell");
            int mappedWaste = mapping->getMappedComponent("waste");

            qDebug() << "media1" << mappedMedia1;
            qDebug() << "media2" << mappedMedia2;
            qDebug() << "cell" << mappedCell;
            qDebug() << "waste" << mappedWaste;

            QVERIFY2(mappedCell == 6 || mappedCell == 7, "Mapped cell is not 7 or 6");
            QVERIFY2(mappedMedia1 != mappedMedia2, "media containers are the same.");
            QVERIFY2(mappedWaste == 0 || mappedWaste == 2, "Mapped waste is not 0 or 2");
            QVERIFY2(mappedMedia1 == 1 || mappedMedia1 == 3 || mappedMedia1 == 4 || mappedMedia1 == 5,
                     "Mapped media1 is not: 1,3,4,5");
            QVERIFY2(mappedMedia2 == 1 || mappedMedia2 == 3 || mappedMedia2 == 4 || mappedMedia2 == 5,
                     "Mapped media2 is not: 1,3,4,5");

        } catch(std::exception & e) {
            QFAIL(e.what());
        }
    } else {
        QFAIL("imposible to create temporary file");
    }
}

/*
 *
 *                    +--------+----------+---------+
 *                    |0:close | 1:c0 >c2 | 2:c1 >c2|
 *   +---+            +--------+--------------------+
 *   |C_1+--------+
 *   +---+        |
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
 *  C_2: close container, od_sensor
 */
std::shared_ptr<MachineGraph> MappingTest::makeMachineGraph() {
    std::shared_ptr<MachineGraph> mGraph = std::make_shared<MachineGraph>();
    PluginConfiguration config;
    std::shared_ptr<PluginAbstractFactory> factory = nullptr;

    std::shared_ptr<Function> pumpf = std::make_shared<PumpPluginFunction>(factory, config, PumpWorkingRange(0 * units::ml/units::hr, 999 * units::ml/units::hr));
    std::shared_ptr<Function> routef = std::make_shared<ValvePluginRouteFunction>(factory, config);
    std::shared_ptr<Function> odSensorf = std::make_shared<MeasureOdFunction>(factory, config, 1*units::ml, MeasureOdWorkingRange(500 * units::nm, 650*units::nm));

    int c0 = mGraph->emplaceContainer(1, ContainerNode::open, 100.0);
    int c1 = mGraph->emplaceContainer(1, ContainerNode::open, 100.0);

    int c2 = mGraph->emplaceContainer(2, ContainerNode::close, 100.0);
    mGraph->getContainer(c2)->addOperation(odSensorf);

    int c3 = mGraph->emplaceContainer(1, ContainerNode::open, 100.0);

    int p = mGraph->emplacePump(2, PumpNode::bidirectional, pumpf);

    ValveNode::TruthTable table;
    std::vector<std::unordered_set<int>> empty;
    table.insert(std::make_pair(0, empty));
    std::vector<std::unordered_set<int>> pos1 = {{0,2}};
    table.insert(std::make_pair(1, pos1));
    std::vector<std::unordered_set<int>> pos2 = {{1,2}};
    table.insert(std::make_pair(2, pos2));

    int v= mGraph->emplaceValve(3, table, routef);

    mGraph->connectNodes(c0,v,0,0);
    mGraph->connectNodes(c1,v,0,1);
    mGraph->connectNodes(v,c2,2,0);
    mGraph->connectNodes(c2,p,1,0);
    mGraph->connectNodes(p,c3,1,0);

    return mGraph;
}

/*
 * +--+    +--+     +--+    +---+
 * |C1+---->P8+----->C6+---->V12|                                      +-V10-V11-V12+
 * +--+    +--+     +-++    +-+-+                             +--+     |0:close     |
 *                    |       |                        +------+C5|     +------------+
 *                    2       |                        |      +--+     |1:open      |
 *  +-----V17----+  +---+     |      +---+             |               +------------+
 *  |0:close     |  |V14|1----------0>V16<2-----+      3
 *  +------------+  +---+     |      +---+    +--+   +-v-+    +--+
 *  |1:1>0       |    0       |        1      |P9<--0|V17<2---+C4|     +V13-V14-V15-V16-+
 *  +------------+    |     +-v+       |      +-++   +-^-+    +--+     |0:close         |
 *  |2:2>0       |    |     |C2<-------+        |      1               +----------------+
 *  +------------+    2     +-^+       1        |      |               |1:2>0           |
 *  |3:3>0       |  +-v-+     |      +---+      |      |      +--+     +----------------+
 *  +------------+  |V13|1----------0>V15<2-----+      +------+C3|     |2:2>1           |
 *                  +---+     |      +---+                    +--+     +----------------+
 *                    0       |                                        |3:1>0           |
 *                    |       |                                        +----------------+
 * +--+   +---+     +-v+    +-+-+
 * |C0+--->V10+----->C7+---->V11|
 * +--+   +---+     +--+    +---+
 *
 * C0,C1,C2,C3,C4,C5: open container,
 * C6,C7: close container,
 * P8,P9: unidirectional pump,
 * V10,V11,V12,V13,V14,V15,V16,V17: valve with the corresponding thruth table.
 *
 */
std::shared_ptr<MachineGraph> MappingTest::makeMultipathWashMachineGraph() {
    std::shared_ptr<MachineGraph> mGraph = std::make_shared<MachineGraph>();

    PluginConfiguration config;
    std::shared_ptr<PluginAbstractFactory> factory = nullptr;

    std::shared_ptr<Function> pumpf1 =
            std::make_shared<PumpPluginFunction>(factory, config, PumpWorkingRange(0 * units::ml/units::hr, 999 * units::ml/units::hr));
    std::shared_ptr<Function> pumpf2 =
            std::make_shared<PumpPluginFunction>(factory, config, PumpWorkingRange(100 * units::ml/units::hr, 200 * units::ml/units::hr));
    std::shared_ptr<Function> routef = std::make_shared<ValvePluginRouteFunction>(factory, config);

    std::shared_ptr<Function> odSsensor =
            std::make_shared<MeasureOdFunction>(factory, config, 1*units::ml, MeasureOdWorkingRange(500 * units::nm, 650 * units::nm));

    int sample = mGraph->emplaceContainer(1, ContainerNode::open, 100.0);
    int media = mGraph->emplaceContainer(1, ContainerNode::open, 100.0);
    int waste = mGraph->emplaceContainer(4, ContainerNode::open, 100.0);
    int water = mGraph->emplaceContainer(1, ContainerNode::open, 100.0);
    int ethanol = mGraph->emplaceContainer(1, ContainerNode::open, 100.0);
    int naoh = mGraph->emplaceContainer(1, ContainerNode::open, 100.0);

    int chemo = mGraph->emplaceContainer(3, ContainerNode::close, 100.0);
    int cell = mGraph->emplaceContainer(3, ContainerNode::close, 100.0);
    mGraph->getContainer(cell)->addOperation(odSsensor);

    int p1 = mGraph->emplacePump(2, PumpNode::unidirectional, pumpf1);
    int p2 = mGraph->emplacePump(3, PumpNode::unidirectional, pumpf2);

    ValveNode::TruthTable tableType1;
    std::vector<std::unordered_set<int>> empty;
    tableType1.insert(std::make_pair(0, empty));
    std::vector<std::unordered_set<int>> pos11 = {{0,1}};
    tableType1.insert(std::make_pair(1, pos11));

    ValveNode::TruthTable tableType2;
    tableType2.insert(std::make_pair(0, empty));
    std::vector<std::unordered_set<int>> pos12 = {{0,2}};
    tableType2.insert(std::make_pair(1, pos12));
    std::vector<std::unordered_set<int>> pos22 = {{1,2}};
    tableType2.insert(std::make_pair(2, pos22));
    std::vector<std::unordered_set<int>> pos32 = {{0,1}};
    tableType2.insert(std::make_pair(3, pos32));

    ValveNode::TruthTable tableType3;
    tableType3.insert(std::make_pair(0, empty));
    std::vector<std::unordered_set<int>> pos13 = {{1,0}};
    tableType3.insert(std::make_pair(1, pos13));
    std::vector<std::unordered_set<int>> pos23 = {{2,0}};
    tableType3.insert(std::make_pair(2, pos23));
    std::vector<std::unordered_set<int>> pos33 = {{3,0}};
    tableType3.insert(std::make_pair(3, pos33));

    int v1 = mGraph->emplaceValve(2, tableType1, routef);
    int v5 = mGraph->emplaceValve(2, tableType1, routef);
    int v4 = mGraph->emplaceValve(2, tableType1, routef);

    int v2 = mGraph->emplaceValve(3, tableType2, routef);
    int v3 = mGraph->emplaceValve(3, tableType2, routef);
    int v6 = mGraph->emplaceValve(3, tableType2, routef);
    int v7 = mGraph->emplaceValve(3, tableType2, routef);

    int v8 = mGraph->emplaceValve(4, tableType3, routef);

    mGraph->connectNodes(media,p1,0,0);
    mGraph->connectNodes(p1,chemo,1,0);
    mGraph->connectNodes(chemo,v3,1,2);
    mGraph->connectNodes(chemo,v4,2,0);
    mGraph->connectNodes(v4,waste,1,1);
    mGraph->connectNodes(v3,v2,0,2);
    mGraph->connectNodes(v3,v7,1,0);
    mGraph->connectNodes(v7,waste,1,2);
    mGraph->connectNodes(p2,v7,1,2);
    mGraph->connectNodes(v8,p2,0,2);
    mGraph->connectNodes(water,v8,0,1);
    mGraph->connectNodes(ethanol,v8,0,2);
    mGraph->connectNodes(naoh,v8,0,3);
    mGraph->connectNodes(v2,cell,0,1);
    mGraph->connectNodes(v2,v6,1,0);
    mGraph->connectNodes(v6,waste,1,3);
    mGraph->connectNodes(p2,v6,0,2);
    mGraph->connectNodes(cell,v1,0,1);
    mGraph->connectNodes(cell,v5,2,0);
    mGraph->connectNodes(v1,sample,0,0);
    mGraph->connectNodes(v5,waste,1,0);

    return mGraph;
}

std::shared_ptr<FluidicMachineModel> MappingTest::makeModel(std::shared_ptr<MachineGraph> machine) {
    std::shared_ptr<PrologTranslationStack> translationStack = std::make_shared<PrologTranslationStack>();
    std::shared_ptr<FluidicMachineModel> model =
            std::make_shared<FluidicMachineModel>(machine, translationStack, 3, 2, 300, units::ml/units::hr);
    return model;
}

void MappingTest::copyResourceFile(const QString & resourcePath, QTemporaryFile* tempFile) throw(std::invalid_argument) {
    QFile resourceFile(resourcePath);
    if(!resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw(std::invalid_argument("imposible to open" + resourcePath.toStdString()));
    }

    QTextStream out(tempFile);

    QTextStream in(&resourceFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        out << line;
    }
    out.flush();
}

QTEST_APPLESS_MAIN(MappingTest)

#include "tst_mappingtest.moc"

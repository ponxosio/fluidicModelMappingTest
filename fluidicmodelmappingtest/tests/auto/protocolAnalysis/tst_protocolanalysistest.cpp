#include <QString>
#include <QtTest>
#include <QTemporaryFile>
#include <QFile>

#include <sstream>

#include <bioblocksTranslation/bioblockstranslator.h>

#include <commonmodel/functions/measureodfunction.h>
#include <commonmodel/functions/pumppluginfunction.h>
#include <commonmodel/functions/valvepluginroutefunction.h>
#include <constraintengine/prologtranslationstack.h>

#include <fluidicmachinemodel/fluidicmachinemodel.h>
#include <fluidicmachinemodel/machinegraph.h>

#include <fluidicmodelmapping/heuristic/containercharacteristics.h>
#include <fluidicmodelmapping/protocolAnalysis/analysisexecutor.h>

class ProtocolAnalysisTest : public QObject
{
    Q_OBJECT

public:
    ProtocolAnalysisTest();

private:
    void copyResourceFile(const QString & resourcePath, QTemporaryFile* file) throw(std::invalid_argument);
    std::shared_ptr<FluidicMachineModel> makeMachineModel();

    std::string ccToString(const ContainerCharacteristics & container);
    std::string waorkingRangeToString(const ContainerCharacteristics::WorkingRangeMap & map);

private Q_SLOTS:
    void switchingFlows();
};

ProtocolAnalysisTest::ProtocolAnalysisTest()
{
}

/*
 * SetContinuosFlow[0s:30s](A,B,300ml/hr);
 * SetContinuosFlow[0s:60s](B,C,300ml/hr);
 * SetContinuosFlow[30s:30s](D,B,300ml/hr);
 */
void ProtocolAnalysisTest::switchingFlows()
{
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/switchingProtocol.json", tempFile);

            BioBlocksTranslator translator(5*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile();

            qDebug() << protocol->toString().c_str();

            AnalysisExecutor executor(protocol, 300 * units::ml / units::hr);




        } catch (std::exception & e) {
            delete tempFile;
            QFAIL(e.what());
        }
    } else {
        delete tempFile;
        QFAIL("imposible to create temporary file");
    }
    delete tempFile;
}

/*
 *
 *                    +--------+----------+---------+
 *                    |0:close | 1:c0 >c2 | 2:c1 >c2|
 *   +---+            +--------+--------------------+
 *   |C_1+--------+   |            V_5              |
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
std::shared_ptr<FluidicMachineModel> ProtocolAnalysisTest::makeMachineModel() {

    std::shared_ptr<MachineGraph> mGraph = std::make_shared<MachineGraph>();
    PluginConfiguration config;
    std::shared_ptr<PluginAbstractFactory> factory = nullptr;

    std::shared_ptr<Function> pumpf =
            std::make_shared<PumpPluginFunction>(factory,
                                                 config,
                                                 PumpWorkingRange(300 * units::ml/units::hr,
                                                                  600 * units::ml/units::hr));

    std::shared_ptr<Function> routef = std::make_shared<ValvePluginRouteFunction>(factory, config);
    std::shared_ptr<Function> measureOd =
            std::make_shared<MeasureOdFunction>(factory,
                                                config,
                                                100 * units::ml,
                                                MeasureOdWorkingRange(500 * units::nm, 680 * units::nm));

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

void ProtocolAnalysisTest::copyResourceFile(const QString & resourcePath, QTemporaryFile* tempFile) throw(std::invalid_argument) {
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

std::string ProtocolAnalysisTest::ccToString(const ContainerCharacteristics & container) {
    std::stringstream stream;

    stream << "name: " << container.getName() << ";";
    stream << "arriving connections: " << container.getArrivingConnections() << ";";
    stream << "leaving connections: " << container.getLeavingConnections() << ";";
    stream << "functions: " << container.getNeccesaryFunctionsMask().to_string << ";";
    stream << "type: " << container.getType() << ";";
}

std::string ProtocolAnalysisTest::workingRangeToString(const ContainerCharacteristics::WorkingRangeMap & map) {
    std::stringstream stream;

    stream << "[";
}

QTEST_APPLESS_MAIN(ProtocolAnalysisTest)

#include "tst_protocolanalysistest.moc"

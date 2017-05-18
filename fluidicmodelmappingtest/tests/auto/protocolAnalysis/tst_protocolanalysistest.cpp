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
    std::string workingRangeToString(const ContainerCharacteristics::WorkingRangeMap & map);
    std::string flowsInTimeToString(const std::vector<MachineFlowStringAdapter::FlowsVector> & flowInTime);

private Q_SLOTS:
    void switchingFlowsTest();
    void parallelFlowsTest();
    void workingRangesTest();
    void turbidostatTest();
};

ProtocolAnalysisTest::ProtocolAnalysisTest()
{
}

/*
 * SetContinuosFlow[0s:30s](A,B,300ml/hr);
 * SetContinuosFlow[0s:30s](B,C,300ml/hr);
 * SetContinuosFlow[30s:30s](D,B,300ml/hr);
 * SetContinuosFlow[30s:30s](B,C,300ml/hr);
 */
void ProtocolAnalysisTest::switchingFlowsTest()
{
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/switchingProtocol.json", tempFile);

            BioBlocksTranslator translator(5*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile();

            qDebug() << protocol->toString().c_str();

            AnalysisExecutor executor(protocol, 300 * units::ml/units::hr);
            const std::vector<ContainerCharacteristics> & ccVector(executor.getVCVector());

            std::vector<std::string> generatedStrCcVector;
            generatedStrCcVector.reserve(ccVector.size());

            qDebug() << "generated:";
            qDebug() << "containers:";
            for(const ContainerCharacteristics & container: ccVector) {
                std::string tempStr = ccToString(container);

                qDebug() << tempStr.c_str();
                generatedStrCcVector.push_back(tempStr);
            }

            std::string generatedFlowsStr = flowsInTimeToString(executor.getFlowsInTime());
            qDebug() << "flows in time:";
            qDebug() << generatedFlowsStr.c_str();

            std::vector<std::string> expectedStrCcVector {
                    "name: A;arriving connections: 0;leaving connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: B;arriving connections: 2;leaving connections: 1;functions: 0000000000000;type: 1;workingRanges[]",
                    "name: C;arriving connections: 1;leaving connections: 0;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: D;arriving connections: 0;leaving connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
                };
            std::string expectedFlowsStr = "[[{[A,B,C,],300 ml/hr},],[{[D,B,C,],300 ml/hr},],]";

            qDebug() << "expected:";
            qDebug() << "containers";
            for(const std::string & str : expectedStrCcVector) {
                qDebug() << str.c_str();
            }
            qDebug() << "flows in time:";
            qDebug() << expectedFlowsStr.c_str();

            QVERIFY2(generatedFlowsStr.compare(expectedFlowsStr) == 0, "flows in time are not the same, check debug for more info");

            QVERIFY2(expectedStrCcVector.size() == generatedStrCcVector.size(), "expected and generated container characteristic has not the same size");
            for(int i = 0; i < expectedStrCcVector.size(); i++) {
                QVERIFY2(expectedStrCcVector[i].compare(generatedStrCcVector[i]) == 0,
                         std::string(std::to_string(i) + " position in container characteristics vector is not as expected, check debug for more info").c_str());
            }
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
 * setContinuosFlow[0:30s](A,B,300ml/hr);
 * setContinuosFlow[0:30s](B,C,300ml/hr);
 * setContinuosFlow[0:30s](D,B,300ml/hr);
 * setContinuosFlow[0:30s](B,C,300ml/hr);
 *
 */
void ProtocolAnalysisTest::parallelFlowsTest() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/paralelleProtocol.json", tempFile);

            BioBlocksTranslator translator(5*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile();

            qDebug() << protocol->toString().c_str();

            AnalysisExecutor executor(protocol, 300 * units::ml/units::hr);
            const std::vector<ContainerCharacteristics> & ccVector(executor.getVCVector());

            std::vector<std::string> generatedStrCcVector;
            generatedStrCcVector.reserve(ccVector.size());

            qDebug() << "generated:";
            qDebug() << "containers:";
            for(const ContainerCharacteristics & container: ccVector) {
                std::string tempStr = ccToString(container);

                qDebug() << tempStr.c_str();
                generatedStrCcVector.push_back(tempStr);
            }

            std::string generatedFlowsStr = flowsInTimeToString(executor.getFlowsInTime());
            qDebug() << "flows in time:";
            qDebug() << generatedFlowsStr.c_str();

            std::vector<std::string> expectedStrCcVector {
                    "name: A;arriving connections: 0;leaving connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: B;arriving connections: 2;leaving connections: 1;functions: 0000000000000;type: 1;workingRanges[]",
                    "name: C;arriving connections: 1;leaving connections: 0;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: D;arriving connections: 0;leaving connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
                };
            std::string expectedFlowsStr = "[[{[A,B,C,],300 ml/hr},{[D,B,C,],300 ml/hr},],]";

            qDebug() << "expected:";
            qDebug() << "containers";
            for(const std::string & str : expectedStrCcVector) {
                qDebug() << str.c_str();
            }
            qDebug() << "flows in time:";
            qDebug() << expectedFlowsStr.c_str();

            QVERIFY2(generatedFlowsStr.compare(expectedFlowsStr) == 0, "flows in time are not the same, check debug for more info");

            QVERIFY2(expectedStrCcVector.size() == generatedStrCcVector.size(), "expected and generated container characteristic has not the same size");
            for(int i = 0; i < expectedStrCcVector.size(); i++) {
                QVERIFY2(expectedStrCcVector[i].compare(generatedStrCcVector[i]) == 0,
                         std::string(std::to_string(i) + " position in container characteristics vector is not as expected, check debug for more info").c_str());
            }
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
 * measureOD[0s:3s](A,650nm);
 * centrifugation[x:2s](A,50Hz,26ºC);
 * centrifugation[x:2s](A,50Hz,26ºC);
 * measureFluorescence[x:2s](A,650nm,680nm);
 *
 */
void ProtocolAnalysisTest::workingRangesTest() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/workingrangeProtocol.json", tempFile);

            BioBlocksTranslator translator(200*units::ms, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile();

            qDebug() << protocol->toString().c_str();

            AnalysisExecutor executor(protocol, 300 * units::ml/units::hr);
            const std::vector<ContainerCharacteristics> & ccVector(executor.getVCVector());

            std::vector<std::string> generatedStrCcVector;
            generatedStrCcVector.reserve(ccVector.size());

            qDebug() << "generated:";
            qDebug() << "containers:";
            for(const ContainerCharacteristics & container: ccVector) {
                std::string tempStr = ccToString(container);

                qDebug() << tempStr.c_str();
                generatedStrCcVector.push_back(tempStr);
            }

            std::string generatedFlowsStr = flowsInTimeToString(executor.getFlowsInTime());
            qDebug() << "flows in time:";
            qDebug() << generatedFlowsStr.c_str();

            std::vector<std::string> expectedStrCcVector {
                    "name: A;arriving connections: 0;leaving connections: 0;functions: 0100001010100;type: 2;workingRanges[4:[650 nm, 650nm],2:[26 Cº, 26Cº],6:[25 Hz, 50Hz],11:emission:[680 nm, 680nm]excitation:[650 nm, 650nm],]"
                };
            std::string expectedFlowsStr = "[]";

            qDebug() << "expected:";
            qDebug() << "containers";
            for(const std::string & str : expectedStrCcVector) {
                qDebug() << str.c_str();
            }
            qDebug() << "flows in time:";
            qDebug() << expectedFlowsStr.c_str();

            QVERIFY2(generatedFlowsStr.compare(expectedFlowsStr) == 0, "flows in time are not the same, check debug for more info");

            QVERIFY2(expectedStrCcVector.size() == generatedStrCcVector.size(), "expected and generated container characteristic has not the same size");
            for(int i = 0; i < expectedStrCcVector.size(); i++) {
                QVERIFY2(expectedStrCcVector[i].compare(generatedStrCcVector[i]) == 0,
                         std::string(std::to_string(i) + " position in container characteristics vector is not as expected, check debug for more info").c_str());
            }
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

void ProtocolAnalysisTest::turbidostatTest() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/trubidostat.json", tempFile);

            BioBlocksTranslator translator(200*units::ms, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile();

            qDebug() << protocol->toString().c_str();

            AnalysisExecutor executor(protocol, 300 * units::ml/units::hr);
            const std::vector<ContainerCharacteristics> & ccVector(executor.getVCVector());

            std::vector<std::string> generatedStrCcVector;
            generatedStrCcVector.reserve(ccVector.size());

            qDebug() << "generated:";
            qDebug() << "containers:";
            for(const ContainerCharacteristics & container: ccVector) {
                std::string tempStr = ccToString(container);

                qDebug() << tempStr.c_str();
                generatedStrCcVector.push_back(tempStr);
            }

            std::string generatedFlowsStr = flowsInTimeToString(executor.getFlowsInTime());
            qDebug() << "flows in time:";
            qDebug() << generatedFlowsStr.c_str();

            std::vector<std::string> expectedStrCcVector {
                    "name: cell;arriving connections: 1;leaving connections: 1;functions: 0000000010000;type: 1;workingRanges[4:[650 nm, 650nm],]",
                    "name: waste;arriving connections: 1;leaving connections: 0;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: media;arriving connections: 0;leaving connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
                };
            std::string expectedFlowsStr = "[[{[media,cell,waste,],300 ml/hr},],]";

            qDebug() << "expected:";
            qDebug() << "containers";
            for(const std::string & str : expectedStrCcVector) {
                qDebug() << str.c_str();
            }
            qDebug() << "flows in time:";
            qDebug() << expectedFlowsStr.c_str();

            QVERIFY2(generatedFlowsStr.compare(expectedFlowsStr) == 0, "flows in time are not the same, check debug for more info");

            QVERIFY2(expectedStrCcVector.size() == generatedStrCcVector.size(), "expected and generated container characteristic has not the same size");
            for(int i = 0; i < expectedStrCcVector.size(); i++) {
                QVERIFY2(expectedStrCcVector[i].compare(generatedStrCcVector[i]) == 0,
                         std::string(std::to_string(i) + " position in container characteristics vector is not as expected, check debug for more info").c_str());
            }
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

    std::string functions = container.getNeccesaryFunctionsMask().to_string();
    stream << "functions: " << functions << ";";

    stream << "type: " << container.getType() << ";";
    stream << "workingRanges" << workingRangeToString(container.getWorkingRangeMap());

    return stream.str();
}

std::string ProtocolAnalysisTest::workingRangeToString(const ContainerCharacteristics::WorkingRangeMap & map) {
    std::stringstream stream;

    stream << "[";
    for(const auto & actualPair : map) {
        stream << actualPair.first << ":";
        stream << actualPair.second->toString() << ",";
    }
    stream << "]";
    return stream.str();
}

std::string ProtocolAnalysisTest::flowsInTimeToString(const std::vector<MachineFlowStringAdapter::FlowsVector> & flowInTime) {
    std::stringstream stream;

    stream << "[";
    for(const MachineFlowStringAdapter::FlowsVector & flow: flowInTime) {
        stream << "[";
        for(const auto & flowPair : flow) {
            stream << "{[";
            for(std::string cStep : std::get<0>(flowPair)) {
                stream << cStep << ",";
            }
            stream <<  "],";
            stream << std::get<1>(flowPair).to(units::ml/units::hr) << " ml/hr";
            stream << "},";
        }
        stream << "],";
    }
    stream << "]";
    return stream.str();
}

























QTEST_APPLESS_MAIN(ProtocolAnalysisTest)

#include "tst_protocolanalysistest.moc"

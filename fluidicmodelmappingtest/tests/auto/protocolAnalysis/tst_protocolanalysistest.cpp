#include <QString>
#include <QtTest>
#include <QTemporaryFile>
#include <QFile>

#include <sstream>

#include <bioblocksExecution/bioblocksSimulation/bioblocksrunningsimulator.h>

#include <bioblocksTranslation/bioblockstranslator.h>
#include <bioblocksTranslation/logicblocksmanager.h>

#include <commonmodel/functions/measureodfunction.h>
#include <commonmodel/functions/pumppluginfunction.h>
#include <commonmodel/functions/valvepluginroutefunction.h>
#include <constraintengine/prologtranslationstack.h>

#include <fluidicmachinemodel/fluidicmachinemodel.h>
#include <fluidicmachinemodel/machinegraph.h>

#include <fluidicmodelmapping/heuristic/containercharacteristics.h>
#include <fluidicmodelmapping/protocolAnalysis/analysisexecutor.h>

#include "stringactuatorsinterface.h"

class ProtocolAnalysisTest : public QObject
{
    Q_OBJECT

public:
    ProtocolAnalysisTest();

private:
    void copyResourceFile(const QString & resourcePath, QTemporaryFile* file) throw(std::invalid_argument);

    std::string ccToString(const ContainerCharacteristics & container);
    std::string workingRangeToString(const ContainerCharacteristics::WorkingRangeMap & map);
    std::string flowsInTimeToString(const std::vector<MachineFlowStringAdapter::FlowsVector> & flowInTime);

private Q_SLOTS:
    void switchingFlowsTest();
    void switchingFlows2Test();
    void parallelFlowsTest();
    void workingRangesTest();
    void turbidostatTest();
    void turbidostat2Test();
    void ifColissionTest();
    void ifNormalTest();
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

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(5*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<BioBlocksRunningSimulator> simulator = std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            AnalysisExecutor executor(simulator, 300 * units::ml/units::hr);
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
                    "name: A;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: B;number connections: 3;functions: 0000000000000;type: 1;workingRanges[]",
                    "name: C;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: D;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
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
 * countre = 0;
 * while[0s:x](counter < 10) {
 *  setContinuousflow[x:30min](media1,cell,waste,300ml/hr);
 *  setContinuousflow[x:30min](media2,cell,waste,300ml/hr);
 *  counter = counter + 1;
 * }
 */
void ProtocolAnalysisTest::switchingFlows2Test()
{
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/switchingProtocol2.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::minute, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<BioBlocksRunningSimulator> simulator = std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            AnalysisExecutor executor(simulator, 300 * units::ml/units::hr);
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
                    "name: media1;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: cell;number connections: 3;functions: 0000000000000;type: 1;workingRanges[]",
                    "name: waste;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: media2;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
                };
            std::string expectedFlowsStr = "[[{[media1,cell,waste,],300 ml/hr},],[{[media2,cell,waste,],300 ml/hr},],]";

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

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(5*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<BioBlocksRunningSimulator> simulator = std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            AnalysisExecutor executor(simulator, 300 * units::ml/units::hr);
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
                    "name: A;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: B;number connections: 3;functions: 0000000000000;type: 1;workingRanges[]",
                    "name: C;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: D;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
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

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<BioBlocksRunningSimulator> simulator = std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            AnalysisExecutor executor(simulator, 300 * units::ml/units::hr);
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
                    "name: A;number connections: 0;functions: 0100001010100;type: 2;workingRanges[4:[650 nm, 650nm],2:[26 Cº, 26Cº],6:[25 Hz, 50Hz],11:emission:[680 nm, 680nm]excitation:[650 nm, 650nm],]"
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

/*
 * rate = 300;
 * od = 0;
 * while[0s:-]( od < 600) {
 *  od = measureOD[x:2s](cell,650nm);
 *  rate = rate - (rate *(od - 600));
 *  setContinuosFlow[x:30s]([media,cell,waste], rate ml/hr);
 * }
 * setContinuosFlow[x:30s]([posion,cell,waste], rate ml/hr);
 */
void ProtocolAnalysisTest::turbidostatTest() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/trubidostat.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<BioBlocksRunningSimulator> simulator = std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            AnalysisExecutor executor(simulator, 300 * units::ml/units::hr);
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
                    "name: cell;number connections: 3;functions: 0000000010000;type: 1;workingRanges[4:[650 nm, 650nm],]",
                    "name: waste;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: media;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: poison;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
                };
            std::string expectedFlowsStr = "[[{[media,cell,waste,],300 ml/hr},],[{[poison,cell,waste,],300 ml/hr},],]";

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
 * rate = 300;
 * od = 0;
 * while[0s:-]( od < 600) {
 *  od = measureOD[x:2s](cell,650nm);
 *  rate = rate - (rate *(od - 600));
 *  setContinuosFlow[x:30s]([media,cell,waste], rate ml/hr);
 * }
 */
void ProtocolAnalysisTest::turbidostat2Test() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/trubidostat2.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<BioBlocksRunningSimulator> simulator = std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            AnalysisExecutor executor(simulator, 300 * units::ml/units::hr);
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
                    "name: cell;number connections: 2;functions: 0000000010000;type: 1;workingRanges[4:[650 nm, 650nm],]",
                    "name: waste;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: media;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
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
 * od = measureOD[0s:2s](A, 50Hz, 650nm);
 * if[x:-](od < 600) {
 *  setContinuousFlow[x:10s](B, A, 300 ml/hr);
 * } else {
 *  setContinuousFlow[x:20s](B, C, 300 ml/hr);
 * }
 * setContinuosFlow[15s:10s](D, E, 300ml/hr);
 */
void ProtocolAnalysisTest::ifColissionTest() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/ifColission.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<BioBlocksRunningSimulator> simulator = std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            AnalysisExecutor executor(simulator, 300 * units::ml/units::hr);
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
                    "name: A;number connections: 1;functions: 0000000010000;type: 0;workingRanges[4:[650 nm, 650nm],]",
                    "name: B;number connections: 2;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: C;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: D;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: E;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
                };
            std::string expectedFlowsStr = "[[{[B,A,],300 ml/hr},],[{[B,C,],300 ml/hr},],[{[B,C,],300 ml/hr},{[D,E,],300 ml/hr},],[{[D,E,],300 ml/hr},],]";

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
 * od = measureOD[0s:2s](50Hz, 650nm);
 * if[x:-](od < 600) {
 *  setContinuousFlow[x:10s](B, A, 300 ml/hr);
 * } else {
 *  setContinuousFlow[x:20s](B, A, 300 ml/hr);
 * }
 */
void ProtocolAnalysisTest::ifNormalTest() {
    QTemporaryFile* tempFile = new QTemporaryFile();
    if (tempFile->open()) {
        try {
            copyResourceFile(":/protocol/protocolos/ifNormal.json", tempFile);

            std::shared_ptr<LogicBlocksManager> logicBlocks = std::make_shared<LogicBlocksManager>();
            BioBlocksTranslator translator(1*units::s, tempFile->fileName().toStdString());
            std::shared_ptr<ProtocolGraph> protocol = translator.translateFile(logicBlocks);

            qDebug() << protocol->toString().c_str();

            std::shared_ptr<BioBlocksRunningSimulator> simulator = std::make_shared<BioBlocksRunningSimulator>(protocol, logicBlocks);

            AnalysisExecutor executor(simulator, 300 * units::ml/units::hr);
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
                    "name: A;number connections: 1;functions: 0000000010000;type: 0;workingRanges[4:[650 nm, 650nm],]",
                    "name: B;number connections: 2;functions: 0000000000000;type: 0;workingRanges[]",
                    "name: C;number connections: 1;functions: 0000000000000;type: 0;workingRanges[]"
                };
            std::string expectedFlowsStr = "[[{[B,A,],300 ml/hr},],[{[B,C,],300 ml/hr},],]";

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
    stream << "number connections: " << container.getNumberConnections() << ";";

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

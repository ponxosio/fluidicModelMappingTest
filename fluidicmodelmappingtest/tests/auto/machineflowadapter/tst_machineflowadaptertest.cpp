#include <QString>
#include <QtTest>

#include <utils/machineflowstringadapter.h>

class MachineflowadapterTest : public QObject
{
    Q_OBJECT

public:
    MachineflowadapterTest();

private Q_SLOTS:
    void staticMethods();
    void flowsTest();
};

MachineflowadapterTest::MachineflowadapterTest()
{
}

void MachineflowadapterTest::staticMethods()
{
    MachineFlowStringAdapter::PathRateTuple t1 = std::make_tuple(std::deque<string>{"1","2","3"}, 300 * units::ml / units::hr);
    MachineFlowStringAdapter::PathRateTuple t2 = std::make_tuple(std::deque<string>{"1","4","3"}, 200 * units::ml / units::hr);
    MachineFlowStringAdapter::PathRateTuple t3 = std::make_tuple(std::deque<string>{"4","5","6"}, 100 * units::ml / units::hr);

    MachineFlowStringAdapter::FlowsVector v1 {t1,t2,t3};
    MachineFlowStringAdapter::FlowsVector v2 {t2,t1,t3};
    MachineFlowStringAdapter::FlowsVector v3 {t1};

    QVERIFY2(MachineFlowStringAdapter::flowsVectorEquals(v1,v2), "v1 and v2 vector are not equals");
    QVERIFY2(!MachineFlowStringAdapter::flowsVectorEquals(v1,v3), "v1 and v3 vector are equals");
}

void MachineflowadapterTest::flowsTest() {
    MachineFlowStringAdapter mfsa;

    mfsa.addFlow("4","5", 300 * units::ml/units::hr);
    mfsa.addFlow("4","5", 300 * units::ml/units::hr);
    mfsa.addFlow("1","2", 300 * units::ml/units::hr);
    mfsa.addFlow("2","4", 300 * units::ml/units::hr);
    mfsa.addFlow("1","3", 300 * units::ml/units::hr);
    mfsa.addFlow("3","4", 300 * units::ml/units::hr);

    mfsa.updateFlows();

    std::string expected = "[[1245:300ml/hr],[1345:300ml/hr],]";
    std::string calculated = mfsa.flowToStr();

    qDebug() << "expected:" << expected.c_str();
    qDebug() << "calculated:" << calculated.c_str();
    QVERIFY2(expected.compare(calculated) == 0, "flows is not as expected check debug info");
}

QTEST_APPLESS_MAIN(MachineflowadapterTest)

#include "tst_machineflowadaptertest.moc"

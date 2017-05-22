#include "stringactuatorsinterface.h"

StringActuatorsInterface::StringActuatorsInterface(const std::vector<double> & measureValues) :
    measureValues(measureValues)
{

}

StringActuatorsInterface::~StringActuatorsInterface()
{

}

void StringActuatorsInterface::applyLigth(const std::string & sourceId, units::Length wavelength, units::LuminousIntensity intensity) {
    stream << "applyLight(" << sourceId << "," << wavelength.to(units::nm) << "nm," << intensity.to(units::cd) << "cd);";
}

void StringActuatorsInterface::stopApplyLigth(const std::string & sourceId) {
    stream << "stopApplyLight(" << sourceId << ");";
}

void StringActuatorsInterface::applyTemperature(const std::string & sourceId, units::Temperature temperature) {
    stream << "applyTemperature(" << sourceId << "," << temperature.to(units::C) << "Cº);";
}

void StringActuatorsInterface::stopApplyTemperature(const std::string & sourceId) {
    stream << "stopApplyTemperature(" << sourceId << ");" ;
}

void StringActuatorsInterface::stir(const std::string & idSource, units::Frequency intensity) {
    stream << "stir(" << idSource << "," << intensity.to(units::Hz) << "Hz);";
}

void StringActuatorsInterface::stopStir(const std::string & idSource) {
    stream << "stopStir(" << idSource << ");";
}

void StringActuatorsInterface::centrifugate(const std::string & idSource, units::Frequency intensity) {
    stream << "centrifugate(" << idSource << "," << intensity.to(units::Hz) << "Hz);";
}

void StringActuatorsInterface::stopCentrifugate(const std::string & idSource) {
    stream << "stopCentrifugate(" << idSource << ");";
}

void StringActuatorsInterface::shake(const std::string & idSource, units::Frequency intensity) {
    stream << "shake(" << idSource << "," << intensity.to(units::Hz) << "Hz);";
}

void StringActuatorsInterface::stopShake(const std::string & idSource) {
    stream << "stopShake(" << idSource << ");";
}

void StringActuatorsInterface::startElectrophoresis(const std::string & idSource, units::ElectricField fieldStrenght) {
    stream << "startElectrophoresis(" << idSource << "," << fieldStrenght.to(units::V / units::cm) << "V/cm);";
}

std::shared_ptr<ElectrophoresisResult> StringActuatorsInterface::stopElectrophoresis(const std::string & idSource) {
    stream << "stopElectrophoresis(" << idSource << ");";
    return std::make_shared<ElectrophoresisResult>();
}

units::Volume StringActuatorsInterface::getVirtualVolume(const std::string & sourceId) {
    stream << "getVirtualVolume(" << sourceId << ");";
    return -1*units::l;
}

void StringActuatorsInterface::loadContainer(const std::string & sourceId, units::Volume initialVolume) {
    stream << "loadContainer(" << sourceId << "," << initialVolume.to(units::ml) << "ml);";
}

void StringActuatorsInterface::startMeasureOD(
        const std::string & sourceId,
        units::Frequency measurementFrequency,
        units::Length wavelength)
{
    stream << "measureOD(" << sourceId << "," << measurementFrequency.to(units::Hz) << "Hz,"
           << wavelength.to(units::nm) << "nm);";
}

double StringActuatorsInterface::getMeasureOD(const std::string & sourceId) {
    stream << "getMeasureOD(" << sourceId << ");";
    return getNextReadValue();
}

void StringActuatorsInterface::startMeasureTemperature(
        const std::string & sourceId,
        units::Frequency measurementFrequency)
{
    stream << "measureTemperature(" << sourceId << "," << measurementFrequency.to(units::Hz) << "Hz);";
}

units::Temperature StringActuatorsInterface::getMeasureTemperature(const std::string & sourceId) {
    stream << "getMeasureTemperature(" << sourceId << ");";
    return getNextReadValue()*units::C;
}

void StringActuatorsInterface::startMeasureLuminiscense(
        const std::string & sourceId,
        units::Frequency measurementFrequency)
{
    stream << "measureLuminiscense(" << sourceId << ","  << measurementFrequency.to(units::Hz) << "Hz);";
}

units::LuminousIntensity StringActuatorsInterface::getMeasureLuminiscense(const std::string & sourceId) {
    stream << "getMeasureLuminiscense(" << sourceId << ");";
    return getNextReadValue()*units::cd;
}

void StringActuatorsInterface::startMeasureVolume(
        const std::string & sourceId,
        units::Frequency measurementFrequency)
{
    stream << "measureVolume(" << sourceId << ","  << measurementFrequency.to(units::Hz) << "Hz);";
}

units::Volume StringActuatorsInterface::getMeasureVolume(const std::string & sourceId) {
    stream << "getMeasureVolume(" << sourceId << ");";
    return getNextReadValue()*units::ml;
}

void StringActuatorsInterface::startMeasureFluorescence(
        const std::string & sourceId,
        units::Frequency measurementFrequency,
        units::Length excitation,
        units::Length emission)
{
    stream << "measureFluorescence(" << sourceId << "," << measurementFrequency.to(units::Hz) << "Hz,"
           << excitation.to(units::nm) << "nm, " << emission.to(units::nm) << "nm);";
}

units::LuminousIntensity StringActuatorsInterface::getMeasureFluorescence(const std::string & sourceId) {
    stream << "getMeasureFluorescence(" << sourceId << ");";
    return getNextReadValue()*units::cd;
}

units::Time StringActuatorsInterface::mix(
        const std::string & idSource1,
        const std::string & idSource2,
        const std::string & idTarget,
        units::Volume volume1,
        units::Volume volume2)
{
    stream << "mix(" << idSource1 << "," << idSource2 << "," << idTarget << "," << volume1.to(units::ml) << "ml," << volume2.to(units::ml) << ");";
    return (volume1.to(units::ml) * units::s + volume2.to(units::ml) * units::s);
}

void StringActuatorsInterface::stopMix(
        const std::string & idSource1,
        const std::string & idSource2,
        const std::string & idTarget)
{
    stream << "stopMix(" << idSource1 << "," << idSource2 << "," << idTarget << ");";
}

void StringActuatorsInterface::setContinuosFlow(
        const std::string & idSource,
        const std::string & idTarget,
        units::Volumetric_Flow rate)
{
    stream << "setContinuosFlow(" << idSource << "," << idTarget << "," << rate.to(units::ml/units::hr) << "ml/h" << ");";
}

void StringActuatorsInterface::stopContinuosFlow(const std::string & idSource, const std::string & idTarget)
{
    stream << "stopContinuosFlow(" << idSource << "," << idTarget << ");";
}

units::Time StringActuatorsInterface::transfer(
        const std::string & idSource,
        const std::string & idTarget,
        units::Volume volume)
{
    stream << "transfer(" << idSource << "," << idTarget << "," << volume.to(units::ml) << "ml" << ");";
    return (volume.to(units::ml) * units::s);
}

void StringActuatorsInterface::stopTransfer(const std::string & idSource, const std::string & idTarget) {
    stream << "stopTransfer(" << idSource << "," << idTarget << ");";
}

void StringActuatorsInterface::setTimeStep(units::Time time) {
    stream << "setTimeStep(" << time.to(units::ms) << "ms" << ");";
    timeSlice = time;
}

units::Time StringActuatorsInterface::timeStep() {
    stream << "timeStep();";
    return timeSlice;
}

double StringActuatorsInterface::getNextReadValue() {
    if (actualValueSerie.getValue() >= measureValues.size()) {
        actualValueSerie.reset();
    }
    int nextValue = actualValueSerie.getNextValue();
    return measureValues[nextValue];
}

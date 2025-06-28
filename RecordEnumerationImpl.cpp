#include "RecordEnumerationImpl.h"
// Пустые реализации, просто чтобы все работало
RecordEnumerationImpl::RecordEnumerationImpl(std::vector<std::vector<int8_t>> data) {}
RecordEnumerationImpl::RecordEnumerationImpl() {}
RecordEnumerationImpl::~RecordEnumerationImpl() {}
int RecordEnumerationImpl::getNumberOfRecords() { return 0; }
std::vector<int8_t> RecordEnumerationImpl::getNextRecord() { return {}; }
int RecordEnumerationImpl::addRecord(std::vector<int8_t> bytes) { return 0; }
void RecordEnumerationImpl::setRecord(int index, std::vector<int8_t> bytes) {}
void RecordEnumerationImpl::reset() {}
int RecordEnumerationImpl::getNextRecordId() { return 0; }
void RecordEnumerationImpl::destroy() {}
void RecordEnumerationImpl::serialize(FileStream* outStream) {}
void RecordEnumerationImpl::deserialize(FileStream* inStream) {}
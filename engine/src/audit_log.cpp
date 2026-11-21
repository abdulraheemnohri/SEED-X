#include "seed/audit_log.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <utility>
namespace seed {
AuditLog::AuditLog(std::size_t capacity):capacity_(capacity?capacity:1){}
std::string AuditLog::calculateHash(const AuditEntry&e){
 // FNV-1a is an integrity checksum, not a cryptographic signature.
 std::ostringstream data;data<<e.sequence<<'|'<<e.timestampMs<<'|'<<e.tool.size()<<':'<<e.tool<<'|'<<e.operation.size()<<':'<<e.operation<<'|'<<e.resource.size()<<':'<<e.resource<<'|'<<static_cast<unsigned>(e.decision)<<'|'<<e.reason.size()<<':'<<e.reason<<'|'<<e.previousHash;
 const auto value=data.str();std::uint64_t h=14695981039346656037ULL;for(const unsigned char c:value){h^=c;h*=1099511628211ULL;}
 std::ostringstream out;out<<std::hex<<std::setfill('0')<<std::setw(16)<<h;return out.str();
}
bool AuditLog::append(const ToolRequest&r,const PolicyResult&p){
 std::lock_guard<std::mutex> lock(mutex_);if(entries_.size()>=capacity_)return false;AuditEntry e;
 e.sequence=entries_.empty()?1:entries_.back().sequence+1;
 e.timestampMs=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
 e.tool=r.tool;e.operation=r.operation;e.resource=r.resource;e.decision=p.decision;e.reason=p.explanation;e.previousHash=entries_.empty()?"GENESIS":entries_.back().hash;e.hash=calculateHash(e);entries_.push_back(std::move(e));return true;
}
bool AuditLog::verify()const{std::lock_guard<std::mutex> lock(mutex_);std::string previous="GENESIS";std::uint64_t seq=1;for(const auto&e:entries_){if(e.sequence!=seq++||e.previousHash!=previous||e.hash!=calculateHash(e))return false;previous=e.hash;}return true;}
std::vector<AuditEntry> AuditLog::snapshot()const{std::lock_guard<std::mutex> lock(mutex_);return entries_;}
std::size_t AuditLog::size()const{std::lock_guard<std::mutex> lock(mutex_);return entries_.size();}
}

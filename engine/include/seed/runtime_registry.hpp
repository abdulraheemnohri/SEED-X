#pragma once
#include "seed/local_ai.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace seed {
enum class RuntimeAvailability { Available, OptionalBuildDisabled, AdapterMissing, RemoteService };
struct RuntimeDescriptor {
 std::string id;
 std::string displayName;
 std::string category;
 std::vector<ModelFormat> formats;
 RuntimeAvailability availability{RuntimeAvailability::AdapterMissing};
 bool embedded{true};
 std::string detail;
};
using RuntimeFactory=std::function<std::unique_ptr<InferenceBackend>()>;
struct RuntimeRegistration {RuntimeDescriptor descriptor;RuntimeFactory factory;};
class RuntimeRegistry final {
public:
 bool add(RuntimeDescriptor descriptor,RuntimeFactory factory={});
 const RuntimeRegistration* find(const std::string& id)const noexcept;
 std::vector<RuntimeDescriptor> snapshot()const;
 std::unique_ptr<InferenceBackend> create(const std::string& id)const;
 bool canRun(const std::string& id)const noexcept;
 static RuntimeRegistry builtins();
private:std::vector<RuntimeRegistration> entries_;
};
}

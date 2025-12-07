#include "Data/Validators/ReferenceValidator.h"
#include <unordered_set>

namespace New::Data::Validators {

namespace {
void AddIssue(std::vector<ValidationIssue> &issues, Severity severity,
              const std::string &path, const std::string &message) {
  issues.push_back({severity, path, message});
}
} // namespace

ValidationReport
ReferenceValidator::ValidateStateClipConsistency(const nlohmann::json &states,
                                                 const nlohmann::json &clips) {
  std::vector<ValidationIssue> issues;

  if (!states.is_array()) {
    AddIssue(issues, Severity::Error, "states", "states must be an array");
    return {issues};
  }

  std::unordered_set<std::string> clipIds;
  if (clips.is_array()) {
    for (const auto &clip : clips) {
      if (clip.contains("id") && clip.at("id").is_string()) {
        clipIds.insert(clip.at("id").get<std::string>());
      }
    }
  }

  std::unordered_set<std::string> stateIds;
  for (size_t i = 0; i < states.size(); ++i) {
    const auto &state = states.at(i);
    const std::string path = "states[" + std::to_string(i) + "]";

    if (!state.contains("id") || !state.at("id").is_string()) {
      AddIssue(issues, Severity::Error, path, "state id is missing or invalid");
      continue;
    }

    const std::string id = state.at("id").get<std::string>();
    if (!stateIds.insert(id).second) {
      AddIssue(issues, Severity::Error, path,
               "duplicate state id '" + id + "'");
    }

    if (!state.contains("clipId")) {
      AddIssue(issues, Severity::Warning, path, "clipId is not assigned");
      continue;
    }

    if (!state.at("clipId").is_string()) {
      AddIssue(issues, Severity::Error, path, "clipId must be a string");
      continue;
    }

    const std::string clipId = state.at("clipId").get<std::string>();
    if (clipId.empty()) {
      AddIssue(issues, Severity::Warning, path, "clipId is empty");
    } else if (!clipIds.empty() && clipIds.count(clipId) == 0) {
      AddIssue(issues, Severity::Warning, path,
               "clipId '" + clipId + "' is not defined");
    }
  }

  return {issues};
}

ValidationReport ReferenceValidator::ValidateEventPayloads(
    const nlohmann::json &timeline,
    const std::unordered_map<std::string, std::vector<std::string>>
        &requiredFieldsByEvent) {
  std::vector<ValidationIssue> issues;

  if (!timeline.is_array()) {
    AddIssue(issues, Severity::Error, "timeline", "timeline must be an array");
    return {issues};
  }

  for (size_t i = 0; i < timeline.size(); ++i) {
    const auto &ev = timeline.at(i);
    const std::string path = "timeline[" + std::to_string(i) + "]";
    if (!ev.contains("type") || !ev.at("type").is_string()) {
      AddIssue(issues, Severity::Error, path, "event type is missing");
      continue;
    }
    const std::string type = ev.at("type").get<std::string>();

    auto requiredFieldsIt = requiredFieldsByEvent.find(type);
    if (requiredFieldsIt == requiredFieldsByEvent.end()) {
      continue; // unknown event types are ignored here; SchemaValidator handles
                // type
    }

    if (!ev.contains("payload") || !ev.at("payload").is_object()) {
      AddIssue(issues, Severity::Error, path,
               "payload must be an object for event '" + type + "'");
      continue;
    }

    const auto &payload = ev.at("payload");
    for (const auto &field : requiredFieldsIt->second) {
      if (!payload.contains(field)) {
        AddIssue(issues, Severity::Error, path,
                 "missing payload field '" + field + "' for event '" + type +
                     "'");
      }
    }
  }

  return {issues};
}

ValidationReport
ReferenceValidator::ValidateTimelineScopes(const nlohmann::json &timeline) {
  std::vector<ValidationIssue> issues;

  if (!timeline.is_array()) {
    AddIssue(issues, Severity::Error, "timeline", "timeline must be an array");
    return {issues};
  }

  for (size_t i = 0; i < timeline.size(); ++i) {
    const auto &ev = timeline.at(i);
    const std::string path = "timeline[" + std::to_string(i) + "]";
    if (!ev.contains("scope")) {
      AddIssue(issues, Severity::Warning, path,
               "scope is missing (expected 'clip' or 'state')");
      continue;
    }
    if (!ev.at("scope").is_string()) {
      AddIssue(issues, Severity::Error, path, "scope must be a string");
      continue;
    }
    const auto scope = ev.at("scope").get<std::string>();
    if (scope != "clip" && scope != "state") {
      AddIssue(issues, Severity::Error, path,
               "scope must be either 'clip' or 'state'");
    }
  }

  return {issues};
}

ValidationReport ReferenceValidator::ValidateHudBindings(
    const nlohmann::json &layout, const std::vector<std::string> &required) {
  std::vector<ValidationIssue> issues;

  if (!layout.is_object()) {
    AddIssue(issues, Severity::Error, "layout", "layout must be an object");
    return {issues};
  }

  if (!layout.contains("bindings")) {
    AddIssue(issues, Severity::Warning, "layout.bindings",
             "bindings section is missing");
    return {issues};
  }

  const auto &bindings = layout.at("bindings");
  if (!bindings.is_object()) {
    AddIssue(issues, Severity::Error, "layout.bindings",
             "bindings must be an object");
    return {issues};
  }

  for (const auto &key : required) {
    if (!bindings.contains(key)) {
      AddIssue(issues, Severity::Warning, "layout.bindings." + key,
               "binding missing; use default fallback");
    }
  }

  return {issues};
}

} // namespace New::Data::Validators

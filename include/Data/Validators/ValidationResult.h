#pragma once

#include <string>
#include <vector>

namespace New::Data::Validators {

enum class Severity { Warning, Error };

struct ValidationIssue {
  Severity severity = Severity::Error;
  std::string path;
  std::string message;
};

struct ValidationReport {
  std::vector<ValidationIssue> issues;

  bool HasErrors() const {
    for (const auto &issue : issues) {
      if (issue.severity == Severity::Error) {
        return true;
      }
    }
    return false;
  }

  bool HasWarnings() const {
    for (const auto &issue : issues) {
      if (issue.severity == Severity::Warning) {
        return true;
      }
    }
    return false;
  }
};

} // namespace New::Data::Validators

#pragma once
#include <vector>
#include <string>
#include "record.h"
#include "analysis_result.h"
#include "i18n.h"

// 复杂分析主入口
AnalysisResult complex_analysis(const std::vector<Record>& records, const I18N& i18n);

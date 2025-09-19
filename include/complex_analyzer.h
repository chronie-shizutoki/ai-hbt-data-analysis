#pragma once
#include <vector>
#include <string>
#include "record.h"
#include "i18n.h"
#include "apriori.h"
#include "sentiment_analyzer.h"
#include "anomaly_detector.h"
#include "cluster_analyzer.h"
#include "analysis_result.h"

// 复杂分析主入口
AnalysisResult complex_analysis(const std::vector<Record>& records, const I18N& i18n);



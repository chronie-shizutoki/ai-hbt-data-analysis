
# ai-hbt-data-analysis

English documentation. [中文说明请点此查看 (Chinese version)](./README.zh_CN.md)

An advanced household expense analysis system featuring a high-performance C++ backend, intelligent Python analytics, and a hybrid Web/Flutter frontend. The system supports multi-dimensional clustering, anomaly detection, time series forecasting, association rule mining, user profiling, sentiment analysis, and interactive visualization. It is designed for modularity, multi-language support, standard JSON interfaces, extensibility, and compliance.

---

## System Architecture

```
┌────────────┐      ┌──────────────┐      ┌────────────┐
│  Flutter   │◀───▶│ RESTful API  │◀───▶│   C++ Core  │
│/Web Front  │      │ (JSON API)   │      │ (Analysis) │
└────────────┘      └──────────────┘      └─────┬──────┘
                                               │
                                        ┌──────▼─────┐
                                        │ Python AI  │
                                        │ Analytics  │
                                        └────────────┘
```

### Module Responsibilities & Tech Stack
- **C++ Backend**: High-performance basic clustering, anomaly detection, statistics, data export, i18n, standard JSON output.
- **Python Analytics**: Advanced clustering (KMeans/DBSCAN), time series forecasting (Prophet/LSTM/ARIMA), association rules (Apriori/FP-Growth), advanced NLP/sentiment, multi-dimensional profiling.
- **Frontend (Flutter/Web)**: Interactive visualization (ECharts/Plotly), report display, user-defined analytics, data drill-down.

**Recommended Stack:**
- C++17/20, nlohmann/json, STL, GoogleTest
- Python 3.8+, pandas, scikit-learn, prophet, transformers, mlxtend, etc.
- Flutter 3.x, Dart, ECharts/Plotly, RESTful API

---

## Key Features
- Multi-dimensional clustering, anomaly detection, user profiling, time series stats & forecasting, sentiment analysis, association rule mining
- Multi-language support (dynamic loading, currently Simplified Chinese/English, extensible)
- Standard JSON interface for easy frontend/multi-platform integration
- High-performance parallel/distributed analytics for large data (optional Spark/distributed)
- Extensible plugin/module mechanism
- Compliance: data masking, access control, audit

---

## Typical Usage
### 1. C++ Backend Analysis
```bash
make
./expense_analyzer expenses_initial.csv -o analysis.json --lang en_US
```
Supports CLI args: input CSV, output JSON/text, language, analysis type, etc.

### 2. Python Smart Analytics
1. Export standard data from C++ (CSV/JSON)
2. Python scripts for forecasting/clustering/NLP, etc.
3. Output results as JSON for C++/frontend integration

### 3. Frontend Visualization
- Fetch analysis results via RESTful API, visualize with ECharts/Plotly
- Support for Flutter desktop/mobile/web

---

## API & Data Interface
- C++ backend outputs standard JSON (see `analysis.json` sample)
- Python analytics must be JSON-compatible for integration
- Frontend fetches via RESTful API, supports multi-language/themes

---

## Development & Deployment
1. C++: Build with `make`, main binary `expense_analyzer`, cross-platform
2. Python: Use venv, dependencies in `requirements.txt` (to be created)
3. Frontend: Flutter 3.x, see frontend subproject
4. CI/CD: Makefile/CMake + GitHub Actions (reserved)

---

## Progress & Completed Work
- [x] Multi-language resource files (zh_CN/en_US) & dynamic I18N loader
- [x] nlohmann/json integration reserved
- [x] Project structure & README improved
- [x] Task list & user review suggestions integrated
- [x] Fixed csv_parser.cpp full-width bracket char bug, now handles UTF-8
- [x] analysis_result.cpp to_json trims/validates all string fields, auto-replaces invalid UTF-8
- [x] Compiles & runs, analysis.json outputs valid UTF-8
- [x] All analysis.json fields valid UTF-8, no errors
- [ ] Backend JSON output & i18n integration (in progress)
- [ ] Advanced analytics & performance optimization (in progress)
- [ ] Flutter frontend API & CI reserved (todo)

---

## TODO & Roadmap
See `todo.md` for detailed division, priorities, tech choices, recent fixes, and review notes.

---

## Contribution & Extension
- Contributions welcome: new analytics, frontend modules, languages, etc.
- Plugin/extension mechanism for future expansion

---

## Compliance & Security
- Data masking, access control, compliance checks (see `todo.md`)

---

## Contact & Support
- Use issues or email to contact maintainers

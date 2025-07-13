# ai-hbt-data-analysis

一个面向家庭消费账本的高级消费分析系统，采用 C++ 高性能后端、Web/Flutter 前端混合架构，支持多维聚类、异常检测、时序预测、关联规则、用户画像、情感分析、可视化等功能，具备多语言、模块化、标准 JSON 接口、可扩展性和合规性。所有分析与智能算法均用C++实现。

---

## 系统架构

 ```mermaid
graph LR
    A[Flutter/Web 前端] -- JSON API --> B[RESTful API]
    B -- JSON API --> C[C++ 高性能分析]
    C -- JSON API --> B
    B -- JSON API --> A
 ```

### 分工与技术选型
- **C++后端**：高性能聚类、异常检测、统计、时序预测、关联规则、用户画像、情感分析、数据导出、国际化、JSON标准输出，所有分析与智能算法均用C++实现。
- **前端（Flutter/Web）**：交互式可视化（ECharts/Plotly）、报表展示、用户自定义分析、数据钻取。

### 推荐技术栈
- C++17/20，nlohmann/json，标准库，GoogleTest，OpenCV/mlpack/dlib/Eigen/Armadillo（按需）
- Flutter 3.x，Dart，ECharts/Plotly，RESTful API

---

## 主要功能
- 多维消费聚类、异常检测、用户画像、时序统计与预测、情感分析、关联规则挖掘
- 多语言支持（动态加载，已支持简体中文/英语，预留多语言扩展）
- 标准JSON接口，便于前端/多平台集成
- 支持大数据量下的高性能并行/分布式分析（可选Spark/分布式）
- 可扩展的插件/模块机制
- 合规性设计（数据脱敏、权限控制、合规检查）

---

## 典型用法
### 1. C++后端分析
```bash
make
./expense_analyzer expenses_initial.csv -o analysis.json --lang zh_CN
```
支持命令行参数：输入CSV、输出JSON/文本、选择语言、分析类型等。

### 2. 前端可视化
- 通过RESTful API获取分析结果，支持ECharts/Plotly等可视化库
- 支持Flutter桌面/移动/Web多端展示

---

## 接口规范
- C++后端输出标准JSON，字段含义详见 `analysis.json` 示例
- 前端通过RESTful接口获取数据，支持多语言/多主题

---

## 开发与部署
1. C++：`make` 编译，主程序 `expense_analyzer`，支持多平台
2. 前端：Flutter 3.x，详见前端子项目说明
3. CI/CD：Makefile/CMake + GitHub Actions（持续集成预留）

---

## 进度与已完成工作
- [x] 多语言资源文件（zh_CN/en_US）与动态加载类I18N实现
- [x] 预留nlohmann/json集成位置
- [x] 项目结构与README文档完善
- [x] 任务清单与用户审核建议集成
- [x] 修复 csv_parser.cpp 全角括号导致的 char 类型错误，改为字符串查找并处理 UTF-8 字节长度。
- [x] analysis_result.cpp 的 to_json 方法对所有 string 字段（含 map key、vector 元素）做 trim 和 UTF-8 校验，发现非法内容自动替换并输出调试信息。
- [x] 编译并运行，分析结果 analysis.json 已正常输出，无非法 UTF-8 报错。
- [x] 检查 analysis.json，所有字段均为有效 UTF-8，无异常替换内容。
- [ ] 后端分析JSON输出与多语言集成（进行中）
- [ ] 复杂分析算法开发与性能优化（进行中）
- [ ] Flutter前端集成接口与CI预留（待办）

---

## TODO与路线图
详见 `todo.md`，包含详细分工、优先级、技术选型、近期修复与审核建议。

---

## 贡献与扩展
- 欢迎贡献新分析算法、前端可视化模块、多语言资源等
- 支持插件/扩展机制，便于后续功能拓展

---

## 合规与安全
- 数据脱敏、权限控制、合规检查，详见 `todo.md` 及后续文档

---

## 联系与支持
- issue区或邮件联系项目维护者

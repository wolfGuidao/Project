### 环境 

python3

### 执行步骤

- 运行jobInfo.py获取数据

  输出数据在 `save-data/招聘_关键词_C++_城市_上海.csv`

- 运行analysis.py分析数据

  如下这行代码对应配置csv文件名，修改成上一步输出的文件名

   `df1 = pd.read_csv('save-data/招聘_关键词_C++_城市_上海.csv')`
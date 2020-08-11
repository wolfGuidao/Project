#!/usr/bin/env python
# coding: utf-8

# 数据处理可视化

import numpy as np
import pandas as pd 
import matplotlib.pyplot as plt

import jieba
import jieba.analyse
import re
import xlrd

# 读入数据
df1 = pd.read_csv('save-data/招聘_关键词_C++_城市_上海.csv')

# 合并数据
df_all = pd.concat([df1])

# 处理字段
df_all.rename({'职位名称': 'position'}, axis=1, inplace=True) #axis=1代表index; axis=0代表column
df_all.rename({'详细链接': 'url'}, axis=1, inplace=True)
df_all.rename({'工作地点': 'region'}, axis=1, inplace=True)
df_all.rename({'薪资': 'salary'}, axis=1, inplace=True)
df_all.rename({'公司名称': 'company'}, axis=1, inplace=True)
df_all.rename({'经验要求': 'experience'}, axis=1, inplace=True)
df_all.rename({'学历': 'edu'}, axis=1, inplace=True)
df_all.rename({'福利': 'welfare'}, axis=1, inplace=True)
df_all.rename({'职位信息': 'detail'}, axis=1, inplace=True)

# 去除重复值
df_all.drop_duplicates(inplace=True)

# 重置索引
df_all.index = range(df_all.shape[0]) 
print(df_all.shape)
print(df_all.head(2))

# 区域频率分布
region = df_all.region.value_counts()
print(region)

# pyecharts通用配置
from pyecharts import options as opts

# 区域分布条形图
from pyecharts.charts import Bar
regBar = Bar(init_opts=opts.InitOpts(width='1350px', height='750px'))
regBar.add_xaxis(region.index.tolist())
regBar.add_yaxis("区域", region.values.tolist())
regBar.set_global_opts(title_opts=opts.TitleOpts(title="工作区域分布"),
                     toolbox_opts=opts.ToolboxOpts(),
                     visualmap_opts=opts.VisualMapOpts())


# 薪资分布范围图
salary = df_all.salary.str.replace('k','').str.split('-')
#salary_high = df_all.salary.str.replace('k','').str.split('-')[1]
print(salary)
low_list = []
high_list = []
for i in salary:
    low_list.append(i[0])
    high_list.append(i[1])

print(low_list)
print(high_list)

#薪酬区间Top10分布计算
df_all['sala_low'] = low_list
df_all['sala_high'] = high_list
sala_low = df_all.sala_low.value_counts()[:10]
sala_high = df_all.sala_high.value_counts()[:10]
print(sala_low)
print(sala_high)

# 薪酬最高最低分布-条形图
from pyecharts.globals import ThemeType
slBar = Bar({"theme": ThemeType.MACARONS})
slBar.add_xaxis(sala_low.index.tolist())
slBar.add_yaxis("区域", sala_low.values.tolist())
slBar.set_global_opts(title_opts=opts.TitleOpts(title="最低薪资范围分布"),toolbox_opts=opts.ToolboxOpts())

from pyecharts.commons.utils import JsCode
shBar = Bar(init_opts=opts.InitOpts(width='1350px', height='750px'))
shBar.add_xaxis(sala_high.index.tolist())
shBar.add_yaxis("区域", sala_high.values.tolist())
shBar.set_series_opts(itemstyle_opts={
            "normal": {
                "color": JsCode("""new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
                    offset: 0,
                    color: 'rgba(0, 244, 255, 1)'
                }, {
                    offset: 1,
                    color: 'rgba(0, 77, 167, 1)'
                }], false)"""),
                "barBorderRadius": [30, 30, 30, 30],
                "shadowColor": 'rgb(0, 160, 221)',
            }})
shBar.set_global_opts(title_opts=opts.TitleOpts(title="最高薪资范围分布"), toolbox_opts=opts.ToolboxOpts())

# 学历分布
edu_per = df_all.edu.value_counts() / df_all.edu.value_counts().sum()
edu_per = np.round(edu_per*100, 2)
print(df_all.edu.value_counts().sum())
print(edu_per)

from pyecharts.charts import Pie
eduPie = Pie(init_opts=opts.InitOpts(width='1350px', height='750px'))
eduPie.add("",
         [list(z) for z in zip(edu_per.index, edu_per.values)],
         radius=["40%","75%"]) #设置图形大小
eduPie.set_global_opts(title_opts=opts.TitleOpts(title='学历分布图'),
                     legend_opts=opts.LegendOpts(orient="vertical", pos_top="15%", pos_left="2%"),
                     toolbox_opts=opts.ToolboxOpts())
eduPie.set_series_opts(label_opts=opts.LabelOpts(formatter="{b}:{c}%")) #设置展示样式

# 生成职位信息词云
def get_comment_word(df):
    # 集合形式存储-去重
    stop_words = {'C++', 'c++', '具有', '掌握', '熟悉', '开发', '优先', '编写', '以上', '能力', '经验', '任职', '要求'
                  , '熟练掌握', '常用', '功能', '熟练', '参与', '软件', '完成', '职位', '模块', '扎实', '岗位职责',
                  '以上学历','本科', '负责', '具备', '工作', '了解', 'BIGO', '良好', '相关', '使用', '热爱', '精通'
                  , '岗位', '环境', '技术', '语言'}

    # 合并职位信息
    df_comment_all = df['detail'].str.cat()

    # 使用TF-IDF算法提取关键词
    word_num = jieba.analyse.extract_tags(df_comment_all, topK=100, withWeight=True, allowPOS=())

    # 筛选掉停用词
    word_num_selected = []
    for i in word_num:
        if i[0] not in stop_words:
            word_num_selected.append(i)
        else:
            pass
    return word_num_selected

key_words = get_comment_word(df_all)
key_words = pd.DataFrame(key_words, columns=['words', 'num'])
print(key_words.head())

from pyecharts.charts import WordCloud
from pyecharts.globals import SymbolType, ThemeType

word = WordCloud(init_opts=opts.InitOpts(width='1350px', height='750px'))
word.add("", [*zip(key_words.words, key_words.num)],
         word_size_range=[20, 200], shape='diamond')
word.set_global_opts(title_opts=opts.TitleOpts(title="职位需求关键词云图"),
                     toolbox_opts=opts.ToolboxOpts())

# 生成图表页面
from pyecharts.charts import Page
page = Page()
page.add(regBar, slBar, shBar, eduPie, word)
page.render('C++工作区域分布.html')


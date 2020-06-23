#pragma once
#include <vector>
#include <string>

#include <ctemplate/template.h>

#include "oj_model.hpp"

class OjView 
{
    public:
        //通过谷歌的模版技术动态加载题目信息至html当中
        //questions是题目的信息
        //html是经过渲染后的模版信息
        static void RenderAllQuestions(const std::vector<Question>& questions,
                std::string* html) 
        {
            //定义一个字典
            ctemplate::TemplateDictionary dict("all_questions");

            //遍历所有的题目
            for (const auto& question : questions) 
            {
                //定义一个子字典，该子字典"question"必须和html模版当中的标识一致
                //循环加载
                ctemplate::TemplateDictionary* table_dict
                    = dict.AddSectionDictionary("question");
                table_dict->SetValue("id", question.id);
                table_dict->SetValue("title", question.title);
                table_dict->SetValue("star", question.star);
            }

            //获取对应路径上的html模版
            ctemplate::Template* tpl = ctemplate::Template::GetTemplate(
                    "./template/all_questions.html", ctemplate::DO_NOT_STRIP);
            //渲染html模版，把字典当中的内容逐字加载到html模版当中对应的位置
            tpl->Expand(html, &dict);
        }

        static void RenderQuestion(const Question& question,
                const std::string& code, const std::string& desc,
                std::string* html) 
        {
            ctemplate::TemplateDictionary dict("question");
            dict.SetValue("id", question.id);
            dict.SetValue("title", question.title);
            dict.SetValue("star", question.star);
            dict.SetValue("code", code);
            dict.SetValue("desc", desc);
            ctemplate::Template* tpl = ctemplate::Template::GetTemplate(
                    "./template/question.html", ctemplate::DO_NOT_STRIP);
            tpl->Expand(html, &dict);
        }

        // 前三个参数分别对应 JSON 中的 stdout, reason 字段
        static void RenderCompileResult(const std::string& question_stdout,
                const std::string& question_reason,
                std::string* html) {
            ctemplate::TemplateDictionary dict("case_result");
            dict.SetValue("case_result", question_stdout);
            dict.SetValue("compile_result", question_reason);
            ctemplate::Template* tpl = ctemplate::Template::GetTemplate(
                    "./template/case_result.html", ctemplate::DO_NOT_STRIP);
            tpl->Expand(html, &dict);
        }
};

/**
 * Licensed under the MIT licenses.
 *
 * @file CmdShell.h
 * @brief Shell命令行管理器<br />
 *	占位符: <br />
 *		1. @PH_CUR_FILE_LIST
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2012-12-11
 *
 * @history
 * 		2013.03.05 增加占位符功能
 *
 */
#ifndef _SIMU_UTILITY_CMD_SHELL_H__
#define _SIMU_UTILITY_CMD_SHELL_H__

#include <list>
#include <map>
#include <string>

#include <ext/Singleton.h>

namespace simulator
{
    struct AutoCompleteWrapper
    {
        typedef std::map<std::string, AutoCompleteWrapper> value_type;
        value_type stChildren;
        std::string strName;

        AutoCompleteWrapper& operator[](const std::string& strName);
    };

    class CmdShell: public Singleton<CmdShell>
    {
    private:
        /** shell 名称 **/
        std::string m_strShellName;
        /** 提示符 **/
        std::string m_strPrompt;
        /** EOF标记 **/
        bool m_bIsEOF;
        /** Close标记 **/
        bool m_bIsClosed;

        /** 历史记录文件名 **/
        std::string m_strHistoryFileName;
        /** 最大历史记录长度 **/
        int m_iMaxHostoryLength;

        /**
         * 自动完成表<br />
         * char => list<string>
         */
        AutoCompleteWrapper m_stAutoComplete;

        /**
         * 候选自动完成命令列表
         */
        std::list<std::string> m_stAutoCompleteCmds;

    private:
        /**
         * 从历史列表里读历史记录
         */
        void _read_history();

        /**
         * 追加历史记录到历史列表文件
         */
        void _write_history();

    public:
        /**
         * 构造函数
         * @param strName               Shell的名称
         * @param strHistoryFileName    历史文件保存路径
         */
        CmdShell();

        ~CmdShell();

        void SetEOF();
        bool IsEOF() const;

        void Close();
        bool IsClosed() const;

        /**
         * 设置输入提示前缀
         * @param strMsg	前缀内容
         */
        void SetPrompt(const std::string& strMsg);

        /**
         * 设置最大历史Size，返回原始值
         * @param iNum 最大历史记录Size
         * @return 更改前的值
         */
        int SetMaxHistoryNumber(int iNum);

        /**
         * 初始化函数
         */
        void Init(const std::string& strName, const std::string strHistoryFileName = ".shell_history");

        /**
         * 读一行
         * @return 读到的数据
         */
        const std::string ReadLine();

        /**
         * 插入一条自动完成规则
         * @param cKey 索引字符（不区分大小写）
         * @param strValue 填充项
         */
        AutoCompleteWrapper& GetAutoComplete();

    public:
        /**
         * 自动完成回调
         * @param [in] 当前单词
         * @param [in] 当前单词其实位置
         * @param [in] 当前单词结束位置
         * @return 候选词列表
         */
        static char** CompleteCmd(const char*, int, int);

        /**
         * 自动完成指令生成回调
         * @param [in] 当前单词
         * @param [in] 状态(第N的单词)
         * @return 候选词指针(候选词必须由malloc分配内存)
         */
        static char* CompleteCmdGenerator(const char *, int);
    };
}

#endif

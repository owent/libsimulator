/**
 * Licensed under the MIT licenses.
 *
 * @file simple_test.cpp
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2014年6月30日
 *
 * @history
 *
 */

#include <cstdio>
#include <sstream>
#include "PlayerImpl.h"
#include "Simulator.h"
#include "DispatcherImpl.h"
#include "ReqRspBase.h"

// ======================= 定义Player类型 =======================
struct RealPlayer: public simulator::PlayerImpl, public std::enable_shared_from_this<RealPlayer>
{
    virtual int OnCreated()
    {
        puts("player on created.");
        // 可以在这里分配资源或者做一些网络连接相关的操作

        // 这里返回负值会导致创建失败，CreatePlayer函数返回空指针
        return 0;
    }

    virtual int Init()
    {
        puts("player on init.");
        // 可以在这里初始化资源

        // 这里返回负值不会导致创建失败，但是玩家会进入退出状态而不再响应命令和回包
        return 0;
    }
};

// ======================= 定义协议MSG类型 =======================
struct MsgType
{
    int msg_type;
    std::string action;

    // [可选] 可视化序列化接口
    static std::string ToString(const MsgType& m)
    {
        std::stringstream ss;
        ss << m.msg_type<< "-"<< m.action;
        return ss.str();
    }

    // [可选] 初始化接口
    static void Init(MsgType& m)
    {
        m.msg_type = 0;
        m.action = "do nothing";
    }

    // [必须] 发送接口
    static int Send(std::shared_ptr<RealPlayer>, MsgType&)
    {
        puts("at last, send req package.");
        return 0;
    }

    // [必须] 获取消息ID
    static int PickID(std::shared_ptr<RealPlayer>, const MsgType& msg)
    {
        return msg.msg_type;
    }
};

typedef simulator::Simulator<int, MsgType, RealPlayer> app_simu_t;

// ======================= 定义协议Cpp调度器 =======================
class CppDispatcher: public simulator::DispatcherBase<RealPlayer>
{
public:
    CppDispatcher(){}
    ~CppDispatcher(){}

    virtual int Init(){ return 0; }

    virtual int ProcOne(std::shared_ptr<RealPlayer> player) {
        return 0;
    }

    virtual bool IsInitEnabled() { return true; };

    static int ProcHandler(std::shared_ptr<RealPlayer> player) {
        // 如果使用的是epoll，可以在这里对单个玩家接收到的数据进行一次或多次解包并多次调用app_simu_t::ProcOnePlayer函数重入玩家Proc事件
        // app_simu_t::ProcOnePlayer(player);
        return 0;
    }
};

// ======================= 命令绑定 =======================
namespace proto
{

    class TestProto: public simulator::ReqRspBase<TestProto, app_simu_t>
    {
    private:
        /**
         * 初始化自动完成
         */
        void _init_auto_complete()
        {
            RegisterAutoComplete()["Test"]["Cmd"];
        }

        /**
         * 初始化请求命令
         */
        void _init_req_command()
        {
            std::shared_ptr<copt::CmdOptionCI> pBase = GetCmdManager("Test");

            pBase->BindCmd("Cmd", &TestProto::OnCmdGet, this)
                ->SetHelpMsg(simulator::ReqMgrBase::GenCmdHelpMsg("Cmd", "just a test cmd.").c_str());
        }

        /**
         * 初始化回包
         */
        void _init_rsp_callback()
        {
            // 没有接入网络所以没有回包,随便注册一个,权作示例仅供参考
            SetRspProcessFunc(static_cast<uint64_t>(123),
                std::bind(&TestProto::OnRspCmd, this, std::placeholders::_1, std::placeholders::_2));
        }

    public:
        TestProto()
        {
            _init_auto_complete();
            _init_req_command();
            _init_rsp_callback();
        }
        ~TestProto(){}

    public:
        // ============== 请求命令函数 =============
        void OnCmdGet(copt::callback_param stParam)
        {
            simulator::ReqInfo<MsgType, RealPlayer> & stReqInfo = GetReqInfo(stParam);
            // 必须保证玩家有效
            if (!stReqInfo.pPlayer)
            {
                stReqInfo.pPlayer = app_simu_t::CreatePlayer("test player id");
            }

            // 标记为已登入
            stReqInfo.pPlayer->SetLogined();
            // 设为命令行当前操作玩家
            app_simu_t::SetCmdPlayer(stReqInfo.pPlayer);

            puts("Got Test Cmd, Parameter(s) listed below:");
            for (copt::CmdOptionList::size_type i = 0 ; i < stParam.GetParamsNumber(); ++ i)
            {
                printf("Param %02d: %s\n", (int)(i + 1), stParam[i]->AsString());
            }

            // 填充协议包
            MsgType& stPkg = GetReqPkg(stParam);
            stPkg.msg_type = 123;
            if (stParam.GetParamsNumber() > 0)
            {
                stPkg.action = stParam[0]->AsCppString();
            }
        }

        void OnRspCmd(std::shared_ptr<RealPlayer>, const MsgType& stPkg)
        {
            // nothing to do..
        }
    };

    // 激活协议分发器
    ACTIVE_PROTO(TestProto)
}


int main(int argc, char *argv[])
{
    std::string strIP;
    int iPort;

    // 追加命令绑定
    {
        using namespace simulator::phoenix;
        using simulator::detail::GenCmdHelpMsg;
        app_simu_t::option_mgr_t handler = app_simu_t::GetArgsMgr();
        // 绑定设置IP
        handler->BindCmd("-ip, --ip-address, --addr", AssignAction<std::string>(strIP))
            ->SetHelpMsg(GenCmdHelpMsg("-ip, --ip-address, --addr", "<ip address> Set Server IP Address(default: 127.0.0.1)").c_str());

        // 绑定设置端口
        handler->BindCmd("-p, --port", AssignAction<int>(iPort))
            ->SetHelpMsg(GenCmdHelpMsg("-p, --port", "<port> Set Server Port(default: 8002)").c_str());

    }

    // 添加Cpp和Lua调度器
    app_simu_t::AddDispatcher<CppDispatcher>(simulator::EN_DT_CPP);

    // 添加协议包接收器
    app_simu_t::AddPlayerProcListener(CppDispatcher::ProcHandler);

    // 启动初始化
    app_simu_t::Init(argc, argv, MsgType::ToString, MsgType::PickID,MsgType::Send, MsgType::Init);

    // 进入主循环
    return app_simu_t::MainLoop();
}

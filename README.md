libsimulator
======

依赖工具
------
1. git
2. bash

依赖系统库
------
1. history
2. readline 
3. termcap
4. pthread
5. [owent-utils/c-cpp](https://github.com/owent-utils/c-cpp) 中的部分内容

本工具引用的组件
------
1. CmdShell	    终端输入捕获工具，	依赖 CmdShell.h, CmdShell.cpp, readline 库
2. ShellFont	终端字体生成工具，	依赖 ShellFont.h, ShellFont.cpp
3. CmdOption	指令绑定工具，		依赖 CmdOption 内全部内容

更新提示
------
如果需要使用自定义的readline库，请替换include文件夹内的readline文件夹和lib目录下的libreadline.a和libhistory.a


终端说明
------
### 终端控制指令说明见
http://cnswww.cns.cwru.edu/php/chet/readline/rluserman.html

### 仿真工具默认命令:
```bash
?, -h, --help, help                     查看帮助
-his, --history, --history-file         <file path> 操作历史文件名
-pro, --proto, --proto-log              <file path> 协议可视化文件名
-ptm, --player-threadpool-max           <number> 玩家线程池最大线程数
-ni, --no-interactive                   无用户输入
-f, --rf, --read-file                   执行命令文件
-c, --cmd                               执行命令
......
自定义命令
```


### 命令行模式下默认系统命令:
```bash
!, sh                                   <cmd> [args ...] 执行系统命令
exit, quit                              退出
?, help                                 查看命令列表
SetCmdPlayer                            设置命令行所控制的用户
```

程序接口
------

框架接口 @see include/Simulator.h

自定义调度器 @see include/DispatcherImpl.h

玩家类型 @see include/PlayerImpl.h

自定义命令 @see include/ReqRspBase.h

定时器 @see include/TimerImpl.h


注记
------
写得比较乱，目前仅给自己所在的项目中使用
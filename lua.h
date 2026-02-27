/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/

// 【C语法】条件编译。防止这个头文件被同一个 .c 文件重复包含 (Include Guard)
#ifndef lua_h
#define lua_h

// 【C语法】引入标准库头文件
#include <stdarg.h> // 用于处理可变参数 (va_list)
#include <stddef.h> // 定义了基础类型如 size_t (表示内存大小的无符号整数)


// 定义 Lua 的版权和作者信息
#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


// 定义 Lua 的版本号 (当前是 5.5.1)
#define LUA_VERSION_MAJOR_N	5 // 主版本号
#define LUA_VERSION_MINOR_N	5 // 次版本号
#define LUA_VERSION_RELEASE_N	1 // 发布版本号

// 将版本号转为数字，方便在 C 代码中用 #if 进行版本判断 (505)
#define LUA_VERSION_NUM  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_NUM  (LUA_VERSION_NUM * 100 + LUA_VERSION_RELEASE_N)


// 引入 Lua 的配置头文件。里面包含了与平台、操作系统相关的底层类型定义（如整数类型长度等）
#include "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
// 预编译字节码的签名（魔数）。如果用 luac 编译出一个二进制字节码文件，它的开头几个字节就是这个
#define LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
// 当你调用一个 Lua 函数，但不知道它会返回多少个返回值时，传入这个宏表示“接收所有返回值”
#define LUA_MULTRET	(-1)


/*
** Pseudo-indices (伪索引)
** Lua 与 C 交互是通过一个虚拟栈进行的。普通的栈索引从 1 开始（栈底）或 -1 开始（栈顶）。
** 伪索引看起来像栈索引，但它们并不指向真正的栈，而是指向 Lua 内部特定的全局或局部环境。
** (栈的限制大小是 INT_MAX/2，这之后留了一些空间用来检测溢出)
*/
#define LUA_REGISTRYINDEX	(-(INT_MAX/2 + 1000)) // 指向"注册表"的伪索引。注册表是C代码专属的全局储物箱
#define lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i)) // 用于访问 C闭包(C Closure) 绑定的外部变量(Upvalue)


/* thread status (线程/协程 运行状态) */
#define LUA_OK		0       // 运行成功
#define LUA_YIELD	1       // 协程挂起 (yield)
#define LUA_ERRRUN	2       // 运行时错误 (Runtime error)
#define LUA_ERRSYNTAX	3   // 语法错误 (Syntax error)
#define LUA_ERRMEM	4       // 内存分配错误 (Memory error)
#define LUA_ERRERR	5       // 错误处理函数本身发生了错误


// 【C语法】前置声明并用 typedef 定义别名。
// lua_State 是 Lua 中最重要的数据结构！它代表了一个 Lua 的执行状态/主线程（也就是一个 Lua 虚拟机实例）。
// 在外部暴露为不透明指针（只声明不提供实现细节），保证了封装性。
typedef struct lua_State lua_State;


/*
** basic types (Lua 语言的 8 种基础数据类型)
*/
#define LUA_TNONE		(-1) // 无效类型（比如你访问了栈外的位置）

#define LUA_TNIL		0    // nil 类型
#define LUA_TBOOLEAN		1    // 布尔类型 (true/false)
#define LUA_TLIGHTUSERDATA	2    // 轻量级用户数据 (本质上是一个 C 指针)
#define LUA_TNUMBER		3    // 数字类型 (整数或浮点数)
#define LUA_TSTRING		4    // 字符串类型
#define LUA_TTABLE		5    // 表类型 (Lua 唯一的数据结构)
#define LUA_TFUNCTION		6    // 函数类型
#define LUA_TUSERDATA		7    // 用户数据 (C 分配的一块内存，由 Lua GC 管理)
#define LUA_TTHREAD		8    // 线程类型 (也就是 Lua 协程)

#define LUA_NUMTYPES		9    // 类型的总数



/* minimum Lua stack available to a C function */
// 当 Lua 调用 C 函数时，保证栈上至少有 20 个空闲位置可用
#define LUA_MINSTACK	20


/* predefined values in the registry (注册表中的预定义索引) */
/* index 1 is reserved for the reference mechanism (索引 1 被 luaL_ref 引用机制保留) */
#define LUA_RIDX_GLOBALS	2    // 注册表中全局环境(_G)的索引
#define LUA_RIDX_MAINTHREAD	3    // 注册表中主线程(main thread)的索引
#define LUA_RIDX_LAST		3


/* type of numbers in Lua */
// Lua 的数字类型别名。LUA_NUMBER 通常在 luaconf.h 中被定义为 double (双精度浮点数)
typedef LUA_NUMBER lua_Number;


/* type for integer functions */
// Lua 的整数类型。通常被定义为 long long (64位整数)
typedef LUA_INTEGER lua_Integer;

/* unsigned integer type */
// 无符号整数类型
typedef LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
// 用于在协程 yield/resume 时保存 C 语言上下文的类型，通常是一个整数指针大小
typedef LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
** 【重点】【C语法】函数指针的 typedef。
** 定义了 lua_CFunction 为一个指针，指向这样一个函数：接收 lua_State* 参数，返回 int（表示返回值的数量）。
** 所有想要注册给 Lua 调用的 C 函数都必须符合这个签名！
*/
typedef int (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
** 延续函数指针。用于支持 C 函数在协程 yield 恢复后继续执行。
*/
typedef int (*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
** 用于加载或导出 Lua 字节码时的 读/写 回调函数指针
*/
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);
typedef int (*lua_Writer) (lua_State *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
** 内存分配器的函数指针。Lua 的所有内存分配(malloc/free/realloc)都可以由宿主(C程序)通过这个接口接管。
** ud 是自定义数据指针，ptr 是旧内存指针，osize 旧大小，nsize 新大小。
*/
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
** 触发警告时的回调函数指针
*/
typedef void (*lua_WarnFunction) (void *ud, const char *msg, int tocont);


/*
** Type used by the debug API to collect debug information
** 用于收集调试信息的结构体（前置声明）
*/
typedef struct lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
** 调试器钩子(Hook)的回调函数指针，在特定事件（如执行了一行代码，调用了函数）触发。
*/
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
** 【C语法】如果你在编译时定义了 LUA_USER_H 宏，这里就会引入你自定义的头文件
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/*
** RCS ident string
** 用于源码版本控制的字符串标识
*/
extern const char lua_ident[];


/*
** state manipulation (状态/虚拟机操作函数)
** 注：LUA_API 是一个宏，用来声明导出函数（在 Windows 上通常展开为 __declspec(dllexport)，在 Linux 通常为空）
*/
// 创建一个新的 Lua 虚拟机环境，需要传入自定义的内存分配函数
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud, unsigned seed);
// 关闭虚拟机，释放所有内存
LUA_API void       (lua_close) (lua_State *L);
// 创建一个新的协程（线程）
LUA_API lua_State *(lua_newthread) (lua_State *L);
// 关闭线程
LUA_API int        (lua_closethread) (lua_State *L, lua_State *from);

// 设置"恐慌函数"(Panic function)——当 Lua 发生未捕获的严重错误即将崩溃时，会调用此函数
LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);

// 获取 Lua 版本号
LUA_API lua_Number (lua_version) (lua_State *L);


/*
** basic stack manipulation (基础栈操作)
** Lua 所有的 API 操作几乎都在跟一个虚拟栈打交道。
*/
// 将相对索引（比如 -1 代表栈顶）转为绝对索引（从栈底开始的正数）
LUA_API int   (lua_absindex) (lua_State *L, int idx);
// 获取栈顶元素的索引（其实也就是栈里元素的总个数）
LUA_API int   (lua_gettop) (lua_State *L);
// 设置栈顶（如果你设小了，多出的元素会被抛弃；设大了，缺少的会用 nil 填充）
LUA_API void  (lua_settop) (lua_State *L, int idx);
// 复制指定索引的值，并将其压入栈顶
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);
// 将栈中某些元素轮转移动
LUA_API void  (lua_rotate) (lua_State *L, int idx, int n);
// 从 fromidx 复制元素直接覆盖到 toidx 的位置
LUA_API void  (lua_copy) (lua_State *L, int fromidx, int toidx);
// 检查栈中是否有额外 n 个空闲位置，防止栈溢出
LUA_API int   (lua_checkstack) (lua_State *L, int n);

// 在不同的 lua_State（通常是不同的协程）之间移动栈顶的 n 个元素
LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n);


/*
** access functions (stack -> C)
** 访问函数：将栈里的 Lua 值转换为 C 的类型，或检查类型
** 注意：这些函数通常不会改变栈的结构（不弹出元素）
*/
LUA_API int             (lua_isnumber) (lua_State *L, int idx);  // 检查是否为数字
LUA_API int             (lua_isstring) (lua_State *L, int idx);  // 检查是否为字符串
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);// 检查是否为 C 函数
LUA_API int             (lua_isinteger) (lua_State *L, int idx); // 检查是否为整数
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);// 检查是否为 userdata
LUA_API int             (lua_type) (lua_State *L, int idx);      // 获取类型(返回 LUA_T... 宏的值)
LUA_API const char     *(lua_typename) (lua_State *L, int tp);   // 将类型宏转换为可读字符串(如 "string")

// 转换为 C 的浮点数 (isnum 是可选参数，用于接收转换是否成功的布尔值)
LUA_API lua_Number      (lua_tonumberx) (lua_State *L, int idx, int *isnum);
// 转换为 C 的整数
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, int idx, int *isnum);
// 转换为布尔值 (在 C 中 0 为 false，非 0 为 true)
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
// 转换为 C 字符串（len 指针用于接收字符串的长度，因为 Lua 字符串可以包含 '\0'，所以需要返回长度）
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
// 获取对象的原始长度 (类似 Lua 里的 # 操作符，但忽略元表)
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, int idx);
// 转换为 C 函数指针
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
// 转换为 userdata 的内存指针
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);
// 转换为协程(lua_State)指针
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
// 转换为一般通用指针 (常用于作为哈希表的 key 或用于调试打印)
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);


/*
** Comparison and arithmetic functions (比较和算术运算)
*/

// 算术运算的操作码 (加减乘除等)
#define LUA_OPADD	0	/* ORDER TM, ORDER OP */ // 加
#define LUA_OPSUB	1                            // 减
#define LUA_OPMUL	2                            // 乘
#define LUA_OPMOD	3                            // 取模 (%)
#define LUA_OPPOW	4                            // 乘方 (^)
#define LUA_OPDIV	5                            // 浮点除法 (/)
#define LUA_OPIDIV	6                            // 向下取整除法 (//)
#define LUA_OPBAND	7                            // 位与 (&)
#define LUA_OPBOR	8                            // 位或 (|)
#define LUA_OPBXOR	9                            // 位异或 (~)
#define LUA_OPSHL	10                           // 左移 (<<)
#define LUA_OPSHR	11                           // 右移 (>>)
#define LUA_OPUNM	12                           // 一元负号 (-)
#define LUA_OPBNOT	13                           // 按位取反 (~)

// 执行算术运算：会从栈顶弹出操作数，并将结果压回栈顶
LUA_API void  (lua_arith) (lua_State *L, int op);

// 比较运算的操作码：等于、小于、小于等于
#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

// "原生"比较两个值是否相等 (忽略元方法 metamethod `__eq`)
LUA_API int   (lua_rawequal) (lua_State *L, int idx1, int idx2);
// 比较两个值 (op 参数使用上面的宏，会触发元方法)
LUA_API int   (lua_compare) (lua_State *L, int idx1, int idx2, int op);


/*
** push functions (C -> stack)
** 压栈函数：将 C 变量压入 Lua 的虚拟栈顶中
*/
LUA_API void        (lua_pushnil) (lua_State *L);            // 压入 nil
LUA_API void        (lua_pushnumber) (lua_State *L, lua_Number n);   // 压入浮点数
LUA_API void        (lua_pushinteger) (lua_State *L, lua_Integer n); // 压入整数
LUA_API const char *(lua_pushlstring) (lua_State *L, const char *s, size_t len); // 压入指定长度的字符串
LUA_API const char *(lua_pushexternalstring) (lua_State *L, // 压入外部管理的字符串 (Lua不负责这块内存)
		const char *s, size_t len, lua_Alloc falloc, void *ud);
LUA_API const char *(lua_pushstring) (lua_State *L, const char *s);  // 压入以 '\0' 结尾的 C 字符串
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt, // 用可变参数压入格式化字符串
                                                      va_list argp);
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...); // 类似 printf 的压入格式化字符串
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n); // 压入带有 n 个 Upvalue 的 C 闭包
LUA_API void  (lua_pushboolean) (lua_State *L, int b);       // 压入布尔值
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p); // 压入轻量级用户数据
LUA_API int   (lua_pushthread) (lua_State *L);               // 压入当前的协程本身


/*
** get functions (Lua -> stack)
** 获取函数：从 Lua 表/环境中取出数据，并压入栈顶
** 这些函数通常返回压入元素的数据类型 (LUA_T...)
*/
LUA_API int (lua_getglobal) (lua_State *L, const char *name); // 获取全局变量 (_G[name]) 压入栈
LUA_API int (lua_gettable) (lua_State *L, int idx);           // 以栈顶为 key，取表(位于idx)的 value，压栈
LUA_API int (lua_getfield) (lua_State *L, int idx, const char *k); // 获取表中的字符串 key 的值 (table[k])
LUA_API int (lua_geti) (lua_State *L, int idx, lua_Integer n);     // 获取表中的整数索引的值 (table[n])
LUA_API int (lua_rawget) (lua_State *L, int idx);             // 原生获取表数据 (不触发 __index 元方法)
LUA_API int (lua_rawgeti) (lua_State *L, int idx, lua_Integer n);  // 原生获取表中的整数索引的值
LUA_API int (lua_rawgetp) (lua_State *L, int idx, const void *p);  // 原生获取表中的指针(作为key)对应的值

// 创建一个新表并压栈。narr 预分配数组空间，nrec 预分配哈希表空间
LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);
// 分配指定大小(sz)的 Userdata(用户数据)内存，压栈并返回其 C 指针。nuvalue 是绑定的额外 Lua 值的数量
LUA_API void *(lua_newuserdatauv) (lua_State *L, size_t sz, int nuvalue);
// 获取栈上对象的元表(Metatable)并压入栈顶
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);
// 获取绑定在 userdata 上的额外 Lua 值(uv)
LUA_API int  (lua_getiuservalue) (lua_State *L, int idx, int n);


/*
** set functions (stack -> Lua)
** 设置函数：将栈里的值赋给 Lua 的表/环境等。通常会把栈顶的值（作为 value）弹出来。
*/
LUA_API void  (lua_setglobal) (lua_State *L, const char *name); // 将栈顶的值赋给全局变量 (_G[name] = 栈顶)
LUA_API void  (lua_settable) (lua_State *L, int idx);           // table[key] = value (key是次栈顶，value是栈顶)
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k); // table[k] = 栈顶值
LUA_API void  (lua_seti) (lua_State *L, int idx, lua_Integer n);     // table[n] = 栈顶值
LUA_API void  (lua_rawset) (lua_State *L, int idx);             // 原生设置表数据 (不触发 __newindex)
LUA_API void  (lua_rawseti) (lua_State *L, int idx, lua_Integer n);
LUA_API void  (lua_rawsetp) (lua_State *L, int idx, const void *p);
// 弹出栈顶的一张表，将其设置为 idx 处对象的元表
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);
// 将栈顶的值绑定到 userdata 上的第 n 个用户值槽位中
LUA_API int   (lua_setiuservalue) (lua_State *L, int idx, int n);


/*
** 'load' and 'call' functions (load and run Lua code)
** 核心逻辑：加载并执行 Lua 代码
*/
// 调用函数（支持延续 ctx 和 k 用于协程 yield，初学者先忽略 k 和 ctx 即可）
// nargs 是参数个数，nresults 是期待的返回值个数
LUA_API void  (lua_callk) (lua_State *L, int nargs, int nresults,
                           lua_KContext ctx, lua_KFunction k);
// 【C语法】封装成宏，方便日常使用，不带协程延续处理。
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, NULL)

// 保护模式调用(protected call)。如果函数出错不会导致 C 程序崩溃，而是返回错误码，并将错误信息压栈
// errfunc 是错误处理函数（比如 traceback）在栈上的索引
LUA_API int   (lua_pcallk) (lua_State *L, int nargs, int nresults, int errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, NULL)

// 加载 Lua 源码或字节码（将其编译为函数压入栈顶，但不执行）
LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                          const char *chunkname, const char *mode);

// 将 Lua 函数导出为二进制字节码 (Dump)
LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data, int strip);


/*
** coroutine functions (协程相关函数)
*/
// 挂起(Yield)当前协程
LUA_API int  (lua_yieldk)     (lua_State *L, int nresults, lua_KContext ctx,
                               lua_KFunction k);
// 恢复(Resume)运行挂起的协程
LUA_API int  (lua_resume)     (lua_State *L, lua_State *from, int narg,
                               int *nres);
// 获取协程当前状态 (返回 LUA_OK, LUA_YIELD 等)
LUA_API int  (lua_status)     (lua_State *L);
// 检查当前是否可以 yield
LUA_API int (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, NULL)


/*
** Warning-related functions (警告相关函数 - Lua 5.4 新增特性)
*/
LUA_API void (lua_setwarnf) (lua_State *L, lua_WarnFunction f, void *ud); // 设置警告回调函数
LUA_API void (lua_warning)  (lua_State *L, const char *msg, int tocont);  // 发出警告信息


/*
** garbage-collection options
** 垃圾回收 (GC) 的操作选项
*/
#define LUA_GCSTOP		0    // 停止 GC
#define LUA_GCRESTART		1    // 重启 GC
#define LUA_GCCOLLECT		2    // 执行一次全量 GC
#define LUA_GCCOUNT		3    // 获取 Lua 使用的内存总量 (返回 KB 整数部分)
#define LUA_GCCOUNTB		4    // 获取内存总量的零头 (返回剩余的 Bytes)
#define LUA_GCSTEP		5    // 走一步增量 GC 步伐
#define LUA_GCISRUNNING		6    // 检查 GC 是否正在运行
#define LUA_GCGEN		7    // 将 GC 切换为分代(Generational)模式
#define LUA_GCINC		8    // 将 GC 切换为增量(Incremental)模式
#define LUA_GCPARAM		9    // 设置或获取 GC 内部参数


/*
** garbage-collection parameters
** 垃圾回收的各项可调参数
*/
/* parameters for generational mode (分代模式参数) */
#define LUA_GCPMINORMUL		0  /* control minor collections */
#define LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#define LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode (增量模式参数) */
#define LUA_GCPPAUSE		3  /* size of pause between successive GCs (两次GC间的停顿时间) */
#define LUA_GCPSTEPMUL		4  /* GC "speed" (GC 回收速度乘数) */
#define LUA_GCPSTEPSIZE		5  /* GC granularity (GC 单步步长) */

/* number of parameters */
#define LUA_GCPN		6

// C 接口触发 GC 操作的唯一入口函数
LUA_API int (lua_gc) (lua_State *L, int what, ...);


/*
** miscellaneous functions (杂项函数)
*/

// 从 C 中抛出一个 Lua 错误（会利用 longjmp 机制跳转，函数不会通过正常的 return 结束）
LUA_API int   (lua_error) (lua_State *L);

// 遍历表的操作 (相当于 Lua 里的 pairs)。传入前一个 key，弹出后压入下一个 key-value 组
LUA_API int   (lua_next) (lua_State *L, int idx);

// 字符串拼接。从栈顶弹出 n 个字符串，拼成一个新字符串压栈
LUA_API void  (lua_concat) (lua_State *L, int n);
// 获取长度 (相当于 # 操作符)
LUA_API void  (lua_len)    (lua_State *L, int idx);

#define LUA_N2SBUFFSZ	64 // 数字转字符串所需的缓冲区大小
// 将数字转换为 C 字符串
LUA_API unsigned  (lua_numbertocstring) (lua_State *L, int idx, char *buff);
// 将 C 字符串尝试解析为数字并压栈
LUA_API size_t  (lua_stringtonumber) (lua_State *L, const char *s);

// 获取/设置全局内存分配器函数
LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
LUA_API void      (lua_setallocf) (lua_State *L, lua_Alloc f, void *ud);

// Lua 5.4 新增：用来支持 to-be-closed 变量 (类似 defer 机制，退出作用域自动清理)
LUA_API void (lua_toclose) (lua_State *L, int idx);
LUA_API void (lua_closeslot) (lua_State *L, int idx);


/*
** {==============================================================
** some useful macros (一些极其常用的宏定义封装)
** 这些宏让上面的 C API 使用起来更加友好
** ===============================================================
*/

// 获取额外空间。在分配 lua_State 时，Lua 允许在其内存前附带一块供宿主用的额外空间
#define lua_getextraspace(L)	((void *)((char *)(L) - LUA_EXTRASPACE))

// 最常用的两个宏：不用传 isnum 指针的类型转换
#define lua_tonumber(L,i)	lua_tonumberx(L,(i),NULL)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),NULL)

// 弹出栈顶的 n 个元素 (就是把栈顶往下挪 n 个位置)
#define lua_pop(L,n)		lua_settop(L, -(n)-1)

// 创建空表
#define lua_newtable(L)		lua_createtable(L, 0, 0)

// 注册 C 函数到全局环境的快捷方式
#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

// 压入一个普通的 C 函数 (0 个 upvalue)
#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

// 快速判断类型的各种宏
#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

// 压入字符串字面量 (利用 C 语言相邻字符串字面量自动拼接的特性 "" s)
#define lua_pushliteral(L, s)	lua_pushstring(L, "" s)

// 压入全局环境表 _G 到栈顶
#define lua_pushglobaltable(L)  \
	((void)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

// 转换为 C 字符串，不获取长度
#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)


// 将栈顶元素移动插入到指定的 idx 位置
#define lua_insert(L,idx)	lua_rotate(L, (idx), 1)

// 移除 idx 位置的元素，上面的元素往下补齐
#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_pop(L, 1))

// 把栈顶的值弹出，并替换掉 idx 位置原本的值
#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_pop(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros (向后兼容的宏)
** 为了让为老版本 Lua 写的 C 代码在最新版里也能跑而保留的别名
** ===============================================================
*/

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,NULL)

/* }============================================================== */

/*
** {======================================================================
** Debug API (调试接口 API)
** 如果你要写 Lua 调试器(debugger) 或者性能分析工具(profiler)，会频繁用到这里
** =======================================================================
*/


/*
** Event codes (调试钩子事件码)
*/
#define LUA_HOOKCALL	0    // 函数调用事件
#define LUA_HOOKRET	1        // 函数返回事件
#define LUA_HOOKLINE	2    // 执行到新的一行代码事件
#define LUA_HOOKCOUNT	3    // 执行了指定数量的指令事件
#define LUA_HOOKTAILCALL 4   // 尾调用事件 (Tail call)


/*
** Event masks (调试掩码)
** 利用按位操作 (1 << x)，让开发者可以用位运算组合多个事件一起监听
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


// 获取函数调用栈 (获取指定 level 的函数活动记录，填入 lua_Debug 结构体)
LUA_API int (lua_getstack) (lua_State *L, int level, lua_Debug *ar);
// 根据 what 描述符(如 "n", "S", "l")，解析出具体的函数名字、源码位置等信息
LUA_API int (lua_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
// 获取局部变量名
LUA_API const char *(lua_getlocal) (lua_State *L, const lua_Debug *ar, int n);
// 修改局部变量
LUA_API const char *(lua_setlocal) (lua_State *L, const lua_Debug *ar, int n);
// 获取 Upvalue（外部闭包变量）名字
LUA_API const char *(lua_getupvalue) (lua_State *L, int funcindex, int n);
// 修改 Upvalue 的值
LUA_API const char *(lua_setupvalue) (lua_State *L, int funcindex, int n);

// 获取 Upvalue 唯一的标识符指针，用于比较
LUA_API void *(lua_upvalueid) (lua_State *L, int fidx, int n);
// 让两个闭包共享同一个 Upvalue
LUA_API void  (lua_upvaluejoin) (lua_State *L, int fidx1, int n1,
                                               int fidx2, int n2);

// 设置调试钩子 Hook，mask 参数控制触发时机（见前面的 LUA_MASK*）
LUA_API void (lua_sethook) (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API int (lua_gethookmask) (lua_State *L);
LUA_API int (lua_gethookcount) (lua_State *L);


// 调试记录结构体。当 Hook 触发或调用 getinfo 时，Lua 引擎会把相关数据填入这里。
struct lua_Debug {
  int event;            // 当前触发的事件 (LUA_HOOK...)
  const char *name;	/* (n) */    // 函数的名称
  const char *namewhat;	/* (n) 'global', 'local', 'field', 'method' */ // 名字的性质
  const char *what;	/* (S) 'Lua', 'C', 'main', 'tail' */ // 函数的类型（Lua写的，还是C写的...）
  const char *source;	/* (S) */    // 源码所在文件或字符串
  size_t srclen;	/* (S) */        // 源码字符串的长度
  int currentline;	/* (l) */    // 当前执行在哪一行
  int linedefined;	/* (S) */    // 该函数定义在源文件哪一行开始
  int lastlinedefined;	/* (S) */ // 该函数在源文件哪一行结束
  unsigned char nups;	/* (u) number of upvalues */  // 该函数的 upvalue 数量
  unsigned char nparams;/* (u) number of parameters */ // 期望的参数个数
  char isvararg;        /* (u) */ // 是否是可变参数函数 (...)
  unsigned char extraargs;  /* (t) number of extra arguments */ // 传入的额外参数数量
  char istailcall;	/* (t) */ // 当前调用是否为尾调用
  int ftransfer;   /* (r) index of first value transferred */ // 【Lua5.4新增】用于获取返回值相关
  int ntransfer;   /* (r) number of transferred values */
  char short_src[LUA_IDSIZE]; /* (S) */ // 精简版的源码描述（方便打印报错）
  /* private part (私有部分，开发者不应直接访问这些字段) */
  struct CallInfo *i_ci;  /* active function */ // 指向内部的调用帧结构
};

/* }====================================================================== */


// 【C语法】字符串化宏的常用技巧。
// 将数字宏先转成 "#x"，从而把它转换成一个字符串常量。
#define LUAI_TOSTRAUX(x)	#x
#define LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

// 用宏组合出像 "5", "5", "1" 这样的字符串
#define LUA_VERSION_MAJOR	LUAI_TOSTR(LUA_VERSION_MAJOR_N)
#define LUA_VERSION_MINOR	LUAI_TOSTR(LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE	LUAI_TOSTR(LUA_VERSION_RELEASE_N)

// 拼接出 Lua 的全名版本字符串： "Lua 5.5" 和 "Lua 5.5.1"
#define LUA_VERSION	"Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#define LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE


/******************************************************************************
* Copyright (C) 1994-2026 Lua.org, PUC-Rio.
* (以下是 MIT 协议的内容，声明 Lua 是完全开源且可商用的)
* ...
******************************************************************************/


// 结束文件开头的 #ifndef lua_h 条件编译
#endif
//说明部分

//API
#include "xtp_trader_api.h"

//系统
//#ifdef WIN32
//#include "stdafx.h"
//#endif
#include <string>
#include <queue>

//Boost
#define BOOST_PYTHON_STATIC_LIB
#include <boost/python/module.hpp>	//python封装
#include <boost/python/def.hpp>		//python封装
#include <boost/python/dict.hpp>	//python封装
#include <boost/python/list.hpp>	//python封装
#include <boost/python/object.hpp>	//python封装
#include <boost/python.hpp>			//python封装
#include <boost/thread.hpp>			//任务队列的线程功能
#include <boost/bind.hpp>			//任务队列的线程功能


//命名空间
using namespace std;
using namespace boost::python;
using namespace boost;


//常量
#define ONDISCONNECTED 1
#define ONERROR 2
#define ONORDEREVENT 3
#define ONTRADEEVENT 4
#define ONCANCELORDERERROR 5
#define ONQUERYORDER 6
#define ONQUERYTRADE 7
#define ONQUERYPOSITION 8
#define ONQUERYASSET 9
#define ONQUERYSTRUCTUREDFUND 10
#define ONQUERYFUNDTRANSFER 11
#define ONFUNDTRANSFER 12
#define ONQUERYETF 13
#define ONQUERYETFBASKET 14
#define ONQUERYIPOINFOLIST 15
#define ONQUERYIPOQUOTAINFO 16

#define ONQUERYOPTIONAUCTIONINFO 17

#define ONCREDITCASHREPAY 18
#define ONQUERYCREDITCASHREPAYINFO 19
#define ONQUERYCREDITFUNDINFO 20
#define ONQUERYCREDITDEBTINFO 21
#define ONQUERYCREDITTICKERDEBTINFO 22
#define ONQUERYCREDITASSETDEBTINFO 23
#define ONQUERYCREDITTICKERASSIGNINFO 24
#define ONQUERYCREDITEXCESSSTOCK 25

#define ONCREDITEXTENDDEBTDATE 26
#define ONQUERYCREDITEXTENDDEBTDATEORDERS 27
#define ONQUERYCREDITFUNDEXTRAINFO 28
#define ONQUERYCREDITPOSITIONEXTRAINFO 29

#define ONQUERYORDERBYPAGE 30
#define ONQUERYTRADEBYPAGE 31
#define ONCREDITCASHREPAYDEBTINTERESTFEE 32
#define ONQUERYMULCREDITEXCESSSTOCK 33

#define ONOPTIONCOMBINEDORDEREVENT 34
#define ONOPTIONCOMBINEDTRADEEVENT 35
#define ONQUERYOPTIONCOMBINEDORDERS 36
#define ONQUERYOPTIONCOMBINEDORDERSBYPAGE 37
#define ONQUERYOPTIONCOMBINEDTRADES 38
#define ONQUERYOPTIONCOMBINEDTRADESBYPAGE 39
#define ONQUERYOPTIONCOMBINEDPOSITION 40
#define ONQUERYOPTIONCOMBINEDSTRATEGYINFO 41
#define ONCANCELOPTIONCOMBINEDORDERERROR 42

#define ONQUERYOPTIONCOMBINEDEXECPOSITION 43
#define ONQUERYOTHERSERVERFUND 44


#define ONQUERYSTRATEGY 45
#define ONSTRATEGYASTATEREPORT 46
#define ONALGOUSERESTABLISHCHANNEL 47
#define ONINSERTALGOORDER 48
#define ONCANCELALGOORDER 49
#define ONALGODISCONNECTED 50
#define ONALGOCONNECTED 51
#define ONQUERYORDEREX 52
#define ONQUERYORDERBYPAGEEX 53
#define ONQUERYOPTIONCOMBINEDORDERSEX 54
#define ONQUERYOPTIONCOMBINEDORDERSBYPAGEEX 55
#define ONSTRATEGYSYMBOLSTATEREPORT 56
#define ONQUERYACCOUNTTRADEMARKET 57

#define ONQUERYBONDIPOINFOLIST	58
#define ONQUERYBONDSWAPSTOCKINFO 59
#define ONNEWSTRATEGYCREATEREPORT 60
#define ONSTRATEGYRECOMMENDATION 61
#define ONMODIFYALGOORDER 62
#define	ONPAUSEALGOORDER 63
#define	ONRESUMEALGOORDER 64

///-------------------------------------------------------------------------------------
///API中的部分组件
///-------------------------------------------------------------------------------------

//GIL全局锁简化获取用，
//用于帮助C++线程获得GIL锁，从而防止python崩溃
class PyLock
{
private:
	PyGILState_STATE gil_state;

public:
	//在某个函数方法中创建该对象时，获得GIL锁
	PyLock()
	{
		gil_state = PyGILState_Ensure();
	}

	//在某个函数完成后销毁该对象时，解放GIL锁
	~PyLock()
	{
		PyGILState_Release(gil_state);
	}
};


//任务结构体
struct Task
{
	int task_name;		//回调函数名称对应的常量
	void *task_data;	//数据结构体
	void *task_error;	//错误结构体
	int task_id;		//请求id
	bool task_last;		//是否为最后返回
	bool addtional_bool;		//补充bool字段
	uint64_t addtional_int;		//补充整数字段
	double remain_amount;		//补充double字段
	int64_t addtional_int_two;		//补充整数字段
	int64_t addtional_int_three;		//补充整数字段
	int64_t addtional_int_four;		//补充整数字段

	int64_t req_count;
	int64_t order_sequence; 
	int64_t query_reference;
	int64_t trade_sequence;

	string strategy_param;
	string user;
	int reason;
};


///线程安全的队列
template<typename Data>

class ConcurrentQueue
{
private:
	queue<Data> the_queue;								//标准库队列
	mutable boost::mutex the_mutex;						//boost互斥锁				
	boost::condition_variable the_condition_variable;	//boost条件变量

public:

	//存入新的任务
	void push(Data const& data)
	{
		boost::mutex::scoped_lock lock(the_mutex);		//获取互斥锁
		the_queue.push(data);							//向队列中存入数据
		lock.unlock();									//释放锁
		the_condition_variable.notify_one();			//通知正在阻塞等待的线程
	}

	//检查队列是否为空
	bool empty() const
	{
		boost::mutex::scoped_lock lock(the_mutex);
		return the_queue.empty();
	}

	//取出
	Data wait_and_pop()
	{
		boost::mutex::scoped_lock lock(the_mutex);

		while (the_queue.empty())						//当队列为空时
		{
			the_condition_variable.wait(lock);			//等待条件变量通知
		}

		Data popped_value = the_queue.front();			//获取队列中的最后一个任务
		the_queue.pop();								//删除该任务
		return popped_value;							//返回该任务
	}

};


void getNestedDictValue(dict d, string key1, string key2, int *value);

void getNestedDictChar(dict d, string key1, string key2, char *value);

void getNestedDictChar2(dict d, string key1, string key2, string key3, char *value, int index);

void getNestedDictValue2(dict d, string key1, string key2, string key3, int *value, int index);

//从字典中获取某个建值对应的整数，并赋值到请求结构体对象的值上
void getInt(dict d, string key, int *value);

void getUint64(dict d, string key, uint64_t *value);

void getUint32(dict d, string key, uint32_t *value);

void getInt64(dict d, string key, int64_t *value);



//从字典中获取某个建值对应的浮点数，并赋值到请求结构体对象的值上
void getDouble(dict d, string key, double* value);


//从字典中获取某个建值对应的字符，并赋值到请求结构体对象的值上
void getChar(dict d, string key, char* value);


//从字典中获取某个建值对应的字符串，并赋值到请求结构体对象的值上
void getStr(dict d, string key, char* value);


///-------------------------------------------------------------------------------------
///C++ SPI的回调函数方法实现
///-------------------------------------------------------------------------------------

//API的继承实现
class TraderApi : public XTP::API::TraderSpi
{
private:
	XTP::API::TraderApi* api;			//API对象
	boost::thread *task_thread;			//工作线程指针（向python中推送数据）
	ConcurrentQueue<Task*> task_queue;	//任务队列

public:
	TraderApi()
	{
		function0<void> f = boost::bind(&TraderApi::processTask, this);
		boost::thread t(f);
		this->task_thread = &t;
	};

	~TraderApi()
	{
	};

	//-------------------------------------------------------------------------------------
	//API回调函数
	//-------------------------------------------------------------------------------------

	///当客户端的某个连接与交易后台通信连接断开时，该方法被调用。
	///@param reason 错误原因，请与错误代码表对应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 用户主动调用logout导致的断线，不会触发此函数。api不会自动重连，当断线发生时，请用户自行选择后续操作，可以在此函数中调用Login重新登录，并更新session_id，此时用户收到的数据跟断线之前是连续的
	virtual void OnDisconnected(uint64_t session_id, int reason) ;

	///错误应答
	///@param error_info 当服务器响应发生错误时的具体的错误代码和错误信息,当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@remark 此函数只有在服务器发生错误时才会调用，一般无需用户处理
	virtual void OnError(XTPRI *error_info) ;

	///请求查询用户在本节点上可交易市场的响应
	///@param trade_location 查询到的交易市场信息，按位来看，从低位开始数，第0位表示沪市，即如果(trade_location&0x01) == 0x01，代表可交易沪市，第1位表示深市，即如果(trade_location&0x02) == 0x02，表示可交易深市，如果第0位和第1位均是1，即(trade_location&(0x01|0x02)) == 0x03，就表示可交易沪深2个市场
	///@param error_info 查询可交易市场发生错误时，返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 此查询只会有一个结果
	virtual void OnQueryAccountTradeMarket(int trade_location, XTPRI *error_info, int request_id, uint64_t session_id);

	///报单通知
	///@param order_info 订单响应具体信息，用户可以通过order_info.order_xtp_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单，order_info.qty_left字段在订单为未成交、部成、全成、废单状态时，表示此订单还没有成交的数量，在部撤、全撤状态时，表示此订单被撤的数量。order_info.order_cancel_xtp_id为其所对应的撤单ID，不为0时表示此单被撤成功
	///@param error_info 订单被拒绝或者发生错误时错误代码和错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@remark 每次订单状态更新时，都会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线，在订单未成交、全部成交、全部撤单、部分撤单、已拒绝这些状态时会有响应，对于部分成交的情况，请由订单的成交回报来自行确认。所有登录了此用户的客户端都将收到此用户的订单响应
	virtual void OnOrderEvent(XTPOrderInfo *order_info, XTPRI *error_info, uint64_t session_id) ;

	///成交通知
	///@param trade_info 成交回报的具体信息，用户可以通过trade_info.order_xtp_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单。对于上交所，exec_id可以唯一标识一笔成交。当发现2笔成交回报拥有相同的exec_id，则可以认为此笔交易自成交了。对于深交所，exec_id是唯一的，暂时无此判断机制。report_index+market字段可以组成唯一标识表示成交回报。
	///@remark 订单有成交发生的时候，会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。所有登录了此用户的客户端都将收到此用户的成交回报。相关订单为部成状态，需要用户通过成交回报的成交数量来确定，OnOrderEvent()不会推送部成状态。
	virtual void OnTradeEvent(XTPTradeReport *trade_info, uint64_t session_id) ;

	///撤单出错响应
	///@param cancel_info 撤单具体信息，包括撤单的order_cancel_xtp_id和待撤单的order_xtp_id
	///@param error_info 撤单被拒绝或者发生错误时错误代码和错误信息，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@remark 此响应只会在撤单发生错误时被回调
	virtual void OnCancelOrderError(XTPOrderCancelInfo *cancel_info, XTPRI *error_info, uint64_t session_id) ;

	///请求查询报单响应
	///@param order_info 查询到的一个报单
	///@param error_info 查询报单时发生错误时，返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 由于支持分时段查询，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryOrder(XTPQueryOrderRsp *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) ;

	///请求查询报单响应-新版本接口
	///@param order_info 查询到的一个报单信息
	///@param error_info 查询报单时发生错误时，返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 由于支持分时段查询，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryOrderEx(XTPOrderInfoEx *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///分页请求查询报单响应
	///@param order_info 查询到的一个报单
	///@param req_count 分页请求的最大数量
	///@param order_sequence 分页请求的当前回报数量
	///@param query_reference 当前报单信息所对应的查询索引，需要记录下来，在进行下一次分页查询的时候需要用到
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 当order_sequence为0，表明当次查询没有查到任何记录，当is_last为true时，如果order_sequence等于req_count，那么表示还有报单，可以进行下一次分页查询，如果不等，表示所有报单已经查询完毕。一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryOrderByPage(XTPQueryOrderRsp *order_info, int64_t req_count, int64_t order_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id);

	///分页请求查询报单响应-新版本接口
	///@param order_info 查询到的一个报单
	///@param req_count 分页请求的最大数量
	///@param order_sequence 分页请求的当前回报数量
	///@param query_reference 当前报单信息所对应的查询索引，需要记录下来，在进行下一次分页查询的时候需要用到
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 当order_sequence为0，表明当次查询没有查到任何记录，当is_last为true时，如果order_sequence等于req_count，那么表示还有报单，可以进行下一次分页查询，如果不等，表示所有报单已经查询完毕。一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryOrderByPageEx(XTPOrderInfoEx *order_info, int64_t req_count, int64_t order_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id);

	///请求查询成交响应
	///@param trade_info 查询到的一个成交回报
	///@param error_info 查询成交回报发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 由于支持分时段查询，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryTrade(XTPQueryTradeRsp *trade_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) ;

	///分页请求查询成交响应
	///@param trade_info 查询到的一个成交信息
	///@param req_count 分页请求的最大数量
	///@param trade_sequence 分页请求的当前回报数量
	///@param query_reference 当前报单信息所对应的查询索引，需要记录下来，在进行下一次分页查询的时候需要用到
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 当trade_sequence为0，表明当次查询没有查到任何记录，当is_last为true时，如果trade_sequence等于req_count，那么表示还有回报，可以进行下一次分页查询，如果不等，表示所有回报已经查询完毕。一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryTradeByPage(XTPQueryTradeRsp *trade_info, int64_t req_count, int64_t trade_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id);

	///请求查询投资者持仓响应
	///@param position 查询到的一只股票的持仓情况
	///@param error_info 查询账户持仓发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 由于用户可能持有多个股票，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryPosition(XTPQueryStkPositionRsp *position, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) ;

	///请求查询资金账户响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param asset 查询到的资金账户情况
	///@param error_info 查询资金账户发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryAsset(XTPQueryAssetRsp *asset, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) ;


	///请求查询分级基金信息响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param fund_info 查询到的分级基金情况
	///@param error_info 查询分级基金发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryStructuredFund(XTPStructuredFundInfo *fund_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询资金划拨订单响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param fund_transfer_info 查询到的资金账户情况
	///@param error_info 查询资金账户发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryFundTransfer(XTPFundTransferNotice *fund_transfer_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///资金划拨通知
	///@param fund_transfer_info 资金划拨通知的具体信息，用户可以通过fund_transfer_info.serial_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单。
	///@param error_info 资金划拨订单被拒绝或者发生错误时错误代码和错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@remark 当资金划拨订单有状态变化的时候，会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。所有登录了此用户的客户端都将收到此用户的资金划拨通知。
	virtual void OnFundTransfer(XTPFundTransferNotice *fund_transfer_info, XTPRI *error_info, uint64_t session_id);

	///请求查询ETF清单文件的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param etf_info 查询到的ETF清单文件情况
	///@param error_info 查询ETF清单文件发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryETF(XTPQueryETFBaseRsp *etf_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询ETF股票篮的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param etf_component_info 查询到的ETF合约的相关成分股信息
	///@param error_info 查询ETF股票篮发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryETFBasket(XTPQueryETFComponentRsp *etf_component_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询今日新股申购信息列表的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param ipo_info 查询到的今日新股申购的一只股票信息
	///@param error_info 查询今日新股申购信息列表发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryIPOInfoList(XTPQueryIPOTickerRsp *ipo_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询用户新股申购额度信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param quota_info 查询到的用户某个市场的今日新股申购额度信息
	///@param error_info 查查询用户新股申购额度信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryIPOQuotaInfo(XTPQueryIPOQuotaRsp *quota_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询今日可转债申购信息列表的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param ipo_info 查询到的今日可转债申购的一只可转债信息
	///@param error_info 查询今日可转债申购信息列表发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryBondIPOInfoList(XTPQueryIPOTickerRsp *ipo_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询用户可转债转股信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param swap_stock_info 查询到某条可转债转股信息
	///@param error_info 查查询可转债转股信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryBondSwapStockInfo(XTPQueryBondSwapStockRsp *swap_stock_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);
	
	///请求查询期权合约的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param option_info 查询到的期权合约情况
	///@param error_info 查询期权合约发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryOptionAuctionInfo(XTPQueryOptionAuctionInfoRsp *option_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///融资融券业务中现金直接还款的响应
	///@param cash_repay_info 现金直接还款通知的具体信息，用户可以通过cash_repay_info.xtp_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单。
	///@param error_info 现金还款发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnCreditCashRepay(XTPCrdCashRepayRsp *cash_repay_info, XTPRI *error_info, uint64_t session_id);

	///融资融券业务中现金还息的响应
	///@param cash_repay_info 现金还息通知的具体信息，用户可以通过cash_repay_info.xtp_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单。
	///@param error_info 现金还息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnCreditCashRepayDebtInterestFee(XTPCrdCashRepayDebtInterestFeeRsp *cash_repay_info, XTPRI *error_info, uint64_t session_id);

	///请求查询融资融券业务中的现金直接还款报单的响应
	///@param cash_repay_info 查询到的某一笔现金直接还款通知的具体信息
	///@param error_info 查询现金直接报单发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditCashRepayInfo(XTPCrdCashRepayInfo *cash_repay_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询信用账户额外信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param fund_info 查询到的信用账户额外信息情况
	///@param error_info 查询信用账户额外信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditFundInfo(XTPCrdFundInfo *fund_info, XTPRI *error_info, int request_id, uint64_t session_id) ;

	///请求查询信用账户负债信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param debt_info 查询到的信用账户合约负债情况
	///@param error_info 查询信用账户负债信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditDebtInfo(XTPCrdDebtInfo *debt_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) ;

	///请求查询信用账户指定证券负债未还信息响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param debt_info 查询到的信用账户指定证券负债未还信息情况
	///@param error_info 查询信用账户指定证券负债未还信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditTickerDebtInfo(XTPCrdDebtStockInfo *debt_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询信用账户待还资金的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param remain_amount 查询到的信用账户待还资金
	///@param error_info 查询信用账户待还资金发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditAssetDebtInfo(double remain_amount, XTPRI *error_info, int request_id, uint64_t session_id);

	///请求查询信用账户可融券头寸信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param assign_info 查询到的信用账户可融券头寸信息
	///@param error_info 查询信用账户可融券头寸信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditTickerAssignInfo(XTPClientQueryCrdPositionStkInfo *assign_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///融资融券业务中请求查询指定余券信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param stock_info 查询到的余券信息
	///@param error_info 查询信用账户余券信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditExcessStock(XTPClientQueryCrdSurplusStkRspInfo* stock_info, XTPRI *error_info, int request_id, uint64_t session_id);

	///融资融券业务中请求查询余券信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param stock_info 查询到的余券信息
	///@param error_info 查询信用账户余券信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryMulCreditExcessStock(XTPClientQueryCrdSurplusStkRspInfo* stock_info, XTPRI *error_info, int request_id, uint64_t session_id, bool is_last);

	///融资融券业务中负债合约展期的通知
	///@param debt_extend_info 负债合约展期通知的具体信息，用户可以通过debt_extend_info.xtpid来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单。
	///@param error_info 负债合约展期订单被拒绝或者发生错误时错误代码和错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误。
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 当负债合约展期订单有状态变化的时候，会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。所有登录了此用户的客户端都将收到此用户的负债合约展期通知。
	virtual void OnCreditExtendDebtDate(XTPCreditDebtExtendNotice *debt_extend_info, XTPRI *error_info, uint64_t session_id);

	///查询融资融券业务中负债合约展期订单响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param debt_extend_info 查询到的负债合约展期情况
	///@param error_info 查询负债合约展期发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误。当error_info.error_id=11000350时，表明没有记录，当为其他非0值时，表明合约发生拒单时的错误原因
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditExtendDebtDateOrders(XTPCreditDebtExtendNotice *debt_extend_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///查询融资融券业务中信用账户附加信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param fund_info 信用账户附加信息
	///@param error_info 查询信用账户附加信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditFundExtraInfo(XTPCrdFundExtraInfo *fund_info, XTPRI *error_info, int request_id, uint64_t session_id);

	///查询融资融券业务中信用账户指定证券的附加信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param fund_info 信用账户指定证券的附加信息
	///@param error_info 查询信用账户附加信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryCreditPositionExtraInfo(XTPCrdPositionExtraInfo *fund_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///期权组合策略报单通知
	///@param order_info 订单响应具体信息，用户可以通过order_info.order_xtp_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单，order_info.qty_left字段在订单为未成交、部成、全成、废单状态时，表示此订单还没有成交的数量，在部撤、全撤状态时，表示此订单被撤的数量。order_info.order_cancel_xtp_id为其所对应的撤单ID，不为0时表示此单被撤成功
	///@param error_info 订单被拒绝或者发生错误时错误代码和错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 每次订单状态更新时，都会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线，在订单未成交、全部成交、全部撤单、部分撤单、已拒绝这些状态时会有响应，对于部分成交的情况，请由订单的成交回报来自行确认。所有登录了此用户的客户端都将收到此用户的订单响应
	virtual void OnOptionCombinedOrderEvent(XTPOptCombOrderInfo *order_info, XTPRI *error_info, uint64_t session_id);

	///期权组合策略成交通知
	///@param trade_info 成交回报的具体信息，用户可以通过trade_info.order_xtp_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单。对于上交所，exec_id可以唯一标识一笔成交。当发现2笔成交回报拥有相同的exec_id，则可以认为此笔交易自成交了。对于深交所，exec_id是唯一的，暂时无此判断机制。report_index+market字段可以组成唯一标识表示成交回报。
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 订单有成交发生的时候，会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。所有登录了此用户的客户端都将收到此用户的成交回报。相关订单为部成状态，需要用户通过成交回报的成交数量来确定，OnOrderEvent()不会推送部成状态。
	virtual void OnOptionCombinedTradeEvent(XTPOptCombTradeReport *trade_info, uint64_t session_id);

	///期权组合策略撤单出错响应
	///@param cancel_info 撤单具体信息，包括撤单的order_cancel_xtp_id和待撤单的order_xtp_id
	///@param error_info 撤单被拒绝或者发生错误时错误代码和错误信息，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 此响应只会在撤单发生错误时被回调
	virtual void OnCancelOptionCombinedOrderError(XTPOptCombOrderCancelInfo *cancel_info, XTPRI *error_info, uint64_t session_id) ;

	///请求查询期权组合策略报单响应
	///@param order_info 查询到的一个报单
	///@param error_info 查询报单时发生错误时，返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 由于支持分时段查询，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。此对应的请求函数不建议轮询使用，当报单量过多时，容易造成用户线路拥堵，导致api断线
	virtual void OnQueryOptionCombinedOrders(XTPQueryOptCombOrderRsp *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) ;

	///请求查询期权组合策略报单响应-新版本接口
	///@param order_info 查询到的一个报单
	///@param error_info 查询报单时发生错误时，返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 由于支持分时段查询，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。此对应的请求函数不建议轮询使用，当报单量过多时，容易造成用户线路拥堵，导致api断线
	virtual void OnQueryOptionCombinedOrdersEx(XTPOptCombOrderInfoEx *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///分页请求查询期权组合策略报单响应
	///@param order_info 查询到的一个报单
	///@param req_count 分页请求的最大数量
	///@param order_sequence 分页请求的当前回报数量
	///@param query_reference 当前报单信息所对应的查询索引，需要记录下来，在进行下一次分页查询的时候需要用到
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 当order_sequence为0，表明当次查询没有查到任何记录，当is_last为true时，如果order_sequence等于req_count，那么表示还有报单，可以进行下一次分页查询，如果不等，表示所有报单已经查询完毕。一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryOptionCombinedOrdersByPage(XTPQueryOptCombOrderRsp *order_info, int64_t req_count, int64_t order_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id) ;

	///分页请求查询期权组合策略报单响应-新版本接口
	///@param order_info 查询到的一个报单
	///@param req_count 分页请求的最大数量
	///@param order_sequence 分页请求的当前回报数量
	///@param query_reference 当前报单信息所对应的查询索引，需要记录下来，在进行下一次分页查询的时候需要用到
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 当order_sequence为0，表明当次查询没有查到任何记录，当is_last为true时，如果order_sequence等于req_count，那么表示还有报单，可以进行下一次分页查询，如果不等，表示所有报单已经查询完毕。一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryOptionCombinedOrdersByPageEx(XTPOptCombOrderInfoEx *order_info, int64_t req_count, int64_t order_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id) ;

	///请求查询期权组合策略成交响应
	///@param trade_info 查询到的一个成交回报
	///@param error_info 查询成交回报发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 由于支持分时段查询，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。此对应的请求函数不建议轮询使用，当报单量过多时，容易造成用户线路拥堵，导致api断线
	virtual void OnQueryOptionCombinedTrades(XTPQueryOptCombTradeRsp *trade_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) ;

	///分页请求查询期权组合策略成交响应
	///@param trade_info 查询到的一个成交信息
	///@param req_count 分页请求的最大数量
	///@param trade_sequence 分页请求的当前回报数量
	///@param query_reference 当前报单信息所对应的查询索引，需要记录下来，在进行下一次分页查询的时候需要用到
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 当trade_sequence为0，表明当次查询没有查到任何记录，当is_last为true时，如果trade_sequence等于req_count，那么表示还有回报，可以进行下一次分页查询，如果不等，表示所有回报已经查询完毕。一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryOptionCombinedTradesByPage(XTPQueryOptCombTradeRsp *trade_info, int64_t req_count, int64_t trade_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id);

	///请求查询期权组合策略持仓响应
	///@param position_info 查询到的一个持仓信息
	///@param error_info 查询持仓发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryOptionCombinedPosition(XTPQueryOptCombPositionRsp *position_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询期权组合策略信息响应
	///@param strategy_info 查询到的一个组合策略信息
	///@param error_info 查询成交回报发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryOptionCombinedStrategyInfo(XTPQueryCombineStrategyInfoRsp *strategy_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///查询期权行权合并头寸的响应
	///@param position_info 查询到的一个行权合并头寸信息
	///@param error_info 查询持仓发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。
	virtual void OnQueryOptionCombinedExecPosition(XTPQueryOptCombExecPosRsp *position_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

	///请求查询其他节点可用资金的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	///@param fund_info 查询到的其他节点可用资金情况
	///@param error_info 查询其他节点可用资金发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryOtherServerFund(XTPFundQueryRsp *fund_info, XTPRI *error_info, int request_id, uint64_t session_id);


	////////////////////////////////////////////////////////////////
	/*****algo algo algo algo algo algo algo algo algo algo  ******/
	////////////////////////////////////////////////////////////////

	///algo业务中查询策略列表的响应
	///@param strategy_info 策略具体信息
	///@param strategy_param 此策略中包含的参数，如果error_info.error_id为0时，有意义
	///@param error_info 查询查询策略列表发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnQueryStrategy(XTPStrategyInfoStruct* strategy_info, char* strategy_param, XTPRI *error_info, int32_t request_id, bool is_last, uint64_t session_id);

	///algo业务中策略运行时策略状态通知
	///@param strategy_state 用户策略运行情况的状态通知
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnStrategyStateReport(XTPStrategyStateReportStruct* strategy_state, uint64_t session_id);

	///algo业务中用户建立算法通道的消息响应
	///@param user 用户名
	///@param error_info 建立算法通道发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误，即算法通道成功
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 算法通道建立成功后，才能对用户创建策略等操作，一个用户只能拥有一个算法通道，如果之前已经建立，则无需重复建立
	virtual void OnALGOUserEstablishChannel(char* user, XTPRI* error_info, uint64_t session_id);

	///algo业务中报送策略单的响应
	///@param strategy_info 用户报送的策略单的具体信息
	///@param error_info 报送策略单发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnInsertAlgoOrder(XTPStrategyInfoStruct* strategy_info, XTPRI *error_info, uint64_t session_id);

	///algo业务中撤销策略单的响应
	///@param strategy_info 用户撤销的策略单的具体信息
	///@param error_info 撤销策略单发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnCancelAlgoOrder(XTPStrategyInfoStruct* strategy_info, XTPRI *error_info, uint64_t session_id);

	///当客户端与AlgoBus通信连接断开时，该方法被调用。
	///@param reason 错误原因，请与错误代码表对应
	///@remark 请不要堵塞此线程，否则会影响algo的登录，与Algo之间的连接，断线后会自动重连，用户无需做其他操作
	virtual void OnAlgoDisconnected(int reason) ;

	///当客户端与AlgoBus断线后重新连接时，该方法被调用，仅在断线重连成功后会被调用。
	virtual void OnAlgoConnected();

	///algo业务中策略运行时策略指定证券执行状态通知
	///@param strategy_symbol_state 用户策略指定证券运行情况的状态通知
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnStrategySymbolStateReport(XTPStrategySymbolStateReport* strategy_symbol_state, uint64_t session_id) ;

	///algo业务中报送母单创建时的推送消息(包括其他客户端创建的母单)
	///@param strategy_info 策略具体信息
	///@param strategy_param 此策略中包含的参数
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnNewStrategyCreateReport(XTPStrategyInfoStruct* strategy_info, char* strategy_param, uint64_t session_id);

	///algo业务中算法推荐的响应
	///@param basket_flag 是否将满足条件的推荐结果打包成母单篮的标志，与请求一致，如果此参数为true，那么请以返回的strategy_param为准
	///@param recommendation_info 推荐算法的具体信息，当basket_flag=true时，此结构体中的market和ticker将没有意义，此时请以strategy_param为准
	///@param strategy_param 算法参数，可直接用来创建母单，如果error_info.error_id为0时，有意义
	///@param error_info 请求推荐算法发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnStrategyRecommendation(bool basket_flag, XTPStrategyRecommendationInfo* recommendation_info, char* strategy_param, XTPRI *error_info, int32_t request_id, bool is_last, uint64_t session_id);

	///algo业务中修改已有策略单的响应
	///@param strategy_info 用户修改后策略单的具体信息
	///@param error_info 修改策略单发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnModifyAlgoOrder(XTPStrategyInfoStruct* strategy_info, XTPRI *error_info, uint64_t session_id);	

	///algo业务中暂停指定策略指定证券算法单的响应
	///@param xtp_strategy_id xtp算法单策略ID
	///@param ticker_info 指定证券信息，可以为null，为null表示暂停指定策略中所有证券的算法单
	///@param error_info 修改策略单发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnPauseAlgoOrder(uint64_t xtp_strategy_id, XTPStrategyTickerInfo* ticker_info, XTPRI *error_info, int32_t request_id, uint64_t session_id);

	///algo业务中重启指定策略指定证券算法单的响应
	///@param xtp_strategy_id xtp算法单策略ID
	///@param ticker_info 指定证券信息，可以为null，为null表示重启指定策略中所有证券的算法单
	///@param error_info 修改策略单发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
	///@param request_id 此消息响应函数对应的请求ID
	///@param session_id 资金账户对应的session_id，登录时得到
	///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
	virtual void OnResumeAlgoOrder(uint64_t xtp_strategy_id, XTPStrategyTickerInfo* ticker_info, XTPRI *error_info, int32_t request_id, uint64_t session_id);


	//-------------------------------------------------------------------------------------
	//task：任务
	//-------------------------------------------------------------------------------------

	void processTask();

	void processDisconnected(Task *task);

	void processError(Task *task);

	void processQueryAccountTradeMarket(Task *task);

	void processOrderEvent(Task *task);

	void processTradeEvent(Task *task);

	void processCancelOrderError(Task *task);

	void processQueryOrder(Task *task);

	void processQueryOrderEx(Task *task);

	void processQueryOrderByPage(Task *task);

	void processQueryOrderByPageEx(Task *task);

	void processQueryTrade(Task *task);

	void processQueryTradeByPage(Task *task);

	void processQueryPosition(Task *task);

	void processQueryAsset(Task *task);

	void processQueryStructuredFund(Task *task);

	void processQueryFundTransfer(Task *task);

	void processFundTransfer(Task *task);

	void processQueryETF(Task *task);

	void processQueryETFBasket(Task *task);

	void processQueryIPOInfoList(Task *task);

	void processQueryIPOQuotaInfo(Task *task);

	void processQueryBondIPOInfoList(Task *task);

	void processQueryBondSwapStockInfo(Task *task);

	void processQueryOptionAuctionInfo(Task *task);

	void processCreditCashRepay(Task *task);

	void processQueryCreditCashRepayInfo(Task *task);

	void processQueryCreditFundInfo(Task *task);

	void processQueryCreditDebtInfo(Task *task);

	void processQueryCreditTickerDebtInfo(Task *task);

	void processQueryCreditAssetDebtInfo(Task *task);

	void processQueryCreditTickerAssignInfo(Task *task);

	void processQueryCreditExcessStock(Task *task);

	void processQueryMulCreditExcessStock(Task *task);

	void processCreditExtendDebtDate(Task *task);

	void processQueryCreditExtendDebtDateOrders(Task *task);

	void processQueryCreditFundExtraInfo(Task *task);

	void processQueryCreditPositionExtraInfo(Task *task);

	void processCreditCashRepayDebtInterestFee(Task *task);


	void processOptionCombinedOrderEvent(Task *task);

	void processOptionCombinedTradeEvent(Task *task);

	void processQueryOptionCombinedOrders(Task *task);

	void processQueryOptionCombinedOrdersEx(Task *task);

	void processQueryOptionCombinedOrdersByPage(Task *task);

	void processQueryOptionCombinedOrdersByPageEx(Task *task);

	void processQueryOptionCombinedTrades(Task *task);

	void processQueryOptionCombinedTradesByPage(Task *task);

	void processQueryOptionCombinedPosition(Task *task);

	void processQueryOptionCombinedStrategyInfo(Task *task);

	void processCancelOptionCombinedOrderError(Task *task);

	void processQueryOptionCombinedExecPosition(Task *task);

	void processQueryOtherServerFund(Task *task);

	//////////algo/////////
	void processQueryStrategy(Task *task);

	void processStrategyStateReport(Task *task);

	void processALGOUserEstablishChannel(Task *task);

	void processInsertAlgoOrder(Task *task);

	void processCancelAlgoOrder(Task *task);

	void processAlgoDisconnected(Task *task);

	void processAlgoConnected(Task *task);

	void processStrategySymbolStateReport(Task *task);
	
	void processNewStrategyCreateReport(Task *task);
	
	void processStrategyRecommendation(Task *task);   
	
	void processModifyAlgoOrder(Task *task);

	void processPauseAlgoOrder(Task *task);

	void processResumeAlgoOrder(Task *task);
	
	//-------------------------------------------------------------------------------------
	//data：回调函数的数据字典
	//error：回调函数的错误字典
	//reqid：请求id
	//last：是否为最后返回
	//-------------------------------------------------------------------------------------

	virtual void onDisconnected(uint64_t session, int reason) {};

	virtual void onError(dict data) {};

	virtual void onQueryAccountTradeMarket(int trade_location, dict error, int request_id, uint64_t session_id) {};

	virtual void onOrderEvent(dict data, dict error, uint64_t session) {};

	virtual void onTradeEvent(dict data, uint64_t session) {};

	virtual void onCancelOrderError(dict data, dict error, uint64_t session) {};

	virtual void onQueryOrder(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryOrderEx(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryTrade(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryPosition(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryAsset(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryStructuredFund(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryFundTransfer(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onFundTransfer(dict data, dict error, uint64_t session) {};

	virtual void onQueryETF(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryETFBasket(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryIPOInfoList(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryIPOQuotaInfo(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryBondIPOInfoList(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryBondSwapStockInfo(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onQueryOptionAuctionInfo(dict data, dict error, int reqid, bool last, uint64_t session) {};

	virtual void onCreditCashRepay(dict data, dict error_info, uint64_t session) {};

	virtual void onQueryCreditCashRepayInfo(dict data, dict error_info, int request_id, bool is_last, uint64_t session) {};

	virtual void onQueryCreditFundInfo(dict data, dict error_info, int request_id, uint64_t session) {};

	virtual void onQueryCreditDebtInfo(dict data, dict error_info, int request_id, bool is_last, uint64_t session) {};

	virtual void onQueryCreditTickerDebtInfo(dict data, dict error_info, int request_id, bool is_last, uint64_t session) {};

	virtual void onQueryCreditAssetDebtInfo(double remain_amount, dict error_info, int request_id, uint64_t session) {};

	virtual void onQueryCreditTickerAssignInfo(dict data, dict error_info, int request_id, bool is_last, uint64_t session) {};

	virtual void onQueryCreditExcessStock(dict data, dict error_info, int request_id, uint64_t session) {};

	virtual void onQueryMulCreditExcessStock(dict data, dict error_info, int request_id, uint64_t session, bool last) {};

	virtual void onCreditExtendDebtDate(dict data, dict error_info, uint64_t session) {};

	virtual void onQueryCreditExtendDebtDateOrders(dict data, dict error_info, int request_id, bool is_last, uint64_t session) {};

	virtual void onQueryCreditFundExtraInfo(dict data, dict error_info, int request_id, uint64_t session) {};

	virtual void onQueryCreditPositionExtraInfo(dict data, dict error_info, int request_id, bool is_last, uint64_t session) {};

	virtual void onQueryOrderByPage(dict data, int64_t req_count, int64_t order_sequence, int64_t query_reference, int reqid, bool last, uint64_t session) {};

	virtual void onQueryOrderByPageEx(dict data, int64_t req_count, int64_t order_sequence, int64_t query_reference, int reqid, bool last, uint64_t session) {};

	virtual void onQueryTradeByPage(dict data, int64_t req_count, int64_t trade_sequence, int64_t query_reference, int reqid, bool last, uint64_t session) {};
	
	virtual void onCreditCashRepayDebtInterestFee(dict data, dict error_info, uint64_t session) {};




	virtual void onOptionCombinedOrderEvent(dict data, dict error_info, uint64_t session_id) {};

	virtual void onOptionCombinedTradeEvent(dict data, uint64_t session_id) {};

	virtual void onQueryOptionCombinedOrders(dict data, dict error_info, int request_id, bool is_last, uint64_t session_id) {};

	virtual void onQueryOptionCombinedOrdersEx(dict data, dict error_info, int request_id, bool is_last, uint64_t session_id) {};

	virtual void onQueryOptionCombinedOrdersByPage(dict data, int64_t req_count, int64_t order_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id) {};

	virtual void onQueryOptionCombinedOrdersByPageEx(dict data, int64_t req_count, int64_t order_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id) {};

	virtual void onQueryOptionCombinedTrades(dict data, dict error_info, int request_id, bool is_last, uint64_t session_id) {};

	virtual void onQueryOptionCombinedTradesByPage(dict data, int64_t req_count, int64_t trade_sequence, int64_t query_reference, int request_id, bool is_last, uint64_t session_id) {};

	virtual void onQueryOptionCombinedPosition(dict data, dict error_info, int request_id, bool is_last, uint64_t session_id) {};

	virtual void onQueryOptionCombinedStrategyInfo(dict data, dict error_info, int request_id, bool is_last, uint64_t session_id) {};

	virtual void onCancelOptionCombinedOrderError(dict data, dict error_info, uint64_t session) {};

	virtual void onQueryOptionCombinedExecPosition(dict data, dict error_info, int request_id, bool is_last, uint64_t session) {};

	virtual void onQueryOtherServerFund(dict data, dict error_info, int request_id, uint64_t session) {};


	//////////////algo////////

	virtual void onQueryStrategy(dict data, string strategy_param, dict error_info, int32_t request_id, bool is_last, uint64_t session_id){};
	
	virtual void onStrategyStateReport(dict data,  uint64_t session_id){};

	virtual void onALGOUserEstablishChannel(string user,dict error_info, uint64_t session_id){};

	virtual void onInsertAlgoOrder(dict data,dict error_info, uint64_t session_id){};

	virtual void onCancelAlgoOrder(dict data,dict error_info, uint64_t session_id){};

	virtual void onAlgoDisconnected(int reason){};

	virtual void onAlgoConnected(){};

	virtual void onStrategySymbolStateReport(dict data, uint64_t session_id) {};
	
	virtual void onNewStrategyCreateReport(dict data, string strategy_param, uint64_t session_id) {};
	
	virtual void onStrategyRecommendation(bool basket_flag, dict data, string strategy_param, dict error_info, int32_t request_id, bool is_last, uint64_t session_id) {};
	
	virtual void onModifyAlgoOrder(dict data,dict error_info, uint64_t session_id) {};

	virtual void onPauseAlgoOrder(uint64_t xtp_strategy_id, dict data, dict error_info, int32_t request_id, uint64_t session_id) {};

	virtual void onResumeAlgoOrder(uint64_t xtp_strategy_id, dict data, dict error_info, int32_t request_id, uint64_t session_id) {};
	
	//-------------------------------------------------------------------------------------
	//req:主动函数的请求字典
	//-------------------------------------------------------------------------------------

	void createTraderApi(uint8_t clientid, string path, int log_level);

	void release();

	int exit();

	string getTradingDay();

	dict getApiLastError();

	string getApiVersion();

	uint8_t getClientIDByXTPID(uint64_t orderid);

	string getAccountByXTPID(uint64_t orderid);

	void subscribePublicTopic(int tpye);

	void setSoftwareKey(string key);

	void setSoftwareVersion(string version);

	void setHeartBeatInterval(uint32_t interval);

	uint64_t login(string ip, int port, string user, string password, int socktype,string local_ip);

	int logout(uint64_t sessionid);

	int modifyUserTerminalInfo(dict info, uint64_t session_id);

	int queryAccountTradeMarket(uint64_t session_id, int request_id);

	uint64_t getANewOrderXTPID(uint64_t session_id);

	uint64_t insertOrder(dict req, uint64_t sessionid);

	uint64_t insertOrderExtra(dict req, uint64_t session_id);

	uint64_t cancelOrder(uint64_t orderid, uint64_t sessionid);

	int queryOrderByXTPID(uint64_t orderid, uint64_t sessionid, int reqid);

	int queryOrderByXTPIDEx(uint64_t orderid, uint64_t sessionid, int reqid);

	int queryOrders(dict req, uint64_t sessionid, int reqid);
	
	int queryOrdersEx(dict req, uint64_t sessionid, int reqid);

	int queryUnfinishedOrders(uint64_t sessionid, int reqid);

	int queryUnfinishedOrdersEx(uint64_t sessionid, int reqid);

	int queryTradesByXTPID(uint64_t orderid, uint64_t sessionid, int reqid);

	int queryTrades(dict req, uint64_t sessionid, int reqid);

	int queryPosition(string ticker, uint64_t sessionid, int reqid);

	int queryAsset(uint64_t sessionid, int reqid);

	int queryStructuredFund(dict req, uint64_t sessionid, int reqid);

	uint64_t fundTransfer(dict req, uint64_t sessionid);

	int queryFundTransfer(dict req, uint64_t sessionid, int reqid);

	int queryETF(dict req, uint64_t sessionid, int reqid);

	int queryETFTickerBasket(dict req, uint64_t sessionid, int reqid);

	int queryIPOInfoList(uint64_t sessionid, int reqid);

	int queryIPOQuotaInfo(uint64_t sessionid, int reqid);

	int queryBondIPOInfoList(uint64_t session_id, int request_id);
	
	int queryBondSwapStockInfo(dict req, uint64_t session_id, int request_id);

	int queryOptionAuctionInfo(dict req,uint64_t session_id, int request_id);

	uint64_t creditCashRepay(double amount, uint64_t session_id);

	int queryCreditCashRepayInfo(uint64_t session_id, int request_id);

	int queryCreditFundInfo(uint64_t session_id, int request_id);

	int queryCreditDebtInfo(uint64_t session_id, int request_id);

	int queryCreditTickerDebtInfo(dict req, uint64_t session_id, int request_id);

	int queryCreditAssetDebtInfo(uint64_t session_id, int request_id);

	int queryCreditTickerAssignInfo(dict req, uint64_t session_id, int request_id);

	int queryCreditExcessStock(dict req, uint64_t session_id, int request_id);

	int queryMulCreditExcessStock(dict req, uint64_t session_id, int request_id);

	uint64_t creditExtendDebtDate(dict req, uint64_t session_id);

	int queryCreditExtendDebtDateOrders(uint64_t xtp_id,uint64_t session_id, int request_id);

	int queryCreditFundExtraInfo( uint64_t session_id, int request_id);

	int queryCreditPositionExtraInfo(dict req, uint64_t session_id, int request_id);

	int queryOrdersByPage(dict req, uint64_t sessionid, int reqid);

	int queryOrdersByPageEx(dict req, uint64_t sessionid, int reqid);

	int queryTradesByPage(dict req, uint64_t sessionid, int reqid);

	bool isServerRestart(uint64_t session_id);

	uint64_t creditCashRepayDebtInterestFee(string debt_id, double amount, uint64_t session_id);

	uint64_t creditSellStockRepayDebtInterestFee(dict req, string debt_id, uint64_t session_id);

	uint64_t insertOptionCombinedOrder(dict req, uint64_t session_id);

	uint64_t insertOptionCombinedOrderExtra(dict req, uint64_t session_id);

	int queryOptionCombinedUnfinishedOrders(uint64_t session_id, int request_id);

	int queryOptionCombinedUnfinishedOrdersEx(uint64_t session_id, int request_id);

	int queryOptionCombinedOrderByXTPID(const uint64_t order_xtp_id, uint64_t session_id, int request_id);

	int queryOptionCombinedOrderByXTPIDEx(const uint64_t order_xtp_id, uint64_t session_id, int request_id);

	int queryOptionCombinedOrders(dict req, uint64_t session_id, int request_id);

	int queryOptionCombinedOrdersEx(dict req, uint64_t session_id, int request_id);

	int queryOptionCombinedOrdersByPage(dict req, uint64_t session_id, int request_id);

	int queryOptionCombinedOrdersByPageEx(dict req, uint64_t session_id, int request_id);

	int queryOptionCombinedTradesByXTPID(const uint64_t order_xtp_id, uint64_t session_id, int request_id);

	int queryOptionCombinedTrades(dict req, uint64_t session_id, int request_id);

	int queryOptionCombinedTradesByPage(dict req, uint64_t session_id, int request_id);

	int queryOptionCombinedPosition(dict req, uint64_t session_id, int request_id);

	int queryOptionCombinedStrategyInfo(uint64_t session_id, int request_id);

	uint64_t cancelOptionCombinedOrder(const uint64_t order_xtp_id, uint64_t session_id);

	int queryOptionCombinedExecPosition(dict req, uint64_t session_id, int request_id);

	int queryOtherServerFund(dict req, uint64_t session_id, int request_id);

	/////////////////////algo///////////////////////
	
	int loginALGO(string ip, int port, string user, string password, int socktype,string local_ip);

	int queryStrategy(uint32_t strategy_type, uint64_t client_strategy_id, uint64_t xtp_strategy_id, uint64_t session_id, int32_t request_id);

	int aLGOUserEstablishChannel(string oms_ip, int oms_port, string user, string password, uint64_t session_id);

	int insertAlgoOrder(uint32_t strategy_type, uint64_t client_strategy_id, string strategy_param, uint64_t session_id);

	int cancelAlgoOrder(bool cancel_flag, uint64_t xtp_strategy_id, uint64_t session_id);

	uint64_t getAlgorithmIDByOrder(uint64_t order_xtp_id, uint32_t order_client_id);
	
	int strategyRecommendation(bool basket_flag, char* basket_param, uint64_t session_id, int32_t request_id);
	
	int modifyAlgoOrder(uint64_t xtp_strategy_id, char* strategy_param, uint64_t session_id);

	int pauseAlgoOrder(uint64_t xtp_strategy_id, dict req, uint64_t session_id, int32_t request_id);

	int resumeAlgoOrder(uint64_t xtp_strategy_id, dict req, uint64_t session_id, int32_t request_id);

};

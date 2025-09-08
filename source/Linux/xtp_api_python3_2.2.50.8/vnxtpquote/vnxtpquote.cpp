// vnctpmd.cpp : ���� DLL Ӧ�ó���ĵ���������
//
//#include "stdafx.h"
#include "vnxtpquote.h"

///-------------------------------------------------------------------------------------
///��Python����C++����ת���õĺ���
///-------------------------------------------------------------------------------------

void getInt(dict d, string key, int *value)
{
	if (d.has_key(key))		//����ֵ����Ƿ���ڸü�ֵ
	{
		object o = d[key];	//��ȡ�ü�ֵ
		extract<int> x(o);	//������ȡ��
		if (x.check())		//���������ȡ
		{
			*value = x();	//��Ŀ������ָ�븳ֵ
		}
	}
};

void getDouble(dict d, string key, double *value)
{
	if (d.has_key(key))
	{
		object o = d[key];
		extract<double> x(o);
		if (x.check())
		{
			*value = x();
		}
	}
};

void getInt64(dict d, string key, int64_t *value)
{
	if (d.has_key(key))		//����ֵ����Ƿ���ڸü�ֵ
	{
		object o = d[key];	//��ȡ�ü�ֵ
		extract<int64_t> x(o);	//������ȡ��
		if (x.check())		//���������ȡ
		{
			*value = x();	//��Ŀ������ָ�븳ֵ
		}
	}
};

void getInt16(dict d, string key, int16_t *value)
{
	if (d.has_key(key))		//����ֵ����Ƿ���ڸü�ֵ
	{
		object o = d[key];	//��ȡ�ü�ֵ
		extract<int> x(o);	//������ȡ��
		if (x.check())		//���������ȡ
		{
			*value = x();	//��Ŀ������ָ�븳ֵ
		}
	}
}

void getStr(dict d, string key, char *value)
{
	if (d.has_key(key))
	{
		object o = d[key];
		extract<string> x(o);
		if (x.check())
		{
			string s = x();
			const char *buffer = s.c_str();
			//���ַ���ָ�븳ֵ����ʹ��strcpy_s, vs2013ʹ��strcpy����ͨ����
			//+1Ӧ������ΪC++�ַ����Ľ�β���ţ������ر�ȷ�����������1�����
#ifdef _MSC_VER //WIN32
			strcpy_s(value, strlen(buffer) + 1, buffer);
#elif __GNUC__
			strncpy(value, buffer, strlen(buffer) + 1);
#endif
		}
	}
};

void getChar(dict d, string key, char *value)
{
	if (d.has_key(key))
	{
		object o = d[key];
		extract<string> x(o);
		if (x.check())
		{
			string s = x();
			const char *buffer = s.c_str();
			*value = *buffer;
		}
	}
};

string addEndingChar(char *value){
	string endStr = value;
	return endStr;
}

///-------------------------------------------------------------------------------------
///C++�Ļص����������ݱ��浽������
///-------------------------------------------------------------------------------------

void QuoteApi::OnDisconnected(int reason)
{
    Task* task = new Task();
    task->task_name = ONDISCONNECTED;
    task->task_id = reason;
    this->task_queue.push(task);
};

void QuoteApi::OnError(XTPRI *error_info)
{
    Task* task = new Task();
    task->task_name = ONERROR;

    if (error_info)
    {
        XTPRI *task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    this->task_queue.push(task);
};

void QuoteApi::OnTickByTickLossRange(int begin_seq, int end_seq)
{
	Task* task = new Task();
	task->task_name = ONTICKBYTICKLOSSRANGE;
	task->task_id = begin_seq;
	task->task_one_counts = end_seq;
	this->task_queue.push(task);
};

void QuoteApi::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
    Task* task = new Task();
    task->task_name = ONSUBMARKETDATA;

    if (ticker)
    {
        XTPST *task_data = new XTPST();
        *task_data = *ticker;
        task->task_data = task_data;
    }

    if (error_info)
    {
        XTPRI *task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->task_last = is_last;
    this->task_queue.push(task);
};

void QuoteApi::OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
    Task* task = new Task();
    task->task_name = ONUNSUBMARKETDATA;

    if (ticker)
    {
        XTPST *task_data = new XTPST();
        *task_data = *ticker;
        task->task_data = task_data;
    }

    if (error_info)
    {
        XTPRI *task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->task_last = is_last;
    this->task_queue.push(task);
};

void QuoteApi::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{
    Task* task = new Task();
    task->task_name = ONDEPTHMARKETDATA;

    if (market_data)
    {
        XTPMD *task_data = new XTPMD();
        *task_data = *market_data;
        task->task_data = task_data;
    }

	if (bid1_qty && bid1_count>0)
	{
		int64_t *task_data_one = new int64_t[bid1_count];
		for (int i=0;i<bid1_count;i++)
		{
			task_data_one[i]=bid1_qty[i];
		}
		task->task_data_one = task_data_one;
	}
	task->task_one_counts = bid1_count;
	task->task_one_all_counts = max_bid1_count;
	if (ask1_qty && ask1_count>0)
	{
		int64_t *task_data_two = new int64_t[ask1_count];
		for (int i=0;i<ask1_count;i++)
		{
			task_data_two[i]=ask1_qty[i];
		}
		task->task_data_two = task_data_two;
	}
	task->task_two_counts = ask1_count;
	task->task_two_all_counts =max_ask1_count;

    this->task_queue.push(task);
};

void QuoteApi::OnETFIOPVData(IOPV *iopv)
{
	Task* task = new Task();
	task->task_name = ONETFIOPVDATA;

	if (iopv)
	{
		IOPV *task_data = new IOPV();
		*task_data = *iopv;
		task->task_data = task_data;
	}
	
	this->task_queue.push(task);
};

void QuoteApi::OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
    Task* task = new Task();
    task->task_name = ONSUBORDERBOOK;

    if (ticker)
    {
        XTPST *task_data = new XTPST();
        *task_data = *ticker;
        task->task_data = task_data;
    }

    if (error_info)
    {
        XTPRI *task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->task_last = is_last;
    this->task_queue.push(task);
};

void QuoteApi::OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
    Task* task = new Task();
    task->task_name = ONUNSUBORDERBOOK;

    if (ticker)
    {
        XTPST *task_data = new XTPST();
        *task_data = *ticker;
        task->task_data = task_data;
    }

    if (error_info)
    {
        XTPRI *task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->task_last = is_last;
    this->task_queue.push(task);
};

void QuoteApi::OnOrderBook(XTPOB *order_book)
{
    Task* task = new Task();
    task->task_name = ONORDERBOOK;

    if (order_book)
    {
        XTPOB *task_data = new XTPOB();
        *task_data = *order_book;
        task->task_data = task_data;
    }
    this->task_queue.push(task);
};

void QuoteApi::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) 
{
	Task* task = new Task();
	task->task_name = ONSUBTICKBYTICK;

	if (ticker)
	{
		XTPST *task_data = new XTPST();
		*task_data = *ticker;
		task->task_data = task_data;
	}

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}

	task->task_last = is_last;
	this->task_queue.push(task);
};

void QuoteApi::OnUnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	Task* task = new Task();
	task->task_name = ONUNSUBTICKBYTICK;

	if (ticker)
	{
		XTPST *task_data = new XTPST();
		*task_data = *ticker;
		task->task_data = task_data;
	}

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}

	task->task_last = is_last;
	this->task_queue.push(task);
};

void QuoteApi::OnTickByTick(XTPTBT *tbt_data)
{
	Task* task = new Task();
	task->task_name = ONTICKBYTICK;

	if (tbt_data)
	{
		XTPTBT *task_data = new XTPTBT();
		*task_data = *tbt_data;
		task->task_data = task_data;
	}

	this->task_queue.push(task);
};

void QuoteApi::OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchage_id, XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONSUBSCRIBEALLMARKETDATA;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnUnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchage_id, XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONUNSUBSCRIBEALLMARKETDATA;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchage_id,XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONSUBSCRIBEALLORDERBOOK;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};


void QuoteApi::OnUnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchage_id,XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONUNSUBSCRIBEALLORDERBOOK;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};


void QuoteApi::OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchage_id,XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONSUBSCRIBEALLTICKBYTICK;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnUnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchage_id,XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONUNSUBSCRIBEALLTICKBYTICK;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnQueryAllTickers(XTPQSI* ticker_info, XTPRI *error_info, bool is_last)
{
    Task* task = new Task();
    task->task_name = ONQUERYALLTICKERS;

    if (ticker_info)
    {
        XTPQSI *task_data = new XTPQSI();
        *task_data = *ticker_info;
        task->task_data = task_data;
    }

    if (error_info)
    {
        XTPRI *task_error = new XTPRI();
        *task_error = *error_info;
        task->task_error = task_error;
    }

    task->task_last = is_last;
    this->task_queue.push(task);
};

void QuoteApi::OnQueryTickersPriceInfo(XTPTPI* ticker_info, XTPRI *error_info, bool is_last)
{
	Task* task = new Task();
	task->task_name = ONQUERYTICKERSPRICEINFO;

	if (ticker_info)
	{
		XTPTPI *task_data = new XTPTPI();
		*task_data = *ticker_info;
		task->task_data = task_data;
	}

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}

	task->task_last = is_last;
	this->task_queue.push(task);
};


void QuoteApi::OnQueryAllTickersFullInfo(XTPQFI * ticker_info, XTPRI * error_info, bool is_last) {

	Task* task = new Task();
	task->task_name = ONQUERYALLTICKERSFULLINFO;

	if (ticker_info)
	{
		XTPQFI *task_data = new XTPQFI();
		*task_data = *ticker_info;
		task->task_data = task_data;
	}

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}

	task->task_last = is_last;
	this->task_queue.push(task);
}

void QuoteApi::OnQueryAllNQTickersFullInfo(XTPNQFI* ticker_info, XTPRI *error_info, bool is_last)
{
	Task* task = new Task();
	task->task_name = ONQUERYALLNQTICKERSFULLINFO;

	if (ticker_info)
	{
		XTPNQFI *task_data = new XTPNQFI();
		*task_data = *ticker_info;
		task->task_data = task_data;
	}

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}

	task->task_last = is_last;
	this->task_queue.push(task);
}

void QuoteApi::OnRebuildQuoteServerDisconnected(int reason)
{
	Task* task = new Task();
	task->task_name = ONREBUILDQUOTESERVERDISCONNECTED;

	task->task_id = reason;
	this->task_queue.push(task);
}

void QuoteApi::OnRequestRebuildQuote(XTPQuoteRebuildResultRsp* rebuild_result)
{
	Task* task = new Task();
	task->task_name = ONREQUESTREBUILDQUOTE;
	if (rebuild_result)
	{
		XTPQuoteRebuildResultRsp* task_data = new XTPQuoteRebuildResultRsp();
		*task_data = *rebuild_result;
		task->task_data = task_data;
	}
	this->task_queue.push(task);
}

void QuoteApi::OnRebuildTickByTick(XTPTBT *tbt_data)
{
	Task* task = new Task();
	task->task_name = ONREBUILDTICKBYTICK;

	if (tbt_data)
	{
		XTPTBT *task_data = new XTPTBT();
		*task_data = *tbt_data;
		task->task_data = task_data;
	}

	this->task_queue.push(task);
}

void QuoteApi::OnRebuildMarketData(XTPMD *md_data)
{
	Task* task = new Task();
	task->task_name = ONREBUILDMARKETDATA;
	if (md_data)
	{
		XTPMD *task_data = new XTPMD();
		*task_data = *md_data;
		task->task_data = task_data;
	}
	this->task_queue.push(task);
}


void QuoteApi::OnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchage_id, XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONSUBSCRIBEALLOPTIONMARKETDATA;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnUnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchage_id, XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONUNSUBSCRIBEALLOPTIONMARKETDATA;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchage_id, XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONSUBSCRIBEALLOPTIONORDERBOOK;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnUnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchage_id, XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONUNSUBSCRIBEALLOPTIONORDERBOOK;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchage_id, XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONSUBSCRIBEALLOPTIONTICKBYTICK;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};

void QuoteApi::OnUnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchage_id, XTPRI *error_info)
{
	Task* task = new Task();
	task->task_name = ONUNSUBSCRIBEALLOPTIONTICKBYTICK;

	if (error_info)
	{
		XTPRI *task_error = new XTPRI();
		*task_error = *error_info;
		task->task_error = task_error;
	}
	task->exchange_id = exchage_id;
	this->task_queue.push(task);
};
///-------------------------------------------------------------------------------------
///�����̴߳Ӷ�����ȡ�����ݣ�ת��Ϊpython����󣬽�������
///-------------------------------------------------------------------------------------

void QuoteApi::processTask()
{
	while (1)
	{
		Task* task = this->task_queue.wait_and_pop();

		switch (task->task_name)
		{
			case ONDISCONNECTED:
			{
			    this->processDisconnected(task);
			    break;
			}

			case ONERROR:
			{
			    this->processError(task);
			    break;
			}

			case ONSUBMARKETDATA:
			{
			    this->processSubMarketData(task);
			    break;
			}

			case ONUNSUBMARKETDATA:
			{
			    this->processUnSubMarketData(task);
			    break;
			}

			case ONDEPTHMARKETDATA:
			{
			    this->processDepthMarketData(task);
			    break;
			}

			case ONSUBORDERBOOK:
			{
			    this->processSubOrderBook(task);
			    break;
			}

			case ONUNSUBORDERBOOK:
			{
			    this->processUnSubOrderBook(task);
			    break;
			}

			case ONORDERBOOK:
			{
			    this->processOrderBook(task);
			    break;
			}

			case ONSUBTICKBYTICK:
			{
				this->processSubTickByTick(task);
				break;
			}

			case ONUNSUBTICKBYTICK:
			{
				this->processUnSubTickByTick(task);
				break;
			}

			case ONTICKBYTICK:
			{
				this->processTickByTick(task);
				break;
			}

			case ONSUBSCRIBEALLMARKETDATA:
			{
				this->processSubscribeAllMarketData(task);
				break;
			}

			case ONUNSUBSCRIBEALLMARKETDATA:
			{
				this->processUnSubscribeAllMarketData(task);
				break;
			}

			case ONSUBSCRIBEALLORDERBOOK:
			{
				this->processSubscribeAllOrderBook(task);
				break;
			}

			case ONUNSUBSCRIBEALLORDERBOOK:
			{
				this->processUnSubscribeAllOrderBook(task);
				break;
			}

			case ONSUBSCRIBEALLTICKBYTICK:
			{
				this->processSubscribeAllTickByTick(task);
				break;
			}

			case ONUNSUBSCRIBEALLTICKBYTICK:
			{
				this->processUnSubscribeAllTickByTick(task);
				break;
			}

			case ONQUERYALLTICKERS:
			{
			    this->processQueryAllTickers(task);
			    break;
			}

			case ONQUERYTICKERSPRICEINFO:
			{
				this->processQueryTickersPriceInfo(task);
				break;
			}



			case ONSUBSCRIBEALLOPTIONMARKETDATA:
				{
					this->processSubscribeAllOptionMarketData(task);
					break;
				}
			case ONUNSUBSCRIBEALLOPTIONMARKETDATA:
				{
					this->processUnSubscribeAllOptionMarketData(task);
					break;
				}
			case ONSUBSCRIBEALLOPTIONORDERBOOK:
				{
					this->processSubscribeAllOptionOrderBook(task);
					break;
				}
			case ONUNSUBSCRIBEALLOPTIONORDERBOOK:
				{
					this->processUnSubscribeAllOptionOrderBook(task);
					break;
				}
			case ONSUBSCRIBEALLOPTIONTICKBYTICK:
				{
					this->processSubscribeAllOptionTickByTick(task);
					break;
				}
			case ONUNSUBSCRIBEALLOPTIONTICKBYTICK:
				{
					this->processUnSubscribeAllOptionTickByTick(task);
					break;
				}
			case ONQUERYALLTICKERSFULLINFO: {
				this->processQueryAllTickersFullInfo(task);
				break;
											}
			case ONREBUILDQUOTESERVERDISCONNECTED:
			{
				this->processRebuildQuoteServerDisconnected(task);
				break;
			}
			case ONREQUESTREBUILDQUOTE:
			{
				this->processRequestRebuildQuote(task);
				break;
			}
			case ONREBUILDTICKBYTICK:
			{
				this->processRebuildTickByTick(task);
				break;
			}
			case ONREBUILDMARKETDATA:
			{
				this->processRebuildMarketData(task);
				break;
			}
			case ONQUERYALLNQTICKERSFULLINFO: 
			{
				this->processQueryAllNQTickersFullInfo(task);
				break;
			}

			case ONTICKBYTICKLOSSRANGE:
			{
				this->processTickByTickLossRange(task);
				break;
			}

			case ONETFIOPVDATA:
			{
				this->processETFIOPVData(task);
				break;
			}
		};
	}
};



void QuoteApi::processDisconnected(Task *task)
{
    PyLock lock;
    this->onDisconnected(task->task_id);
    delete task;
};

void QuoteApi::processError(Task *task)
{
    PyLock lock;
    dict error;
    if (task->task_error)
    {
        XTPRI *task_error = (XTPRI*) task->task_error;
        error["error_id"] = task_error->error_id;
        error["error_msg"] = addEndingChar(task_error->error_msg);
        delete task->task_error;
    }

    this->onError(error);
    delete task;
};

void QuoteApi::processTickByTickLossRange(Task *task)
{
	PyLock lock;
	this->onTickByTickLossRange(task->task_id, task->task_one_counts);
	delete task;
};

void QuoteApi::processSubMarketData(Task *task)
{
    PyLock lock;
    dict data;
    if (task->task_data)
    {
        XTPST *task_data = (XTPST*) task->task_data;
        data["exchange_id"] = (int)task_data->exchange_id;
        data["ticker"] = addEndingChar(task_data->ticker);
        delete task->task_data;
    }

    dict error;
    if (task->task_error)
    {
        XTPRI *task_error = (XTPRI*) task->task_error;
        error["error_id"] = task_error->error_id;
        error["error_msg"] = addEndingChar(task_error->error_msg);
        delete task->task_error;
    }

    this->onSubMarketData(data, error, task->task_last);
    delete task;
};

void QuoteApi::processUnSubMarketData(Task *task)
{
    PyLock lock;
    dict data;
    if (task->task_data)
    {
        XTPST *task_data = (XTPST*) task->task_data;
        data["exchange_id"] = (int)task_data->exchange_id;
        data["ticker"] = addEndingChar(task_data->ticker);
        delete task->task_data;
    }

    dict error;
    if (task->task_error)
    {
        XTPRI *task_error = (XTPRI*) task->task_error;
        error["error_id"] = task_error->error_id;
        error["error_msg"] = addEndingChar(task_error->error_msg);
        delete task->task_error;
    }

    this->onUnSubMarketData(data, error, task->task_last);
    delete task;
};

void QuoteApi::processDepthMarketData(Task *task)
{
    PyLock lock;
    dict data;
    if (task->task_data)
    {
			XTPMD *task_data = (XTPMD*)task->task_data;
			data["exchange_id"] = (int)task_data->exchange_id;
			data["ticker"] = addEndingChar(task_data->ticker);
			data["last_price"] = task_data->last_price;
			data["pre_close_price"] = task_data->pre_close_price;
			data["open_price"] = task_data->open_price;
			data["high_price"] = task_data->high_price;
			data["low_price"] = task_data->low_price;
			data["close_price"] = task_data->close_price;

			data["pre_total_long_positon"] = task_data->pre_total_long_positon;
			data["total_long_positon"] = task_data->total_long_positon;
			data["pre_settl_price"] = task_data->pre_settl_price;
			data["settl_price"] = task_data->settl_price;
			
			data["upper_limit_price"] = task_data->upper_limit_price;
			data["lower_limit_price"] = task_data->lower_limit_price;
			data["pre_delta"] = task_data->pre_delta;
			data["curr_delta"] = task_data->curr_delta;
			
			data["data_time"] = task_data->data_time;
			
			data["qty"] = task_data->qty;
			data["turnover"] = task_data->turnover;
			data["avg_price"] = task_data->avg_price;

			data["trades_count"] = task_data->trades_count;
			char str_ticker_status[9] = {"\0"};
#ifdef _MSC_VER //WIN32
			strncpy(str_ticker_status,  task_data->ticker_status,sizeof(task_data->ticker_status));
#elif __GNUC__
			strncpy(str_ticker_status, task_data->ticker_status, sizeof(task_data->ticker_status));
#endif
			data["ticker_status"] = addEndingChar(str_ticker_status);

			boost::python::list ask;
			boost::python::list bid;
			boost::python::list ask_qty;
			boost::python::list bid_qty;

			for (int i = 0; i < 10; i++)
			{
				ask.append(task_data->ask[i]);
				bid.append(task_data->bid[i]);
				ask_qty.append(task_data->ask_qty[i]);
				bid_qty.append(task_data->bid_qty[i]);
			}

			data["ask"] = ask;
			data["bid"] = bid;
			data["bid_qty"] = bid_qty;
			data["ask_qty"] = ask_qty;

			data["data_type"] = (int)task_data->data_type;
			data["data_type_v2"] = (int)task_data->data_type_v2;
			if (task_data->data_type_v2 == XTP_MARKETDATA_V2_ACTUAL){
				data["total_bid_qty"] = task_data->stk.total_bid_qty;
				data["total_ask_qty"] = task_data->stk.total_ask_qty;
				data["ma_bid_price"] = task_data->stk.ma_bid_price;
				data["ma_ask_price"] = task_data->stk.ma_ask_price;
				data["ma_bond_bid_price"] = task_data->stk.ma_bond_bid_price;
				data["ma_bond_ask_price"] = task_data->stk.ma_bond_ask_price;
				data["yield_to_maturity"] = task_data->stk.yield_to_maturity;
				data["iopv"] = task_data->stk.iopv;
				data["etf_buy_count"] = task_data->stk.etf_buy_count;
				data["etf_sell_count"] = task_data->stk.etf_sell_count;
				data["etf_buy_qty"] = task_data->stk.etf_buy_qty;
				data["etf_buy_money"] = task_data->stk.etf_buy_money;
				data["etf_sell_qty"] = task_data->stk.etf_sell_qty;
				data["etf_sell_money"] = task_data->stk.etf_sell_money;
				data["total_warrant_exec_qty"] = task_data->stk.total_warrant_exec_qty;
				data["warrant_lower_price"] = task_data->stk.warrant_lower_price;
				data["warrant_upper_price"] = task_data->stk.warrant_upper_price;
				data["cancel_buy_count"] = task_data->stk.cancel_buy_count;
				data["cancel_sell_count"] = task_data->stk.cancel_sell_count;
				data["cancel_buy_qty"] = task_data->stk.cancel_buy_qty;
				data["cancel_sell_qty"] = task_data->stk.cancel_sell_qty;
				data["cancel_buy_money"] = task_data->stk.cancel_buy_money;
				data["cancel_sell_money"] = task_data->stk.cancel_sell_money;
				data["total_buy_count"] = task_data->stk.total_buy_count;
				data["total_sell_count"] = task_data->stk.total_sell_count;
				data["duration_after_buy"] = task_data->stk.duration_after_buy;
				data["duration_after_sell"] = task_data->stk.duration_after_sell;
				data["num_bid_orders"] = task_data->stk.num_bid_orders;
				data["num_ask_orders"] = task_data->stk.num_ask_orders;
				data["pre_iopv"] = task_data->stk.pre_iopv;
				data["r1"] = task_data->stk.r1;
				data["r2"] = task_data->stk.r2;
			}else if (task_data->data_type_v2 == XTP_MARKETDATA_V2_OPTION){
				data["auction_price"] = task_data->opt.auction_price;
				data["auction_qty"] = task_data->opt.auction_qty;
				data["last_enquiry_time"] = task_data->opt.last_enquiry_time;
			}
			else if (task_data->data_type_v2 == XTP_MARKETDATA_V2_BOND)
			{
				data["total_bid_qty"] = task_data->bond.total_bid_qty;
				data["total_ask_qty"] = task_data->bond.total_ask_qty;
				data["ma_bid_price"] = task_data->bond.ma_bid_price;
				data["ma_ask_price"] = task_data->bond.ma_ask_price;
				data["ma_bond_bid_price"] = task_data->bond.ma_bond_bid_price;
				data["ma_bond_ask_price"] = task_data->bond.ma_bond_ask_price;
				data["yield_to_maturity"] = task_data->bond.yield_to_maturity;
				data["match_lastpx"] = task_data->bond.match_lastpx;
				data["ma_bond_price"] = task_data->bond.ma_bond_price;
				data["match_qty"] = task_data->bond.match_qty;
				data["match_turnover"] = task_data->bond.match_turnover;
				data["r4"] = task_data->bond.r4;
				data["r5"] = task_data->bond.r5;
				data["r6"] = task_data->bond.r6;
				data["r7"] = task_data->bond.r7;
				data["r8"] = task_data->bond.r8;
				data["cancel_buy_count"] = task_data->bond.cancel_buy_count;
				data["cancel_sell_count"] = task_data->bond.cancel_sell_count;
				data["cancel_buy_qty"] = task_data->bond.cancel_buy_qty;
				data["cancel_sell_qty"] = task_data->bond.cancel_sell_qty;
				data["cancel_buy_money"] = task_data->bond.cancel_buy_money;
				data["cancel_sell_money"] = task_data->bond.cancel_sell_money;
				data["total_buy_count"] = task_data->bond.total_buy_count;
				data["total_sell_count"] = task_data->bond.total_sell_count;
				data["duration_after_buy"] = task_data->bond.duration_after_buy;
				data["duration_after_sell"] = task_data->bond.duration_after_sell;
				data["num_bid_orders"] = task_data->bond.num_bid_orders;
				data["num_ask_orders"] = task_data->bond.num_ask_orders;
				data["instrument_status"] = addEndingChar(task_data->bond.instrument_status);
			}

			//data["r4"] = task_data->r4;

			delete task->task_data;
    }
	boost::python::list bid1_qty_list;
	if (task->task_data_one && task->task_one_counts>0)
	{
		for (int i=0;i<task->task_one_counts;i++)
		{
			int64_t *bid1_qty = (int64_t *)task->task_data_one+i;
			bid1_qty_list.append(*bid1_qty);
		}
		delete[] task->task_data_one;
	}
	int  bid1_count= task->task_one_counts;
	int  max_bid1_count= task->task_one_all_counts;

	boost::python::list ask1_qty_list;
	if (task->task_data_two && task->task_two_counts>0)
	{
		for (int i=0;i<task->task_two_counts;i++)
		{
			int64_t *ask1_qty = (int64_t *)task->task_data_two+i;
			ask1_qty_list.append(*ask1_qty);
		}
		delete[] task->task_data_two;
	}
	int  ask1_count= task->task_two_counts;
	int  max_ask1_count= task->task_two_all_counts;
    this->onDepthMarketData(data,bid1_qty_list,bid1_count,max_bid1_count,ask1_qty_list,ask1_count,max_ask1_count);
    delete task;
};

void QuoteApi::processETFIOPVData(Task *task)
{
	PyLock lock;
	dict data;
	if (task->task_data)
	{
		IOPV *task_data = (IOPV*)task->task_data;
		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		data["data_time"] = task_data->data_time;
		data["iopv"] = task_data->iopv;
		delete task->task_data;
	}	

	this->onETFIOPVData(data);
	delete task;
};

void QuoteApi::processSubOrderBook(Task *task)
{
    PyLock lock;
    dict data;
    if (task->task_data)
    {
        XTPST *task_data = (XTPST*) task->task_data;
        data["exchange_id"] = (int)task_data->exchange_id;
        data["ticker"] = addEndingChar(task_data->ticker);
        delete task->task_data;
    }

    dict error;
    if (task->task_error)
    {
        XTPRI *task_error = (XTPRI*) task->task_error;
        error["error_id"] = task_error->error_id;
        error["error_msg"] = addEndingChar(task_error->error_msg);
        delete task->task_error;
    }

    this->onSubOrderBook(data, error, task->task_last);
    delete task;
};

void QuoteApi::processUnSubOrderBook(Task *task)
{
    PyLock lock;
    dict data;
    if (task->task_data)
    {
        XTPST *task_data = (XTPST*) task->task_data;
        data["exchange_id"] = (int)task_data->exchange_id;
        data["ticker"] = addEndingChar(task_data->ticker);
        delete task->task_data;
    }

    dict error;
    if (task->task_error)
    {
        XTPRI *task_error = (XTPRI*) task->task_error;
        error["error_id"] = task_error->error_id;
        error["error_msg"] = addEndingChar(task_error->error_msg);
        delete task->task_error;
    }

    this->onUnSubOrderBook(data, error, task->task_last);
    delete task;
};

void QuoteApi::processOrderBook(Task *task)
{
    PyLock lock;
    dict data;
    if (task->task_data)
    {
        XTPOB *task_data = (XTPOB*) task->task_data;
				data["exchange_id"] = (int)task_data->exchange_id;
				data["ticker"] = addEndingChar(task_data->ticker);
				data["data_time"] = task_data->data_time;

				data["last_price"] = task_data->last_price;
				data["qty"] = task_data->qty;
				data["turnover"] = task_data->turnover;
				data["trades_count"] = task_data->trades_count;

				boost::python::list ask;
				boost::python::list bid;
				boost::python::list ask_qty;
				boost::python::list bid_qty;

				for (int i = 0; i < 10; i++)
				{
					ask.append(task_data->ask[i]);
					bid.append(task_data->bid[i]);
					ask_qty.append(task_data->ask_qty[i]);
					bid_qty.append(task_data->bid_qty[i]);
				}

				data["ask"] = ask;
				data["bid"] = bid;
				data["bid_qty"] = bid_qty;
				data["ask_qty"] = ask_qty;
        delete task->task_data;
    }

    this->onOrderBook(data);
    delete task;
};

void QuoteApi::processSubTickByTick(Task *task)
{
	PyLock lock;
	dict data;
	if (task->task_data)
	{
		XTPST *task_data = (XTPST*)task->task_data;
		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		delete task->task_data;
	}

	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onSubTickByTick(data, error, task->task_last);
	delete task;
};

void QuoteApi::processUnSubTickByTick(Task *task)
{
	PyLock lock;
	dict data;
	if (task->task_data)
	{
		XTPST *task_data = (XTPST*)task->task_data;
		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		delete task->task_data;
	}

	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onUnSubTickByTick(data, error, task->task_last);
	delete task;
};

void QuoteApi::processTickByTick(Task *task)
{
	PyLock lock;
	dict data;
	if (task->task_data)
	{
		XTPTBT *task_data = (XTPTBT*)task->task_data;


		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		data["struct_seq"] = task_data->seq;
		data["data_time"] = task_data->data_time;
		data["type"] = (int)task_data->type;

		if (task_data->type == XTP_TBT_ENTRUST)
		{
			data["channel_no"] = task_data->entrust.channel_no;
			data["seq"] = task_data->entrust.seq;
			data["price"] = task_data->entrust.price;
			data["qty"] = task_data->entrust.qty;
			data["side"] = task_data->entrust.side;
			data["ord_type"] = task_data->entrust.ord_type;
			data["order_no"] = task_data->entrust.order_no;
		}
		else if(task_data->type == XTP_TBT_TRADE)
		{
			data["channel_no"] = task_data->trade.channel_no;
			data["seq"] = task_data->trade.seq;
			data["price"] = task_data->trade.price;
			data["qty"] = task_data->trade.qty;
			data["money"] = task_data->trade.money;
			data["bid_no"] = task_data->trade.bid_no;
			data["ask_no"] = task_data->trade.ask_no;
			data["trade_flag"] = task_data->trade.trade_flag;
		}
		else
		{
			data["channel_no"] = task_data->state.channel_no;
			data["seq"] = task_data->state.seq;
			data["flag"] = addEndingChar(task_data->state.flag);
		}

		delete task->task_data;
	}

	this->onTickByTick(data);
	delete task;
};

void QuoteApi::processSubscribeAllMarketData(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}
	this->onSubscribeAllMarketData(task->exchange_id,error);
	delete task;
};

void QuoteApi::processUnSubscribeAllMarketData(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onUnSubscribeAllMarketData(task->exchange_id,error);
	delete task;
};

void QuoteApi::processSubscribeAllOrderBook(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onSubscribeAllOrderBook(task->exchange_id,error);
	delete task;
};

void QuoteApi::processUnSubscribeAllOrderBook(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onUnSubscribeAllOrderBook(task->exchange_id,error);
	delete task;
};

void QuoteApi::processSubscribeAllTickByTick(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onSubscribeAllTickByTick(task->exchange_id,error);
	delete task;
};

void QuoteApi::processUnSubscribeAllTickByTick(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onUnSubscribeAllTickByTick(task->exchange_id,error);
	delete task;
};

void QuoteApi::processQueryAllTickers(Task *task)
{
    PyLock lock;

		//�ֶ��޸�
		dict data;
		if (task->task_data)
		{
			XTPQSI *task_data = (XTPQSI*)task->task_data;
			data["exchange_id"] = (int)task_data->exchange_id;
			data["ticker"] = addEndingChar(task_data->ticker);
			data["ticker_name"] = addEndingChar(task_data->ticker_name);
			data["ticker_type"] = (int)task_data->ticker_type;
			data["pre_close_price"] = task_data->pre_close_price;
			data["upper_limit_price"] = task_data->upper_limit_price;
			data["lower_limit_price"] = task_data->lower_limit_price;
			data["price_tick"] = task_data->price_tick;
			data["buy_qty_unit"] = task_data->buy_qty_unit;
			data["sell_qty_unit"] = task_data->sell_qty_unit;

			delete task->task_data;
		}

    dict error;
    if (task->task_error)
    {
        XTPRI *task_error = (XTPRI*) task->task_error;
        error["error_id"] = task_error->error_id;
        error["error_msg"] = addEndingChar(task_error->error_msg);
        delete task->task_error;
    }

    this->onQueryAllTickers(data, error, task->task_last);
    delete task;
};

void QuoteApi::processQueryTickersPriceInfo(Task *task)
{
	PyLock lock;

	//�ֶ��޸�
	dict data;
	if (task->task_data)
	{
		XTPTPI *task_data = (XTPTPI*)task->task_data;
		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		data["last_price"] = task_data->last_price;

		delete task->task_data;
	}

	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onQueryTickersPriceInfo(data, error, task->task_last);
	delete task;
};





void QuoteApi::processSubscribeAllOptionMarketData(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onSubscribeAllOptionMarketData(task->exchange_id,error);
	delete task;
};

void QuoteApi::processUnSubscribeAllOptionMarketData(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onUnSubscribeAllOptionMarketData(task->exchange_id,error);
	delete task;
};

void QuoteApi::processSubscribeAllOptionOrderBook(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onSubscribeAllOptionOrderBook(task->exchange_id,error);
	delete task;
};

void QuoteApi::processUnSubscribeAllOptionOrderBook(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onUnSubscribeAllOptionOrderBook(task->exchange_id,error);
	delete task;
};

void QuoteApi::processSubscribeAllOptionTickByTick(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onSubscribeAllOptionTickByTick(task->exchange_id,error);
	delete task;
};

void QuoteApi::processUnSubscribeAllOptionTickByTick(Task *task)
{
	PyLock lock;
	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onUnSubscribeAllOptionTickByTick(task->exchange_id,error);
	delete task;
};


void QuoteApi::processQueryAllTickersFullInfo(Task* task) {
	PyLock lock;

	dict data;
	if (task->task_data)
	{
		XTPQFI *task_data = (XTPQFI*)task->task_data;
		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		data["ticker_name"] = addEndingChar(task_data->ticker_name);
		data["security_type"] = (int)task_data->security_type;
		data["ticker_qualification_class"] = (int)task_data->ticker_qualification_class;
		data["is_registration"] = task_data->is_registration;
		data["is_VIE"] = task_data->is_VIE;
		data["is_noprofit"] = task_data->is_noprofit;
		data["is_weighted_voting_rights"] = task_data->is_weighted_voting_rights;
		data["is_have_price_limit"] = task_data->is_have_price_limit;
		data["is_inventory"] = task_data->is_inventory;
		data["upper_limit_price"] = task_data->upper_limit_price;
		data["lower_limit_price"] = task_data->lower_limit_price;
		data["pre_close_price"] = task_data->pre_close_price;
		data["price_tick"] = task_data->price_tick;
		data["bid_qty_upper_limit"] = task_data->bid_qty_upper_limit;
		data["bid_qty_lower_limit"] = task_data->bid_qty_lower_limit;
		data["bid_qty_unit"] = task_data->bid_qty_unit;
		data["ask_qty_upper_limit"] = task_data->ask_qty_upper_limit;
		data["ask_qty_lower_limit"] = task_data->ask_qty_lower_limit;
		data["ask_qty_unit"] = task_data->ask_qty_unit;
		data["market_bid_qty_upper_limit"] = task_data->market_bid_qty_upper_limit;
		data["market_bid_qty_lower_limit"] = task_data->market_bid_qty_lower_limit;
		data["market_bid_qty_unit"] = task_data->market_bid_qty_unit;
		data["market_ask_qty_upper_limit"] = task_data->market_ask_qty_upper_limit;
		data["market_ask_qty_lower_limit"] = task_data->market_ask_qty_lower_limit;
		data["market_ask_qty_unit"] = task_data->market_ask_qty_unit;
		data["security_status"] = (int)task_data->security_status;

		delete task->task_data;
	}

	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onQueryAllTickersFullInfo(data, error, task->task_last);

	delete task;
}

void QuoteApi::processQueryAllNQTickersFullInfo(Task* task) {
	PyLock lock;

	dict data;
	if (task->task_data)
	{
		XTPNQFI *task_data = (XTPNQFI*)task->task_data;
		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		data["ticker_name"] = addEndingChar(task_data->ticker_name);
		data["security_type"] = (int)task_data->security_type;
		data["ticker_qualification_class"] = (int)task_data->ticker_qualification_class;
		data["ticker_abbr_en"] = addEndingChar(task_data->ticker_abbr_en);
		data["base_ticker"] = addEndingChar(task_data->base_ticker);
		data["industry_type"] = addEndingChar(task_data->industry_type);
		data["currency_type"] = addEndingChar(task_data->currency_type);
		data["trade_unit"] = task_data->trade_unit;
		data["hang_out_date"] = task_data->hang_out_date;
		data["value_date"] = task_data->value_date;
		data["maturity_date"] = task_data->maturity_date;
		data["per_limit_vol"] = task_data->per_limit_vol;
		data["buy_vol_unit"] = task_data->buy_vol_unit;
		data["sell_vol_unit"] = task_data->sell_vol_unit;
		data["mini_declared_vol"] = task_data->mini_declared_vol;
		data["limit_price_attr"] = task_data->limit_price_attr;
		data["market_maker_quantity"] = task_data->market_maker_quantity;
		data["price_gear"] = task_data->price_gear;
		data["first_limit_trans"] = task_data->first_limit_trans;
		data["subsequent_limit_trans"] = task_data->subsequent_limit_trans;
		data["limit_upper_price"] = task_data->limit_upper_price;
		data["limit_lower_price"] = task_data->limit_lower_price;
		data["block_trade_upper"] = task_data->block_trade_upper;
		data["block_trade_lower"] = task_data->block_trade_lower;
		data["convert_into_ration"] = task_data->convert_into_ration;
		data["trade_status"] = (int)task_data->trade_status;
		data["security_level"] = (int)task_data->security_level;
		data["trade_type"] = (int)task_data->trade_type;
		data["suspend_flag"] = (int)task_data->suspend_flag;
		data["ex_dividend_flag"] = (int)task_data->ex_dividend_flag;
		data["layer_type"] = (int)task_data->layer_type;
		data["reserved1"] = task_data->reserved1;		
		data["trade_places"] = addEndingChar(task_data->trade_places);
		data["is_rzbd"] = task_data->is_rzbd;
		data["is_rqbd"] = task_data->is_rqbd;
		data["is_drrz"] = task_data->is_drrz;
		data["is_drrq"] = task_data->is_drrq;
		data["reserved"] = task_data->reserved;
		
		delete task->task_data;
	}

	dict error;
	if (task->task_error)
	{
		XTPRI *task_error = (XTPRI*)task->task_error;
		error["error_id"] = task_error->error_id;
		error["error_msg"] = addEndingChar(task_error->error_msg);
		delete task->task_error;
	}

	this->onQueryAllNQTickersFullInfo(data, error, task->task_last);

	delete task;
}

void QuoteApi::processRebuildQuoteServerDisconnected(Task *task)
{
	PyLock lock;
	this->onRebuildQuoteServerDisconnected(task->task_id);
	delete task;
}

void QuoteApi::processRequestRebuildQuote(Task *task)
{
	PyLock lock;
	dict data;
	if (task->task_data)
	{
		XTPQuoteRebuildResultRsp* task_data = (XTPQuoteRebuildResultRsp*)task->task_data;
		data["request_id"] = task_data->request_id;
		data["exchange_id"] = (int)task_data->exchange_id;
		data["size"] = task_data->size;
		data["channel_number"] = task_data->channel_number;
		data["begin"] = task_data->begin;
		data["end"] = task_data->end;
		data["result_code"] = (int)task_data->result_code;
		data["msg"] = addEndingChar(task_data->msg);
		delete task->task_data;
	}
	this->onRequestRebuildQuote(data);
	delete task;
}

void QuoteApi::processRebuildTickByTick(Task *task)
{
	PyLock lock;
	dict data;
	if (task->task_data)
	{
		XTPTBT *task_data = (XTPTBT*)task->task_data;


		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		data["struct_seq"] = task_data->seq;
		data["data_time"] = task_data->data_time;
		data["type"] = (int)task_data->type;

		if (task_data->type == XTP_TBT_ENTRUST)
		{
			data["channel_no"] = task_data->entrust.channel_no;
			data["seq"] = task_data->entrust.seq;
			data["price"] = task_data->entrust.price;
			data["qty"] = task_data->entrust.qty;
			data["side"] = task_data->entrust.side;
			data["ord_type"] = task_data->entrust.ord_type;
			data["order_no"] = task_data->entrust.order_no;
		}
		else if (task_data->type == XTP_TBT_TRADE)
		{
			data["channel_no"] = task_data->trade.channel_no;
			data["seq"] = task_data->trade.seq;
			data["price"] = task_data->trade.price;
			data["qty"] = task_data->trade.qty;
			data["money"] = task_data->trade.money;
			data["bid_no"] = task_data->trade.bid_no;
			data["ask_no"] = task_data->trade.ask_no;
			data["trade_flag"] = task_data->trade.trade_flag;
		}
		else
		{
			data["channel_no"] = task_data->state.channel_no;
			data["seq"] = task_data->state.seq;
			data["flag"] = addEndingChar(task_data->state.flag);
		}

		delete task->task_data;
	}

	this->onRebuildTickByTick(data);
	delete task;
}

void QuoteApi::processRebuildMarketData(Task *task)
{
	PyLock lock;
	dict data;
	if (task->task_data)
	{
		XTPMD *task_data = (XTPMD*)task->task_data;
		data["exchange_id"] = (int)task_data->exchange_id;
		data["ticker"] = addEndingChar(task_data->ticker);
		data["last_price"] = task_data->last_price;
		data["pre_close_price"] = task_data->pre_close_price;
		data["open_price"] = task_data->open_price;
		data["high_price"] = task_data->high_price;
		data["low_price"] = task_data->low_price;
		data["close_price"] = task_data->close_price;

		data["pre_total_long_positon"] = task_data->pre_total_long_positon;
		data["total_long_positon"] = task_data->total_long_positon;
		data["pre_settl_price"] = task_data->pre_settl_price;
		data["settl_price"] = task_data->settl_price;

		data["upper_limit_price"] = task_data->upper_limit_price;
		data["lower_limit_price"] = task_data->lower_limit_price;
		data["pre_delta"] = task_data->pre_delta;
		data["curr_delta"] = task_data->curr_delta;

		data["data_time"] = task_data->data_time;

		data["qty"] = task_data->qty;
		data["turnover"] = task_data->turnover;
		data["avg_price"] = task_data->avg_price;

		data["trades_count"] = task_data->trades_count;
		char str_ticker_status[9] = { "\0" };
#ifdef _MSC_VER //WIN32
		strncpy(str_ticker_status, task_data->ticker_status, sizeof(task_data->ticker_status));
#elif __GNUC__
		strncpy(str_ticker_status, task_data->ticker_status, sizeof(task_data->ticker_status));
#endif
		data["ticker_status"] = addEndingChar(str_ticker_status);

		boost::python::list ask;
		boost::python::list bid;
		boost::python::list ask_qty;
		boost::python::list bid_qty;

		for (int i = 0; i < 10; i++)
		{
			ask.append(task_data->ask[i]);
			bid.append(task_data->bid[i]);
			ask_qty.append(task_data->ask_qty[i]);
			bid_qty.append(task_data->bid_qty[i]);
		}

		data["ask"] = ask;
		data["bid"] = bid;
		data["bid_qty"] = bid_qty;
		data["ask_qty"] = ask_qty;

		data["data_type"] = (int)task_data->data_type;
		data["data_type_v2"] = (int)task_data->data_type_v2;
		if (task_data->data_type_v2 == XTP_MARKETDATA_V2_ACTUAL) {
			data["total_bid_qty"] = task_data->stk.total_bid_qty;
			data["total_ask_qty"] = task_data->stk.total_ask_qty;
			data["ma_bid_price"] = task_data->stk.ma_bid_price;
			data["ma_ask_price"] = task_data->stk.ma_ask_price;
			data["ma_bond_bid_price"] = task_data->stk.ma_bond_bid_price;
			data["ma_bond_ask_price"] = task_data->stk.ma_bond_ask_price;
			data["yield_to_maturity"] = task_data->stk.yield_to_maturity;
			data["iopv"] = task_data->stk.iopv;
			data["etf_buy_count"] = task_data->stk.etf_buy_count;
			data["etf_sell_count"] = task_data->stk.etf_sell_count;
			data["etf_buy_qty"] = task_data->stk.etf_buy_qty;
			data["etf_buy_money"] = task_data->stk.etf_buy_money;
			data["etf_sell_qty"] = task_data->stk.etf_sell_qty;
			data["etf_sell_money"] = task_data->stk.etf_sell_money;
			data["total_warrant_exec_qty"] = task_data->stk.total_warrant_exec_qty;
			data["warrant_lower_price"] = task_data->stk.warrant_lower_price;
			data["warrant_upper_price"] = task_data->stk.warrant_upper_price;
			data["cancel_buy_count"] = task_data->stk.cancel_buy_count;
			data["cancel_sell_count"] = task_data->stk.cancel_sell_count;
			data["cancel_buy_qty"] = task_data->stk.cancel_buy_qty;
			data["cancel_sell_qty"] = task_data->stk.cancel_sell_qty;
			data["cancel_buy_money"] = task_data->stk.cancel_buy_money;
			data["cancel_sell_money"] = task_data->stk.cancel_sell_money;
			data["total_buy_count"] = task_data->stk.total_buy_count;
			data["total_sell_count"] = task_data->stk.total_sell_count;
			data["duration_after_buy"] = task_data->stk.duration_after_buy;
			data["duration_after_sell"] = task_data->stk.duration_after_sell;
			data["num_bid_orders"] = task_data->stk.num_bid_orders;
			data["num_ask_orders"] = task_data->stk.num_ask_orders;
			data["pre_iopv"] = task_data->stk.pre_iopv;
			data["r1"] = task_data->stk.r1;
			data["r2"] = task_data->stk.r2;
		}
		else if (task_data->data_type_v2 == XTP_MARKETDATA_V2_OPTION) {
			data["auction_price"] = task_data->opt.auction_price;
			data["auction_qty"] = task_data->opt.auction_qty;
			data["last_enquiry_time"] = task_data->opt.last_enquiry_time;
		}
		else if (task_data->data_type_v2 == XTP_MARKETDATA_V2_BOND)
		{
			data["total_bid_qty"] = task_data->bond.total_bid_qty;
			data["total_ask_qty"] = task_data->bond.total_ask_qty;
			data["ma_bid_price"] = task_data->bond.ma_bid_price;
			data["ma_ask_price"] = task_data->bond.ma_ask_price;
			data["ma_bond_bid_price"] = task_data->bond.ma_bond_bid_price;
			data["ma_bond_ask_price"] = task_data->bond.ma_bond_ask_price;
			data["yield_to_maturity"] = task_data->bond.yield_to_maturity;
			data["match_lastpx"] = task_data->bond.match_lastpx;
			data["ma_bond_price"] = task_data->bond.ma_bond_price;
			data["match_qty"] = task_data->bond.match_qty;
			data["match_turnover"] = task_data->bond.match_turnover;
			data["r4"] = task_data->bond.r4;
			data["r5"] = task_data->bond.r5;
			data["r6"] = task_data->bond.r6;
			data["r7"] = task_data->bond.r7;
			data["r8"] = task_data->bond.r8;
			data["cancel_buy_count"] = task_data->bond.cancel_buy_count;
			data["cancel_sell_count"] = task_data->bond.cancel_sell_count;
			data["cancel_buy_qty"] = task_data->bond.cancel_buy_qty;
			data["cancel_sell_qty"] = task_data->bond.cancel_sell_qty;
			data["cancel_buy_money"] = task_data->bond.cancel_buy_money;
			data["cancel_sell_money"] = task_data->bond.cancel_sell_money;
			data["total_buy_count"] = task_data->bond.total_buy_count;
			data["total_sell_count"] = task_data->bond.total_sell_count;
			data["duration_after_buy"] = task_data->bond.duration_after_buy;
			data["duration_after_sell"] = task_data->bond.duration_after_sell;
			data["num_bid_orders"] = task_data->bond.num_bid_orders;
			data["num_ask_orders"] = task_data->bond.num_ask_orders;
			data["instrument_status"] = addEndingChar(task_data->bond.instrument_status);
		}

		//data["r4"] = task_data->r4;

		delete task->task_data;
	}
	
	this->onRebuildMarketData(data);
	delete task;
}


///-------------------------------------------------------------------------------------
///��������
///-------------------------------------------------------------------------------------

void QuoteApi::createQuoteApi(int clientid, string path, int log_level)
{
	this->api = XTP::API::QuoteApi::CreateQuoteApi(clientid, path.c_str(),(XTP_LOG_LEVEL)log_level);
	this->api->RegisterSpi(this);
};

void QuoteApi::release()
{
	this->api->Release();
};

int QuoteApi::exit()
{
	//�ú�����ԭ��API��û�У����ڰ�ȫ�˳�API�ã�ԭ����join�ƺ���̫�ȶ�
	this->api->RegisterSpi(NULL);
	this->api->Release();
	this->api = NULL;
	return 1;
};

string QuoteApi::getTradingDay()
{
	string ret ="";
	const char* p = this->api->GetTradingDay();
	if (p == NULL)
		ret = "NULL";
	else
		ret = p;
	return ret;
};

string QuoteApi::getApiVersion()
{
	string ret ="";
	const char* p = this->api->GetApiVersion();
	if (p == NULL)
		ret = "NULL";
	else
		ret = p;
	return ret;
};

dict QuoteApi::getApiLastError()
{
	XTPRI *error = this->api->GetApiLastError();
	dict err;
	if(error == NULL)
		return err;

	err["error_id"] = error->error_id;
	err["error_msg"] = error->error_msg;

	return err;
};

void QuoteApi::setUDPBufferSize(int size)
{
	this->api->SetUDPBufferSize(size);
};

void QuoteApi::setHeartBeatInterval(int interval)
{
	this->api->SetHeartBeatInterval(interval);
};

void QuoteApi::setUDPRecvThreadAffinity(int32_t cpu_no)
{
	this->api->SetUDPRecvThreadAffinity(cpu_no);
};

void QuoteApi::setUDPRecvThreadAffinityArray(boost::python::list tickerList,int count)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return;
	int32_t *myreqList = new int32_t[listLength];
	for(int i=0;i<listLength;i++){
		dict req = (dict)tickerList[i];
		getInt(req,"cpu_no",&myreqList[i]);
	}    
	this->api->SetUDPRecvThreadAffinityArray(myreqList, count);

	delete[] myreqList;
	myreqList = NULL;
};

void QuoteApi::setUDPParseThreadAffinity(int32_t cpu_no)
{
	this->api->SetUDPParseThreadAffinity(cpu_no);
};

void QuoteApi::setUDPParseThreadAffinityArray(boost::python::list tickerList,int count)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return;
	int32_t *myreqList = new int32_t[listLength];
	for(int i=0;i<listLength;i++){
		dict req = (dict)tickerList[i];
		getInt(req,"cpu_no",&myreqList[i]);
	}    
	this->api->SetUDPParseThreadAffinityArray(myreqList, count);

	delete[] myreqList;
	myreqList = NULL;
};

void QuoteApi::setUDPSeqLogOutPutFlag(bool flag)
{
	this->api->SetUDPSeqLogOutPutFlag(flag);
};

int QuoteApi::subscribeMarketData(boost::python::list tickerList,int count, int exchange)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return -1;
	char **myreqList = new char *[listLength];
	for(int i=0;i<listLength;i++){
		myreqList[i]=new char[256];
		dict req = (dict)tickerList[i];
		getStr(req,"ticker",myreqList[i]);
		//printf("i:%d,myreqList[i]:%s\n",i,myreqList[i]);
	}    
	int i = this->api->SubscribeMarketData(myreqList, count, (XTP_EXCHANGE_TYPE) exchange);
	for(int i=0;i<listLength;i++){
		delete myreqList[i];
	}    
	delete[] myreqList;
	myreqList = NULL;

	return i;
};

int QuoteApi::unSubscribeMarketData(boost::python::list tickerList,int count, int exchange)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return -1;
	char **myreqList = new char *[listLength];
	for(int i=0;i<listLength;i++){
		myreqList[i]=new char[256];
		dict req = (dict)tickerList[i];
		getStr(req,"ticker",myreqList[i]);
	}    
	int i = this->api->UnSubscribeMarketData(myreqList, count, (XTP_EXCHANGE_TYPE) exchange);
	for(int i=0;i<listLength;i++){
		delete myreqList[i];
	}    
	delete[] myreqList;
	myreqList = NULL;

	return i;
};

int QuoteApi::subscribeOrderBook(boost::python::list tickerList,int count, int exchange)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return -1;
	char **myreqList = new char *[listLength];
	for(int i=0;i<listLength;i++){
		myreqList[i]=new char[256];
		dict req = (dict)tickerList[i];
		getStr(req,"ticker",myreqList[i]);
	}    
	int i = this->api->SubscribeOrderBook(myreqList, count, (XTP_EXCHANGE_TYPE) exchange);
	for(int i=0;i<listLength;i++){
		delete myreqList[i];
	}    
	delete[] myreqList;
	myreqList = NULL;
	return i;
};

int QuoteApi::unSubscribeOrderBook(boost::python::list tickerList,int count, int exchange)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return -1;
	char **myreqList = new char *[listLength];
	for(int i=0;i<listLength;i++){
		myreqList[i]=new char[256];
		dict req = (dict)tickerList[i];
		getStr(req,"ticker",myreqList[i]);
	}    
	int i = this->api->UnSubscribeOrderBook(myreqList, count, (XTP_EXCHANGE_TYPE) exchange);
	for(int i=0;i<listLength;i++){
		delete myreqList[i];
	}    
	delete[] myreqList;
	myreqList = NULL;

	return i;
};

int QuoteApi::subscribeTickByTick(boost::python::list tickerList,int count, int exchange)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return -1;
	char **myreqList = new char *[listLength];
	for(int i=0;i<listLength;i++){
		myreqList[i]=new char[256];
		dict req = (dict)tickerList[i];
		getStr(req,"ticker",myreqList[i]);
	}    
	int i = this->api->SubscribeTickByTick(myreqList, count, (XTP_EXCHANGE_TYPE) exchange);
	for(int i=0;i<listLength;i++){
		delete myreqList[i];
	}    
	delete[] myreqList;
	myreqList = NULL;
	return i;
};

int QuoteApi::unSubscribeTickByTick(boost::python::list tickerList,int count, int exchange)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return -1;
	char **myreqList = new char *[listLength];
	for(int i=0;i<listLength;i++){
		myreqList[i]=new char[256];
		dict req = (dict)tickerList[i];
		getStr(req,"ticker",myreqList[i]);
	}    
	int i = this->api->UnSubscribeTickByTick(myreqList, count, (XTP_EXCHANGE_TYPE) exchange);
	for(int i=0;i<listLength;i++){
		delete myreqList[i];
	}    
	delete[] myreqList;
	myreqList = NULL;

	return i;
};

int QuoteApi::subscribeAllMarketData(int exchange)
{
	return this->api->SubscribeAllMarketData((XTP_EXCHANGE_TYPE)exchange);
};

int QuoteApi::unSubscribeAllMarketData(int exchange)
{
	return this->api->UnSubscribeAllMarketData((XTP_EXCHANGE_TYPE)exchange);
};

int QuoteApi::subscribeAllOrderBook(int exchange)
{
	return this->api->SubscribeAllOrderBook((XTP_EXCHANGE_TYPE)exchange);
};

int QuoteApi::unSubscribeAllOrderBook(int exchange)
{
	return this->api->UnSubscribeAllOrderBook((XTP_EXCHANGE_TYPE)exchange);
};

int QuoteApi::subscribeAllTickByTick(int exchange)
{
	return this->api->SubscribeAllTickByTick((XTP_EXCHANGE_TYPE)exchange);
};

int QuoteApi::unSubscribeAllTickByTick(int exchange)
{
	return this->api->UnSubscribeAllTickByTick((XTP_EXCHANGE_TYPE)exchange);
};

int QuoteApi::login(string ip, int port, string user, string password, int socktype,string local_ip)
{
	int i = this->api->Login(ip.c_str(), port, user.c_str(), password.c_str(), (XTP_PROTOCOL_TYPE)socktype,local_ip.c_str());
	return i;
};

int QuoteApi::logout()
{
	int i = this->api->Logout();
	return i;
};

int QuoteApi::queryAllTickers(int exchange)
{
	int i = this->api->QueryAllTickers((XTP_EXCHANGE_TYPE)exchange);
	return i;
};

int QuoteApi::queryTickersPriceInfo(boost::python::list tickerList,int count, int exchange)
{
	int listLength = boost::python::len(tickerList);
	if(listLength <= 0)
		return -1;
	//printf("listLength:%d\n",listLength);
	char **myreqList = new char *[listLength];
	for(int i=0;i<listLength;i++){
		myreqList[i]=new char[256];
		dict req = (dict)tickerList[i];
		getStr(req,"ticker",myreqList[i]);
		//printf("i:%d,myreqList[i]:%s\n",i,myreqList[i]);
	}    
	int i = this->api->QueryTickersPriceInfo(myreqList, count, (XTP_EXCHANGE_TYPE) exchange);
	//printf("return i:%d\n",i);
	for(int i=0;i<listLength;i++){
		delete myreqList[i];
	}    
	delete[] myreqList;
	myreqList = NULL;
	return i;
}

int QuoteApi::queryAllTickersPriceInfo()
{
	int i = this->api->QueryAllTickersPriceInfo();
	return i;
}

int QuoteApi::queryAllTickersFullInfo(int exchange) {
	int i =this->api->QueryAllTickersFullInfo((XTP_EXCHANGE_TYPE) exchange);
	return i;
}

int QuoteApi::queryAllNQTickersFullInfo()
 {
	return this->api->QueryAllNQTickersFullInfo();
}

int QuoteApi::subscribeAllOptionMarketData(int exchange)
{
	int i = this->api->SubscribeAllOptionMarketData((XTP_EXCHANGE_TYPE)exchange);
	return i;
};

int QuoteApi::unSubscribeAllOptionMarketData(int exchange)
{
	int i = this->api->UnSubscribeAllOptionMarketData((XTP_EXCHANGE_TYPE)exchange);
	return i;
};

int QuoteApi::subscribeAllOptionOrderBook(int exchange)
{
	int i = this->api->SubscribeAllOptionOrderBook((XTP_EXCHANGE_TYPE)exchange);
	return i;
};

int QuoteApi::unSubscribeAllOptionOrderBook(int exchange)
{
	int i = this->api->UnSubscribeAllOptionOrderBook((XTP_EXCHANGE_TYPE)exchange);
	return i;
};

int QuoteApi::subscribeAllOptionTickByTick(int exchange)
{
	int i = this->api->SubscribeAllOptionTickByTick((XTP_EXCHANGE_TYPE)exchange);
	return i;
};

int QuoteApi::unSubscribeAllOptionTickByTick(int exchange)
{
	int i = this->api->UnSubscribeAllOptionTickByTick((XTP_EXCHANGE_TYPE)exchange);
	return i;
};

int QuoteApi::loginToRebuildQuoteServer(string ip, int port, string user, string password, int sock_type, string local_ip)
{
	int i = this->api->LoginToRebuildQuoteServer(ip.c_str(), port, user.c_str(), password.c_str(), (XTP_PROTOCOL_TYPE)sock_type, local_ip.c_str());
	return i;
}

int QuoteApi::logoutFromRebuildQuoteServer()
{
	int i = this->api->LogoutFromRebuildQuoteServer();
	return i;
}

int QuoteApi::requestRebuildQuote(dict req)
{
	XTPQuoteRebuildReq myreq = XTPQuoteRebuildReq();
	memset(&myreq, 0, sizeof(myreq));
	getStr(req, "ticker", myreq.ticker);
	getInt(req, "request_id", &myreq.request_id);
	int data_type;
	int exchange_id;
	int channel_number;
	getInt(req, "data_type", &data_type);
	getInt(req, "exchange_id", &exchange_id);
	getInt16(req, "channel_number", &myreq.channel_number);
	getInt64(req, "begin", &myreq.begin);
	getInt64(req, "end", &myreq.end);
	
	myreq.data_type = (XTP_QUOTE_REBUILD_DATA_TYPE)data_type;
	myreq.exchange_id = (XTP_EXCHANGE_TYPE)exchange_id;

	int ret = api->RequestRebuildQuote(&myreq);

	return ret;
}

///-------------------------------------------------------------------------------------
///Boost.Python��װ
///-------------------------------------------------------------------------------------

struct QuoteApiWrap : QuoteApi, wrapper < QuoteApi >
{
	virtual void onDisconnected(int reason)
	{
	    try
	    {
	        this->get_override("onDisconnected")(reason);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onError(dict data)
	{
	    try
	    {
	        this->get_override("onError")(data);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onTickByTickLossRange(int begin_seq, int end_seq)
	{
		try
		{
			this->get_override("onTickByTickLossRange")(begin_seq, end_seq);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onSubMarketData(dict data, dict error, bool last)
	{
	    try
	    {
	        this->get_override("onSubMarketData")(data, error, last);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onUnSubMarketData(dict data, dict error, bool last)
	{
	    try
	    {
	        this->get_override("onUnSubMarketData")(data, error, last);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onDepthMarketData(dict data,boost::python::list bid1_qty_list,int bid1_count,int max_bid1_count,boost::python::list ask1_qty_list,int ask1_count,int max_ask1_count)
	{
	    try
	    {
	        this->get_override("onDepthMarketData")(data,bid1_qty_list,bid1_count,max_bid1_count,ask1_qty_list,ask1_count,max_ask1_count);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onETFIOPVData(dict data)
	{
		try
		{
			this->get_override("onETFIOPVData")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onSubOrderBook(dict data, dict error, bool last)
	{
	    try
	    {
	        this->get_override("onSubOrderBook")(data, error, last);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onUnSubOrderBook(dict data, dict error, bool last)
	{
	    try
	    {
	        this->get_override("onUnSubOrderBook")(data, error, last);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onOrderBook(dict data)
	{
	    try
	    {
	        this->get_override("onOrderBook")(data);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onSubTickByTick(dict data, dict error, bool last)
	{
		try
		{
			this->get_override("onSubTickByTick")(data, error, last);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onUnSubTickByTick(dict data, dict error, bool last)
	{
		try
		{
			this->get_override("onUnSubTickByTick")(data, error, last);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onTickByTick(dict data)
	{
		try
		{
			this->get_override("onTickByTick")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onSubscribeAllMarketData(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onSubscribeAllMarketData")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onUnSubscribeAllMarketData(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onUnSubscribeAllMarketData")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onSubscribeAllOrderBook(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onSubscribeAllOrderBook")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onUnSubscribeAllOrderBook(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onUnSubscribeAllOrderBook")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onSubscribeAllTickByTick(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onSubscribeAllTickByTick")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onUnSubscribeAllTickByTick(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onUnSubscribeAllTickByTick")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onQueryAllTickers(dict data, dict error, bool last)
	{
	    try
	    {
	        this->get_override("onQueryAllTickers")(data, error, last);
	    }
	    catch (error_already_set const &)
	    {
	        PyErr_Print();
	    }
	};

	virtual void onQueryTickersPriceInfo(dict data, dict error, bool last)
	{
		try
		{
			this->get_override("onQueryTickersPriceInfo")(data, error, last);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};





	virtual void onSubscribeAllOptionMarketData(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onSubscribeAllOptionMarketData")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onUnSubscribeAllOptionMarketData(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onUnSubscribeAllOptionMarketData")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onSubscribeAllOptionOrderBook(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onSubscribeAllOptionOrderBook")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onUnSubscribeAllOptionOrderBook(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onUnSubscribeAllOptionOrderBook")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};
	virtual void onSubscribeAllOptionTickByTick(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onSubscribeAllOptionTickByTick")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onUnSubscribeAllOptionTickByTick(int exchange_id,dict data)
	{
		try
		{
			this->get_override("onUnSubscribeAllOptionTickByTick")(exchange_id,data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onQueryAllTickersFullInfo(dict data, dict error, bool last) {
		PyLock lock;

		try {
			this->get_override("onQueryAllTickersFullInfo")(data, error, last);
		} catch (error_already_set const &) {
			PyErr_Print();
		}
	};

	virtual void onQueryAllNQTickersFullInfo(dict data, dict error, bool last) {
		PyLock lock;

		try {
			this->get_override("onQueryAllNQTickersFullInfo")(data, error, last);
		}
		catch (error_already_set const &) {
			PyErr_Print();
		}
	}
	
	virtual void onRebuildQuoteServerDisconnected(int reason)
	{
		try
		{
			this->get_override("onRebuildQuoteServerDisconnected")(reason);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	}

	virtual void onRequestRebuildQuote(dict data)
	{
		try
		{
			this->get_override("onRequestRebuildQuote")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	}

	virtual void onRebuildTickByTick(dict data)
	{
		try
		{
			this->get_override("onRebuildTickByTick")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	}

	virtual void onRebuildMarketData(dict data)
	{
		try
		{
			this->get_override("onRebuildMarketData")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	}
};




BOOST_PYTHON_MODULE(vnxtpquote)
{
	//PyEval_InitThreads();	//����ʱ���У���֤�ȴ���GIL
	Py_Initialize();

	class_<QuoteApiWrap, boost::noncopyable>("QuoteApi")
		.def("createQuoteApi", &QuoteApiWrap::createQuoteApi)
		.def("release", &QuoteApiWrap::release)
		.def("exit", &QuoteApiWrap::exit)
		.def("getTradingDay", &QuoteApiWrap::getTradingDay)
		.def("getApiVersion", &QuoteApiWrap::getApiVersion)
		.def("getApiLastError", &QuoteApiWrap::getApiLastError)
		.def("setUDPBufferSize", &QuoteApiWrap::setUDPBufferSize)
		.def("setHeartBeatInterval", &QuoteApiWrap::setHeartBeatInterval)
		.def("setUDPRecvThreadAffinity", &QuoteApiWrap::setUDPRecvThreadAffinity)
		.def("setUDPRecvThreadAffinityArray", &QuoteApiWrap::setUDPRecvThreadAffinityArray)
		.def("setUDPParseThreadAffinity", &QuoteApiWrap::setUDPParseThreadAffinity)
		.def("setUDPParseThreadAffinityArray", &QuoteApiWrap::setUDPParseThreadAffinityArray)
		.def("setUDPSeqLogOutPutFlag", &QuoteApiWrap::setUDPSeqLogOutPutFlag)
		.def("subscribeMarketData", &QuoteApiWrap::subscribeMarketData)
		.def("unSubscribeMarketData", &QuoteApiWrap::unSubscribeMarketData)
		.def("subscribeOrderBook", &QuoteApiWrap::subscribeOrderBook)
		.def("unSubscribeOrderBook", &QuoteApiWrap::unSubscribeOrderBook)
		.def("subscribeTickByTick", &QuoteApiWrap::subscribeTickByTick)
		.def("unSubscribeTickByTick", &QuoteApiWrap::unSubscribeTickByTick)
		.def("subscribeAllMarketData", &QuoteApiWrap::subscribeAllMarketData)
		.def("unSubscribeAllMarketData", &QuoteApiWrap::unSubscribeAllMarketData)
		.def("subscribeAllOrderBook", &QuoteApiWrap::subscribeAllOrderBook)
		.def("unSubscribeAllOrderBook", &QuoteApiWrap::unSubscribeAllOrderBook)
		.def("subscribeAllTickByTick", &QuoteApiWrap::subscribeAllTickByTick)
		.def("unSubscribeAllTickByTick", &QuoteApiWrap::unSubscribeAllTickByTick)
		.def("login", &QuoteApiWrap::login)
		.def("logout", &QuoteApiWrap::logout)
		.def("queryAllTickers", &QuoteApiWrap::queryAllTickers)
		.def("queryTickersPriceInfo", &QuoteApiWrap::queryTickersPriceInfo)
		.def("queryAllTickersPriceInfo", &QuoteApiWrap::queryAllTickersPriceInfo)
		.def("queryAllTickersFullInfo", &QuoteApiWrap::queryAllTickersFullInfo)
		.def("queryAllNQTickersFullInfo", &QuoteApiWrap::queryAllNQTickersFullInfo)
		.def("subscribeAllOptionMarketData", &QuoteApiWrap::subscribeAllOptionMarketData)
		.def("unSubscribeAllOptionMarketData", &QuoteApiWrap::unSubscribeAllOptionMarketData)
		.def("subscribeAllOptionOrderBook", &QuoteApiWrap::subscribeAllOptionOrderBook)
		.def("unSubscribeAllOptionOrderBook", &QuoteApiWrap::unSubscribeAllOptionOrderBook)
		.def("subscribeAllOptionTickByTick", &QuoteApiWrap::subscribeAllOptionTickByTick)
		.def("unSubscribeAllOptionTickByTick", &QuoteApiWrap::unSubscribeAllOptionTickByTick)
		.def("loginToRebuildQuoteServer", &QuoteApiWrap::loginToRebuildQuoteServer)
		.def("requestRebuildQuote", &QuoteApiWrap::requestRebuildQuote)
		.def("logoutFromRebuildQuoteServer", &QuoteApiWrap::logoutFromRebuildQuoteServer)


		.def("onDisconnected", pure_virtual(&QuoteApiWrap::onDisconnected))
		.def("onError", pure_virtual(&QuoteApiWrap::onError))
		.def("onTickByTickLossRange", pure_virtual(&QuoteApiWrap::onTickByTickLossRange))
		.def("onSubMarketData", pure_virtual(&QuoteApiWrap::onSubMarketData))
		.def("onUnSubMarketData", pure_virtual(&QuoteApiWrap::onUnSubMarketData))
		.def("onDepthMarketData", pure_virtual(&QuoteApiWrap::onDepthMarketData))
		.def("onETFIOPVData", pure_virtual(&QuoteApiWrap::onETFIOPVData))
		.def("onSubOrderBook", pure_virtual(&QuoteApiWrap::onSubOrderBook))
		.def("onUnSubOrderBook", pure_virtual(&QuoteApiWrap::onUnSubOrderBook))
		.def("onOrderBook", pure_virtual(&QuoteApiWrap::onOrderBook))
		.def("onSubTickByTick", pure_virtual(&QuoteApiWrap::onSubTickByTick))
		.def("onUnSubTickByTick", pure_virtual(&QuoteApiWrap::onUnSubTickByTick))
		.def("onTickByTick", pure_virtual(&QuoteApiWrap::onTickByTick))
		.def("onSubscribeAllMarketData", pure_virtual(&QuoteApiWrap::onSubscribeAllMarketData))
		.def("onUnSubscribeAllMarketData", pure_virtual(&QuoteApiWrap::onUnSubscribeAllMarketData))
		.def("onSubscribeAllOrderBook", pure_virtual(&QuoteApiWrap::onSubscribeAllOrderBook))
		.def("onUnSubscribeAllOrderBook", pure_virtual(&QuoteApiWrap::onUnSubscribeAllOrderBook))
		.def("onSubscribeAllTickByTick", pure_virtual(&QuoteApiWrap::onSubscribeAllTickByTick))
		.def("onUnSubscribeAllTickByTick", pure_virtual(&QuoteApiWrap::onUnSubscribeAllTickByTick))
		.def("onQueryAllTickers", pure_virtual(&QuoteApiWrap::onQueryAllTickers))
		.def("onQueryTickersPriceInfo", pure_virtual(&QuoteApiWrap::onQueryTickersPriceInfo))
		.def("onQueryAllTickersFullInfo", &QuoteApiWrap::onQueryAllTickersFullInfo)
		.def("onQueryAllNQTickersFullInfo", &QuoteApiWrap::onQueryAllNQTickersFullInfo)
		.def("onRebuildQuoteServerDisconnected", &QuoteApiWrap::onRebuildQuoteServerDisconnected)
		.def("onRequestRebuildQuote", &QuoteApiWrap::onRequestRebuildQuote)
		.def("onRebuildTickByTick", &QuoteApiWrap::onRebuildTickByTick)
		.def("onRebuildMarketData", &QuoteApiWrap::onRebuildMarketData)

		.def("onSubscribeAllOptionMarketData", pure_virtual(&QuoteApiWrap::onSubscribeAllOptionMarketData))
		.def("onUnSubscribeAllOptionMarketData", pure_virtual(&QuoteApiWrap::onUnSubscribeAllOptionMarketData))
		.def("onSubscribeAllOptionOrderBook", pure_virtual(&QuoteApiWrap::onSubscribeAllOptionOrderBook))
		.def("onUnSubscribeAllOptionOrderBook", pure_virtual(&QuoteApiWrap::onUnSubscribeAllOptionOrderBook))
		.def("onSubscribeAllOptionTickByTick", pure_virtual(&QuoteApiWrap::onSubscribeAllOptionTickByTick))
		.def("onUnSubscribeAllOptionTickByTick", pure_virtual(&QuoteApiWrap::onUnSubscribeAllOptionTickByTick))
		;
};

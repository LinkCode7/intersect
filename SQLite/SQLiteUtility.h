#pragma once
#include "../SindyGlobal.h"
#include "sqlite3.h"

struct sqlite3;
struct sqlite3_stmt;




namespace Sindy
{
	// sqlite3采用utf16编码，即wstring
	class SINDY_API SQLite
	{
	public:
		explicit SQLite(const PString& strDbPath = ":memory:");
		explicit SQLite(const SQLite& opened);
		~SQLite();

		int Open(const PString& strSqliteDbPath);
		int Close();

	public:

		// 在同一时间使用多个数据库中的一个
		// ATTACH DATABASE语句使用后，所有的语句将在附加的数据库下执行
		int Attach(const PString& strDBPath, const PString& strDBName);
		int Detach(const PString& strDBName);


		// 将SQL语句编译为sqlite3_stmt，该结构体包含了将要执行的SQL语句信息
		int Prepare(const PString& strSql);

		// 执行语句，SQLITE_DONE和SQLITE_ROW都是表示执行成功，SQLITE_DONE表示没有查询结果
		// 每次Step只返回一行数据，SQLITE_OK表示执行结束并有查询结果
		int Step();

		// 释放语句对象，在SQL语句使用完之后，内存是Prepare申请的
		int Finalize();

		// 重置语句对象到它的初始状态，准备被重新执行（如果SQL语句对象要重复使用）
		int ResetSyntax();


		// 使用场景：不返回结果集的单一语句
		int Execute(const PString& strSql);

	public:

		// 结果集
		PString GetValueText(int iCol);
		bool GetValueText(const PString& strColName, PString& strColValue);

		bool GetValueDouble(const PString& strColName, double& dblColValue);
		double GetValueDouble(int iCol);

		bool GetValueInt(const PString& strColName, int& iColValue);
		int GetValueInt(int iCol);

		// 大字段
		long LargeFieldSize(int iCol);
		const void* GetValueLargeField(int iCol);
		bool GetValueLargeField(const PString& strColName, const void*& pBuffer);
		bool GetValueLargeField(const PString& strColName, const void*& pBuffer, long& lBufferSize);

		// 获取记录的字段个数,只有语句句柄具有活动游标才可以工作
		int GetFieldCount();
		// 获取列的值类型
		int GetFieldType(const PString& strColName);

		void InitColName2ColIndex();
		PString ColumnName(int iCol);



		// 绑定数据
		int BindText(const PString& strColName, const PString& strValue);
		int BindText(int iIndex, const PString& strValue);

		int BindInt(int iIndex, int iValue);
		int BindInt(const PString& strColName, int iValue);
		int BindInt64(int iIndex, __int64 llValue);
		int BindInt64(const PString& strColName, __int64 llValue);

		int BindDouble(int iIndex, double dblValue);
		int BindDouble(const PString& strColName, double dblValue);

		int BindLargeField(int iIndex, const void* pBuffer, int iByte);
		int BindLargeField(const PString& strColName, const void* pBuffer, int iByte);

		int BindParameterIndex(const PString& strColName);

		// 清空绑定值
		int ClearBindings();

	public:

		// 删除
		int DropTable(const PString& strTableName, const PString& strDbName = "main");
		int DropView(const PString& strTableName, const PString& strDbName = "main");
		int ClearTable(const PString& strTableName, const PString& strDbName = "main");

		// 是否存在
		bool IsTableExist(const PString& strTableName, const PString& strDbName = "main");
		bool IsViewExist(const PString& strTableName, const PString& strDbName = "main");
		bool IsFieldExist(const PString& strTableName, const PString& strFieldName, const PString& strDbName = "main");

	public:

		// 开启事务，在需要多次执行SQL语句时，先开启事务可以提升很大效率
		// SQLite缺省为每个操作启动一个事务
		void BeginTransaction();

		// 回滚
		void RollBack();

		// 提交
		void Commit();

		// 获取最近一次错误信息
		PString ErrorMessage();

	private:

		sqlite3* m_pDb;
		sqlite3_stmt* m_pStmt;

		unsigned int* m_pUseCount;
		std::map<PString, int> m_mapFieldName2Index;
	};

} // namespace Sindy

#ifndef SQLITE_UTILITY_U
#define SQLITE_UTILITY_U

#include "sqlite3.h"

#include <map>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

#define SINDY_API
// using sindy_int64 = __int64;
using sindy_int64 = int64_t;

namespace Sindy {

using PString = std::string;

class SINDY_API SQLite
{
public:
    explicit SQLite(const PString &strDbPath = ":memory:");
    explicit SQLite(const SQLite &opened);
    ~SQLite();

    int open(const PString &strSqliteDbPath);
    int close();

public:
    // 在同一时间使用多个数据库中的一个
    // ATTACH DATABASE语句使用后，所有的语句将在附加的数据库下执行
    int attach(const PString &strDBPath, const PString &strDBName);
    int detach(const PString &strDBName);

    // 将SQL语句编译为sqlite3_stmt，该结构体包含了将要执行的SQL语句信息
    int prepare(const PString &strSql);

    // 执行语句，SQLITE_DONE和SQLITE_ROW都是表示执行成功，SQLITE_DONE表示没有查询结果
    // 每次step只返回一行数据，SQLITE_OK表示执行结束并有查询结果
    int step();

    // 释放语句对象，在SQL语句使用完之后，内存是prepare申请的
    int finalize();

    // 重置语句对象到它的初始状态，准备被重新执行（如果SQL语句对象要重复使用）
    int resetSyntax();

    // 使用场景：不返回结果集的单一语句
    int execute(const PString &strSql);

public:
    // 结果集
    PString getValueText(int iCol);
    bool    getValueText(const PString &strColName, PString &strColValue);

    bool   getValueDouble(const PString &strColName, double &dblColValue);
    double getValueDouble(int iCol);

    bool getValueInt(const PString &strColName, int &iColValue);
    int  getValueInt(int iCol);

    // 大字段
    long        largeFieldSize(int iCol);
    const void *getValueLargeField(int iCol);
    bool        getValueLargeField(const PString &strColName, const void *&pBuffer);
    bool        getValueLargeField(const PString &strColName, const void *&pBuffer, long &lBufferSize);

    // 获取记录的字段个数,只有语句句柄具有活动游标才可以工作
    int getFieldCount();
    // 获取列的值类型
    int getFieldType(const PString &strColName);

    void    initColName2ColIndex();
    PString columnName(int iCol);

    // 绑定数据
    int bindText(const PString &strColName, const PString &strValue);
    int bindText(int iIndex, const PString &strValue);

    int bindInt(int iIndex, int iValue);
    int bindInt(const PString &strColName, int iValue);
    int bindInt64(int iIndex, sindy_int64 llValue);
    int bindInt64(const PString &strColName, sindy_int64 llValue);

    int bindDouble(int iIndex, double dblValue);
    int bindDouble(const PString &strColName, double dblValue);

    int bindLargeField(int iIndex, const void *pBuffer, int iByte);
    int bindLargeField(const PString &strColName, const void *pBuffer, int iByte);

    int bindParameterIndex(const PString &strColName);

    // 清空绑定值
    int clearBindings();

public:
    // 删除
    int dropTable(const PString &strTableName, const PString &strDbName = "main");
    int dropView(const PString &strTableName, const PString &strDbName = "main");
    int clearTable(const PString &strTableName, const PString &strDbName = "main");

    // 是否存在
    bool isTableExist(const PString &strTableName, const PString &strDbName = "main");
    bool isViewExist(const PString &strTableName, const PString &strDbName = "main");
    bool isFieldExist(const PString &strTableName, const PString &strFieldName, const PString &strDbName = "main");

public:
    // 开启事务，在需要多次执行SQL语句时，先开启事务可以提升很大效率
    // SQLite缺省为每个操作启动一个事务
    void beginTransaction();

    // 回滚
    void rollBack();

    // 提交
    void commit();

    // 获取最近一次错误信息
    PString errorMessage();

private:
    sqlite3 *     m_pDb;
    sqlite3_stmt *m_pStmt;

    unsigned int *         m_pUseCount;
    std::map<PString, int> m_mapFieldName2Index;
};

}  // namespace Sindy

#endif  // !SQLITE_UTILITY_U

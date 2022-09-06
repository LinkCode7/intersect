#include "SQLiteUtility.h"
#include <sstream>
#include <algorithm>
#include <codecvt>
//#include "../Common/RunTimeTest.h"



namespace Sindy
{
	SQLite::SQLite(const PString& strDbPath) :
		m_pDb(nullptr),
		m_pStmt(nullptr),
		m_pUseCount(new unsigned int(1))
	{
		open(strDbPath);
	}

	SQLite::SQLite(const SQLite& opened) :
		m_pDb(opened.m_pDb),
		m_pStmt(nullptr),
		m_pUseCount(opened.m_pUseCount)
	{
		(*m_pUseCount)++;
	}

	SQLite::~SQLite()
	{
		--(*m_pUseCount);
		if ((*m_pUseCount) == 0)
		{
			close();
			delete m_pUseCount;
		}
	}

	int SQLite::open(const PString& strSqliteDbPath)
	{
#ifdef SINDY_UNICODE
		int rc = sqlite3_open16(strSqliteDbPath.c_str(), &m_pDb);
#else
		int rc = sqlite3_open(strSqliteDbPath.c_str(), &m_pDb);
#endif

		if (SQLITE_OK != rc)
		{
			close();
			m_pDb = nullptr;
		}
		return rc;
	}

	int SQLite::close()
	{
		if (!m_pDb)
			return SQLITE_OK;

		// 释放附加在连接上的语句
		sqlite3_stmt* pStmt = sqlite3_next_stmt(m_pDb, nullptr);
		sqlite3_stmt* pStmtNext = nullptr;
		while (nullptr != pStmt)
		{
			pStmtNext = sqlite3_next_stmt(m_pDb, pStmt);
			sqlite3_finalize(pStmt);
			pStmt = pStmtNext;
		}

		// 关闭数据库
		int rc = SQLITE_OK;
		if ((*m_pUseCount) == 0)
			rc = sqlite3_close(m_pDb);
		m_pDb = nullptr;
		return SQLITE_OK;
	}

	int SQLite::attach(const PString& strDBPath, const PString& strDBName)
	{
		// 附加数据库
		std::ostringstream oss;
		oss << "ATTACH :DBPath AS \"" << strDBName << "\"";
		PString strSql = oss.str();

		int rc = prepare(strSql);
		if (SQLITE_OK != rc)
			return rc;

		bindText(":DBPath", strDBPath);

		// 执行SQL语句
		rc = step();

		// 完成SQL语句
		m_mapFieldName2Index.clear();
		finalize();

		return rc;
	}

	int SQLite::detach(const PString& strDBName)
	{
		// 分离附加的数据库
		std::ostringstream oss;
		oss << "DETACH \"" << strDBName << "\"";
		PString strSql = oss.str();

		return execute(strSql);
	}

	int SQLite::prepare(const PString& strSql)
	{
		if (m_pStmt)
		{
			m_mapFieldName2Index.clear();
			finalize();
		}

		// 准备编译SQL语句，这里要用v2版本
#ifdef SINDY_UNICODE
		int rc = sqlite3_prepare16_v2(m_pDb, strSql.c_str(), -1, &m_pStmt, nullptr);
#else
		int rc = sqlite3_prepare_v2(m_pDb, strSql.c_str(), -1, &m_pStmt, nullptr);
#endif

		if (SQLITE_OK != rc)
		{
			m_pStmt = nullptr;
		}
		return rc;
	}

	int SQLite::execute(const PString& strSql)
	{
		// 编译SQL语句
		int rc = prepare(strSql);
		if (SQLITE_OK != rc)
		{
			// 准备语句失败
			return rc;
		}

		// 执行SQL语句
		rc = step();

		// 完成SQL语句
		m_mapFieldName2Index.clear();
		finalize();

		return rc;
	}

	// 执行语句
	int SQLite::step()
	{
		return sqlite3_step(m_pStmt);
	}

	int SQLite::finalize()
	{
		int rc = sqlite3_finalize(m_pStmt);
		m_pStmt = nullptr;
		return rc;
	}

	// 重置语句对象到它的初始状态，准备被重新执行
	int SQLite::resetSyntax()
	{
		return sqlite3_reset(m_pStmt);
	}

	void SQLite::initColName2ColIndex()
	{
		if (!m_mapFieldName2Index.empty())
			return;

		// 保存列名和其对应的索引
		int iCount = sqlite3_column_count(m_pStmt);
		for (int i = 0; i < iCount; i++)
		{
			PString strColName = columnName(i);
			m_mapFieldName2Index[strColName] = i;
		}
	}

	PString SQLite::columnName(int iCol)
	{
		// 获取列名
#ifdef SINDY_UNICODE
		const void* pColumnName = sqlite3_column_name16(m_pStmt, iCol);
		return static_cast<const wchar_t*>(pColumnName);
#else
		return sqlite3_column_name(m_pStmt, iCol);
#endif
	}

	bool SQLite::getValueText(const PString& strColName, PString& strColValue)
	{
		initColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
		{
			strColValue.clear();
			return false;
		}

		strColValue = getValueText(it->second);
		return true;
	}

	PString SQLite::getValueText(int iCol)
	{
#ifdef SINDY_UNICODE
		const void* pColumnName = sqlite3_column_text16(m_pStmt, iCol);
		return static_cast<const wchar_t*>(pColumnName);
#else
		const unsigned char* pColumnName = sqlite3_column_text(m_pStmt, iCol);
		return reinterpret_cast<const char*>(pColumnName);
#endif
	}

	bool SQLite::getValueDouble(const PString& strColName, double& dblColValue)
	{
		initColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
			return false;

		dblColValue = getValueDouble(it->second);
		return true;
	}

	double SQLite::getValueDouble(int iCol)
	{
		return sqlite3_column_double(m_pStmt, iCol);
	}

	bool SQLite::getValueInt(const PString& strColName, int& iColValue)
	{
		initColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
		{
			iColValue = 0;
			return false;
		}

		iColValue = getValueInt(it->second);
		return true;
	}

	int SQLite::getValueInt(int iCol)
	{
		return sqlite3_column_int(m_pStmt, iCol);
	}

	const void* SQLite::getValueLargeField(int iCol)
	{
		// 获取列的值
		return sqlite3_column_blob(m_pStmt, iCol);
	}

	bool SQLite::getValueLargeField(const PString& strColName, const void*& pBuffer)
	{
		initColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
		{
			pBuffer = nullptr;
			return false;
		}
		pBuffer = getValueLargeField(it->second);
		return true;
	}

	bool SQLite::getValueLargeField(const PString& strColName, const void*& pBuffer, long& lBufferSize)
	{
		initColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
		{
			pBuffer = nullptr;
			lBufferSize = 0;
			return false;
		}
		pBuffer = getValueLargeField(it->second);
		lBufferSize = largeFieldSize(it->second);

		return true;
	}

	long SQLite::largeFieldSize(int iCol)
	{
		return sqlite3_column_bytes(m_pStmt, iCol);
	}

	int SQLite::getFieldType(const PString& strColName)
	{
		initColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
			return 0;

		return sqlite3_column_type(m_pStmt, it->second);
	}

	int SQLite::getFieldCount()
	{
		return sqlite3_data_count(m_pStmt);
	}

	int SQLite::bindText(const PString& strColName, const PString& strValue)
	{
		int iIndex = bindParameterIndex(strColName);
		return bindText(iIndex, strValue);
	}

	int SQLite::bindText(int iIndex, const PString& strValue)
	{
#ifdef SINDY_UNICODE
		return sqlite3_bind_text16(m_pStmt, iIndex, strValue.c_str(), -1, SQLITE_TRANSIENT);
#else
		return sqlite3_bind_text(m_pStmt, iIndex, strValue.c_str(), -1, SQLITE_TRANSIENT);
#endif

	}

	int SQLite::bindInt(int iIndex, int iValue)
	{
		return	sqlite3_bind_int(m_pStmt, iIndex, iValue);
	}

	int SQLite::bindInt(const PString& strColName, int iValue)
	{
		int iIndex = bindParameterIndex(strColName);
		return bindInt(iIndex, iValue);
	}

	int SQLite::bindInt64(int iIndex, __int64 llValue)
	{
		return	sqlite3_bind_int64(m_pStmt, iIndex, llValue);
	}

	int SQLite::bindInt64(const PString& strColName, __int64 llValue)
	{
		int iIndex = bindParameterIndex(strColName);
		return bindInt64(iIndex, llValue);
	}

	int SQLite::bindDouble(int iIndex, double dblValue)
	{
		return	sqlite3_bind_double(m_pStmt, iIndex, dblValue);
	}

	int SQLite::bindDouble(const PString& strColName, double dblValue)
	{
		int iIndex = bindParameterIndex(strColName);
		return bindDouble(iIndex, dblValue);
	}

	int SQLite::bindLargeField(int iIndex, const void* pBuffer, int iByte)
	{
		return sqlite3_bind_blob(m_pStmt, iIndex, pBuffer, iByte, SQLITE_TRANSIENT);
	}

	int SQLite::bindLargeField(const PString& strColName, const void* pBuffer, int iByte)
	{
		int iIndex = bindParameterIndex(strColName);
		return bindLargeField(iIndex, pBuffer, iByte);
	}

	int SQLite::bindParameterIndex(const PString& strColName)
	{
		// 获取绑定参数对应的索引号
#ifdef SINDY_UNICODE
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return sqlite3_bind_parameter_index(m_pStmt, converter.to_bytes(strColName).c_str());
#else
		return sqlite3_bind_parameter_index(m_pStmt, strColName.c_str());
#endif
	}

	int SQLite::clearBindings()
	{
		return sqlite3_clear_bindings(m_pStmt);
	}

	void SQLite::beginTransaction()
	{
		execute("BEGIN");
	}

	void SQLite::rollBack()
	{
		execute("ROLLBACK");
	}

	void SQLite::commit()
	{
		execute("COMMIT");
	}



	int SQLite::clearTable(const PString& strTableName, const PString& strDbName)
	{
		std::ostringstream oss;
		oss << "DELETE TABLE IF EXISTS \"" << strDbName << "\".\"" << strTableName << "\"";
		PString strSql = oss.str();
		return execute(strSql);
	}

	int SQLite::dropTable(const PString& strTableName, const PString& strDbName /*= "main"*/)
	{
		//strSql.Format("DROP TABLE IF EXISTS \"%s\".\"%s\"", strDbName, strTableName);
		std::ostringstream oss;
		oss << "DROP TABLE " << strTableName;
		PString strSql = oss.str();

		return execute(strSql);
	}

	int SQLite::dropView(const PString& strTableName, const PString& strDbName /*= "main"*/)
	{
		std::ostringstream oss;
		oss << "DROP VIEW IF EXISTS \"" << strDbName << "\".\"" << strTableName << "\"";
		PString strSql = oss.str();

		return execute(strSql);
	}

	bool SQLite::isTableExist(const PString& strTableName, const PString& strDbName /*= "main"*/)
	{
		// 查询sqlite_master表
		PString strLower(strDbName);
		std::transform(strLower.begin(), strLower.end(), strLower.begin(), (int(*)(int))tolower);

		PString strSql;
		if (strLower == "temp")
		{
			std::ostringstream oss;
			oss << "SELECT * FROM sqlite_temp_master WHERE type = 'table' AND name = '" << strTableName << "'";
			strSql = oss.str();
		}
		else
		{
			std::ostringstream oss;
			oss << "SELECT * FROM \"" << strDbName << "\".sqlite_master WHERE type = 'table' AND name = '" << strTableName << "'";
			strSql = oss.str();
		}


		int rc = prepare(strSql);
		if (SQLITE_OK != rc)
			return false;

		step();

		// 如果没有记录,表不存在
		rc = getFieldCount();

		// 释放
		m_mapFieldName2Index.clear();
		finalize();
		if (0 == rc)
		{
			return false;
		}
		return true;
	}

	bool SQLite::isViewExist(const PString& strTableName, const PString& strDbName /*= "main"*/)
	{
		PString strLower(strDbName);
		std::transform(strLower.begin(), strLower.end(), strLower.begin(), (int(*)(int))tolower);

		// 查询sqlite_master表
		PString strSql;
		if (strDbName == "temp")
		{
			std::ostringstream oss;
			oss << "SELECT * FROM sqlite_temp_master WHERE type = 'view' AND name = '" << strTableName << "'";
			strSql = oss.str();
		}
		else
		{
			std::ostringstream oss;
			oss << "SELECT * FROM \"" << strDbName << "\".sqlite_master WHERE type = 'view' AND name = '" << strTableName << "'";
			strSql = oss.str();
		}

		int rc = prepare(strSql);
		if (SQLITE_OK != rc)
		{
			return false;
		}

		step();

		// 如果没有记录,表不存在
		rc = getFieldCount();

		m_mapFieldName2Index.clear();
		finalize();

		return 0 == rc ? false : true;
	}

	bool SQLite::isFieldExist(const PString& strTableName, const PString& strFieldName, const PString& strDbName /*= "main"*/)
	{
		// 获取表信息
		std::ostringstream oss;
		oss << "PRAGMA \"" << strDbName << "\".PRAGMA (\"" << strTableName << "\")";
		PString strSql = oss.str();

		int rc = prepare(strSql);
		if (SQLITE_OK != rc)
		{
			return false;
		}

		// 遍历字段
		rc = step();

		bool isExist = false;
		while (SQLITE_ROW == rc)
		{
			PString strColName;
			getValueText("name", strColName);

			if (strColName == strFieldName)
			{
				isExist = true;
				break;
			}
			rc = step();
		}

		m_mapFieldName2Index.clear();
		finalize();
		return isExist;
	}

	// 获取最近一次错误信息
	PString SQLite::errorMessage()
	{
#ifdef SINDY_UNICODE
		const void * pErrMsg = sqlite3_errmsg16(m_pDb);
		return static_cast<const wchar_t*>(pErrMsg);
#else
		return sqlite3_errmsg(m_pDb);
#endif
	}

} // namespace Sindy


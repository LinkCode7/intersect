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
		Open(strDbPath);
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
			Close();
			delete m_pUseCount;
		}
	}

	int SQLite::Open(const PString& strSqliteDbPath)
	{
#ifdef UNICODE
		int rc = sqlite3_open16(strSqliteDbPath.c_str(), &m_pDb);
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		int rc = sqlite3_open16(converter.from_bytes(strSqliteDbPath.c_str()).c_str(), &m_pDb);
#endif

		if (SQLITE_OK != rc)
		{
			Close();
			m_pDb = nullptr;
		}
		return rc;
	}

	int SQLite::Close()
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

	int SQLite::Attach(const PString& strDBPath, const PString& strDBName)
	{
		// 附加数据库
		std::ostringstream oss;
		oss << "ATTACH :DBPath AS \"" << strDBName << "\"";
		PString strSql = oss.str();

		int rc = Prepare(strSql);
		if (SQLITE_OK != rc)
			return rc;

		BindText(":DBPath", strDBPath);

		// 执行SQL语句
		rc = Step();

		// 完成SQL语句
		m_mapFieldName2Index.clear();
		Finalize();

		return rc;
	}

	int SQLite::Detach(const PString& strDBName)
	{
		// 分离附加的数据库
		std::ostringstream oss;
		oss << "DETACH \"" << strDBName << "\"";
		PString strSql = oss.str();

		return Execute(strSql);
	}

	int SQLite::Prepare(const PString& strSql)
	{
		if (m_pStmt)
		{
			m_mapFieldName2Index.clear();
			Finalize();
		}

		// 准备编译SQL语句，这里要用v2版本
#ifdef UNICODE
		int rc = sqlite3_prepare16_v2(m_pDb, strSql.c_str(), -1, &m_pStmt, nullptr);
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		int rc = sqlite3_prepare16_v2(m_pDb, converter.from_bytes(strSql.c_str()).c_str(), -1, &m_pStmt, nullptr);
#endif

		if (SQLITE_OK != rc)
		{
			m_pStmt = nullptr;
		}
		return rc;
	}

	int SQLite::Execute(const PString& strSql)
	{
		// 编译SQL语句
		int rc = Prepare(strSql);
		if (SQLITE_OK != rc)
		{
			// 准备语句失败
			return rc;
		}

		// 执行SQL语句
		rc = Step();

		// 完成SQL语句
		m_mapFieldName2Index.clear();
		Finalize();

		return rc;
	}

	// 执行语句
	int SQLite::Step()
	{
		return sqlite3_step(m_pStmt);
	}

	int SQLite::Finalize()
	{
		int rc = sqlite3_finalize(m_pStmt);
		m_pStmt = nullptr;
		return rc;
	}

	// 重置语句对象到它的初始状态，准备被重新执行
	int SQLite::ResetSyntax()
	{
		return sqlite3_reset(m_pStmt);
	}

	void SQLite::InitColName2ColIndex()
	{
		if (!m_mapFieldName2Index.empty())
			return;

		// 保存列名和其对应的索引
		int iCount = sqlite3_column_count(m_pStmt);
		for (int i = 0; i < iCount; i++)
		{
			PString strColName = ColumnName(i);
			m_mapFieldName2Index[strColName] = i;
		}
	}

	PString SQLite::ColumnName(int iCol)
	{
		// 获取列名
		const void* pColumnName = sqlite3_column_name16(m_pStmt, iCol);
		auto lpColumnName = static_cast<const wchar_t*>(pColumnName);

#ifdef UNICODE
		return lpColumnName;
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return PString(converter.to_bytes(lpColumnName));
#endif
	}

	bool SQLite::GetValueText(const PString& strColName, PString& strColValue)
	{
		InitColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
		{
			strColValue.clear();
			return false;
		}

		strColValue = GetValueText(it->second);
		return true;
	}

	PString SQLite::GetValueText(int iCol)
	{
		const void* pColumnName = sqlite3_column_text16(m_pStmt, iCol);
		auto lpColumnName = static_cast<const wchar_t*>(pColumnName);

#ifdef UNICODE
		return lpColumnName;
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return PString(converter.to_bytes(lpColumnName));
#endif
	}

	bool SQLite::GetValueDouble(const PString& strColName, double& dblColValue)
	{
		InitColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
			return false;

		dblColValue = GetValueDouble(it->second);
		return true;
	}

	double SQLite::GetValueDouble(int iCol)
	{
		return sqlite3_column_double(m_pStmt, iCol);
	}

	bool SQLite::GetValueInt(const PString& strColName, int& iColValue)
	{
		InitColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
		{
			iColValue = 0;
			return false;
		}

		iColValue = GetValueInt(it->second);
		return true;
	}

	int SQLite::GetValueInt(int iCol)
	{
		return sqlite3_column_int(m_pStmt, iCol);
	}

	const void* SQLite::GetValueLargeField(int iCol)
	{
		// 获取列的值
		return sqlite3_column_blob(m_pStmt, iCol);
	}

	bool SQLite::GetValueLargeField(const PString& strColName, const void*& pBuffer)
	{
		InitColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
		{
			pBuffer = nullptr;
			return false;
		}
		pBuffer = GetValueLargeField(it->second);
		return true;
	}

	bool SQLite::GetValueLargeField(const PString& strColName, const void*& pBuffer, long& lBufferSize)
	{
		InitColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
		{
			pBuffer = nullptr;
			lBufferSize = 0;
			return false;
		}
		pBuffer = GetValueLargeField(it->second);
		lBufferSize = LargeFieldSize(it->second);

		return true;
	}

	long SQLite::LargeFieldSize(int iCol)
	{
		return sqlite3_column_bytes(m_pStmt, iCol);
	}

	int SQLite::GetFieldType(const PString& strColName)
	{
		InitColName2ColIndex();

		std::map<PString, int>::iterator it = m_mapFieldName2Index.find(strColName);
		if (m_mapFieldName2Index.end() == it)
			return 0;

		return sqlite3_column_type(m_pStmt, it->second);
	}

	int SQLite::GetFieldCount()
	{
		return sqlite3_data_count(m_pStmt);
	}

	int SQLite::BindText(const PString& strColName, const PString& strValue)
	{
		int iIndex = BindParameterIndex(strColName);
		return BindText(iIndex, strValue);
	}

	int SQLite::BindText(int iIndex, const PString& strValue)
	{
#ifdef UNICODE
		return sqlite3_bind_text16(m_pStmt, iIndex, strValue.c_str(), -1, SQLITE_TRANSIENT);
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return sqlite3_bind_text16(m_pStmt, iIndex, converter.from_bytes(strValue.c_str()).c_str(), -1, SQLITE_TRANSIENT);
#endif
	}

	int SQLite::BindInt(int iIndex, int iValue)
	{
		return	sqlite3_bind_int(m_pStmt, iIndex, iValue);
	}

	int SQLite::BindInt(const PString& strColName, int iValue)
	{
		int iIndex = BindParameterIndex(strColName);
		return BindInt(iIndex, iValue);
	}

	int SQLite::BindInt64(int iIndex, __int64 llValue)
	{
		return	sqlite3_bind_int64(m_pStmt, iIndex, llValue);
	}

	int SQLite::BindInt64(const PString& strColName, __int64 llValue)
	{
		int iIndex = BindParameterIndex(strColName);
		return BindInt64(iIndex, llValue);
	}

	int SQLite::BindDouble(int iIndex, double dblValue)
	{
		return	sqlite3_bind_double(m_pStmt, iIndex, dblValue);
	}

	int SQLite::BindDouble(const PString& strColName, double dblValue)
	{
		int iIndex = BindParameterIndex(strColName);
		return BindDouble(iIndex, dblValue);
	}

	int SQLite::BindLargeField(int iIndex, const void* pBuffer, int iByte)
	{
		return sqlite3_bind_blob(m_pStmt, iIndex, pBuffer, iByte, SQLITE_TRANSIENT);
	}

	int SQLite::BindLargeField(const PString& strColName, const void* pBuffer, int iByte)
	{
		int iIndex = BindParameterIndex(strColName);
		return BindLargeField(iIndex, pBuffer, iByte);
	}

	int SQLite::BindParameterIndex(const PString& strColName)
	{
		// 获取绑定参数对应的索引号
#ifdef UNICODE
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return sqlite3_bind_parameter_index(m_pStmt, converter.to_bytes(strColName).c_str());
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return sqlite3_bind_parameter_index(m_pStmt, strColName.c_str());
#endif
	}

	int SQLite::ClearBindings()
	{
		return sqlite3_clear_bindings(m_pStmt);
	}

	void SQLite::BeginTransaction()
	{
		Execute("BEGIN");
	}

	void SQLite::RollBack()
	{
		Execute("ROLLBACK");
	}

	void SQLite::Commit()
	{
		Execute("COMMIT");
	}



	int SQLite::ClearTable(const PString& strTableName, const PString& strDbName)
	{
		std::ostringstream oss;
		oss << "DELETE TABLE IF EXISTS \"" << strDbName << "\".\"" << strTableName << "\"";
		PString strSql = oss.str();
		return Execute(strSql);
	}

	int SQLite::DropTable(const PString& strTableName, const PString& strDbName /*= "main"*/)
	{
		//strSql.Format("DROP TABLE IF EXISTS \"%s\".\"%s\"", strDbName, strTableName);
		std::ostringstream oss;
		oss << "DROP TABLE " << strTableName;
		PString strSql = oss.str();

		return Execute(strSql);
	}

	int SQLite::DropView(const PString& strTableName, const PString& strDbName /*= "main"*/)
	{
		std::ostringstream oss;
		oss << "DROP VIEW IF EXISTS \"" << strDbName << "\".\"" << strTableName << "\"";
		PString strSql = oss.str();

		return Execute(strSql);
	}

	bool SQLite::IsTableExist(const PString& strTableName, const PString& strDbName /*= "main"*/)
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


		int rc = Prepare(strSql);
		if (SQLITE_OK != rc)
			return false;

		Step();

		// 如果没有记录,表不存在
		rc = GetFieldCount();

		// 释放
		m_mapFieldName2Index.clear();
		Finalize();
		if (0 == rc)
		{
			return false;
		}
		return true;
	}

	bool SQLite::IsViewExist(const PString& strTableName, const PString& strDbName /*= "main"*/)
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

		int rc = Prepare(strSql);
		if (SQLITE_OK != rc)
		{
			return false;
		}

		Step();

		// 如果没有记录,表不存在
		rc = GetFieldCount();

		m_mapFieldName2Index.clear();
		Finalize();

		return 0 == rc ? false : true;
	}

	bool SQLite::IsFieldExist(const PString& strTableName, const PString& strFieldName, const PString& strDbName /*= "main"*/)
	{
		// 获取表信息
		std::ostringstream oss;
		oss << "PRAGMA \"" << strDbName << "\".PRAGMA (\"" << strTableName << "\")";
		PString strSql = oss.str();

		int rc = Prepare(strSql);
		if (SQLITE_OK != rc)
		{
			return false;
		}

		// 遍历字段
		rc = Step();

		bool isExist = false;
		while (SQLITE_ROW == rc)
		{
			PString strColName;
			GetValueText("name", strColName);

			if (strColName == strFieldName)
			{
				isExist = true;
				break;
			}
			rc = Step();
		}

		m_mapFieldName2Index.clear();
		Finalize();
		return isExist;
	}

	// 获取最近一次错误信息
	PString SQLite::ErrorMessage()
	{
		const void* pErrMsg = sqlite3_errmsg16(m_pDb);
		auto lpErrMsg = static_cast<const wchar_t*>(pErrMsg);

#ifdef UNICODE
		return lpErrMsg;
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return PString(converter.to_bytes(lpErrMsg));
#endif
	}

} // namespace Sindy


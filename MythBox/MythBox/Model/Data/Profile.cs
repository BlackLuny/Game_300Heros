using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.SQLite;
using System.Data.SQLite.Generic;
using System.Windows.Forms;
using System.Data.Common;

namespace MythBox.Model.Data
{
	public class Profile
	{
		private SQLiteConnection DbConn;

		public Profile(string dbPath)
		{
			try
			{
				var connectionString = new SQLiteConnectionStringBuilder
				{
					DataSource = dbPath,
				};
				DbConn = new SQLiteConnection(connectionString.ToString());
				DbConn.Open();
			}
			catch
			{
				MessageBox.Show(@"/data/data.sqlite打开失败");
				Environment.Exit(0);
			}
		}

		/// <summary> 
		/// 执行一个查询语句，返回一个关联的SQLiteDataReader实例 
		/// </summary> 
		/// <param name="sql">要执行的查询语句</param> 
		/// <param name="parameters">执行SQL查询语句所需要的参数，参数必须以它们在SQL语句中的顺序为准</param> 
		/// <returns></returns> 
		private SQLiteDataReader ExecuteReader(string sql, SQLiteParameter[] parameters)
		{
			SQLiteCommand command = new SQLiteCommand(sql, DbConn);
			if (parameters != null)
			{
				command.Parameters.AddRange(parameters);
			}
			return command.ExecuteReader();
		}

		/// <summary> 
		/// 对SQLite数据库执行增删改操作，返回受影响的行数。 
		/// </summary> 
		/// <param name="sql">要执行的增删改的SQL语句</param> 
		/// <param name="parameters">执行增删改语句所需要的参数，参数必须以它们在SQL语句中的顺序为准</param> 
		/// <returns></returns> 
        private int ExecuteNonQuery(string sql, SQLiteParameter[] parameters)
		{
			int affectedRows = 0;
			using (DbTransaction transaction = DbConn.BeginTransaction())
			{
				using (SQLiteCommand command = new SQLiteCommand(DbConn))
				{
					command.CommandText = sql;
					if (parameters != null)
					{
						command.Parameters.AddRange(parameters);
					}
					affectedRows = command.ExecuteNonQuery();
				}
				transaction.Commit();
			}
			return affectedRows;
		}

		public string GetValue(string appName, string key, string def = "")
		{
			string value = def;

			string sql = "SELECT * FROM config WHERE app=@App AND key=@Key";
			SQLiteParameter[] sp = new SQLiteParameter[] { 
                new SQLiteParameter("@App",appName),
                new SQLiteParameter("@Key",key)
            };

			using (SQLiteDataReader reader = ExecuteReader(sql, sp))
			{

				if (reader.Read())
				{
					value = reader.GetString(reader.GetOrdinal("value"));
				}
			}

			return value;
		}

		public void SetValue(string appName, string key, string val)
		{
			string sql = "SELECT * FROM config WHERE app=@App AND key=@Key";
			SQLiteParameter[] sp = new SQLiteParameter[] { 
                new SQLiteParameter("@App",appName),
                new SQLiteParameter("@Key",key)
            };

			bool hasFind = false;

			using (SQLiteDataReader reader = ExecuteReader(sql, sp))
			{
				if (reader.Read())
				{
					hasFind = true;
				}
			}

			if (!hasFind)
			{
				sql = "INSERT INTO config(app,key,value) values (@app,@key,@val)";
			}
			else
			{
				sql = "UPDATE config SET value = @val WHERE app=@app AND key = @key";
			}

			SQLiteParameter[] parameters = new SQLiteParameter[]{ 
                                         new SQLiteParameter("@app",appName),
                                         new SQLiteParameter("@key",key), 
                                         new SQLiteParameter("@val",val),
                                         };

			
			ExecuteNonQuery(sql, parameters); 
		}
	}
}

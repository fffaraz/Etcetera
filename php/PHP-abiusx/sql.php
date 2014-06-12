<?php
/**
 * The xSQL function is supposed to run a SQL query and return the appropriate
 * results.
 * For select, it returns 2D associative arrays
 * For UPDATE and DELETE it returns affectedRows
 * For INSERT it returns LastInsertID, if no auto-increment found affectedRows
 * is returned
 *
 * The query should be in format "SELECT * FROM table WHERE Title=?",$Title
 *
 * @param string $Query        	
 * @throws Exception if no database interface found.
 */
class xSQL
{
	
	public static $DB = null;
	static function SQL($Query)
	{
		$args = func_get_args ();
		if (get_class ( self::$DB ) == "PDO")
			return call_user_func_array ( "self::SQL_pdo", $args );
		else 
			if (get_class ( self::$DB ) == "mysqli")
				return call_user_func_array ( "self::SQL_mysqli", $args );
			else
				throw new Exception ( "Unknown database interface type." );
	}
	function SQL_pdo($Query)
	{
		$args = func_get_args ();
		if (count ( $args ) == 1)
		{
			$result = self::$DB->query ( $Query );
			if ($result->rowCount ())
			{
				return $result->fetchAll ( PDO::FETCH_ASSOC );
			}
			return null;
		}
		else
		{
			if (! $stmt = self::$DB->prepare ( $Query ))
			{
				$Error = self::$DB->errorInfo ();
				trigger_error ( "Unable to prepare statement: {$Query}, reason: {$Error[2]}" );
			}
			array_shift ( $args ); // remove $Query from args
			$i = 0;
			foreach ( $args as &$v )
				$stmt->bindValue ( ++ $i, $v );
			$stmt->execute ();
			
			$type = substr ( trim ( strtoupper ( $Query ) ), 0, 6 );
			if ($type == "INSERT")
			{
				$res = self::$DB->lastInsertId ();
				if ($res == 0)
					return self::$DB->rowCount ();
				return $res;
			}
			elseif ($type == "DELETE" or $type == "UPDATE" or $type == "REPLAC")
				return self::$DB->rowCount ();
			elseif ($type == "SELECT")
				return $stmt->fetchAll ( PDO::FETCH_ASSOC );
		}
	}
	function SQL_mysqli( $Query)
	{
		$args = func_get_args ();
		if (count ( $args ) == 1)
		{
			$result = self::$DB->query ( $Query );
			if ($result->num_rows)
			{
				$out = array ();
				while ( null != ($r = $result->fetch_array ( MYSQLI_ASSOC )) )
					$out [] = $r;
				return $out;
			}
			return null;
		}
		else
		{
			if (! $preparedStatement = self::$DB->prepare ( $Query ))
				trigger_error ( "Unable to prepare statement: {$Query}, reason: {self::$DB->error}" );
			array_shift ( $args ); // remove $Query from args
			$a = array ();
			foreach ( $args as $k => &$v )
				$a [$k] = &$v;
			$types = str_repeat ( "s", count ( $args ) ); // all params are
			                                              // strings, works well on
			                                              // MySQL
			                                              // and SQLite
			array_unshift ( $a, $types );
			call_user_func_array ( array ($preparedStatement, 'bind_param' ), $a );
			$preparedStatement->execute ();
			
			$type = substr ( trim ( strtoupper ( $Query ) ), 0, 6 );
			if ($type == "INSERT")
			{
				$res = self::$DB->insert_id;
				if ($res == 0)
					return self::$DB->affected_rows;
				return $res;
			}
			elseif ($type == "DELETE" or $type == "UPDATE" or $type == "REPLAC")
				return self::$DB->affected_rows;
			elseif ($type == "SELECT")
			{
				// fetching all results in a 2D array
				$metadata = $preparedStatement->result_metadata ();
				$out = array ();
				$fields = array ();
				if (! $metadata)
					return null;
				while ( null != ($field = $metadata->fetch_field ()) )
					$fields [] = &$out [$field->name];
				call_user_func_array ( array ($preparedStatement, "bind_result" ), $fields );
				$output = array ();
				$count = 0;
				while ( $preparedStatement->fetch () )
				{
					foreach ( $out as $k => $v )
						$output [$count] [$k] = $v;
					$count ++;
				}
				$preparedStatement->free_result ();
				return ($count == 0) ? null : $output;
			}
			else
				return null;
		}
	}
}
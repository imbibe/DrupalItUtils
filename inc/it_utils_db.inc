<?php
// 
// Imbibe Technologies Pvt Ltd® - http://imbibe.in
// Copyright (c) 2014
// by Imbibe Technologies Pvt Ltd
// 
// This software and associated files including documentation (the "Software") is goverened by Microsoft Public License (MS-PL),
// a copy of which is included with the Software as a text file, License.txt.

function it_utils_db_backup_tables($host, $name, $user, $pass, $tables = '*') {
	//http://davidwalsh.name/backup-mysql-database-php
	$link = mysqli_connect($host, $user, $pass, $name);
	
	//get all of the tables
	if($tables == '*') {
		$tables = array();
		$result = mysqli_query($link, 'SHOW TABLES');
		while($row = mysqli_fetch_row($result)) {
			$tables[] = $row[0];
		}
		mysqli_free_result($result);
	} else {
		$tables = is_array($tables) ? $tables : explode(',', $tables);
	}
	
	$temp_file = tempnam(sys_get_temp_dir(), 'tmp');	
	$handle = fopen($temp_file, 'w+');
	
	$content = "";

	//cycle through
	foreach($tables as $table) {
		_it_utils_db_write_buffered($handle, $content, 'DROP TABLE ' . $table . ';');
		
		$result = mysqli_query($link, 'SHOW CREATE TABLE ' . $table);
		$row2 = mysqli_fetch_row($result);
		mysqli_free_result($result);
		
		_it_utils_db_write_buffered($handle, $content, "\n\n" . $row2[1] . ";\n\n");
		
		$result = mysqli_query($link, 'SELECT * FROM ' . $table);
		$num_fields = mysqli_num_fields($result);
		
		for ($i = 0; $i < $num_fields; $i++) {
			while($row = mysqli_fetch_row($result)) {
				_it_utils_db_write_buffered($handle, $content, 'INSERT INTO '. $table .' VALUES(');
				for($j=0; $j<$num_fields; $j++) {
					$row[$j] = addslashes($row[$j]);
					$row[$j] = preg_replace("/\n/", "\\n", $row[$j]);
					if (isset($row[$j])) { _it_utils_db_write_buffered($handle, $content, '"'.$row[$j].'"') ; } else { _it_utils_db_write_buffered($handle, $content, '""'); }
					if ($j < ($num_fields-1)) { _it_utils_db_write_buffered($handle, $content, ','); }
				}
				
				_it_utils_db_write_buffered($handle, $content, ");\n");
			}
		}
		
		_it_utils_db_write_buffered($handle, $content, "\n\n\n");
		
		mysqli_free_result($result);
	}
		
	mysqli_close($link);
	
	_it_utils_db_write_buffered($handle, $content, "", TRUE);
	
	fclose($handle);
	
	return ($temp_file);
}

function _it_utils_db_write_buffered($handle, &$existing, $new = "", $force = FALSE) {
	$existing .= $new;
	
	if((strlen($existing) > 128 *1024) || $force) {
		fwrite($handle, $existing);
		$existing = "";
	}
}
